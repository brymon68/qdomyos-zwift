#!/usr/bin/env bash
# Build a disposable checkout of the pinned gRPC baseline. Never deploys an APK.
set -euo pipefail

if [[ $# != 2 ]]; then
    echo "Usage: bash tools/x22i/build_baseline.sh SOURCE_CHECKOUT NEW_OUTPUT_DIRECTORY" >&2
    exit 2
fi

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
source_dir=$(realpath "$1")
output_dir=$(realpath -m "$2")
: "${QT_ANDROID_DIR:?Set QT_ANDROID_DIR to the Qt 5.15.0 Android installation}"
: "${ANDROID_SDK_ROOT:?Set ANDROID_SDK_ROOT}"
: "${JAVA_HOME:?Set JAVA_HOME to Java 17}"
export PATH="$QT_ANDROID_DIR/bin:$JAVA_HOME/bin:$PATH"
export ANDROID_NDK_ROOT="$ANDROID_SDK_ROOT/ndk/21.4.7075529"
export ANDROID_NDK="$ANDROID_NDK_ROOT"
export ANDROID_HOME="$ANDROID_SDK_ROOT"
export GRADLE_USER_HOME="$output_dir/gradle-cache"
export X22I_VARIANT=${X22I_VARIANT:-baseline}
case "$X22I_VARIANT" in
    baseline) ;;
    test)
        : "${X22I_SIGNING_STORE_FILE:?Set the private test PKCS12 keystore path}"
        : "${X22I_SIGNING_STORE_PASSWORD:?Set the private test keystore password}"
        [[ -f "$X22I_SIGNING_STORE_FILE" ]] || { echo "Test keystore is missing" >&2; exit 2; }
        ;;
    *) echo "X22I_VARIANT must be baseline or test" >&2; exit 2 ;;
esac
jobs=${X22I_BUILD_JOBS:-2}

for command in git make python3 rsync qmake lrelease androiddeployqt javac; do
    command -v "$command" >/dev/null || { echo "Missing build command: $command" >&2; exit 2; }
done
[[ $(qmake -query QT_VERSION) == 5.15.0 ]] || { echo "Qt 5.15.0 is required" >&2; exit 2; }
[[ $(javac -version 2>&1) == 'javac 17.'* ]] || { echo "Java 17 is required" >&2; exit 2; }
[[ -d "$ANDROID_NDK_ROOT" ]] || { echo "NDK 21.4.7075529 is required" >&2; exit 2; }
[[ ! -e "$output_dir" ]] || { echo "Use a new output directory" >&2; exit 2; }

# Check the source before making the same localhost variant as the nightly.
# No owner credentials or service secrets are needed for this build.
python3 - "$source_dir" "$script_dir/baseline.json" <<'PY'
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

source = Path(sys.argv[1])
baseline = json.loads(Path(sys.argv[2]).read_text())
def git(*args):
    return subprocess.check_output(['git', '-C', str(source), *args], text=True).strip()
if git('rev-parse', 'HEAD') != baseline['source_revision']:
    raise SystemExit('Source revision does not match baseline.json')
if git('status', '--porcelain', '--untracked-files=normal'):
    raise SystemExit('Use a clean, disposable source checkout')
if any(line[0] != ' ' for line in subprocess.check_output(
        ['git', '-C', str(source), 'submodule', 'status', '--recursive'], text=True).splitlines()):
    raise SystemExit('Initialize all submodules at their pinned revisions')
if (source / 'src/secret.h').exists() or (source / 'src/secrets.h').exists():
    raise SystemExit('Private secret headers are not accepted in baseline builds')
replacements = {
    'src/settings.qml': (
        'property string nordictrack_2950_ip: ""',
        'property string nordictrack_2950_ip: "localhost"'),
    'src/qzsettings.cpp': (
        'const QString QZSettings::default_nordictrack_2950_ip = QStringLiteral("");',
        'const QString QZSettings::default_nordictrack_2950_ip = QStringLiteral("localhost");'),
    'src/android/build.gradle': (
        '    defaultConfig {\n        applicationId',
        '    defaultConfig {\n        ndk { abiFilters "arm64-v8a" }\n        applicationId'),
    'src/settings-catalog.json': (
        '"key": "nordictrack_2950_ip",\n      "name": "Nordictrack 2950 IP",\n      "description": null,\n      "parent": "Proform/Nordictrack Options",\n      "type": "string",\n      "qmlType": "string",\n      "control": "text",\n      "visible": true,\n      "defaultValue": "",\n      "defaultExpression": "\\"\\"",',
        '"key": "nordictrack_2950_ip",\n      "name": "Nordictrack 2950 IP",\n      "description": null,\n      "parent": "Proform/Nordictrack Options",\n      "type": "string",\n      "qmlType": "string",\n      "control": "text",\n      "visible": true,\n      "defaultValue": "localhost",\n      "defaultExpression": "\\"localhost\\"",'),
}
for relative, (old, new) in replacements.items():
    path = source / relative
    text = path.read_text()
    if text.count(old) != 1:
        raise SystemExit('Pinned baseline variant no longer matches expected source')
    path.write_text(text.replace(old, new))

if os.environ['X22I_VARIANT'] == 'test':
    gradle = source / 'src/android/build.gradle'
    text = gradle.read_text()
    original = 'applicationId = "org.cagnulen.qdomyoszwift"'
    if text.count(original) != 1 or text.count('\nandroid {\n') != 1:
        raise SystemExit('Expected Android package/signing configuration was not found')
    text = text.replace(original, 'applicationId = "org.cagnulen.qdomyoszwift.x22ibaseline"')
    signing = '''
android {
    signingConfigs {
        debug {
            storeFile file(System.getenv('X22I_SIGNING_STORE_FILE'))
            storePassword System.getenv('X22I_SIGNING_STORE_PASSWORD')
            keyAlias 'x22i-test'
            keyPassword System.getenv('X22I_SIGNING_STORE_PASSWORD')
            storeType 'PKCS12'
        }
    }
'''
    gradle.write_text(text.replace('\nandroid {\n', signing))
    manifest = source / 'src/android/AndroidManifest.xml'
    text = manifest.read_text()
    for label in ('qdomyos-zwift', 'QZ'):
        original = 'android:label="' + label + '"'
        if text.count(original) != 1:
            raise SystemExit('Expected application/launcher label was not found')
        text = text.replace(original, 'android:label="X22i Test"')
    # Keep the source manifest package, Java namespace and JNI names unchanged.
    manifest.write_text(text)

    main = source / 'src/main.cpp'
    text = main.read_text()
    include = '#include <QSettings>\n'
    startup = '''    app->setApplicationName(QStringLiteral("qDomyos-Zwift"));

    QSettings settings;
'''
    if text.count(include) != 1 or text.count(startup) != 1:
        raise SystemExit('Expected settings initialization point was not found')
    text = text.replace(include, include + '#include "x22i_defaults.h"\n')
    text = text.replace(startup, startup + '    applyX22iDefaults(settings);\n')
    main.write_text(text)
    shutil.copyfile(Path(sys.argv[2]).with_name('x22i_defaults.h'), source / 'src/x22i_defaults.h')
PY

mkdir -p "$output_dir/httpserver" "$output_dir/native" "$output_dir/android" "$output_dir/artifacts"
cp "$source_dir"/qt-patches/android/5.15.0/jar/* "$QT_ANDROID_DIR/jar/"
cp "$source_dir"/qHttpServerBin/5.15.2/headers/* "$source_dir/src/qthttpserver/src/3rdparty/http-parser/"
lrelease "$source_dir/src/qdomyos-zwift.pri"

cd "$output_dir/httpserver"
qmake "$source_dir/src/qthttpserver" -spec android-clang ANDROID_ABIS=arm64-v8a "ANDROID_NDK_ROOT=$ANDROID_NDK_ROOT"
make -j"$jobs"
make install

cd "$output_dir/native"
# Preserve the baseline's trial behavior until physical regression checks pass.
qmake "$source_dir/qdomyos-zwift.pro" -spec android-clang ANDROID_ABIS=arm64-v8a "ANDROID_NDK_ROOT=$ANDROID_NDK_ROOT" DEFINES+=LICENSE
make -j"$jobs"
make "INSTALL_ROOT=$output_dir/android" install
rsync -a --exclude build --exclude .gradle "$source_dir/src/android/" "$output_dir/android/"

python3 - "$source_dir" "$output_dir" <<'PY'
import json
import sys
from pathlib import Path
source, output = map(Path, sys.argv[1:])
deployment = output / 'native/src/android-qdomyos-zwift-deployment-settings.json'
data = json.loads(deployment.read_text())
data['android-extra-libs'] = ','.join(str(source / 'android_openssl/no-asm/latest/arm64' / name)
                                     for name in ('libcrypto_1_1.so', 'libssl_1_1.so'))
deployment.write_text(json.dumps(data, indent=2) + '\n')
PY

# --no-build skips copyQtFiles in Qt 5.15.0; deliberately do not use it.
androiddeployqt --input "$output_dir/native/src/android-qdomyos-zwift-deployment-settings.json" \
    --output "$output_dir/android" --android-platform android-35 --gradle --aux-mode
# Auxiliary mode skips copyStdCpp, so supply the runtime from the selected NDK.
cp "$ANDROID_NDK_ROOT/sources/cxx-stl/llvm-libc++/libs/arm64-v8a/libc++_shared.so" "$output_dir/android/libs/arm64-v8a/"

python3 - "$output_dir/android/gradle.properties" "$QT_ANDROID_DIR" <<'PY'
import shutil
import sys
from pathlib import Path
path = Path(sys.argv[1])
# androiddeployqt overlays src/android again, including its four runtime folders.
for abi in ('armeabi-v7a', 'x86', 'x86_64'):
    directory = path.parent / 'libs' / abi
    if directory.exists():
        shutil.rmtree(directory)
lines = [line for line in path.read_text().splitlines()
         if not line.strip().startswith(('android.bundle.enableUncompressedNativeLibs=', 'qt5AndroidDir='))]
lines.append('qt5AndroidDir=' + sys.argv[2] + '/src/android/java')
path.write_text('\n'.join(lines) + '\n')
PY
cd "$output_dir/android"
bash ./gradlew --no-daemon assembleDebug
apk="$output_dir/android/build/outputs/apk/debug/android-debug.apk"
python3 "$script_dir/verify_apk.py" "$apk"
"$ANDROID_SDK_ROOT/build-tools/35.0.0/apksigner" verify --verbose --print-certs "$apk" > "$output_dir/artifacts/signing.txt"
"$ANDROID_SDK_ROOT/build-tools/35.0.0/aapt" dump badging "$apk" > "$output_dir/artifacts/apk-badging.txt"
"$ANDROID_SDK_ROOT/build-tools/35.0.0/aapt" dump xmltree "$apk" AndroidManifest.xml > "$output_dir/artifacts/manifest-tree.txt"
if [[ "$X22I_VARIANT" == test ]]; then
    python3 "$script_dir/verify_manifest.py" "$output_dir/artifacts/manifest-tree.txt" org.cagnulen.qdomyoszwift.x22ibaseline 'X22i Test'
else
    python3 "$script_dir/verify_manifest.py" "$output_dir/artifacts/manifest-tree.txt" org.cagnulen.qdomyoszwift QZ
fi
cp "$apk" "$output_dir/artifacts/x22i-$X22I_VARIANT-arm64.apk"
cp "$source_dir/LICENSE" "$output_dir/artifacts/LICENSE"
if [[ "$X22I_VARIANT" == test ]]; then
    # Include the new header in the reproducible source patch without committing it.
    git -C "$source_dir" add --intent-to-add src/x22i_defaults.h
    cp "$source_dir/src/x22i_defaults.h" "$output_dir/artifacts/"
fi
git -C "$source_dir" diff -- src/main.cpp src/x22i_defaults.h src/settings.qml src/qzsettings.cpp src/settings-catalog.json src/android/build.gradle src/android/AndroidManifest.xml > "$output_dir/artifacts/source.patch"
python3 - "$script_dir" "$source_dir" "$output_dir" <<'PY'
import hashlib
import json
import os
import subprocess
import sys
from pathlib import Path
scripts, source, output = map(Path, sys.argv[1:])
info = json.loads((scripts / 'baseline.json').read_text())
info['variant'] = os.environ['X22I_VARIANT']
if info['variant'] == 'test':
    info['package'] = 'org.cagnulen.qdomyoszwift.x22ibaseline'
    info['label'] = 'X22i Test'
    info['signing'] = 'Persistent private test key; separate from installed QZ signer'
    info['settings_defaults_sha256'] = hashlib.sha256((scripts / 'x22i_defaults.h').read_bytes()).hexdigest()
    info['settings_defaults_policy'] = 'Fill missing nonsecret X22i settings; preserve existing saved values'
apk = output / ('artifacts/x22i-' + info['variant'] + '-arm64.apk')
info['apk_sha256'] = hashlib.sha256(apk.read_bytes()).hexdigest()
info['apk_bytes'] = apk.stat().st_size
info['build_recipe_revision'] = subprocess.check_output(['git', '-C', str(scripts), 'rev-parse', 'HEAD'], text=True).strip()
info['submodules'] = subprocess.check_output(['git', '-C', str(source), 'submodule', 'status', '--recursive'], text=True).splitlines()
info['validation'] = 'Packaging checked; device behavior and migration not yet verified'
(output / 'artifacts/build-info.json').write_text(json.dumps(info, indent=2) + '\n')
PY
echo "Baseline APK and provenance are ready in the output artifacts directory."

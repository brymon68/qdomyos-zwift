#!/usr/bin/env bash
# Build this source checkout. Never deploys an APK.
set -euo pipefail

if [[ $# != 1 ]]; then
    echo "Usage: bash tools/x22i/build_baseline.sh NEW_OUTPUT_DIRECTORY" >&2
    exit 2
fi

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
source_dir=$(realpath "$script_dir/../..")
output_dir=$(realpath -m "$1")
: "${QT_ANDROID_DIR:?Set QT_ANDROID_DIR to the Qt 5.15.0 Android installation}"
: "${ANDROID_SDK_ROOT:?Set ANDROID_SDK_ROOT}"
: "${JAVA_HOME:?Set JAVA_HOME to Java 17}"
export PATH="$QT_ANDROID_DIR/bin:$JAVA_HOME/bin:$PATH"
export ANDROID_NDK_ROOT="$ANDROID_SDK_ROOT/ndk/21.4.7075529"
export ANDROID_NDK="$ANDROID_NDK_ROOT"
export ANDROID_HOME="$ANDROID_SDK_ROOT"
export GRADLE_USER_HOME="$output_dir/gradle-cache"
: "${X22I_SIGNING_STORE_FILE:?Set the private PKCS12 keystore path}"
: "${X22I_SIGNING_STORE_PASSWORD:?Set the private keystore password}"
[[ -f "$X22I_SIGNING_STORE_FILE" ]] || { echo "Signing keystore is missing" >&2; exit 2; }
jobs=${X22I_BUILD_JOBS:-2}

for command in git make python3 rsync qmake androiddeployqt javac; do
    command -v "$command" >/dev/null || { echo "Missing build command: $command" >&2; exit 2; }
done
[[ $(qmake -query QT_VERSION) == 5.15.0 ]] || { echo "Qt 5.15.0 is required" >&2; exit 2; }
[[ $(javac -version 2>&1) == 'javac 17.'* ]] || { echo "Java 17 is required" >&2; exit 2; }
[[ -d "$ANDROID_NDK_ROOT" ]] || { echo "NDK 21.4.7075529 is required" >&2; exit 2; }
[[ ! -e "$output_dir" ]] || { echo "Use a new output directory" >&2; exit 2; }

# Build only committed application source, with the dependency versions recorded by Git.
# No owner credentials or service secrets are needed in the application source.
python3 - "$source_dir" <<'PY_SOURCE'
import subprocess
import sys
from pathlib import Path

source = Path(sys.argv[1])
def git(*args):
    return subprocess.check_output(['git', '-C', str(source), *args], text=True).strip()
if Path(git('rev-parse', '--show-toplevel')).resolve() != source:
    raise SystemExit('Run the build script from a checkout of this repository')
if git('status', '--porcelain', '--untracked-files=normal'):
    raise SystemExit('Use a clean source checkout with committed changes')
if any(line[0] != ' ' for line in subprocess.check_output(
        ['git', '-C', str(source), 'submodule', 'status', '--recursive'], text=True).splitlines()):
    raise SystemExit('Initialize all submodules at their pinned revisions')
if (source / 'src/secret.h').exists() or (source / 'src/secrets.h').exists():
    raise SystemExit('Private secret headers are not accepted in X22i builds')
PY_SOURCE

mkdir -p "$output_dir/httpserver" "$output_dir/httpserver-source" "$output_dir/native" "$output_dir/android" "$output_dir/artifacts"
cp "$source_dir/src/x22i_defaults.h" "$output_dir/artifacts/"
cp "$source_dir"/qt-patches/android/5.15.0/jar/* "$QT_ANDROID_DIR/jar/"
# The Android-compatible HTTP parser is included in this repository. Overlay it in the
# build directory so the dependency checkout remains unchanged.
rsync -a --exclude .git "$source_dir/src/qthttpserver/" "$output_dir/httpserver-source/"
cp "$source_dir"/qHttpServerBin/5.15.2/headers/* "$output_dir/httpserver-source/src/3rdparty/http-parser/"

cd "$output_dir/httpserver"
# The copy has no .git metadata; request Qt forwarding-header generation explicitly.
qmake "$output_dir/httpserver-source" -spec android-clang ANDROID_ABIS=arm64-v8a "ANDROID_NDK_ROOT=$ANDROID_NDK_ROOT" CONFIG+=git_build
make -j"$jobs"
make install

cd "$output_dir/native"
qmake "$source_dir/x22i.pro" -spec android-clang ANDROID_ABIS=arm64-v8a "ANDROID_NDK_ROOT=$ANDROID_NDK_ROOT"
make -j"$jobs"
make "INSTALL_ROOT=$output_dir/android" install
rsync -a --exclude build --exclude .gradle "$source_dir/src/android/" "$output_dir/android/"

python3 - "$source_dir" "$output_dir" <<'PY'
import json
import sys
from pathlib import Path
source, output = map(Path, sys.argv[1:])
deployment = output / 'native/src/android-x22i-deployment-settings.json'
data = json.loads(deployment.read_text())
data['android-extra-libs'] = ','.join(str(source / 'android_openssl/no-asm/latest/arm64' / name)
                                     for name in ('libcrypto_1_1.so', 'libssl_1_1.so'))
deployment.write_text(json.dumps(data, indent=2) + '\n')
PY

# --no-build skips copyQtFiles in Qt 5.15.0; deliberately do not use it.
androiddeployqt --input "$output_dir/native/src/android-x22i-deployment-settings.json" \
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
python3 "$script_dir/verify_apk.py" --no-fan --no-purchasing "$apk"
"$ANDROID_SDK_ROOT/build-tools/35.0.0/apksigner" verify --verbose --print-certs "$apk" > "$output_dir/artifacts/signing.txt"
"$ANDROID_SDK_ROOT/build-tools/35.0.0/aapt" dump badging "$apk" > "$output_dir/artifacts/apk-badging.txt"
"$ANDROID_SDK_ROOT/build-tools/35.0.0/aapt" dump xmltree "$apk" AndroidManifest.xml > "$output_dir/artifacts/manifest-tree.txt"
python3 "$script_dir/verify_manifest.py" "$output_dir/artifacts/manifest-tree.txt"
cp "$apk" "$output_dir/artifacts/x22i-arm64.apk"
cp "$source_dir/LICENSE" "$output_dir/artifacts/LICENSE"
cp "$source_dir/NOTICE" "$output_dir/artifacts/NOTICE"
python3 - "$script_dir" "$source_dir" "$output_dir" <<'PY'
import hashlib
import json
import subprocess
import sys
from pathlib import Path
scripts, source, output = map(Path, sys.argv[1:])
info = json.loads((scripts / 'baseline.json').read_text())
info['signing'] = 'Persistent private signing key'
info['settings_defaults_sha256'] = hashlib.sha256((source / 'src/x22i_defaults.h').read_bytes()).hexdigest()
info['settings_defaults_policy'] = 'Fill missing nonsecret X22i settings; preserve existing saved values'
apk = output / 'artifacts/x22i-arm64.apk'
info['apk_sha256'] = hashlib.sha256(apk.read_bytes()).hexdigest()
info['apk_bytes'] = apk.stat().st_size
info['source_revision'] = subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD'], text=True).strip()
info['build_recipe_revision'] = info['source_revision']
info['submodules'] = subprocess.check_output(['git', '-C', str(source), 'submodule', 'status', '--recursive'], text=True).splitlines()
info['validation'] = 'Packaging checked; device behavior and migration not yet verified'
(output / 'artifacts/build-info.json').write_text(json.dumps(info, indent=2) + '\n')
PY
echo "X22i APK and provenance are ready in the output artifacts directory."

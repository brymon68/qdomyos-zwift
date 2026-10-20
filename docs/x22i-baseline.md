# X22i source and Android build

The application is **X22i**, package `io.github.brymon68.x22i`. Its source lives in `src/` and builds directly through `x22i.pro`. There is one application and one persistent signer.

Two direct dependency submodules provide Android OpenSSL and Qt HTTP Server. Qt HTTP Server also has a nested HTTP parser dependency. Initialize them recursively; none contains the application source. Dependency licenses and source notices remain with their code, alongside the application [LICENSE](../LICENSE) and [NOTICE](../NOTICE).

## Current scope

The X22i connects to local iFIT using `localhost`, which is required by the console's TLS certificate. It displays speed, incline, pace, distance, elapsed time, and heart rate, with Start/Pause/Stop and manual speed/incline controls. The floating overlay provides the same live metrics and controls. Bluetooth and DIRCON remain available for Zwift on the Mac, with Zwift authentication and terrain polling retained for automatic incline.

Other equipment, accessories, Windows and ESP32 support, localization, personal profiles, cloud fitness integrations, trial enforcement, purchasing, fan support, ANT+, power measurement, and local workout programs, editing, recording, charts, history, and FIT/GPX import/export are removed. iFIT's workout-state protocol remains because it controls and monitors the physical treadmill.

## Build from source

The supported target is Android ARM64 (`arm64-v8a`). Install Qt Android **5.15.0** with Network Auth, Java **17**, Android platform **35**, build tools **35.0.0**, NDK **21.4.7075529**, and host tools `git`, `make`, `python3`, and `rsync`. Gradle **8.13** is supplied by the repository. Use a dedicated Qt installation: the recipe installs the repository's Qt Bluetooth JAR and Qt HTTP Server into it.

On the configured workstation, activate the existing toolchain:

```sh
source "$HOME/.local/share/qz-x22i-toolchain/env.sh"
```

Otherwise set `QT_ANDROID_DIR`, `ANDROID_SDK_ROOT`, and `JAVA_HOME` to your installed toolchain. Set `X22I_SIGNING_STORE_FILE` to the persistent private PKCS12 keystore and load `X22I_SIGNING_STORE_PASSWORD` privately. The existing key alias remains `x22i-test`; this is a signing-key identifier, not an application variant. Keep the key and password out of arguments, logs, and committed files.

```sh
git submodule update --init --recursive
python3 -m unittest discover -s tools/x22i -p 'test_*.py' -v
bash tools/x22i/build_baseline.sh /LOCAL/NEW-BUILD-DIRECTORY
```

The output directory must not already exist. The script requires a clean checkout with committed changes and exact dependency revisions. It copies dependency source into the output directory before applying the HTTP parser compatibility overlay. Private authentication headers are rejected. It never installs or launches an APK.

### GitHub Actions

The [X22i workflow](../.github/workflows/x22i-baseline.yml) runs on pushes to `master` and `codex/**` and on manual dispatch. Signing uses the existing repository secrets `X22I_TEST_KEYSTORE_BASE64` and `X22I_TEST_KEYSTORE_PASSWORD`; these secret names remain unchanged. CI removes the temporary keystore afterward.

```sh
gh workflow run x22i-baseline.yml --repo brymon68/qdomyos-zwift --ref "$(git branch --show-current)"
gh run download RUN_ID --repo brymon68/qdomyos-zwift --name x22i-arm64 --dir /LOCAL/ARTIFACTS
```

CI runs Python packaging and configuration checks, QtCore tests of settings preservation and treadmill protocol behavior, and a standalone C++17 Zwift terrain-incline test before building. Personal settings, credentials, and backups must stay outside CI.

### Docker toolchain

The optional image contains the toolchain. With rootless Docker:

```sh
docker build --platform linux/amd64 -t x22i-toolchain docker/x22i
docker run --rm --platform linux/amd64 \
  --mount type=bind,src=/LOCAL/SOURCE-CHECKOUT,dst=/source \
  --mount type=bind,src=/LOCAL/BUILD-PARENT,dst=/build \
  --mount type=bind,src=/PRIVATE/signing.p12,dst=/run/secrets/signing.p12,readonly \
  --mount type=bind,src=/PRIVATE/keystore-password,dst=/run/secrets/password,readonly \
  -e X22I_SIGNING_STORE_FILE=/run/secrets/signing.p12 \
  -e X22I_SIGNING_STORE_PASSWORD_FILE=/run/secrets/password \
  x22i-toolchain
```

Initialize submodules and commit changes in the mounted checkout first. `/LOCAL/BUILD-PARENT/output` must not exist; artifacts appear under `output/artifacts`. The containerized build path has not been runtime-validated on the console.

## Artifacts and verification

`artifacts/` contains `x22i-arm64.apk`, `build-info.json`, the signing report, package badging, merged manifest, defaults header, LICENSE, and NOTICE. Source and recipe provenance identify the same repository commit and record dependency revisions, toolchain versions, and the APK checksum and size.

The build checks ARM64 Qt/OpenSSL/C++ payloads, QML resources, gRPC assets, package/component/provider identities, and removed-code markers in the native library and all DEX files. It rejects removed workout APIs and orphan Android helpers while permitting iFIT control services. Signatures are verified separately. These checks do not establish console connectivity or physical incline behavior.

## Settings

Qt uses organization `brymon68` and application `X22i`, with expected configuration filename `X22i.conf`. Verify the actual storage location on the console before migrating data; the application package, Android data directory, and Qt settings identity all affect where configuration is read.

[`src/x22i_defaults.h`](../src/x22i_defaults.h) writes missing nonsecret keys before normal settings reads and QML startup. Existing values, including false, zero, empty values, and private credentials, take precedence. Retained defaults cover miles, Bluetooth/DIRCON, Zwift auto-incline, gain/calibration, the 0.5% incline step, delay, bounds, and the incline lookup table. Fresh configurations need private Zwift authentication.

## Private migration before first launch

Changing the package ID creates separate Android storage. Using the same signer does not copy settings from another package, and the new Qt organization/application identity also changes the settings filename and location. Migration is an explicit deployment step; the build does not perform it.

1. Discover the application currently used on the console, including its installed package, APK path, signer, and actual QSettings file location. Do not infer the source identity from an old guide or assume its settings path matches the new application.
2. Back up that installed APK and the complete configuration privately. Preserve the source application and rollback backup throughout migration.
3. Before installing or launching on the console, obtain current confirmation that the owner is present and the belt is stopped. Verify the candidate package is `io.github.brymon68.x22i` and its signer matches the persistent key.
4. Install without launching, verify the new Android data directory and intended Qt settings location, and migrate the configuration with the correct owner and permissions. Preserve all source settings, including private authentication and obsolete keys. Read the candidate file back from that verified location before the first launch.
5. Compare the backup and candidate readback privately:

   ```sh
   python3 tools/x22i/verify_settings.py /PRIVATE/SOURCE-SETTINGS.ini /PRIVATE/CANDIDATE-READBACK.ini
   ```

   The comparison reports counts only. Require zero missing and zero changed settings. Added defaults are allowed. Resolve discrepancies before launching.
6. Keep the source application stopped while validating X22i so only one application controls the treadmill connection. Confirm live telemetry, the active Mac transport, and terrain incline separately from build checks. Review any post-launch settings changes against the backup.

Do not commit raw configuration, authentication data, private backups, or unrestricted console logs. Later updates to `io.github.brymon68.x22i` use the same package and persistent signing key; do not uninstall to work around an update failure.

## Retained behavior and limits

Automatic incline follows Zwift authentication/polling and decode, grade gain/offset, `treadmill::changeInclination()`, the X22i update, JNI, and `InclineService.SetIncline`. Existing bounds, correction, difficulty, and rounding remain. Required gRPC/protobuf generation and TLS/session assets remain. No automatic speed behavior was added.

Existing control limitations include target-to-delta conversion across C++ and Java, blocking RPCs without deadlines, and incomplete stale-metric and acknowledgment handling. Hardware checks of downhill clamping, stop/pause, restart/reconnect, and the active Mac transport remain separate from build verification.

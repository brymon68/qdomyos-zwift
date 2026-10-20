# X22i Android baseline

This is the first-phase build and migration tooling for [issue #1](https://github.com/brymon68/qdomyos-zwift/issues/1). The pinned ARM64 baseline and separate X22i Test app build successfully. X22i Test is installed on the console; settings migration, first launch, and physical treadmill comparison remain to be demonstrated. Packaging and installation checks alone do not establish device behavior.

## Verified installation: 2026-10-20 UTC

The separate CI test APK from run `39365509040`, attempt 2, was installed through ADB on the verified Android 9 / ARM64 console. Before installation, its checksum, source provenance, v1/v2 signatures, expected signing certificate, native payload, and merged manifest were checked again.

ADB reported success. The installed package is `org.cagnulen.qdomyoszwift.x22ibaseline`, version **2.21.5**, version code **1403**. Its installed APK SHA-256 is `24aa9d15a4cce01ed403485311bf7c94d196b7a2c732f6390015b5b957b28ecb`, matching the CI artifact. The original QZ APK matched the private rollback backup before installation and remained byte-for-byte unchanged afterward.

Android reported `stopped=true` and `notLaunched=true`, with no test-app process running. No app launch, settings migration, reboot, or physical control was performed. Complete the private settings migration and verification below before first launch; hardware acceptance is still pending.

## Verified separate test app: 2026-10-20 UTC

[GitHub Actions run 39365509040, attempt 2](https://github.com/brymon68/qdomyos-zwift/actions/runs/39365509040/attempts/2) passed using build recipe commit `db25d3b7294f4bb576d5f678629577591d5113f1`. All 36 settings/privacy, APK, and manifest-check tests passed. The downloaded APK independently passed the payload and manifest checks; its checksum matched the provenance and its certificate matched the backed-up persistent test key.

| Measurement | Result |
|---|---|
| APK size | 50,994,070 bytes |
| Native architectures | `arm64-v8a` only |
| Package/version | `org.cagnulen.qdomyoszwift.x22ibaseline`, 2.21.5 / 1403 |
| Launcher label | X22i Test |
| APK SHA-256 | `24aa9d15a4cce01ed403485311bf7c94d196b7a2c732f6390015b5b957b28ecb` |
| Signing certificate SHA-256 | `38a1ec468bd45b37b20e7477613b9951aae24c3171673a5a6186abe7f3c7167d` |
| Signature verification | v1 and v2 passed |
| Provider authorities | Both use the separate test package prefix |

The first attempt was cancelled during active Gradle processing after native compilation completed. The retry finished in approximately 14 minutes. No installation or launch was performed as part of this build verification. The original app remains installed; settings migration and on-device acceptance are still pending.

## Verified build: 2026-10-20 UTC

[GitHub Actions run 39363926662](https://github.com/brymon68/qdomyos-zwift/actions/runs/39363926662) passed using build recipe commit `1ab94b8601b5b626941d484137fd9db4023dde36`. All 20 settings/privacy and APK-check tests passed. The downloaded APK independently passed the payload checks, and its SHA-256 matched the recorded provenance.

| Measurement | Result |
|---|---|
| APK size | 50,993,998 bytes |
| Native architectures | `arm64-v8a` only |
| Package/version | `org.cagnulen.qdomyoszwift`, 2.21.5 / 1403 |
| APK SHA-256 | `1bae6bb0ded6668e6ed92558d69ee24ed7b15c6dfa3abb804d516d2a6f6381f6` |
| Signature verification | v1 and v2 passed |
| Installed rollback APK size | 43,557,271 bytes |
| Upgrade compatibility | Signing certificate differs from installed APK |

The installed APK was backed up privately and its checksum verified against the console. The existing 861-setting backup still matches its recorded checksum. The baseline's iFIT TLS assets match those in the installed APK. No app was installed, launched, or stopped, and no physical controls were exercised. The existing QZ process was not running when inspected, so no runtime memory/startup comparison was recorded.

This baseline is larger than the installed January build; feature removal has not begun. Its new debug signer cannot update the existing package in place. The owner selected a separately identified test app to preserve the original installation. Do not use this successful build as evidence that device acceptance checks passed.

## Pinned artifact

[`tools/x22i/baseline.json`](../tools/x22i/baseline.json) pins application source to `cagnulein/qdomyos-zwift` revision `8b78e9464c55572b9eaf816128bac905c7150a26`. Revision `3044d2d42a54d00c97e4d47f531319d07aa453fd` is the nightly build recipe reference; it is **not** the application source pin. Each artifact records the revision of this repository's build recipe separately.

The target is ARM64 (`arm64-v8a`), version **2.21.5**, version code **1403**, package **`org.cagnulen.qdomyoszwift`**. The disposable source checkout receives the documented localhost-default variant in QML, C++, and the settings catalog, plus a Gradle ARM64 filter for native dependencies. The build retains `DEFINES+=LICENSE` trial enforcement and the baseline fan implementation until the existing installation has been compared on hardware. Trial removal, feature removal, and a smaller application are later changes.

The artifact bundle contains the APK, checksum/provenance in `build-info.json`, `source.patch`, signing information, package badging, and `LICENSE`. The APK checker verifies required ARM64 Qt/OpenSSL/C++ libraries, QML resources, and gRPC client assets. It does not verify the entire native dependency graph, launch, TLS connectivity, or incline behavior.

## Build

The [workflow](../.github/workflows/x22i-baseline.yml) builds the pinned upstream checkout separately from this repository. After the workflow is available on GitHub, dispatch it on the scaffolding branch:

```sh
gh workflow run x22i-baseline.yml --repo brymon68/qdomyos-zwift --ref codex/x22i-baseline -f variant=baseline
gh run list --repo brymon68/qdomyos-zwift --workflow x22i-baseline.yml
gh run watch RUN_ID --repo brymon68/qdomyos-zwift
gh run download RUN_ID --repo brymon68/qdomyos-zwift --name x22i-baseline-arm64 --dir /LOCAL/ARTIFACTS
```

Replace `RUN_ID` and uppercase paths with local values. Matching changes pushed to `codex/x22i-baseline` build the `test` variant. Manual dispatch defaults to `test`; select `baseline` explicitly to reproduce the original package identity.

For a local build, provide a **clean, disposable** checkout at the exact source pin with all recursive submodules initialized. Install Qt Android **5.15.0** with Qt Charts, Network Auth, and Purchasing; Java **17**; Android platform **35**; build tools **35.0.0**; NDK **21.4.7075529**; and host build tools including `make`, `python3`, and `rsync`. The source supplies Gradle **8.13**. Use a dedicated Qt installation: the script copies the baseline's patched Qt JARs into it and installs its Qt HTTP server build there.

```sh
git clone --no-checkout https://github.com/cagnulein/qdomyos-zwift.git /LOCAL/BASELINE-SOURCE
git -C /LOCAL/BASELINE-SOURCE checkout --detach 8b78e9464c55572b9eaf816128bac905c7150a26
git -C /LOCAL/BASELINE-SOURCE submodule update --init --recursive
export QT_ANDROID_DIR=/LOCAL/DEDICATED-QT/5.15.0/android
export ANDROID_SDK_ROOT=/LOCAL/ANDROID-SDK
export JAVA_HOME=/LOCAL/JAVA-17
python3 -m unittest discover -s tools/x22i -p 'test_*.py' -v
bash tools/x22i/build_baseline.sh /LOCAL/BASELINE-SOURCE /LOCAL/NEW-BUILD-DIRECTORY
```

The output directory must not exist. The script modifies the disposable source defaults, builds and checks the APK, and does not install it. Do not supply private secret headers or owner credentials.

### Docker toolchain

[`docker/x22i/Dockerfile`](../docker/x22i/Dockerfile) packages the Android build tools on Ubuntu 22.04 for a Linux AMD64 host, producing ARM64 APKs. It pins Qt 5.15.0, Temurin Java 17.0.20.1, Android command-line tools 19.0, NDK 21.4.7075529, SDK platform 35, build tools 35.0.0, and the Qt installer versions. Java and command-line tool archives are checksum-checked during image creation. The recipe still runs the same `tools/x22i/build_baseline.sh`.

Use [rootless Docker](https://docs.docker.com/engine/security/rootless/) for the commands below. Container UID 0 then maps to the host user, allowing the build to write the mounted source/output and read owner-only signing files. The image needs a writable Qt installation because the recipe installs QtHttpServer and patches its Bluetooth JAR.

From the repository root:

```sh
docker info --format '{{json .SecurityOptions}}'
# Confirm the active daemon reports rootless.
docker build --platform linux/amd64 -t qz-x22i-toolchain:qt5.15.0 docker/x22i
```

Prepare a clean, disposable source checkout at the pinned revision with all recursive submodules, as in the local-build instructions above. Create a fresh output parent; its `output` child must not exist. Replace the uppercase paths below with local paths:

```sh
mkdir -p /LOCAL/NEW-BUILD-PARENT
docker run --rm --init \
  --mount "type=bind,src=$PWD,dst=/recipe,readonly" \
  --mount "type=bind,src=/LOCAL/CLEAN-SOURCE,dst=/source" \
  --mount "type=bind,src=/LOCAL/NEW-BUILD-PARENT,dst=/build" \
  --mount "type=bind,src=/PRIVATE/TEST-SIGNING/test.p12,dst=/run/secrets/test.p12,readonly" \
  --mount "type=bind,src=/PRIVATE/TEST-SIGNING/password,dst=/run/secrets/test-password,readonly" \
  --env X22I_VARIANT=test \
  --env X22I_SIGNING_STORE_FILE=/run/secrets/test.p12 \
  --env X22I_SIGNING_STORE_PASSWORD_FILE=/run/secrets/test-password \
  qz-x22i-toolchain:qt5.15.0
```

The signing key and password enter through runtime-only read-only mounts. The entrypoint loads the password into the build process environment without printing it. The image build context contains only `docker/x22i`; application source and private configuration are not image inputs. Artifacts appear in `/LOCAL/NEW-BUILD-PARENT/output/artifacts`. Omitting the test environment variables and secret mounts builds the original-package `baseline` variant.

Container recipe syntax and entrypoint smoke checks passed, including password-file loading and rejection of empty/missing password files. A Docker image build and containerized APK build are pending the workstation's Docker installation and administrator authentication.

### Installed local toolchain

The development workstation now has a dedicated user-local toolchain at `$HOME/.local/share/qz-x22i-toolchain`. Activate it in a Bash shell:

```sh
source "$HOME/.local/share/qz-x22i-toolchain/env.sh"
```

It provides Qt Android 5.15.0 with Charts, Network Auth, and Purchasing; Temurin Java 17.0.20.1; Android command-line tools 19.0; SDK platform 35; build tools 35.0.0; NDK 21.4.7075529; platform-tools; and GNU Make 4.4.1. The NDK supplies the C++ compiler. No system-wide packages or shell startup files were changed. `toolchain.json` and `python-packages.txt` in that directory record the installation and verification details. The activation file selects four compiler jobs.

A complete local build on Ubuntu 26.04 passed on 2026-10-20 in **275 seconds**, using recipe commit `2fb1486207d5aea759cd02d797b6a23a7daac434`. Its ARM64 payload and merged manifest passed verification, both APK signature schemes v1 and v2 verified, and its signer matched the persistent test key. The APK is 50,993,897 bytes with SHA-256 `d7b56cdaf0f72609f62e4ee23d792d9acde38102d3d8ca08e118f7a3c9b37d85`. The local source, intermediates, log, and artifacts remain under the ignored `build-x22i-local/` directory. This verifies the build toolchain, not treadmill behavior.

## Separate test app

The `test` variant uses application ID `org.cagnulen.qdomyoszwift.x22ibaseline` and launcher label **X22i Test**. Java namespace, source manifest package, JNI classes, QSettings organization/application names, and iFIT protocol behavior stay the same. The variant still contains the baseline trial and fan features.

It uses a persistent private PKCS12 signing key with alias `x22i-test`, backed up outside Git with owner-only permissions. The repository secrets `X22I_TEST_KEYSTORE_BASE64` and `X22I_TEST_KEYSTORE_PASSWORD` provide this key to CI. The workflow fails when they are absent and removes its temporary key afterward. Do not rotate this key casually: future test APKs need the same signer for updates. Never upload the saved QSettings backup, Zwift credentials, or personal configuration to CI.

```sh
gh workflow run x22i-baseline.yml --repo brymon68/qdomyos-zwift --ref codex/x22i-baseline -f variant=test
gh run download RUN_ID --repo brymon68/qdomyos-zwift --name x22i-test-arm64 --dir /LOCAL/TEST-ARTIFACTS
```

For local builds set `X22I_VARIANT=test`, `X22I_SIGNING_STORE_FILE` to the private PKCS12 path, and `X22I_SIGNING_STORE_PASSWORD` through the local secret-handling mechanism before invoking the same script. Do not put passwords in command arguments, committed files, or logs.

The build records the final manifest and checks package identity, launcher identity, component names, and provider authorities. Only one QZ app may run during testing: both identities still advertise the same fitness device and access the same iFIT endpoint. Before copying any private settings, verify the candidate's actual settings/profile locations on the console and stop both QZ apps. Copy into the candidate only, then verify all 861 original settings. Rollback consists of stopping the test app and returning to the original installation; neither installation nor hardware testing is performed by the build workflow.

## Private settings and installation

The existing **861 settings** are the migration source of truth. Newer defaults must not replace them. Preserve every backed-up value, including credentials, selected devices, transport settings, incline limits, Zwift gain/offset and negative-incline behavior, difficulty settings, and calibration. Keep the complete inclination override table and its gain/offset keys even though this baseline's NordicTrack gRPC path does not call `treadmillInclinationOverride()`.

Use the complete native QSettings configuration for migration. A `.qzs` profile export encrypts password/token fields with `cryptoKeySettingsProfiles` but omits that key; import also skips `peloton_refreshtoken`. Profile export/import therefore cannot establish an exact settings migration. Preserve the original configuration's encryption key along with its other values.

Verify the data-directory selection before first launch. The pinned app uses shared `Documents/QZ` when `android_documents_folder` is enabled or Android is at least 14; otherwise it uses context-derived app storage. Startup migration can copy and then delete files from the old profile directory. A separate application ID alone does not guarantee profile/database isolation.

Before replacing the installation:

1. Stop QZ and privately back up the installed APK (and all installed split APKs, if present), settings, and relevant profiles using the device's verified access method. Do not assume that `run-as` or direct access to app-private files works.
2. Record the installed package/version and signing certificate, and verify that the backed-up APK can be recovered. Compare the installed and candidate signing certificates locally. A newly built debug APK is **not presumed update-compatible**, even with the same package/version.
3. Resolve signing or version incompatibility before installation. **Never uninstall as a workaround**: it can destroy app-private settings. Keep the existing installation available until rollback is established.
4. If a private configuration must be copied, copy it only while QZ is stopped, using the verified destination and access method. Preserve the original backup unchanged. After candidate startup, stop QZ again and export its settings privately for comparison.

Example local checks use placeholders and do not print settings names or values:

```sh
adb shell am force-stop org.cagnulen.qdomyoszwift
adb shell pm path org.cagnulen.qdomyoszwift
# Back up each reported APK and the settings using verified device access.
"$ANDROID_SDK_ROOT/build-tools/35.0.0/apksigner" verify --print-certs /PRIVATE/BACKUP/installed.apk
"$ANDROID_SDK_ROOT/build-tools/35.0.0/apksigner" verify --print-certs /LOCAL/ARTIFACTS/x22i-baseline-arm64.apk
python3 tools/x22i/verify_settings.py /PRIVATE/BACKUP/settings.conf /PRIVATE/CANDIDATE/settings.conf
```

Before first launch, the settings comparison must report all **861 source settings preserved**, with **zero missing and zero changed**, and exit successfully. Added settings are counted separately; they do not permit replacing original values. This compares stored INI text, not effective runtime settings. Recheck after first launch and privately review every difference: the pinned app increments `app_opening` on startup, so runtime bookkeeping can change even when migration was exact. Do not silently ignore differences or overwrite the original backup. Exercise the important settings on hardware. Keep backups and exports outside the repository and CI artifacts; do not attach their contents to issues or logs. Avoid collecting unrestricted startup logs: this baseline logs many setting values.

## Behavior retained and known limits

The source already contains the gRPC service. Android initializes `GrpcTreadmillService` for the configured host on port 54321, with its existing client certificate/key assets and `client_id` metadata. Preserve its gRPC 1.63.0/protobuf-lite 3.25.3 generation alongside the Zwift protobuf schemas during later reduction.

Automatic incline remains `trainprogram::scheduler()` → Zwift authentication/polling and Android decode → grade gain/offset → `treadmill::changeInclination()` → NordicTrack `update()` → JNI → `InclineService.SetIncline`. The treadmill step applies configured bounds, correction, difficulty, and rounding before queuing a request. The gRPC path consumes that request without waiting for UDP telemetry. Zwift API auto-incline itself does not request speed; optional incline-to-speed simulation or wattage following can, so verify the actual retained settings.

The pinned implementation has limitations that this scaffolding does not repair:

- C++ converts an absolute incline target into a delta against its cached incline. Java queues that delta, then adds it to its current incline at execution time. A metric change between those steps can shift the requested target. Speed uses the same pattern.
- Metrics and control calls share one executor, use blocking RPCs without deadlines, and retain cached values on errors. There is no freshness guard or queued-command expiry; C++ can continue integrating distance from stale speed. The workout-state stream has no explicit restart after failure.
- Java catches initialization errors while C++ marks initialization successful after the void JNI call. Command results are not checked, and pending requests are cleared before acknowledgment.
- The gRPC output has no final X22i-specific upper incline bound; its Java lower bound is -50%. Generic settings clamp before difficulty adjustments. Preserve and validate the existing zero-floor configuration rather than treating generic defaults as machine limits.
- Legacy UDP/ADB handling remains alongside gRPC. Zwift auth refresh, decode failure handling, and sample history also need review before calling the connection resilient.

## Physical comparison checklist

Record observations against the currently working installation before changing behavior. No item below is considered passed by building or inspecting source.

- [ ] Establish the **actual Mac-to-QZ transport and incline source** in the working setup. Confirm whether incline comes from Zwift API, virtual-device control, or another configured source; confirm the source used by the candidate matches.
- [ ] Confirm manual treadmill speed is reported correctly in QZ and on the Mac, with the intended units. Confirm no automatic speed command is generated during incline-only testing.
- [ ] Compare an uphill request, a flat request, and a downhill request. Verify the configured **zero floor** keeps downhill at 0%, and check physical incline against the requested value after calibration/scaling.
- [ ] Compare start/pause/recording behavior and retained fan controls. Keep the physical stop control immediately available and verify manual stop behavior.
- [ ] Interrupt and restore the actual connection. Check stale metrics, pending incline requests, reconnect behavior, and that reconnection does not change manual speed or replay an unexpected command.
- [ ] Stop QZ, compare the exported candidate settings with the private backup again, and retain the APK/provenance and observations needed to reproduce the result.

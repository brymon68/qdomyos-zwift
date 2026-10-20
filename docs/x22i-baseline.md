# X22i Android baseline

This is first-phase build and migration scaffolding for [issue #1](https://github.com/brymon68/qdomyos-zwift/issues/1). A successful build, installation, settings migration, and physical treadmill comparison remain to be demonstrated. Packaging checks alone do not establish device behavior.

## Pinned artifact

[`tools/x22i/baseline.json`](../tools/x22i/baseline.json) pins application source to `cagnulein/qdomyos-zwift` revision `8b78e9464c55572b9eaf816128bac905c7150a26`. Revision `3044d2d42a54d00c97e4d47f531319d07aa453fd` is the nightly build recipe reference; it is **not** the application source pin. Each artifact records the revision of this repository's build recipe separately.

The target is ARM64 (`arm64-v8a`), version **2.21.5**, version code **1403**, package **`org.cagnulen.qdomyoszwift`**. The disposable source checkout receives the documented localhost-default variant in QML, C++, and the settings catalog, plus a Gradle ARM64 filter for native dependencies. The build retains `DEFINES+=LICENSE` trial enforcement and the baseline fan implementation until the existing installation has been compared on hardware. Trial removal, feature removal, and a smaller application are later changes.

The artifact bundle contains the APK, checksum/provenance in `build-info.json`, `source.patch`, signing information, package badging, and `LICENSE`. The APK checker verifies required ARM64 Qt/OpenSSL/C++ libraries, QML resources, and gRPC client assets. It does not verify the entire native dependency graph, launch, TLS connectivity, or incline behavior.

## Build

The [workflow](../.github/workflows/x22i-baseline.yml) builds the pinned upstream checkout separately from this repository. After the workflow is available on GitHub, dispatch it on the scaffolding branch:

```sh
gh workflow run x22i-baseline.yml --repo brymon68/qdomyos-zwift --ref codex/x22i-baseline
gh run list --repo brymon68/qdomyos-zwift --workflow x22i-baseline.yml
gh run watch RUN_ID --repo brymon68/qdomyos-zwift
gh run download RUN_ID --repo brymon68/qdomyos-zwift --name x22i-baseline-arm64 --dir /LOCAL/ARTIFACTS
```

Replace `RUN_ID` and uppercase paths with local values. The workflow also runs on matching changes pushed to `codex/x22i-baseline`; these commands do not imply that a run has already succeeded.

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

## Private settings and installation

The existing **861 settings** are the migration source of truth. Newer defaults must not replace them. Preserve every backed-up value, including credentials, selected devices, transport settings, incline limits, Zwift gain/offset and negative-incline behavior, difficulty settings, and calibration. Keep the complete inclination override table and its gain/offset keys even though this baseline's NordicTrack gRPC path does not call `treadmillInclinationOverride()`.

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

The settings comparison must report all **861 source settings preserved**, with **zero missing and zero changed**, and exit successfully. Added settings are counted separately; they do not permit replacing original values. This compares stored INI text, not effective runtime settings. Recheck after first launch and exercise the important settings on hardware. Keep backups and exports outside the repository and CI artifacts; do not attach their contents to issues or logs.

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

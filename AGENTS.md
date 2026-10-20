## Project overview

X22i is a Qt/QML Android ARM64 application installed on one NordicTrack X22i console. It reads telemetry and sends necessary manual controls through the local iFIT gRPC service at `localhost`, and follows Zwift terrain while Zwift runs on a Mac. The application package is `io.github.brymon68.x22i`.

## Rules
1. Ask, dont assume. If something's unclear, ask before writing a line and no silent guesses about intent, architecture, or requirements.
2. Simplest solution first and implement the minimum thing that works. No abstractions you didn't request.
3. Dont touch unrelated code and if a file isnt part of the current task, leave it.
4. Flag uncertainty explicitly or if you're not confident, say so before proceeding as confidence without certainty causes more damage than admitting a gap.
5. Avoid regular expressions, they are hard for a human to read. Use plain string operations (split, startswith, find, a short loop) instead.

## Retained architecture

- `src/devices/bluetooth.cpp` creates the X22i driver directly. There is no multi-device discovery factory.
- `src/devices/bluetoothdevice.*` and `treadmill.*` hold live metrics and shared control behavior.
- `src/devices/nordictrackifitadbtreadmill/` and `src/android/src/GrpcTreadmillService.java` implement the local iFIT connection.
- `src/zwiftincline.*` polls authenticated Zwift terrain. `zwiftinclinegrade.h` computes calibrated incline; the treadmill retains its existing clamps and auto-incline toggle.
- `src/virtualdevices/virtualtreadmill.*`, `src/devices/dircon/`, and `src/characteristics/` provide the Bluetooth and DIRCON transports.
- `src/homeform.*`, `main.qml`, and `Home*.qml` provide the fixed dashboard and manual controls. `templateinfosenderbuilder.*`, `webserverinfosender.*`, and `inner_templates/floating/` provide live floating controls.
- Keep app-owned Java packages and JNI class paths/exported symbols synchronized. External iFIT service/protobuf namespaces and TLS assets describe the console's protocol and must retain their existing identities.

## Scope

Only Android ARM64 and the X22i are supported. Workouts, recording/history, FIT/GPX import/export, other equipment, accessories, power, ANT+, fans, localization, user profiles, Windows, ESP32, and iOS/macOS support have been removed. Keep necessary manual speed/incline/Start/Pause/Stop and Zwift terrain incline. Do not introduce automatic speed control.

## Settings

- Persistent settings use QSettings and Qt.labs.settings. Qt organization is `brymon68`, domain is `brymon68.github.io`, and application is `X22i`.
- `src/x22i_defaults.h` supplies missing nonsecret settings only. Existing values, including false/zero/empty values and credentials, take precedence.
- Preserve existing calibration, bounds, units, private credentials, and legacy key spellings during updates.
- `src/settings-catalog.json` is manually maintained. When adding/removing/renaming a QML setting, update its catalog entry and `settingCount` in the same change. Append new persistent QML properties at the end of the property list.
- A package or Qt settings identity change requires explicit private migration before first launch. See `docs/x22i-baseline.md`; never place actual configurations, credentials, signing keys, device addresses, or private backups in Git or public build artifacts.

## Build and checks

- The root `x22i.pro` builds `src/x22i.pro` and `src/x22i.pri`, producing `libx22i_arm64-v8a.so`.
- `tools/x22i/build_baseline.sh` builds a clean committed checkout into a new output directory. It never deploys or launches an APK. See `docs/x22i-baseline.md` for the pinned Qt/Android toolchain and private signing setup.
- Two direct submodules remain: Android OpenSSL for TLS, and Qt HTTP Server for floating controls. The latter has a nested HTTP parser dependency. No submodule provides the application source.
- Current validation lives in `tools/x22i/`; the obsolete Google Test harness is removed.
- Run `python3 -m unittest discover -s tools/x22i -p 'test_*.py'` for packaging, identity, and settings-file checks.
- Compile and run `test_x22i_defaults.cpp` and `test_treadmill_protocol.cpp` with host QtCore, and `test_zwift_incline_grade.cpp` with C++17. The GitHub Actions workflow contains the commands.
- Run `python3 tools/check_settings_catalog.py BASE_REF` and `python3 tools/check_settings_property_order.py BASE_REF` when settings/catalog files change.
- APK verification covers ARM64 Qt/OpenSSL/C++/QML payloads, removed features, app/component identity, and the native library name. Check signatures separately. A successful build does not establish physical console behavior.
- Before installing or launching on the console, obtain current confirmation that the owner is present and the belt is stopped. Preserve the existing APK/configuration backup and run only one controlling application.

## Versions and attribution

When releasing a version, keep the Android manifest version name/code, main QML version display, and `VERSION` in `src/x22i.pri` synchronized. Android version codes must increase.

Retain `LICENSE`, `NOTICE`, source attribution, and third-party license notices.

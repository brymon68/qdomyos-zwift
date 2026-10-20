#!/usr/bin/env python3
"""Check the required ARM64 baseline payload without extracting APK assets.

This checks packaging prerequisites, not the complete native dependency closure
or whether the application starts and connects on the treadmill.
"""

import argparse
import hashlib
import json
from pathlib import Path
import sys
import zipfile


QT_MODULES = (
    "Core", "Gui", "Qml", "Quick", "QuickControls2", "Widgets", "Bluetooth",
    "Network", "HttpServer", "NetworkAuth", "WebSockets",
)
REQUIRED_FILES = (
    "lib/arm64-v8a/libqdomyos-zwift_arm64-v8a.so",
    "lib/arm64-v8a/libc++_shared.so",
    "lib/arm64-v8a/libcrypto_1_1.so",
    "lib/arm64-v8a/libssl_1_1.so",
    "assets/android_rcc_bundle.rcc",
    "assets/client_cert.pem",
    "assets/client_key.pem",
) + tuple(f"lib/arm64-v8a/libQt5{module}_arm64-v8a.so" for module in QT_MODULES)


def verify_apk(path, no_fan=False, no_purchasing=False):
    with zipfile.ZipFile(path) as apk:
        entries = {entry.filename: entry for entry in apk.infolist() if not entry.is_dir()}
        native_files = [name for name in entries if name.startswith("lib/")]
        abis = sorted({name.split("/")[1] for name in native_files})
        errors = []
        if abis != ["arm64-v8a"]:
            errors.append("Expected only arm64-v8a native libraries; found: " + ", ".join(abis or ["none"]))
        for name in REQUIRED_FILES:
            if name not in entries:
                errors.append(f"Missing required APK entry: {name}")
            elif entries[name].file_size == 0:
                errors.append(f"Empty required APK entry: {name}")
        for name in native_files:
            if name.endswith(".so") and name not in REQUIRED_FILES and entries[name].file_size == 0:
                errors.append(f"Empty native library: {name}")
        app_library = "lib/arm64-v8a/libqdomyos-zwift_arm64-v8a.so"
        if app_library in entries:
            payload = apk.read(app_library)
            checks = [("Removed licensing code", (
                "robertoviola.cloud:4010", "licensePopupVisible", "licenseRequest", "licenseTimeout"))]
            checks.append(("Removed ANT and treadmill power code", (
                "antObject", "antbike", "strydrunpowersensor", "getCurrentWatts",
                "treadmill_follow_wattage")))
            checks.append(("Removed equipment, localization and profiles", (
                "kingsmithr2treadmill", "kineticbike", "domyosbike", "GarminConnect",
                "WebTranslation", "cryptoKeySettingsProfiles", "profile_open_clicked")))
            checks.append(("Removed workout programs, recording and history", (
                "WorkoutModel", "FitDatabaseProcessor", "trainprogram_open_clicked",
                "trainprogram_save_clicked", "previewWorkoutDescription", "previewSessionOnChart",
                "preview_workout_points", "clipboard_accept_workout_prompt")))
            if no_fan:
                checks.append(("Fan support", (
                    "changeFanSpeed", "fanSpeedChanged", "fitmetria_fanfit", "wahookickrheadwind",
                    "eliteariafan", "tile_fan_enabled", "shortcut_fan_plus")))
            if no_purchasing:
                checks.append(("Purchasing", ("InAppStore", "InAppPurchaseBackend", "productUnlockVowels")))
            for feature, markers in checks:
                for marker in markers:
                    if any(marker.encode(encoding) in payload for encoding in ("utf-8", "utf-16-le")):
                        errors.append(feature + " remains in the application library: " + marker)
        markers = ["com/dsi/ant/", "org/cagnulen/qdomyoszwift/ChannelService",
                   "WattsServiceGrpc", "ConstantWattsServiceGrpc", "getCurrentWatts",
                   "ResistanceServiceGrpc", "StrokesServiceGrpc", "PaceServiceGrpc", "RpmServiceGrpc"]
        markers.extend("org/cagnulen/qdomyoszwift/" + name for name in (
            "ContentHelper", "Shortcuts", "ScreenCaptureService", "QZAdbRemote",
            "CSafeRowerUSBHID", "Usbserial",
        ))
        if no_fan:
            markers.extend(("FanStateService", "setFanSpeed", "getCurrentFanSpeed"))
        if no_purchasing:
            markers.extend(("com/android/billingclient", "qt/android/purchasing"))
        for name in entries:
            if name.startswith("classes") and name.endswith(".dex"):
                payload = apk.read(name)
                for marker in markers:
                    if marker.encode("utf-8") in payload:
                        errors.append("Removed feature remains in " + name + ": " + marker)
        if errors:
            raise ValueError("\n".join(errors))

    checksum = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            checksum.update(block)
    return {"sha256": checksum.hexdigest(), "size_bytes": path.stat().st_size, "abis": abis}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("apk", type=Path, help="APK to check; no assets are extracted")
    parser.add_argument("--no-fan", action="store_true", help="Reject fan controls, drivers and Java gRPC methods")
    parser.add_argument("--no-purchasing", action="store_true", help="Reject purchasing code and Android billing classes")
    args = parser.parse_args(argv)
    try:
        result = verify_apk(args.apk, no_fan=args.no_fan, no_purchasing=args.no_purchasing)
    except ValueError as error:
        print(error, file=sys.stderr)
        return 1
    except (OSError, zipfile.BadZipFile):
        print("Cannot read APK: the input is unreadable or is not a valid ZIP archive.", file=sys.stderr)
        return 1
    print(json.dumps(result, sort_keys=True))
    return 0


if __name__ == "__main__":
    sys.exit(main())

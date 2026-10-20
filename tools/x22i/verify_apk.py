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
    "Network", "Charts", "HttpServer", "Xml", "Positioning", "NetworkAuth",
    "WebSockets", "Sql",
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


def verify_apk(path):
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
    args = parser.parse_args(argv)
    try:
        result = verify_apk(args.apk)
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

#!/usr/bin/env python3
"""Exercise packaging failures with synthetic APKs; no real certificates used."""

import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import zipfile


SCRIPT = Path(__file__).with_name("verify_apk.py")
LIBRARIES = (
    "libx22i_arm64-v8a.so", "libc++_shared.so", "libcrypto_1_1.so", "libssl_1_1.so",
    "libQt5Core_arm64-v8a.so", "libQt5Gui_arm64-v8a.so", "libQt5Qml_arm64-v8a.so",
    "libQt5Quick_arm64-v8a.so", "libQt5QuickControls2_arm64-v8a.so", "libQt5Widgets_arm64-v8a.so",
    "libQt5Bluetooth_arm64-v8a.so", "libQt5Network_arm64-v8a.so",
    "libQt5HttpServer_arm64-v8a.so", "libQt5NetworkAuth_arm64-v8a.so", "libQt5WebSockets_arm64-v8a.so",
)


class VerifyApkTests(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory(prefix="x22i-apk-test-")
        self.addCleanup(directory.cleanup)
        self.apk = Path(directory.name) / "x22i-arm64.apk"
        self.entries = {f"lib/arm64-v8a/{name}": b"synthetic native payload" for name in LIBRARIES}
        self.entries.update({
            "assets/android_rcc_bundle.rcc": b"synthetic QML payload",
            "assets/client_cert.pem": b"SYNTHETIC_CERTIFICATE_NOT_FOR_OUTPUT",
            "assets/client_key.pem": b"SYNTHETIC_PRIVATE_KEY_NOT_FOR_OUTPUT",
        })

    def run_verifier(self, *options):
        with zipfile.ZipFile(self.apk, "w") as apk:
            for name, data in self.entries.items():
                apk.writestr(name, data)
        original = self.apk.read_bytes()
        result = subprocess.run(
            [sys.executable, str(SCRIPT), *options, str(self.apk)], capture_output=True, text=True, check=False,
        )
        self.assertEqual(self.apk.read_bytes(), original)
        self.assertNotIn("SYNTHETIC_CERTIFICATE", result.stdout + result.stderr)
        self.assertNotIn("SYNTHETIC_PRIVATE_KEY", result.stdout + result.stderr)
        return result

    def test_valid_payload_reports_archive_identity(self):
        result = self.run_verifier()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stderr, "")
        self.assertEqual(json.loads(result.stdout), {
            "sha256": hashlib.sha256(self.apk.read_bytes()).hexdigest(),
            "size_bytes": self.apk.stat().st_size,
            "abis": ["arm64-v8a"],
        })

    def test_missing_rcc(self):
        del self.entries["assets/android_rcc_bundle.rcc"]
        result = self.run_verifier()
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, "")
        self.assertIn("Missing required APK entry: assets/android_rcc_bundle.rcc", result.stderr)

    def test_wrong_application_library_name(self):
        payload = self.entries.pop("lib/arm64-v8a/libx22i_arm64-v8a.so")
        self.entries["lib/arm64-v8a/libother-app_arm64-v8a.so"] = payload
        result = self.run_verifier()
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, "")
        self.assertIn("Missing required APK entry: lib/arm64-v8a/libx22i_arm64-v8a.so", result.stderr)

    def test_stripped_payload(self):
        result = self.run_verifier("--no-fan", "--no-purchasing")
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_removed_features_are_rejected_in_native_code(self):
        for option, marker in (("--no-fan", "changeFanSpeed"), ("--no-fan", "fitmetria_fanfit"),
                               ("--no-purchasing", "InAppStore")):
            for encoding in ("utf-8", "utf-16-le"):
                with self.subTest(option=option, encoding=encoding):
                    self.entries["lib/arm64-v8a/libx22i_arm64-v8a.so"] = marker.encode(encoding)
                    result = self.run_verifier(option)
                    self.assertEqual(result.returncode, 1)
                    self.assertIn(marker, result.stderr)

    def test_removed_features_are_rejected_in_every_dex(self):
        for option, marker in (("--no-fan", "FanStateService"), ("--no-fan", "setFanSpeed"),
                               ("--no-purchasing", "com/android/billingclient")):
            with self.subTest(option=option, marker=marker):
                self.entries["classes2.dex"] = marker.encode()
                result = self.run_verifier(option)
                self.assertEqual(result.returncode, 1)
                self.assertIn("classes2.dex", result.stderr)
                self.assertIn(marker, result.stderr)

    def test_removed_licensing_code_is_always_rejected_in_native_and_qt_string_encodings(self):
        for marker in ("https://example.invalid/?supporter=", "licensePopupVisible", "licenseRequest", "licenseTimeout"):
            for encoding in ("utf-8", "utf-16-le"):
                with self.subTest(marker=marker, encoding=encoding):
                    self.entries["lib/arm64-v8a/libx22i_arm64-v8a.so"] = marker.encode(encoding)
                    result = self.run_verifier()
                    self.assertEqual(result.returncode, 1)
                    self.assertIn("Removed licensing code remains", result.stderr)

    def test_ant_and_treadmill_power_are_always_rejected(self):
        for marker in ("antObject", "antbike", "strydrunpowersensor", "getCurrentWatts",
                       "treadmill_follow_wattage"):
            for encoding in ("utf-8", "utf-16-le"):
                with self.subTest(marker=marker, encoding=encoding):
                    self.entries["lib/arm64-v8a/libx22i_arm64-v8a.so"] = marker.encode(encoding)
                    result = self.run_verifier()
                    self.assertEqual(result.returncode, 1)
                    self.assertIn(marker, result.stderr)

    def test_ant_and_watts_services_are_rejected_in_secondary_dex(self):
        for marker in ("com/dsi/ant/", "io/github/brymon68/x22i/ChannelService",
                       "WattsServiceGrpc", "ConstantWattsServiceGrpc", "getCurrentWatts",
                       "ResistanceServiceGrpc", "StrokesServiceGrpc", "PaceServiceGrpc", "RpmServiceGrpc"):
            with self.subTest(marker=marker):
                self.entries["classes3.dex"] = marker.encode()
                result = self.run_verifier()
                self.assertEqual(result.returncode, 1)
                self.assertIn("classes3.dex", result.stderr)
                self.assertIn(marker, result.stderr)

    def test_unused_equipment_localization_and_profiles_are_rejected(self):
        for marker in ("kingsmithr2treadmill", "kineticbike", "domyosbike", "GarminConnect",
                       "WebTranslation", "cryptoKeySettingsProfiles", "profile_open_clicked"):
            for encoding in ("utf-8", "utf-16-le"):
                with self.subTest(marker=marker, encoding=encoding):
                    self.entries["lib/arm64-v8a/libx22i_arm64-v8a.so"] = marker.encode(encoding)
                    result = self.run_verifier()
                    self.assertEqual(result.returncode, 1)
                    self.assertIn(marker, result.stderr)

    def test_removed_workout_ui_and_history_are_rejected(self):
        for marker in ("WorkoutModel", "FitDatabaseProcessor", "trainprogram_open_clicked",
                       "trainprogram_save_clicked", "previewWorkoutDescription", "previewSessionOnChart",
                       "preview_workout_points", "clipboard_accept_workout_prompt"):
            for encoding in ("utf-8", "utf-16-le"):
                with self.subTest(marker=marker, encoding=encoding):
                    self.entries["lib/arm64-v8a/libx22i_arm64-v8a.so"] = marker.encode(encoding)
                    result = self.run_verifier()
                    self.assertEqual(result.returncode, 1)
                    self.assertIn("Removed workout programs, recording and history remains", result.stderr)
                    self.assertIn(marker, result.stderr)

    def test_orphan_android_classes_are_rejected_in_secondary_dex(self):
        for name in ("ContentHelper", "Shortcuts", "ScreenCaptureService", "QZAdbRemote",
                     "CSafeRowerUSBHID", "Usbserial"):
            with self.subTest(java_class=name):
                descriptor = "Lio/github/brymon68/x22i/" + name + ";"
                self.entries["classes3.dex"] = descriptor.encode()
                result = self.run_verifier()
                self.assertEqual(result.returncode, 1)
                self.assertIn("classes3.dex", result.stderr)
                self.assertIn(name, result.stderr)

    def test_required_ifit_control_and_zwift_metadata_are_allowed(self):
        payload = b"WorkoutServiceGrpc\x00WorkoutState\x00ZwiftWorkoutMetadata\x00"
        self.entries["lib/arm64-v8a/libx22i_arm64-v8a.so"] = payload
        self.entries["classes.dex"] = payload + b"Lcom/ifit/glassos/workout/WorkoutServiceGrpc;"
        result = self.run_verifier("--no-fan", "--no-purchasing")
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_missing_qt_or_runtime_library(self):
        for name in ("libQt5QuickControls2_arm64-v8a.so", "libQt5HttpServer_arm64-v8a.so", "libc++_shared.so", "libssl_1_1.so"):
            with self.subTest(library=name):
                path = f"lib/arm64-v8a/{name}"
                data = self.entries.pop(path)
                result = self.run_verifier()
                self.entries[path] = data
                self.assertEqual(result.returncode, 1)
                self.assertIn(f"Missing required APK entry: {path}", result.stderr)

    def test_wrong_abi(self):
        self.entries = {name.replace("lib/arm64-v8a/", "lib/x86_64/"): data for name, data in self.entries.items()}
        result = self.run_verifier()
        self.assertEqual(result.returncode, 1)
        self.assertIn("Expected only arm64-v8a native libraries; found: x86_64", result.stderr)

    def test_extra_abi_runtime_is_rejected(self):
        self.entries["lib/armeabi-v7a/libc++_shared.so"] = b"extra runtime"
        result = self.run_verifier()
        self.assertEqual(result.returncode, 1)
        self.assertIn("Expected only arm64-v8a native libraries", result.stderr)

    def test_empty_required_entries(self):
        for path in ("assets/android_rcc_bundle.rcc", "assets/client_cert.pem", "assets/client_key.pem", "lib/arm64-v8a/libQt5Core_arm64-v8a.so"):
            with self.subTest(entry=path):
                data = self.entries[path]
                self.entries[path] = b""
                result = self.run_verifier()
                self.entries[path] = data
                self.assertEqual(result.returncode, 1)
                self.assertIn(f"Empty required APK entry: {path}", result.stderr)

    def test_missing_client_key(self):
        del self.entries["assets/client_key.pem"]
        result = self.run_verifier()
        self.assertEqual(result.returncode, 1)
        self.assertIn("Missing required APK entry: assets/client_key.pem", result.stderr)

    def test_empty_additional_native_library(self):
        self.entries["lib/arm64-v8a/libqml_plugin.so"] = b""
        result = self.run_verifier()
        self.assertEqual(result.returncode, 1)
        self.assertIn("Empty native library: lib/arm64-v8a/libqml_plugin.so", result.stderr)

    def test_invalid_zip(self):
        self.apk.write_bytes(b"not an APK")
        result = subprocess.run(
            [sys.executable, str(SCRIPT), str(self.apk)], capture_output=True, text=True, check=False,
        )
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, "")
        self.assertIn("Cannot read APK", result.stderr)


if __name__ == "__main__":
    unittest.main()

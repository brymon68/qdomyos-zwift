#!/usr/bin/env python3
"""Check final manifest identity using synthetic AAPT XML tree dumps."""

import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


SCRIPT = Path(__file__).with_name("verify_manifest.py")
ORIGINAL_PACKAGE = "org.cagnulen.qdomyoszwift"
TEST_PACKAGE = ORIGINAL_PACKAGE + ".x22ibaseline"
TREE = '''N: android=http://schemas.android.com/apk/res/android
  E: manifest (line=1)
    A: package="org.cagnulen.qdomyoszwift.x22ibaseline" (Raw: "org.cagnulen.qdomyoszwift.x22ibaseline")
    E: application (line=10)
      A: android:name(0x01010003)="org.qtproject.qt5.android.bindings.QtApplication" (Raw: "org.qtproject.qt5.android.bindings.QtApplication")
      A: android:label(0x01010001)="X22i Test" (Raw: "X22i Test")
      E: activity (line=11)
        A: android:name(0x01010003)="org.cagnulen.qdomyoszwift.CustomQtActivity" (Raw: "org.cagnulen.qdomyoszwift.CustomQtActivity")
        A: android:label(0x01010001)="X22i Test" (Raw: "X22i Test")
        E: intent-filter (line=12)
          E: action (line=13)
            A: android:name(0x01010003)="android.intent.action.MAIN" (Raw: "android.intent.action.MAIN")
          E: category (line=14)
            A: android:name(0x01010003)="android.intent.category.LAUNCHER" (Raw: "android.intent.category.LAUNCHER")
      E: service (line=20)
        A: android:name(0x01010003)="org.cagnulen.qdomyoszwift.ForegroundService" (Raw: "org.cagnulen.qdomyoszwift.ForegroundService")
      E: activity-alias (line=21)
        A: android:name(0x01010003)="org.cagnulen.qdomyoszwift.ViewPermissionUsageActivity" (Raw: "org.cagnulen.qdomyoszwift.ViewPermissionUsageActivity")
        A: android:targetActivity(0x01010202)="org.cagnulen.qdomyoszwift.HealthConnectPermissionActivity" (Raw: "org.cagnulen.qdomyoszwift.HealthConnectPermissionActivity")
      E: receiver (line=22)
        A: android:name(0x01010003)="org.cagnulen.qdomyoszwift.MediaButtonReceiver" (Raw: "org.cagnulen.qdomyoszwift.MediaButtonReceiver")
'''
PROVIDER = '''      E: provider (line=30)
        A: android:name(0x01010003)="androidx.startup.InitializationProvider" (Raw: "androidx.startup.InitializationProvider")
        A: android:authorities(0x01010018)="org.cagnulen.qdomyoszwift.x22ibaseline.androidx-startup" (Raw: "org.cagnulen.qdomyoszwift.x22ibaseline.androidx-startup")
'''


class VerifyManifestTests(unittest.TestCase):
    def run_verifier(self, tree, package=TEST_PACKAGE, label="X22i Test"):
        with tempfile.TemporaryDirectory(prefix="x22i-manifest-test-") as directory:
            path = Path(directory) / "manifest-tree.txt"
            path.write_text(tree)
            result = subprocess.run(
                [sys.executable, str(SCRIPT), str(path), package, label],
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(path.read_text(), tree)
        return result

    def assert_rejected(self, tree, message):
        result = self.run_verifier(tree)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, "")
        self.assertIn(message, result.stderr)

    def test_test_package_preserves_original_component_namespace(self):
        result = self.run_verifier(TREE + PROVIDER)
        self.assertEqual(result.returncode, 0, result.stderr)
        report = json.loads(result.stdout)
        self.assertEqual(report["package"], TEST_PACKAGE)
        self.assertEqual(report["launcher_activity"], ORIGINAL_PACKAGE + ".CustomQtActivity")
        self.assertEqual(report["provider_authorities"], [TEST_PACKAGE + ".androidx-startup"])
        self.assertEqual(len(report["components"]), 5)

    def test_no_providers_is_valid(self):
        result = self.run_verifier(TREE)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(json.loads(result.stdout)["provider_authorities"], [])

    def test_baseline_labels(self):
        tree = TREE.replace(TEST_PACKAGE, ORIGINAL_PACKAGE).replace('"X22i Test"', '"QZ"')
        tree = tree.replace('      A: android:label(0x01010001)="QZ" (Raw: "QZ")', '      A: android:label(0x01010001)="qdomyos-zwift" (Raw: "qdomyos-zwift")', 1)
        result = self.run_verifier(tree, ORIGINAL_PACKAGE, "QZ")
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_wrong_package(self):
        self.assert_rejected(TREE.replace(TEST_PACKAGE, ORIGINAL_PACKAGE), "Expected package")

    def test_wrong_launcher_label(self):
        tree = TREE.replace('        A: android:label(0x01010001)="X22i Test"', '        A: android:label(0x01010001)="QZ"')
        self.assert_rejected(tree, "Expected launcher label")

    def test_wrong_application_label(self):
        tree = TREE.replace('      A: android:label(0x01010001)="X22i Test"', '      A: android:label(0x01010001)="qdomyos-zwift"', 1)
        self.assert_rejected(tree, "Expected application label")

    def test_wrong_application_class(self):
        self.assert_rejected(TREE.replace("org.qtproject.qt5.android.bindings.QtApplication", TEST_PACKAGE + ".QtApplication"), "Expected application class")

    def test_component_class_rewritten_to_test_id(self):
        self.assert_rejected(TREE.replace(ORIGINAL_PACKAGE + ".ForegroundService", TEST_PACKAGE + ".ForegroundService"), "incorrectly uses test application ID")

    def test_conflicting_provider_authority(self):
        provider = PROVIDER.replace(TEST_PACKAGE + ".androidx-startup", ORIGINAL_PACKAGE + ".androidx-startup")
        self.assert_rejected(TREE + provider, "Provider authority must belong")

    def test_duplicate_provider_authority(self):
        self.assert_rejected(TREE + PROVIDER + PROVIDER, "Duplicate provider authority")

    def test_semicolon_separated_authorities_are_checked(self):
        provider = PROVIDER.replace(TEST_PACKAGE + ".androidx-startup", TEST_PACKAGE + ".first;" + ORIGINAL_PACKAGE + ".second")
        self.assert_rejected(TREE + provider, "Provider authority must belong")

    def test_relative_component_class(self):
        for name in (".ForegroundService", "ForegroundService"):
            with self.subTest(name=name):
                self.assert_rejected(TREE.replace(ORIGINAL_PACKAGE + ".ForegroundService", name), "must be fully qualified")

    def test_alias_target_keeps_original_namespace(self):
        self.assert_rejected(TREE.replace(ORIGINAL_PACKAGE + ".HealthConnectPermissionActivity", "other.package.HealthConnectPermissionActivity"), "alias target must retain original Java namespace")

    def test_relative_alias_target(self):
        self.assert_rejected(TREE.replace(ORIGINAL_PACKAGE + ".HealthConnectPermissionActivity", ".HealthConnectPermissionActivity"), "must be fully qualified")

    def test_launcher_action_and_category_must_share_filter(self):
        tree = TREE.replace('          E: category (line=14)', '        E: intent-filter (line=14)\n          E: category (line=14)')
        self.assert_rejected(tree, "Expected one MAIN/LAUNCHER activity")

    def test_malformed_or_empty_input(self):
        for tree in ("", "not a manifest", "A: package=\"example.app\""):
            with self.subTest(tree=tree):
                result = self.run_verifier(tree)
                self.assertEqual(result.returncode, 1)
                self.assertEqual(result.stdout, "")


if __name__ == "__main__":
    unittest.main()

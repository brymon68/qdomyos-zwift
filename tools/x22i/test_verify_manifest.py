#!/usr/bin/env python3
"""Check final manifest identity using synthetic AAPT XML tree dumps."""

import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


SCRIPT = Path(__file__).with_name("verify_manifest.py")
APP_PACKAGE = "io.github.brymon68.x22i"
OTHER_PACKAGE = "example.other.application"
TREE = '''N: android=http://schemas.android.com/apk/res/android
  E: manifest (line=1)
    A: package="io.github.brymon68.x22i" (Raw: "io.github.brymon68.x22i")
    E: application (line=10)
      A: android:name(0x01010003)="org.qtproject.qt5.android.bindings.QtApplication" (Raw: "org.qtproject.qt5.android.bindings.QtApplication")
      A: android:label(0x01010001)="X22i" (Raw: "X22i")
      E: activity (line=11)
        A: android:name(0x01010003)="io.github.brymon68.x22i.CustomQtActivity" (Raw: "io.github.brymon68.x22i.CustomQtActivity")
        A: android:label(0x01010001)="X22i" (Raw: "X22i")
        E: meta-data (line=12)
          A: android:name(0x01010003)="android.app.lib_name" (Raw: "android.app.lib_name")
          A: android:value(0x01010024)="x22i" (Raw: "x22i")
        E: intent-filter (line=13)
          E: action (line=14)
            A: android:name(0x01010003)="android.intent.action.MAIN" (Raw: "android.intent.action.MAIN")
          E: category (line=15)
            A: android:name(0x01010003)="android.intent.category.LAUNCHER" (Raw: "android.intent.category.LAUNCHER")
      E: service (line=20)
        A: android:name(0x01010003)="io.github.brymon68.x22i.ForegroundService" (Raw: "io.github.brymon68.x22i.ForegroundService")
      E: activity-alias (line=21)
        A: android:name(0x01010003)="io.github.brymon68.x22i.SyntheticAliasActivity" (Raw: "io.github.brymon68.x22i.SyntheticAliasActivity")
        A: android:targetActivity(0x01010202)="io.github.brymon68.x22i.SyntheticAliasTargetActivity" (Raw: "io.github.brymon68.x22i.SyntheticAliasTargetActivity")
      E: receiver (line=22)
        A: android:name(0x01010003)="io.github.brymon68.x22i.SyntheticReceiver" (Raw: "io.github.brymon68.x22i.SyntheticReceiver")
'''
PROVIDER = '''      E: provider (line=30)
        A: android:name(0x01010003)="androidx.startup.InitializationProvider" (Raw: "androidx.startup.InitializationProvider")
        A: android:authorities(0x01010018)="io.github.brymon68.x22i.androidx-startup" (Raw: "io.github.brymon68.x22i.androidx-startup")
'''


class VerifyManifestTests(unittest.TestCase):
    def run_verifier(self, tree):
        with tempfile.TemporaryDirectory(prefix="x22i-manifest-test-") as directory:
            path = Path(directory) / "manifest-tree.txt"
            path.write_text(tree)
            result = subprocess.run(
                [sys.executable, str(SCRIPT), str(path)],
                capture_output=True, text=True, check=False,
            )
            self.assertEqual(path.read_text(), tree)
        return result

    def assert_rejected(self, tree, message):
        result = self.run_verifier(tree)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, "")
        self.assertIn(message, result.stderr)

    def test_standalone_identity_and_owned_component_namespace(self):
        result = self.run_verifier(TREE + PROVIDER)
        self.assertEqual(result.returncode, 0, result.stderr)
        report = json.loads(result.stdout)
        self.assertEqual(report["package"], APP_PACKAGE)
        self.assertEqual(report["application_label"], "X22i")
        self.assertEqual(report["launcher_label"], "X22i")
        self.assertEqual(report["launcher_activity"], APP_PACKAGE + ".CustomQtActivity")
        self.assertEqual(report["provider_authorities"], [APP_PACKAGE + ".androidx-startup"])
        self.assertEqual(len(report["components"]), 5)

    def test_no_providers_is_valid(self):
        result = self.run_verifier(TREE)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(json.loads(result.stdout)["provider_authorities"], [])

    def test_qt_and_androidx_components_are_allowed(self):
        for name in ("org.qtproject.qt5.android.bindings.QtService", "androidx.core.app.SyntheticService"):
            with self.subTest(name=name):
                result = self.run_verifier(TREE.replace(APP_PACKAGE + ".ForegroundService", name))
                self.assertEqual(result.returncode, 0, result.stderr)

    def test_wrong_package(self):
        self.assert_rejected(TREE.replace('package="' + APP_PACKAGE + '"', 'package="' + OTHER_PACKAGE + '"'), "Expected package")

    def test_wrong_launcher_label(self):
        tree = TREE.replace('        A: android:label(0x01010001)="X22i"', '        A: android:label(0x01010001)="Other app"')
        self.assert_rejected(tree, "Expected launcher label")

    def test_wrong_application_label(self):
        tree = TREE.replace('      A: android:label(0x01010001)="X22i"', '      A: android:label(0x01010001)="Other app"', 1)
        self.assert_rejected(tree, "Expected application label")

    def test_wrong_application_class(self):
        self.assert_rejected(TREE.replace("org.qtproject.qt5.android.bindings.QtApplication", APP_PACKAGE + ".QtApplication"), "Expected application class")

    def test_wrong_launcher_activity(self):
        self.assert_rejected(TREE.replace(APP_PACKAGE + ".CustomQtActivity", APP_PACKAGE + ".OtherActivity"), "Expected launcher activity")

    def test_wrong_native_library(self):
        self.assert_rejected(TREE.replace('android:value(0x01010024)="x22i"', 'android:value(0x01010024)="other-app"'), "Expected launcher native library x22i")

    def test_missing_native_library(self):
        self.assert_rejected(TREE.replace('android.app.lib_name', 'android.app.other'), "Expected launcher native library x22i")

    def test_wrong_component_namespace(self):
        for package in (OTHER_PACKAGE, APP_PACKAGE + "other", "androidxother", "org.qtproject.qt5other"):
            with self.subTest(package=package):
                self.assert_rejected(TREE.replace(APP_PACKAGE + ".ForegroundService", package + ".ForegroundService"), "Unexpected component namespace")

    def test_conflicting_provider_authority(self):
        provider = PROVIDER.replace(APP_PACKAGE + ".androidx-startup", OTHER_PACKAGE + ".androidx-startup")
        self.assert_rejected(TREE + provider, "Provider authority must belong")

    def test_duplicate_provider_authority(self):
        self.assert_rejected(TREE + PROVIDER + PROVIDER, "Duplicate provider authority")

    def test_semicolon_separated_authorities_are_checked(self):
        provider = PROVIDER.replace(APP_PACKAGE + ".androidx-startup", APP_PACKAGE + ".first;" + OTHER_PACKAGE + ".second")
        self.assert_rejected(TREE + provider, "Provider authority must belong")

    def test_relative_component_class(self):
        for name in (".ForegroundService", "ForegroundService"):
            with self.subTest(name=name):
                self.assert_rejected(TREE.replace(APP_PACKAGE + ".ForegroundService", name), "must be fully qualified")

    def test_alias_target_requires_owned_namespace(self):
        self.assert_rejected(TREE.replace(APP_PACKAGE + ".SyntheticAliasTargetActivity", OTHER_PACKAGE + ".SyntheticAliasTargetActivity"), "Activity alias target must belong")

    def test_relative_alias_target(self):
        self.assert_rejected(TREE.replace(APP_PACKAGE + ".SyntheticAliasTargetActivity", ".SyntheticAliasTargetActivity"), "must be fully qualified")

    def test_launcher_action_and_category_must_share_filter(self):
        tree = TREE.replace('          E: category (line=15)', '        E: intent-filter (line=15)\n          E: category (line=15)')
        self.assert_rejected(tree, "Expected one MAIN/LAUNCHER activity")

    def test_malformed_or_empty_input(self):
        for tree in ("", "not a manifest", "A: package=\"example.app\""):
            with self.subTest(tree=tree):
                result = self.run_verifier(tree)
                self.assertEqual(result.returncode, 1)
                self.assertEqual(result.stdout, "")


if __name__ == "__main__":
    unittest.main()

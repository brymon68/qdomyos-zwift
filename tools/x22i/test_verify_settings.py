#!/usr/bin/env python3
"""CLI checks using synthetic settings; never reads a real user configuration."""

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


SCRIPT = Path(__file__).with_name("verify_settings.py")


class VerifySettingsTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="private-settings-test-")
        self.addCleanup(self.directory.cleanup)
        self.backup = Path(self.directory.name) / "private-backup.ini"
        self.candidate = Path(self.directory.name) / "private-candidate.ini"

    def run_verifier(self, source, candidate):
        for path, contents in ((self.backup, source), (self.candidate, candidate)):
            if contents is not None:
                path.write_bytes(contents if isinstance(contents, bytes) else contents.encode("utf-8"))
        result = subprocess.run(
            [sys.executable, str(SCRIPT), str(self.backup), str(self.candidate)],
            capture_output=True,
            text=True,
            check=False,
        )
        for path, contents in ((self.backup, source), (self.candidate, candidate)):
            if contents is not None:
                expected = contents if isinstance(contents, bytes) else contents.encode("utf-8")
                self.assertEqual(path.read_bytes(), expected)
        self.assertNotIn(self.directory.name, result.stdout + result.stderr)
        self.assertNotIn(self.backup.name, result.stdout + result.stderr)
        self.assertNotIn(self.candidate.name, result.stdout + result.stderr)
        return result

    def assert_invalid(self, result):
        self.assertEqual(result.returncode, 2)
        self.assertEqual(result.stdout, "")
        self.assertEqual(
            result.stderr,
            "Cannot compare settings: an input is unreadable, invalid, empty, or uses DEFAULT settings.\n",
        )

    def test_preserved_settings_allow_new_defaults(self):
        source = "[General]\nprivateKey=private-value\n[Group]\nnested\\key=@ByteArray(value)\n"
        candidate = source + "newKey=42\n[NewGroup]\nnewDefault=true\n"
        result = self.run_verifier(source, candidate)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stderr, "")
        self.assertEqual(
            result.stdout,
            "Source settings: 2; candidate settings: 4; preserved: 2; missing: 0; changed: 0; added: 2.\n",
        )

    def test_missing_and_changed_settings_report_only_counts(self):
        result = self.run_verifier(
            "[General]\nprivateKey=secret-old\nmissingPrivateKey=secret-missing\nkeep=ok\n",
            "[General]\nprivateKey=secret-new\nkeep=ok\nnewPrivateKey=secret-added\n",
        )
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stderr, "")
        self.assertEqual(
            result.stdout,
            "Source settings: 3; candidate settings: 3; preserved: 1; missing: 1; changed: 1; added: 1.\n",
        )

    def test_setting_names_are_case_sensitive(self):
        result = self.run_verifier(
            "[General]\nPrivateKey=first\nprivatekey=second\n",
            "[General]\nprivatekey=second\n",
        )
        self.assertEqual(result.returncode, 1)
        self.assertIn("preserved: 1; missing: 1; changed: 0; added: 0.", result.stdout)

    def test_values_are_literal_and_colons_are_part_of_keys(self):
        source = "[General]\nprivate:key=50% %(undefined)s #literal; text\n"
        result = self.run_verifier(source, source)
        self.assertEqual(result.returncode, 0)
        self.assertIn("preserved: 1; missing: 0; changed: 0; added: 0.", result.stdout)
        result = self.run_verifier(source, source.replace("50%", "51%"))
        self.assertEqual(result.returncode, 1)
        self.assertIn("changed: 1", result.stdout)

    def test_malformed_inputs_do_not_reveal_parser_details(self):
        valid = "[General]\nkey=value\n"
        for malformed in (
            "secret-content-before-section\n",
            "[General]\nprivateSecretLineWithoutEquals\n",
            "[General]\nprivateKey=secret-one\nprivateKey=secret-two\n",
            "[General]\nkey=value\n[General]\nprivateKey=secret\n",
        ):
            with self.subTest():
                self.assert_invalid(self.run_verifier(malformed, valid))
                self.assert_invalid(self.run_verifier(valid, malformed))

    def test_empty_inputs_are_invalid(self):
        valid = "[General]\nkey=value\n"
        for empty in ("", "# only a comment\n", "[General]\n"):
            with self.subTest():
                self.assert_invalid(self.run_verifier(empty, valid))
                self.assert_invalid(self.run_verifier(valid, empty))

    def test_default_inheritance_is_rejected(self):
        valid = "[General]\nprivateKey=secret\n"
        inherited = "[DEFAULT]\nprivateKey=secret\n[General]\notherKey=value\n"
        self.assert_invalid(self.run_verifier(valid, inherited))
        self.assert_invalid(self.run_verifier(inherited, valid))

    def test_missing_input_does_not_reveal_path(self):
        self.assert_invalid(self.run_verifier(None, "[General]\nkey=value\n"))

    def test_invalid_utf8_does_not_reveal_content(self):
        self.assert_invalid(self.run_verifier("[General]\nkey=value\n", b"[General]\nprivateKey=secret\xff\n"))

    def test_utf8_bom_is_accepted(self):
        result = self.run_verifier("\ufeff[General]\nkey=value\n", "[General]\nkey=value\n")
        self.assertEqual(result.returncode, 0)

    def test_invalid_arguments_do_not_echo_private_input(self):
        result = subprocess.run(
            [sys.executable, str(SCRIPT), "private-path", "private-other-path", "private-extra"],
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertEqual(result.returncode, 2)
        self.assertEqual(result.stdout, "")
        self.assertEqual(
            result.stderr,
            "Usage: python3 tools/x22i/verify_settings.py BACKUP_INI CANDIDATE_INI\n",
        )


if __name__ == "__main__":
    unittest.main()

#!/usr/bin/env python3
"""Check that a migrated Qt INI file preserves every backed-up setting.

Values are compared as INI text, without interpolation or Qt type conversion.
Only counts are reported; neither input file is modified.
"""

import configparser
import sys


USAGE = "Usage: python3 tools/x22i/verify_settings.py BACKUP_INI CANDIDATE_INI"
INVALID_INPUT = "Cannot compare settings: an input is unreadable, invalid, empty, or uses DEFAULT settings."


def read_settings(path):
    parser = configparser.ConfigParser(interpolation=None, delimiters=("=",))
    parser.optionxform = str
    with open(path, encoding="utf-8-sig") as source:
        parser.read_file(source)

    # ConfigParser otherwise adds DEFAULT options to every section.
    if parser.defaults():
        raise ValueError("DEFAULT settings are unsupported")

    settings = {
        (section, key): value
        for section in parser.sections()
        for key, value in parser.items(section, raw=True)
    }
    if not settings:
        raise ValueError("No settings found")
    return settings


def main(argv=None):
    args = sys.argv[1:] if argv is None else argv
    if args in (["--help"], ["-h"]):
        print(USAGE)
        print("Compare settings without displaying names, values, or input paths.")
        print("Exit codes: 0 preserved; 1 missing or changed; 2 invalid input.")
        return 0
    if len(args) != 2:
        print(USAGE, file=sys.stderr)
        return 2

    try:
        backup = read_settings(args[0])
        candidate = read_settings(args[1])
    except (OSError, UnicodeError, configparser.Error, ValueError):
        # Parser and IO exceptions may contain credentials, keys, or private paths.
        print(INVALID_INPUT, file=sys.stderr)
        return 2

    missing = sum(key not in candidate for key in backup)
    changed = sum(key in candidate and candidate[key] != value for key, value in backup.items())
    added = sum(key not in backup for key in candidate)
    preserved = len(backup) - missing - changed
    print(
        f"Source settings: {len(backup)}; candidate settings: {len(candidate)}; "
        f"preserved: {preserved}; missing: {missing}; changed: {changed}; added: {added}."
    )
    return 1 if missing or changed else 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Check final AAPT manifest component names and provider authorities.

Input is `aapt dump xmltree APK AndroidManifest.xml` output. This checks package,
component and authority declarations; it does not validate runtime behavior.
"""

import argparse
import json
from pathlib import Path
import sys


APP_PACKAGE = "io.github.brymon68.x22i"
APP_LABEL = "X22i"
MAIN_ACTIVITY = APP_PACKAGE + ".CustomQtActivity"
QT_APPLICATION = "org.qtproject.qt5.android.bindings.QtApplication"
COMPONENT_TYPES = ("activity", "service", "receiver", "provider", "activity-alias")


def parse_tree(text):
    # AAPT XMLNode.cpp prints E:/A: records with nesting expressed by indentation.
    roots = []
    stack = []
    for line in text.splitlines():
        record = line.lstrip()
        indent = len(line) - len(record)
        if record.startswith("E: "):
            name = record[3:].split(" ", 1)[0]
            while stack and stack[-1][0] >= indent:
                stack.pop()
            node = {"tag": name, "attributes": {}, "children": []}
            (stack[-1][1]["children"] if stack else roots).append(node)
            stack.append((indent, node))
        elif record.startswith("A: "):
            if not stack or indent <= stack[-1][0]:
                raise ValueError("Invalid AAPT attribute nesting")
            key, separator, value = record[3:].partition("=")
            if not separator:
                raise ValueError("Invalid AAPT attribute record")
            key = key.split("(", 1)[0].strip()
            value = value.strip()
            if value.startswith('"'):
                value, closing_quote, _ = value[1:].partition('"')
                if not closing_quote:
                    raise ValueError("Unterminated AAPT string attribute")
            attributes = stack[-1][1]["attributes"]
            if key in attributes:
                raise ValueError(f"Duplicate AAPT attribute: {key}")
            attributes[key] = value
    if len(roots) != 1 or roots[0]["tag"] != "manifest":
        raise ValueError("Expected one manifest element in AAPT output")
    return roots[0]


def verify_manifest(text):
    manifest = parse_tree(text)
    package = manifest["attributes"].get("package")
    errors = []
    if package != APP_PACKAGE:
        errors.append(f"Expected package {APP_PACKAGE}; found {package!r}")
    applications = [node for node in manifest["children"] if node["tag"] == "application"]
    if len(applications) != 1:
        raise ValueError("Expected one application element")
    application = applications[0]
    application_name = application["attributes"].get("android:name")
    application_label = application["attributes"].get("android:label")
    if application_name != QT_APPLICATION:
        errors.append(f"Expected application class {QT_APPLICATION}; found {application_name!r}")
    if application_label != APP_LABEL:
        errors.append(f"Expected application label {APP_LABEL!r}; found {application_label!r}")

    components = []
    launchers = []
    authorities = set()
    for node in application["children"]:
        kind = node["tag"]
        if kind not in COMPONENT_TYPES:
            continue
        attributes = node["attributes"]
        name = attributes.get("android:name", "")
        component = {"type": kind, "name": name}
        class_names = [("name", name)]
        if kind == "activity-alias":
            target = attributes.get("android:targetActivity", "")
            component["targetActivity"] = target
            class_names.append(("targetActivity", target))
            if not target.startswith(APP_PACKAGE + "."):
                errors.append(f"Activity alias target must belong to {APP_PACKAGE}: {target!r}")
        for attribute, class_name in class_names:
            parts = class_name.split(".")
            if len(parts) < 2 or not all(part.replace("$", "_").isidentifier() for part in parts):
                errors.append(f"Component {kind} {attribute} must be fully qualified: {class_name!r}")
            if not class_name.startswith((APP_PACKAGE + ".", "org.qtproject.qt5.", "androidx.")):
                errors.append(f"Unexpected component namespace: {class_name!r}")

        if kind == "provider":
            provider_authorities = attributes.get("android:authorities", "").split(";")
            for authority in provider_authorities:
                authority = authority.strip()
                if not authority or not (authority == APP_PACKAGE or authority.startswith(APP_PACKAGE + ".")):
                    errors.append(f"Provider authority must belong to {APP_PACKAGE}: {authority!r}")
                if authority in authorities:
                    errors.append(f"Duplicate provider authority: {authority!r}")
                authorities.add(authority)

        for intent_filter in node["children"]:
            if intent_filter["tag"] != "intent-filter":
                continue
            actions = {child["attributes"].get("android:name") for child in intent_filter["children"] if child["tag"] == "action"}
            categories = {child["attributes"].get("android:name") for child in intent_filter["children"] if child["tag"] == "category"}
            if "android.intent.action.MAIN" in actions and "android.intent.category.LAUNCHER" in categories:
                launchers.append(node)
        components.append(component)

    if len(launchers) != 1:
        errors.append(f"Expected one MAIN/LAUNCHER activity; found {len(launchers)}")
    else:
        launcher = launchers[0]
        if launcher["tag"] != "activity" or launcher["attributes"].get("android:name") != MAIN_ACTIVITY:
            errors.append(f"Expected launcher activity {MAIN_ACTIVITY}")
        label = launcher["attributes"].get("android:label")
        if label != APP_LABEL:
            errors.append(f"Expected launcher label {APP_LABEL!r}; found {label!r}")
        libraries = [
            child["attributes"].get("android:value")
            for child in launcher["children"]
            if child["tag"] == "meta-data" and child["attributes"].get("android:name") == "android.app.lib_name"
        ]
        if libraries != ["x22i"]:
            errors.append(f"Expected launcher native library x22i; found {libraries!r}")
    if errors:
        raise ValueError("\n".join(errors))
    return {
        "package": package,
        "application": application_name,
        "application_label": application_label,
        "launcher_activity": MAIN_ACTIVITY,
        "launcher_label": APP_LABEL,
        "components": components,
        "provider_authorities": sorted(authorities),
    }


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest_tree", type=Path)
    args = parser.parse_args(argv)
    try:
        result = verify_manifest(args.manifest_tree.read_text())
    except (OSError, UnicodeError):
        print("Cannot read the AAPT manifest tree.", file=sys.stderr)
        return 1
    except ValueError as error:
        print(error, file=sys.stderr)
        return 1
    print(json.dumps(result, sort_keys=True))
    return 0


if __name__ == "__main__":
    sys.exit(main())

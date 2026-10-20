# X22i settings catalog

`src/settings-catalog.json` is the manually maintained catalog of persistent settings declared in `src/settings.qml`. The incline lookup controls are in `src/settings-treadmill-inclination-override.qml`.

Update the catalog in the same change whenever a setting is added, removed, or renamed. Add new QML properties and catalog entries at the end of their lists. Keep `settingCount` equal to the number of unique entries.

Each entry records the persistent key, QML type, default value or expression, label, description, and control metadata. Keep these values aligned with the QML declaration. `pages` describes retained settings navigation; `virtualSettings` is currently empty.

Renaming a saved setting requires an explicit migration decision. Existing saved configuration values take precedence over defaults supplied by `src/x22i_defaults.h`.

Validate the catalog against the branch's base commit:

```sh
python3 tools/check_settings_catalog.py BASE_COMMIT
python3 tools/check_settings_property_order.py BASE_COMMIT
```

The X22i build and configuration guide is in [x22i-baseline.md](x22i-baseline.md).

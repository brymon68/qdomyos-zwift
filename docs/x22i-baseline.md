# X22i source and Android build

This fork owns the application source in `src/` and builds it directly. GitHub Actions checks out this repository once, including its pinned dependency submodules. Application source is no longer downloaded from a separate upstream checkout, and the three feature-removal patches have been deleted.

The source was aligned with the already tested NordicTrack gRPC revision `8b78e9464c55572b9eaf816128bac905c7150a26` in commit `1f48d493`. This alignment is a separate commit because the fork previously contained a different application revision. Subsequent cleanup uses ordinary source edits and deletions. The upstream revision in [`baseline.json`](../tools/x22i/baseline.json) records historical provenance; the build uses the current fork commit.

## Current scope

- Fan UI/settings, shortcuts, automatic rules, workout fields, MQTT/OSC/template commands, generic API, three external drivers, and fan-only JNI/Java/protobuf operations are deleted. Eight unique settings were removed; the catalog covers all 961 remaining QML settings and C++ has 967 settings. Existing obsolete saved keys can remain inert. Shared key-code enum values and the icon reused by heat-zone tiles are retained.
- Purchasing UI, store/product objects, native and Android purchasing classes, billing dependency, and billing permission are deleted.
- Zwift authentication and terrain polling, local iFIT gRPC, required TLS/protobuf assets, incline calibration and limits, manual controls, Bluetooth, DIRCON, and the missing-key settings preset are retained.

Broader equipment/integration pruning, package renaming, and dashboard simplification remain in [issue #1](https://github.com/brymon68/qdomyos-zwift/issues/1). This increment does not claim an X22i-only dependency graph.

## Build from this fork

The supported target is Android ARM64 (`arm64-v8a`), version **2.21.5 / 1403**. Install Qt Android **5.15.0** with Charts and Network Auth, Java **17**, Android platform **35**, build tools **35.0.0**, NDK **21.4.7075529**, and host tools `git`, `make`, `python3`, and `rsync`. Gradle **8.13** is supplied by the repository. Use a dedicated Qt installation: the recipe installs the fork's Qt JARs and Qt HTTP server into it.

On the configured workstation, activate the existing toolchain with:

```sh
source "$HOME/.local/share/qz-x22i-toolchain/env.sh"
```

Otherwise set `QT_ANDROID_DIR`, `ANDROID_SDK_ROOT`, and `JAVA_HOME` to your installed toolchain. From a checkout of **this fork**, initialize dependencies and run:

```sh
git submodule update --init --recursive
python3 -m unittest discover -s tools/x22i -p 'test_*.py' -v
# Configure X22I_SIGNING_STORE_FILE and X22I_SIGNING_STORE_PASSWORD privately.
bash tools/x22i/build_baseline.sh /LOCAL/NEW-BUILD-DIRECTORY
```

The single argument is a new output directory, which must not already exist. Commit source changes before building: the script requires a clean checkout and exact dependency revisions so artifact provenance identifies its full source. It never installs or launches an APK. Private `secret.h`/`secrets.h` headers are rejected. The Qt HTTP parser compatibility overlay is applied to an output-directory copy of the dependency, keeping the Git checkout clean.

`X22I_VARIANT=test` is the default. It builds **X22i Test**, package `org.cagnulen.qdomyoszwift.x22ibaseline`, using the persistent private PKCS12 key with alias `x22i-test`. Set `X22I_SIGNING_STORE_FILE` to its path and load `X22I_SIGNING_STORE_PASSWORD` through your local secret mechanism; never put the password in arguments, logs, or committed files. The expected signing certificate SHA-256 is `38a1ec468bd45b37b20e7477613b9951aae24c3171673a5a6186abe7f3c7167d`.

The compatibility variant `X22I_VARIANT=baseline` uses the original package `org.cagnulen.qdomyoszwift`, QZ launcher label, and ordinary debug signing. It omits the X22i settings preset. It contains the same application source. Its signer does not match the original installed QZ and it must not be treated as an in-place replacement.

### GitHub Actions

The [X22i workflow](../.github/workflows/x22i-baseline.yml) is the application's build workflow. It runs on pushes to `master` and `codex/**` branches and on manual dispatch. The inherited platform/release workflows have been deleted. Test signing uses repository secrets `X22I_TEST_KEYSTORE_BASE64` and `X22I_TEST_KEYSTORE_PASSWORD`; CI removes the temporary key afterward.

```sh
gh workflow run x22i-baseline.yml --repo brymon68/qdomyos-zwift --ref "$(git branch --show-current)" -f variant=test
gh run download RUN_ID --repo brymon68/qdomyos-zwift --name x22i-test-arm64 --dir /LOCAL/ARTIFACTS
```

CI runs the Python checks and a native QtCore test of settings preservation before building. Never upload personal settings, credentials, or backups to CI.

### Docker toolchain

The optional image contains the toolchain only. With rootless Docker:

```sh
docker build --platform linux/amd64 -t x22i-toolchain docker/x22i
docker run --rm --platform linux/amd64 \
  --mount type=bind,src=/LOCAL/FORK-CHECKOUT,dst=/source \
  --mount type=bind,src=/LOCAL/BUILD-PARENT,dst=/build \
  --mount type=bind,src=/PRIVATE/x22i-test.p12,dst=/run/secrets/test.p12,readonly \
  --mount type=bind,src=/PRIVATE/keystore-password,dst=/run/secrets/password,readonly \
  -e X22I_SIGNING_STORE_FILE=/run/secrets/test.p12 \
  -e X22I_SIGNING_STORE_PASSWORD_FILE=/run/secrets/password \
  x22i-toolchain
```

Initialize submodules and commit changes in the mounted fork first. `/LOCAL/BUILD-PARENT/output` must not exist; artifacts appear under `output/artifacts`. The source mount is writable for generated translation files, which Git ignores. The Docker image and containerized APK path have not been runtime-validated; the native workstation build is the locally verified path.

## Artifacts and verification

`artifacts/` contains the APK, `build-info.json`, signing report, package badging, merged manifest, GPL license, and (for `test`) the bundled defaults header. `source_revision` and `build_recipe_revision` identify the same fork commit. Provenance also records dependency revisions, toolchain versions, APK hash/size, and removed-feature flags. No source patch is required to reconstruct the application.

The build verifies ARM64 Qt/OpenSSL/C++ payloads, QML resources, gRPC assets, package/component/provider identities, and removed-code markers in the native app and all DEX files. APK signatures are verified separately. These checks establish packaging and source removal; they do not establish runtime connectivity or physical incline behavior.

ANT+ drivers, services, SDKs, and controls have been deleted. The X22i no longer reads or estimates watts, offers power targets or FTP zones, or records power-derived metrics. Bluetooth/DIRCON treadmill payloads omit watts and reject target-power commands; speed and terrain-incline commands remain supported.

## Settings and installation

[`src/x22i_defaults.h`](../src/x22i_defaults.h) supplies 67 reviewed nonsecret settings in test builds before normal settings reads and QML startup. It only writes missing keys. Saved custom values, explicit false/zero/empty values, and private credentials take precedence. It preserves miles, local iFIT, Bluetooth/DIRCON, Zwift auto-incline, gain/calibration, the 0.5% incline step, three-second delay, bounds, and the 31-entry incline lookup table. Reset Settings still uses the inherited defaults.

Upgrade X22i Test with the same package and persistent signer. Do not uninstall to work around an update failure or rotate the signing key casually. Fresh installs need private Zwift authentication; credentials and the profile encryption key are never bundled.

For migration, preserve the complete native QSettings configuration, including its encryption key. A `.qzs` profile export is insufficient: it omits that key and its import skips `peloton_refreshtoken`. Use [`verify_settings.py`](../tools/x22i/verify_settings.py) privately to compare source and candidate configurations before launch; all original settings must be present and unchanged. Startup bookkeeping changes must be reviewed separately. Keep backups, raw configuration, device addresses, and unrestricted startup logs outside Git and artifact uploads.

Before a later installation or launch, obtain current confirmation that the owner is present and the belt is stopped. Back up the installed APK/configuration, verify the candidate's signer and actual data directories, and run only one QZ app. Both packages use the same fitness connection, and shared `Documents/QZ` storage may be selected by configuration. A package rename requires explicit migration; it is not included here.

## Recorded device and build results

The **direct-source build** from fork commit `b708868f34bf9ea43527b25310a3b2c9db39a26d` passed on October 20, 2026. It built native C++, Java, and protobuf from this checkout, then passed ARM64 payload, manifest, removed-feature, and v1/v2 signature checks. The persistent signer, defaults header, and GPL license matched. All 40 Python tests, modified QML syntax checks, and catalog count checks passed. The checkout remained clean after building.

The resulting APK is **50,665,826 bytes**, SHA-256 **`4615f4bb88f754ef5852739be7c47fba20385a37f70a82fa5a3ab4480b1b830f`**. Its source and recipe provenance both identify the fork commit, with no source patch. It has not been installed. A comparison of 1,775 source files with the previous stripped build found no differences beyond the separately reviewed build-variant integration files.

On October 20, 2026, the separate baseline was installed from [CI run 39365509040, attempt 2](https://github.com/brymon68/qdomyos-zwift/actions/runs/39365509040/attempts/2). All **861 saved settings** were migrated with exact readback. The owner then confirmed auto-incline and the overall app were working. The original QZ installation and private rollback backup remain available.

An earlier update was subsequently installed in place with authorization. All **929 saved settings** were preserved before launch; afterward only the launch counter, internal web-server port, and template-ID ordering changed. Its SHA-256 is `a4b5707a1fb99523e53ef52dc00b0091c7d09bd0ec9573eeefec1dc7881d4211`. No new physical incline regression result was claimed.

The prior patch-based stripped build at recipe `a807b591fd2d12844e11da97c657d0d73afe908a` passed a full native/Java/protobuf build, 40 Python tests, QML syntax checks, payload/manifest checks, removed-feature checks, and v1/v2 signatures. Size: **50,665,957 bytes**; SHA-256: `c46229cd57c0302de1e20212a4ac5dc85fdc18b5ee0b198ac32094c6a1b3b46a`. It was not installed. This is the comparison artifact for the direct-source build.

## Retained behavior and limits

Automatic incline remains Zwift authentication/polling and decode → grade gain/offset → `treadmill::changeInclination()` → NordicTrack update → JNI → `InclineService.SetIncline`. The treadmill step applies configured bounds, correction, difficulty, and rounding. Required gRPC 1.63.0/protobuf-lite 3.25.3 generation and shared TLS/session assets remain. No automatic speed or negative-incline behavior was added.

Existing control limitations remain: C++ converts an absolute target to a delta and Java later adds it to its current metric; blocking RPCs share an executor without deadlines; stale metrics and queued commands lack freshness expiry; and initialization/command acknowledgments are incomplete. Legacy UDP/ADB handling remains. These are follow-up concerns, not fixes claimed by this removal. Hardware checks of downhill clamping, stop/pause, restart/reconnect, and the active Mac transport remain separate from build verification.

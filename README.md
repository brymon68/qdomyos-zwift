# X22i Zwift bridge

An Android fork of [QDomyos-Zwift](https://github.com/cagnulein/qdomyos-zwift) for a NordicTrack X22i console, with Zwift running on a Mac. It reads treadmill telemetry through the local iFIT gRPC service and preserves automatic incline following Zwift terrain. iFIT remains required.

The application source lives in this repository's `src/`. The Android build compiles this checkout directly. Further device/integration pruning, a smaller dashboard, and a new package namespace remain work in [issue #1](https://github.com/brymon68/qdomyos-zwift/issues/1).

## Build

Use the **X22i ARM64 builds** GitHub Actions workflow with variant `test`, or build locally with the documented Qt/Android toolchain:

```sh
git submodule update --init --recursive
# Configure the toolchain and private test signing environment first.
bash tools/x22i/build_baseline.sh /LOCAL/NEW-BUILD-DIRECTORY
```

Builds require committed source and produce an APK plus commit, checksum, manifest, and signing records. See [build and configuration instructions](docs/x22i-baseline.md) for toolchain versions, signing setup, and Docker usage.

## Configuration and updates

The current app is **X22i Test**, package `org.cagnulen.qdomyoszwift.x22ibaseline`. Update it in place with the same signing key to preserve its configuration. Do not uninstall to resolve an update failure.

The app supplies missing nonsecret X22i settings, including `localhost`, miles, and the accepted incline calibration. Existing saved values take precedence. A fresh install still needs private Zwift authentication. Preserve both Bluetooth and DIRCON until the active Mac connection is established. If speed connects but incline does not follow terrain, verify the saved Zwift authentication and auto-incline configuration first.

Keep credentials and configuration backups outside Git and build artifacts. The build never installs or launches the app.

## License

Derived from QDomyos-Zwift by Roberto Viola and contributors. The [GPL-3.0 license](LICENSE) and source attribution are retained. This project is not affiliated with NordicTrack, iFIT, or Zwift.

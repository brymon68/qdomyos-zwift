# X22i Zwift bridge

**X22i** is an Android application for a NordicTrack X22i console, with package ID `io.github.brymon68.x22i`. It reads treadmill telemetry through the local iFIT gRPC service and follows Zwift terrain while Zwift runs on a Mac. iFIT remains required.

The app shows speed, incline, pace, distance, elapsed time, and heart rate, with Start/Pause/Stop and manual speed/incline controls. Bluetooth and DIRCON connect it to Zwift. The source in `src/` builds directly from this repository. The app uses US English and contains no other equipment drivers, ANT+, power measurement, user profiles, cloud fitness accounts, or local workout planning, recording, history, and file import/export. Zwift authentication remains for terrain incline.

## Build

Use the **X22i ARM64 builds** GitHub Actions workflow, or build locally with the documented Qt/Android toolchain:

```sh
git submodule update --init --recursive
# Configure the toolchain and private signing environment first.
bash tools/x22i/build_baseline.sh /LOCAL/NEW-BUILD-DIRECTORY
```

The single application builds from `x22i.pro` and produces `artifacts/x22i-arm64.apk` plus commit, checksum, manifest, and signing records. Builds require committed source. See the [build and configuration guide](docs/x22i-baseline.md) for dependencies, signing, CI, and Docker instructions.

## Configuration and migration

The new package has separate Android storage. Its Qt settings identity is organization `brymon68`, application `X22i`. Existing configuration does not transfer automatically from an earlier application identity.

Before the first launch, perform the [private migration procedure](docs/x22i-baseline.md#private-migration-before-first-launch): discover the installed source package and its actual configuration path on the console, preserve its APK and settings backup, copy settings to the verified new location, and compare the readback with `verify_settings.py`. Keep authentication data and backups outside Git and build artifacts.

The app supplies missing nonsecret settings, including miles and incline calibration; saved values take precedence. It connects to local iFIT using `localhost` for TLS verification. Preserve both Bluetooth and DIRCON until the active Mac connection is established. Future updates to `io.github.brymon68.x22i` must use the same persistent signing key.

The build never installs or launches the app. Development and console validation are tracked in [issue #1](https://github.com/brymon68/qdomyos-zwift/issues/1).

## License

See the [GPL-3.0 license](LICENSE) and [source attribution](NOTICE). This project is not affiliated with NordicTrack, iFIT, or Zwift.

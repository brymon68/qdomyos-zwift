FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential ca-certificates curl git perl pkg-config \
        python3 python3-pip rsync unzip xz-utils \
        libdbus-1-3 libfontconfig1 libfreetype6 libgl1-mesa-dev \
        libx11-xcb1 libxrender1 libxkbcommon-x11-0 libxcb-xinerama0 \
        libxcb-cursor0 libpulse-mainloop-glib0 \
    && rm -rf /var/lib/apt/lists/*

ENV JAVA_HOME=/opt/java \
    ANDROID_SDK_ROOT=/opt/android-sdk \
    ANDROID_HOME=/opt/android-sdk \
    ANDROID_NDK_ROOT=/opt/android-sdk/ndk/21.4.7075529 \
    ANDROID_NDK=/opt/android-sdk/ndk/21.4.7075529 \
    QT_ANDROID_DIR=/opt/qt/5.15.0/android \
    X22I_BUILD_JOBS=4 \
    X22I_VARIANT=test \
    LANG=C.UTF-8
ENV PATH="${QT_ANDROID_DIR}/bin:${JAVA_HOME}/bin:${ANDROID_SDK_ROOT}/cmdline-tools/latest/bin:${PATH}"

RUN curl --fail --location --retry 3 \
        'https://github.com/adoptium/temurin17-binaries/releases/download/jdk-17.0.20.1%2B1/OpenJDK17U-jdk_x64_linux_hotspot_17.0.20.1_1.tar.gz' \
        --output /tmp/temurin17.tar.gz \
    && echo '3808d1d15e3ec6bd5b84057fb5d84c33d8a1536a258146bcea2e603fc726e08e  /tmp/temurin17.tar.gz' | sha256sum --check --status \
    && mkdir -p "$JAVA_HOME" \
    && tar -xzf /tmp/temurin17.tar.gz --strip-components=1 -C "$JAVA_HOME" \
    && rm /tmp/temurin17.tar.gz

RUN curl --fail --location --retry 3 \
        'https://dl.google.com/android/repository/commandlinetools-linux-13114758_latest.zip' \
        --output /tmp/android-commandlinetools.zip \
    && echo '7ec965280a073311c339e571cd5de778b9975026cfcbe79f2b1cdcb1e15317ee  /tmp/android-commandlinetools.zip' | sha256sum --check --status \
    && mkdir -p "$ANDROID_SDK_ROOT/cmdline-tools" \
    && unzip -q /tmp/android-commandlinetools.zip -d "$ANDROID_SDK_ROOT/cmdline-tools" \
    && mv "$ANDROID_SDK_ROOT/cmdline-tools/cmdline-tools" "$ANDROID_SDK_ROOT/cmdline-tools/latest" \
    && rm /tmp/android-commandlinetools.zip

# /bin/sh returns sdkmanager's status, so yes exiting on a closed pipe is harmless.
RUN yes | sdkmanager --sdk_root="$ANDROID_SDK_ROOT" --licenses > /dev/null
RUN sdkmanager --sdk_root="$ANDROID_SDK_ROOT" \
        'ndk;21.4.7075529' 'platforms;android-35' 'build-tools;35.0.0'

RUN python3 -m pip install --no-cache-dir aqtinstall==3.3.0 py7zr==1.0.0 \
    && python3 -m aqt install-qt linux android 5.15.0 android \
        --outputdir /opt/qt --modules qtnetworkauth

# Mount this fork checkout at /source and the output parent at /build.
# Container root maps to the rootless host user.
COPY entrypoint.sh /usr/local/bin/x22i-entrypoint
WORKDIR /build
ENTRYPOINT ["/bin/bash", "/usr/local/bin/x22i-entrypoint"]
CMD ["bash", "/source/tools/x22i/build_baseline.sh", "/build/output"]

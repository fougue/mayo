# syntax=docker/dockerfile:1

# =============================================================================
#  Mayo - open source 3D CAD viewer and converter
#  Build image for linux/amd64
#
#  Based on Debian 13 "trixie" (Qt 6 + OpenCASCADE 7.8 packages).
#
#  Build:
#      docker build --platform linux/amd64 -t mayo:latest .
#
#  Build and run the test suite (executed under Xvfb):
#      docker build --platform linux/amd64 \
#          --build-arg MAYO_BUILD_TESTS=ON -t mayo:latest .
#
#  Extract the built binaries to ./dist on the host:
#      mkdir -p dist
#      docker run --rm -v "$PWD/dist:/out" mayo:latest \
#          cp /mayo/build/mayo /mayo/build/mayo-conv /out/
#
#  Run the GUI (needs access to the host X11/Wayland socket):
#      docker run --rm -it --net=host -e DISPLAY="$DISPLAY" \
#          -v /tmp/.X11-unix:/tmp/.X11-unix mayo:latest /mayo/build/mayo
#
#  NOTE: the amd64 build runs natively on an x86-64 host. On an arm host it
#        requires QEMU/binfmt emulation (docker buildx with --platform).
# =============================================================================

FROM debian:trixie

ARG DEBIAN_FRONTEND=noninteractive

# Qt warns at startup when the locale is not UTF-8 (Debian images default to "C")
ENV LANG=C.UTF-8

# Build configuration knobs
ARG MAYO_BUILD_TESTS=OFF
ARG MAYO_BUILD_ASSIMP=ON

# --- Dependencies ------------------------------------------------------------
# Qt 6: Core, Gui, Widgets, OpenGLWidgets, Test(for the test suite), Svg
# OpenCASCADE: data exchange + draw + misc resources
# Assimp: extra mesh importers(optional plugin)
# libgl/mesa + xvfb + x11-utils: to run the GUI/tests from within the image
RUN apt-get update && apt-get install -y \
        build-essential \
        cmake \
        ninja-build \
        git \
        pkg-config \
        ca-certificates \
        qt6-base-dev \
        qt6-svg-dev \
        libxcb-cursor0 \
        libocct-data-exchange-dev \
        libocct-draw-dev \
        occt-misc \
        libtbb-dev \
        libxi-dev \
        libassimp-dev \
        libgl1 \
        libgl1-mesa-dri \
        libglx-mesa0 \
        libxkbcommon-x11-0 \
        xvfb \
        x11-utils \
    # qt6-gtk-platformtheme is pulled in transitively and makes Qt use the native GTK file dialog
    # on Qt >= 6.5. That dialog routes through the xdg-desktop-portal DBus service which is not
    # available in the container, resulting in an empty/transparent dialog. Purging it makes Qt
    # fall back to its own dialog implementation
    && apt-get purge -y qt6-gtk-platformtheme \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /mayo
COPY . .

# --- Configure & build -------------------------------------------------------
RUN cmake -S . -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DMayo_BuildApp=ON \
        -DMayo_BuildConvCli=ON \
        -DMayo_BuildPluginAssimp="${MAYO_BUILD_ASSIMP}" \
        -DMayo_BuildTests="${MAYO_BUILD_TESTS}" \
    && cmake --build build --parallel "$(nproc)"

# --- Optional: run the test suite(needs a virtual X server) ------------------
RUN if [ "${MAYO_BUILD_TESTS}" = "ON" ]; then \
        Xvfb :99 -screen 0 1280x1024x24 -nolisten tcp & \
        xvfb_pid=$!; \
        sleep 5; \
        DISPLAY=:99 ctest --test-dir build --output-on-failure; \
        status=$?; \
        kill "$xvfb_pid"; \
        exit "$status"; \
    fi

CMD ["/bin/bash"]

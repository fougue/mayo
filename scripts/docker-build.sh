#!/usr/bin/env bash
#
# Build Mayo for linux/amd64 inside a Docker image and extract the produced binaries.
#
# This wraps the two usual steps:
#   1. docker build ...            (see ./Dockerfile)
#   2. docker run ... cp ...       (copy 'mayo' and 'mayo-conv' out of the image)
#
# Usage:
#   scripts/docker-build.sh [options]
#
# Options:
#   -i, --image <name>       Image tag to build/use        (default: mayo:build)
#   -o, --output <dir>       Host directory for binaries   (default: <repo>/dist)
#   -p, --platform <plat>    Target platform               (default: linux/amd64)
#       --tests              Build and run the test suite(inside the image, under Xvfb)
#       --no-assimp          Disable the Assimp plugin(extra mesh importers)
#       --no-cache           Do not use the Docker build cache
#   -h, --help               Show this help
#
# Examples:
#   scripts/docker-build.sh
#   scripts/docker-build.sh --tests --output /tmp/mayo-bin
#   scripts/docker-build.sh --no-assimp --no-cache
#
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_dir="$(cd "${script_dir}/.." && pwd)"

image="mayo:build"
platform="linux/amd64"
output_dir="${project_dir}/dist"
build_tests="OFF"
build_assimp="ON"
extra_build_args=()

usage() {
    cat <<'EOF'
Build Mayo for linux/amd64 inside a Docker image and extract the produced binaries.

Usage: scripts/docker-build.sh [options]

Options:
  -i, --image <name>       Image tag to build/use        (default: mayo:build)
  -o, --output <dir>       Host directory for binaries   (default: <repo>/dist)
  -p, --platform <plat>    Target platform               (default: linux/amd64)
      --tests              Build and run the test suite(inside the image, under Xvfb)
      --no-assimp          Disable the Assimp plugin(extra mesh importers)
      --no-cache           Do not use the Docker build cache
  -h, --help               Show this help
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -i|--image)    image="$2"; shift 2 ;;
        -o|--output)   output_dir="$2"; shift 2 ;;
        -p|--platform) platform="$2"; shift 2 ;;
        --tests)       build_tests="ON"; shift ;;
        --no-assimp)   build_assimp="OFF"; shift ;;
        --no-cache)    extra_build_args+=(--no-cache); shift ;;
        -h|--help)     usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; echo >&2; usage >&2; exit 2 ;;
    esac
done

if ! command -v docker >/dev/null 2>&1; then
    echo "Error: 'docker' was not found in PATH" >&2
    exit 1
fi

echo ">> Building image '${image}' for platform ${platform} ..."
docker build \
    --platform "${platform}" \
    --file "${project_dir}/Dockerfile" \
    --tag "${image}" \
    --build-arg "MAYO_BUILD_TESTS=${build_tests}" \
    --build-arg "MAYO_BUILD_ASSIMP=${build_assimp}" \
    "${extra_build_args[@]+"${extra_build_args[@]}"}" \
    "${project_dir}"

echo ">> Extracting binaries to '${output_dir}' ..."
mkdir -p "${output_dir}"
output_abs="$(cd "${output_dir}" && pwd)"

# Run the copy as the invoking user so the extracted files are not owned by root
docker run --rm \
    --user "$(id -u):$(id -g)" \
    --volume "${output_abs}:/out" \
    "${image}" \
    cp /mayo/build/mayo /mayo/build/mayo-conv /out/

echo ">> Done:"
ls -lh "${output_abs}/mayo" "${output_abs}/mayo-conv"

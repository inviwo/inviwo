#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <path-to-vcpkg-root>" >&2
    exit 1
fi

VCPKG_ROOT="$1"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Got VCPKG: ${VCPKG_ROOT}"

SOURCE_PORT="${VCPKG_ROOT}/ports/python3"
OVERLAY_PORT="${SCRIPT_DIR}/python3"

if [[ ! -d "${SOURCE_PORT}" ]]; then
    echo "Could not find python3 port at ${SOURCE_PORT}" >&2
    exit 1
fi

rm -rf "${OVERLAY_PORT}"
cp -R "${SOURCE_PORT}" "${OVERLAY_PORT}"

# Build the debug python with the debug CRT but the release ABI and file names, so that
# release wheels (numpy etc.) can be loaded and vcpkg applocal finds python3XX.dll in debug/bin
cp "${SCRIPT_DIR}/python3-release-abi-debug.patch" "${OVERLAY_PORT}/0100-release-abi-debug.patch"
patch --ignore-whitespace --directory "${OVERLAY_PORT}" -p1 < "${SCRIPT_DIR}/python3-fix.patch"


SOURCE_PORT="${VCPKG_ROOT}/ports/nifticlib"
OVERLAY_PORT="${SCRIPT_DIR}/nifticlib"

if [[ ! -d "${SOURCE_PORT}" ]]; then
    echo "Could not find nifticlib port at ${SOURCE_PORT}" >&2
    exit 1
fi

rm -rf "${OVERLAY_PORT}"
cp -R "${SOURCE_PORT}" "${OVERLAY_PORT}"

cp ${SCRIPT_DIR}/nifti-libm.patch ${OVERLAY_PORT}/libm.patch
patch --ignore-whitespace --directory "${OVERLAY_PORT}" -p1 < "${SCRIPT_DIR}/nifti.patch"



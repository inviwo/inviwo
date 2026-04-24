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

patch --ignore-whitespace --directory "${OVERLAY_PORT}" -p1 < "${SCRIPT_DIR}/python3-fix.patch"

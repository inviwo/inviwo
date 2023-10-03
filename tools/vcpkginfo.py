# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2020-2023 Inviwo Foundation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ********************************************************************************

import sys
import argparse
import subprocess
import json


# Needs to work with python 3.6 on github
def makeCmdParser():
    parser = argparse.ArgumentParser(
        description="Get vcpkg package info",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('-v', '--vcpkg', type=str, action="store", dest="vcpkg",
                        help='Paths to vcpkg executable', required=True)
    parser.add_argument('-p', '--pkg', type=str, action="store", dest="pkg",
                        help='Vcpkg package name', required=True)
    parser.add_argument('-t', '--triplet', type=str, action="store", dest="triplet",
                        help='Vcpkg Triplet')
    parser.add_argument('-i', '--install', type=str, action="store", dest="install",
                        help='Vcpkg install root', default="")
    parser.add_argument('-m', '--manifest_dir',  type=str, action="store", dest="manifest_dir",
                        help='Vcpkg manifest dir, need to run vcpkg from the manifest dir'
                        ' to pick up settings (overlays) from the manifest', required=True)

    return parser


def toString(items):
    return ';'.join(items)


if __name__ == '__main__':
    parser = makeCmdParser()
    args = parser.parse_args()

    if len(args.install) > 0:
        install = "--x-install-root=" + args.install
    else:
        install = ""

    #  vcpkg x-package-info --x-installed --x-json zlib:x64-windows
    cmd = subprocess.run(
        [
            args.vcpkg,
            install,
            "x-package-info",
            "--x-json",
            args.pkg
        ],
        cwd=args.manifest_dir,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    if cmd.returncode != 0 and len(cmd.stderr) > 0:
        print(f"Error calling vcpkg return {cmd.returncode}", file=sys.stderr)
        print(f"Stdout: {cmd.stdout}", file=sys.stderr)
        print(f"Stderr: {cmd.stderr}", file=sys.stderr)
        print(f"manifest_dir {args.manifest_dir}", file=sys.stderr)
        print("Call:", file=sys.stderr)
        print([args.vcpkg, install, "x-package-info", "--x-json", f"{args.pkg}"],
              file=sys.stderr)
        exit(100)

    if cmd.stdout.startswith("warning: Embedding `vcpkg-configuration` in a manifest file is an EXPERIMENTAL feature."):
        cmd.stdout = '\n'.join(cmd.stdout.splitlines()[1:])

    try:
        portInfo = json.loads(cmd.stdout)
    except Exception as e:
        print("Error parsing json", file=sys.stderr)
        print(e, file=sys.stderr)
        print(cmd.stdout, file=sys.stderr)
        exit(2)

    if args.pkg in portInfo['results']:
        portInfo = portInfo['results'][args.pkg]
    else:
        print(f"Package {args.pkg}:{args.triplet} not found in vcpkg", file=sys.stderr)
        exit(3)

    cmd = subprocess.run(
        [
            args.vcpkg,
            install,
            "x-package-info",
            "--x-installed",
            "--x-json",
            f"{args.pkg}:{args.triplet}"
        ],
        cwd=args.manifest_dir,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    if cmd.returncode != 0 and len(cmd.stderr) > 0:
        print(f"Error calling vcpkg return {cmd.returncode}", file=sys.stderr)
        print(f"Stdout: {cmd.stdout}", file=sys.stderr)
        print(f"Stderr: {cmd.stderr}", file=sys.stderr)
        print(f"manifest_dir {args.manifest_dir}", file=sys.stderr)
        print("Call:", file=sys.stderr)
        print([args.vcpkg, install, "x-package-info", "--x-json", f"{args.pkg}"],
              file=sys.stderr)
        exit(4)

    if cmd.stdout.startswith("warning: Embedding `vcpkg-configuration` in a manifest file is an EXPERIMENTAL feature."):
        cmd.stdout = '\n'.join(cmd.stdout.splitlines()[1:])

    try:
        installInfo = json.loads(cmd.stdout)
    except Exception as e:
        print("Error parsing json", file=sys.stderr)
        print(e, file=sys.stderr)
        print(cmd.stdout, file=sys.stderr)
        exit(5)

    if f"{args.pkg}:{args.triplet}" in installInfo['results']:
        installInfo = installInfo['results'][f"{args.pkg}:{args.triplet}"]
    else:
        print(f"Package {args.pkg}:{args.triplet} not found in vcpkg", file=sys.stderr)
        exit(6)

    result = ""
    if "version" in portInfo:
        result += f"VCPKG_VERSION;{portInfo['version']};"
    elif "version-string" in installInfo:
        result += f"VCPKG_VERSION;{installInfo['version-string']};"

    if "homepage" in portInfo:
        result += f"VCPKG_HOMEPAGE;{portInfo['homepage']};"

    if "dependencies" in installInfo and len(installInfo['dependencies']) > 0:
        result += f"VCPKG_DEPENDENCIES;{toString(installInfo['dependencies'])};"

    if "owns" in installInfo and len(installInfo['owns']) > 0:
        result += f"VCPKG_OWNED_FILES;{toString(installInfo['owns'])};"

    print(result)

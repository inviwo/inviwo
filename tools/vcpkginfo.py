#*********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2020 Inviwo Foundation
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
#*********************************************************************************

import os
import sys
import argparse
import subprocess
import json

## Needs to work with python 3.6 on github

def makeCmdParser():
	parser = argparse.ArgumentParser(
		description="Get vcpkg package info",
		formatter_class=argparse.ArgumentDefaultsHelpFormatter
	)
	parser.add_argument('-v', '--vcpkg', type=str, action="store", dest="vcpkg", help='Paths to vcpgk executable')
	parser.add_argument('-p', '--pkg', type=str, action="store", dest="pkg", help='Vcpkg package name')
	parser.add_argument('-t', '--triplet', type=str, action="store", dest="triplet", help='Triplet')
	parser.add_argument('-o', '--overlay', type=str, action="store", dest="overlay", help='Extra vcpkg overlay', default="")

	return parser.parse_args()

def toString(items):
	return ';'.join(items)

if __name__ == '__main__':
	args = makeCmdParser()

	#  x-package-info --x-installed --x-json zlib:x64-windows 
	
	if len(args.overlay) > 0:
		overlay = "--overlay-ports=" + args.overlay
	else:
		overlay = ""

	cmd = subprocess.run([
		args.vcpkg, 
		overlay,
		"x-package-info", 
		"--x-json", 
		f"{args.pkg}"],
		stdout=subprocess.PIPE,
		stderr=subprocess.STDOUT
	)
	portInfo = json.loads(cmd.stdout)

	cmd = subprocess.run([
		args.vcpkg, 
		overlay,
		"x-package-info", 
		"--x-installed", 
		"--x-json", 
		f"{args.pkg}:{args.triplet}"],
		stdout=subprocess.PIPE,
		stderr=subprocess.STDOUT
	)
	installInfo = json.loads(cmd.stdout)

	info = {}
	info.update(portInfo['results'][args.pkg])
	info.update(installInfo['results'][f"{args.pkg}:{args.triplet}"])


	result = ""
	if "version-string" in info:
		result += f"VCPKG_VERSION;{info['version-string']};"

	if "homepage" in info:
		result += f"VCPKG_HOMEPAGE;{info['homepage']};"

	if "dependencies" in info and len(info['dependencies'])>0:
		result += f"VCPKG_DEPENDENCIES;{toString(info['dependencies'])};"

	if "owns" in info and len(info['owns'])>0:
		result += f"VCPKG_OWNED_FILES;{toString(info['owns'])};"

	print(result)

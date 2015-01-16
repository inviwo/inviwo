#*********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
# Version 0.9
#
# Copyright (c) 2013-2015 Inviwo Foundation
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
import shutil
import subprocess


if os.name == 'posix':
	DOXYGEN='doxygen'
	HELPCOL='/usr/local/opt/qt5/bin/qcollectiongenerator'
	HELPGEN='/usr/local/opt/qt5/bin/qhelpgenerator'
else:
	DOXYGEN='doxygen.exe'
	HELPCOL='qcollectiongenerator.exe'
	HELPGEN='qhelpgenerator.exe'

def run(cmd):
	try:
		with subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True) as proc:
			for line in proc.stdout:
				print(line, end='', flush=True)
	except FileNotFoundError:
		print_error("Could not find " + cmd[0])


print("Run doxygen")
run([DOXYGEN, "Doxyfile_QT"])

print("Copy css")	
shutil.copyfile("style/qt-stylesheet.css", "doc-qt/html/doxygen.css")

print("Run Help gen")
run([HELPGEN, "-o", "../../data/help/inviwo.qch", "doc-qt/html/index.qhp"])

print("Run Help collection")
run([HELPCOL, "-o" "../../data/help/inviwo.qhc", "../../data/help/inviwo.qhcp"])





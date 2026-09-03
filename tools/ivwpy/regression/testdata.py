# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2026 Inviwo Foundation
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

import glob
import os
import json

from . error import *
from .. util import *


class TestData:
    def __init__(self, name, module, path, overridePath=None):
        self.module = module
        self.path = path
        self.name = name
        self.script = ""
        self.config = {}
        self.workspaces = glob.glob(self.path + "/*.inv")
        self.textExtensions = ["txt", "csv"]
        # root of an external tree mirroring <module>/<name>/<file>, may be None
        self.overridePath = overridePath

        configfile = toPath(self.path, "config.json")
        if os.path.exists(configfile):
            with open(configfile, 'r') as f:
                self.config = json.load(f)

        scripts = glob.glob(self.path + "/*.py")
        if len(scripts) > 0:
            self.script = scripts[0]

    def __str__(self):
        return self.toString()

    def toString(self):
        return self.module + "/" + self.name

    def getWorkspaces(self):
        return self.workspaces

    def getOverrideDir(self):
        if not self.overridePath:
            return None
        overrideDir = toPath(self.overridePath, self.module, self.name)
        return overrideDir if os.path.isdir(overrideDir) else None

    def resolveRef(self, filename):
        # per-file fallback: only use the override copy when it actually exists
        overrideDir = self.getOverrideDir()
        if overrideDir:
            overrideFile = toPath(overrideDir, filename)
            if os.path.exists(overrideFile):
                return overrideFile
        return toPath(self.path, filename)

    def isOverridden(self, filename):
        return self.resolveRef(filename) != toPath(self.path, filename)

    def getImages(self):
        imgs = glob.glob(self.path + "/*.png")
        overrideDir = self.getOverrideDir()
        if overrideDir:
            imgs += glob.glob(overrideDir + "/*.png")
        return sorted({os.path.basename(x) for x in imgs})

    def getTextExtensions(self):
        if "textExtensions" in self.config:
            return self.textExtensions + self.config["textExtensions"]
        else:
            return self.textExtensions

    def getTextFiles(self):
        dirs = [self.path]
        overrideDir = self.getOverrideDir()
        if overrideDir:
            dirs.append(overrideDir)
        files = (f for d in dirs for ext in self.getTextExtensions()
                for f in glob.glob(f"{d}/*.{ext}"))
        return sorted({os.path.basename(x) for x in files})

    def report(self, report):
        report['module'] = self.module
        report['name'] = self.name
        report['path'] = self.path
        report['script'] = self.script
        report['config'] = self.config
        return report

    def makeOutputDir(self, base):
        if not os.path.isdir(base):
            raise RegressionError("Output dir does not exsist: " + base)

        mkdir(base, self.module)
        mkdir(base, self.module, self.name)
        return toPath(base, self.module, self.name)

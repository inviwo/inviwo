# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2022 Inviwo Foundation
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

from .. util import *
from .. colorprint import *


class ReportTestSettings:
    def __init__(self, imageDifferenceTolerance=0.0,
        imageDiffLogscale=False, imageDiffInvert=False):
        self.imageDifferenceTolerance = imageDifferenceTolerance
        self.imageDiffLogscale = imageDiffLogscale
        self.imageDiffInvert = imageDiffInvert


class ReportTest:
    def __init__(self, key, testfun, message):
        self.key = key if isinstance(key, list) else [key]
        self.testfun = testfun
        self.message = message

    def test(self, report):
        return self.testfun(safeget(report, *self.key))


class ReportImageTest(ReportTest):
    def __init__(self, key, differenceTolerance=0.0):
        self.key = key
        self.message = {}
        self.differenceTolerance = differenceTolerance

    def test(self, report):
        imgs = report[self.key]["tests"]
        for img in imgs:
            tol = safeget(report, "config", "image_test", "differenceTolerance",
                          img["image"], failure=self.differenceTolerance)
            tol = max(tol, self.differenceTolerance)

            if img['test_mode'] != img['ref_mode']:
                self.message[img['image']] = \
                    ("Image {image} has different modes, "
                     "Test: {test_mode} vs Reference:{ref_mode}").format(**img)
            elif img['test_size'] != img['ref_size']:
                self.message[img['image']] = \
                    ("Image {image} has different sizes, "
                     "Test: {test_size} vs Reference:{ref_size}").format(**img)
            elif img["difference_percent"] > tol:
                self.message[img['image']] = \
                    ("Image {image} has difference greater than the allowed "
                     "tolerance ({difference_percent}% &gt; {tol})  "
                     "difference, {difference_pixels} different pixels, "
                     "largest differences {max_differences}").format(tol=tol, **img)

        return len(self.message) == 0


class ReportTextTest(ReportTest):
    def __init__(self, key):
        self.key = key
        self.message = {}

    def test(self, report):
        tests = report[self.key]["tests"]
        for test in tests:
            if test['diff'] > 0:
                self.message[test['txt']] = f"{test['txt']} differs from the reference" \
                                            f" in {test['diff']} locations"
        return len(self.message) == 0


class ReportLogTest(ReportTest):
    def __init__(self):
        self.key = 'log'
        self.message = []

    def test(self, report):
        try:
            with open(toPath(report['outputdir'], report['log']), 'r') as f:
                lines = f.readlines()
                for line in lines:
                    if "<span class='level'>Error: </span>" in line:
                        self.message.append(line)
        except FileNotFoundError:
            self.message.append("Missing Log")

        return len(self.message) == 0


class ReportTestSuite:
    def __init__(self, settings=ReportTestSettings()):
        self.settings = settings
        self.tests = [
            ReportTest('returncode', lambda x: x == 0, "Non zero return code"),
            ReportTest('timeout', lambda x: x is False, "Inviwo ran out of time"),
            ReportTest(['images', 'missing_refs'], lambda x: len(x) == 0,
                       "Missing reference image"),
            ReportTest(['images', 'missing_imgs'], lambda x: len(x) == 0,
                       "Missing test image"),
            ReportImageTest('images', settings.imageDifferenceTolerance),
            ReportTest(['txts', 'missing_refs'], lambda x: len(x) == 0,
                       "Missing reference text file"),
            ReportTest(['txts', 'missing_txts'], lambda x: len(x) == 0,
                       "Missing test text file"),
            ReportTextTest('txts'),
            ReportLogTest()
        ]

    def checkReport(self, report):
        failures = []
        successes = []
        for t in self.tests:
            if not t.test(report):
                failures.append([t.key, t.message])
            else:
                successes.append(t.key)
        report['failures'] = failures
        report['successes'] = successes
        return report

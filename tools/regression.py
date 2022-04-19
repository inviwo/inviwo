# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2021 Inviwo Foundation
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

import os
import sys
import argparse
import configparser
import logging
from ivwpy.util import *
from ivwpy.colorprint import *
import ivwpy.regression
import ivwpy.regression.app
import ivwpy.regression.error

# Ipython auto reload
# %load_ext autoreload
# %autoreload 2


def makeCmdParser():
    parser = argparse.ArgumentParser(
        description="Run regression tests",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('-i', '--inviwo', type=str, action="store", dest="inviwo",
                        help='Paths to inviwo executable')
    parser.add_argument('-c', '--config', type=str, action="store",
                        dest="config", help='A configure file', default="")
    parser.add_argument('-b', '--build_type', type=str, action="store", dest="build_type",
                        help='Specify the build type (Debug, Release, ...)', default="")
    parser.add_argument('-o', '--output', type=str, action="store",
                        dest="output", help='Path to output')

    parser.add_argument('-r', '--repos', type=str, nargs='*', action="store", dest="repos",
                        help='Paths to inviwo repos')
    parser.add_argument("-m", "--modules", type=str, nargs='*', action="store", dest="modules",
                        default=[], help="Paths to folders with modules")

    parser.add_argument("-s", "--slice", type=str, nargs='?', action="store", dest="slice",
                        default="", help="Specifiy a specific slice of tests to run")
    parser.add_argument("--include", type=str, nargs='?', action="store", dest="include",
                        default="", help="Include filter")
    parser.add_argument("--exclude", type=str, nargs='?', action="store", dest="exclude",
                        default="", help="Exclude filter")
    parser.add_argument("-f", "--failed", action="store_true",
                        dest="failed", help="Only run tests that have faild")
    parser.add_argument("-l", "--list", action="store_true", dest="list", help="List all tests")
    parser.add_argument("--imagetolerance", type=float, action="store", dest="imagetolerance",
                        default=0.0, help="Tolerance when comparing images")
    parser.add_argument('--header', type=str, action="store", dest="header",
                        help='An optional report header', default=None)
    parser.add_argument('--footer', type=str, action="store", dest="footer",
                        help='An optional report footer', default=None)
    parser.add_argument("-v", "--view", action="store_true",
                        dest="view", help="Open the report when done")
    parser.add_argument("--log", type=str, dest="log", default="WARN",
                        help="Select log level: DEBUG, INFO, WARN, ERROR, CRITICAL")
    parser.add_argument("--summary", action="store_true",
                        dest="summary", help="Print summary information")

    return parser.parse_args()


def searchRepoPaths(paths):
    modulePaths = []
    for path in paths:
        if os.path.isdir(toPath(path, "modules")):
            modulePaths.append(toPath(path, "modules"))
    return modulePaths


def makeFilter(inc, exc):
    if inc != "":
        def incfilter(test):
            return inc in test.toString()
    else:
        def incfilter(test):
            return True

    if exc != "":
        def excfilter(test):
            return exc in test.toString()
    else:
        def excfilter(test):
            return False

    def filter(test):
        return incfilter(test) and not excfilter(test)

    return filter


def execonf(file, build):
    parts = os.path.splitext(file)
    return parts[0] + "-" + build + parts[1]


if __name__ == '__main__':

    args = makeCmdParser()
    config = configparser.ConfigParser()

    numeric_level = getattr(logging, args.log.upper(), None)
    if not isinstance(numeric_level, int):
        print_error(f"Invalid log level: {args.log}")
        sys.exit(1)
    logging.basicConfig(level=numeric_level, encoding='utf-8', format='%(message)s')

    if args.inviwo:
        if not os.path.exists(args.inviwo):
            print_error("Path to inviwo executable is invalid: " + args.inviwo)
            sys.exit(1)

        inviwopath = os.path.abspath(args.inviwo)
        configpath = find_pyconfig(inviwopath)
        config.read([
            configpath if configpath else "",
            args.config if args.config else ""
        ])
    elif args.config and args.build_type:
        readfiles = config.read([args.config, execonf(args.config, args.build_type)])
        inviwopath = config.get("Inviwo", "executable")
    else:
        print_error("Regression.py needs either a either an inviwo executable using"
                    " '--inviwo' or a config and build_type using '--config' and '--build_type'")
        sys.exit(1)

    if not os.path.exists(inviwopath):
        print_error("Regression.py was unable to find inviwo executable at " + inviwopath)
        sys.exit(1)

    modulePaths = []
    if args.repos or args.modules:
        if args.repos:
            modulePaths = searchRepoPaths(args.repos)
        if args.modules:
            modulePaths += args.modules
    elif config.has_option("Inviwo", "modulepaths"):
        modulePaths = config.get("Inviwo", "modulepaths").split(";")

    modulePaths = list(set(map(os.path.abspath, modulePaths)))

    if args.output:
        output = os.path.abspath(args.output)
    elif config.has_option("CMake", "binary_dir"):
        output = mkdir(config.get("CMake", "binary_dir"), "regress")
    else:
        print_error(
            "Regression.py was unable to decide on an output dir,"
            " please specify \"-o <output path>\"")
        sys.exit(1)

    activeModules = None
    if config.has_option("Inviwo", "activemodules"):
        activeModules = config.get("Inviwo", "activemodules").split(";")

    runSettings = ivwpy.regression.inviwoapp.RunSettings(
        timeout=60,
        activeModules=activeModules
    )

    testSettings = ivwpy.regression.reporttest.ReportTestSettings(
        imageDifferenceTolerance=args.imagetolerance)

    app = ivwpy.regression.app.App(appPath=inviwopath,
                                   moduleTestPaths=modulePaths,
                                   outputDir=output,
                                   jsonFile="report",
                                   htmlFile="report",
                                   sqlFile="report",
                                   runSettings=runSettings,
                                   testSettings=testSettings)

    testfilter = makeFilter(args.include, args.exclude)
    testrange = makeSlice(args.slice)

    if args.list:
        app.printTestList(testrange=testrange, testfilter=testfilter)
        exit(0)

    try:
        app.runTests(testrange=testrange, testfilter=testfilter, onlyRunFailed=args.failed)
        app.saveJson()

        header = None
        if args.header and os.path.exists(args.header):
            with open(args.header) as f:
                header = f.read()
        footer = None
        if args.footer and os.path.exists(args.footer):
            with open(args.footer) as f:
                footer = f.read()

        app.saveHtml(header=header, footer=footer)

        if app.success():
            print_info("Regression was successful")
            print_info("Report: " + output + "/report.html")
            if args.summary:
                app.printSummary()
            if args.view:
                openWithDefaultApp(output + "/report.html")
            sys.exit(0)
        else:
            print_error("Regression was unsuccessful see report for details")
            print_info("Report: " + output + "/report.html")
            if args.summary:
                app.printSummary()
            if args.view:
                openWithDefaultApp(output + "/report.html")
            sys.exit(1)

    except ivwpy.regression.error.MissingInivioAppError as err:
        print_error(err.error)
        print_info("Check that option '-i' is correct")
        sys.exit(1)

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
import io
import pkgutil
import glob
import datetime
import contextlib
import lesscpy
import json
import html
import difflib

# Beautiful Soup 4 for dom manipulation
import bs4

# Yattag for HTML generation, http://www.yattag.org
import yattag

from .. util import *
from . database import *
from . statistics import *

# Javascript packages
# jQuery              http://jquery.com
# jQuery Sparklines   http://omnipotent.net/jquery.sparkline/
# jQuery Flot         http://www.flotcharts.org
#        Flot tooltip https://github.com/krzysu/flot.tooltip (no longer maintained since 2016)
# jQuery Zoom         https://www.jacklmoore.com/zoom/
# List.js             https://listjs.com
# clipboard.js        https://clipboardjs.com

# Jenkins note: https://wiki.jenkins-ci.org/display/JENKINS/Configuring+Content+Security+Policy
# System.setProperty("hudson.model.DirectoryBrowserSupport.CSP",
#                    "default-src 'self';script-src 'self';style-src 'self' 'unsafe-inline';")
# added to /etc/default/jenkins


def toString(val):
    if val is None:
        return "None"
    if isinstance(val, str):
        if len(val) == 0:
            return "None"
        else:
            return val
    elif isinstance(val, list):
        if len(val) == 0:
            return "None"
        else:
            return "(" + ", ".join(map(toString, val)) + ")"
    elif isinstance(val, float):
        return "{:6f}".format(val)
    elif isinstance(val, int):
        return "{:}".format(val)


def abr(text, length=85):
    abr = text.split("\n")[0][:length]
    return abr + ("..." if len(text.split("\n")) > 1 or len(text) > length else "")


def abrhtml(html, length=85):
    soup = bs4.BeautifulSoup(html.strip(), 'html.parser')
    size = len(soup.getText())
    toRemove = size - length
    if toRemove > 0:
        lastString = soup.find_all(string=True)[-1]
        lastString.replace_with(lastString[:-toRemove] + "...")
        return True, soup.prettify()
    else:
        return False, soup.prettify()


def isValidString(*vars):
    for var in vars:
        if var is None or var == "" or not isinstance(var, str):
            return False
    return True


def formatKey(key):
    if not isinstance(key, list):
        key = [key]
    return " ".join(k.capitalize().replace("_", " ") for k in key)


def keyval(key, val):
    doc, tag, text = yattag.Doc().tagtext()
    with tag("div", klass="row"):
        with tag("div", klass="cell key"):
            text(key)
        with tag("div", klass="cell"):
            doc.asis(val)
    return doc.getvalue()


def listItem(head, body="", status="", toggle=True, hide=True):
    doc, tag, text = yattag.Doc().tagtext()
    toggleclass = "toggle" if toggle else ""
    opts = {"style": "display: none;"} if hide else {}
    with tag('li', klass='row'):
        with tag('div', klass='lihead ' + status + ' ' + toggleclass):
            doc.asis(head)
        if toggle:
            with tag('div', klass='libody', **opts):
                if callable(body):
                    body(doc, tag, text)
                else:
                    doc.asis(body)

    return doc.getvalue()


def gitLink(commit):
    doc, tag, text = yattag.Doc().tagtext()

    if isValidString(commit.server, commit.hash):
        with tag('a', href=commit.server + "/commit/" + commit.hash, title=commit.hash):
            text(commit.hash[:7])
    else:
        text("None")
    return doc.getvalue()

def gitServerName(commit):
    doc, tag, text = yattag.Doc().tagtext()
    if isValidString(commit.server):
        with tag('a', href=commit.server, title=commit.server):
            text(commit.server.rsplit('/', 1)[-1])
    else:
        text("Unknown")
    return doc.getvalue()

def getDiffLink(start, stop):
    doc, tag, text = yattag.Doc().tagtext()
    if isValidString(start.server, start.hash, stop.hash):
        with tag('a', href=start.server + "/compare/" + start.hash + "..." + stop.hash):
            text(start.server + "/compare/" + start.hash + "..." + stop.hash)
    else:
        text("None")
    return doc.getvalue()

def commitSplitMessage(msg):
    # format of git commit message:
    #   <header>
    #   <blank line>
    #   <body>
    #   <blank line>
    #   <footer>
    header, _, part = msg.strip().partition('\n\n')
    body, _, footer = part.rpartition('\n\n')
    if body == '':
        body, footer = footer, body
    return header, body, footer

def commitInfo(commit, abbrevmessage=True):

    def formatMessage(header, body, footer):
        doc, tag, text = yattag.Doc().tagtext()
        with tag('div', klass='gitmsgheader'):
            text(header)
        if body:
            with tag('div', klass='gitmsgbody'):
                text(body)
        if footer:
            with tag('div', klass='gitmsgfooter'):
                text(footer)
        return doc.getvalue()

    doc, tag, text = yattag.Doc().tagtext()
    with tag('ul'):
        header, body, footer = commitSplitMessage(commit.message)

        val = formatMessage(header, body, footer)
        vabr = html.escape(abr(header)) if abbrevmessage else val
        gdate = commit.date.strftime('%Y-%m-%d %H:%M:%S')

        doc.asis(listItem(keyval("Message", vabr), val, toggle=vabr != val))
        doc.asis(listItem(keyval("Author", commit.author), toggle=False))
        doc.asis(listItem(keyval("Date", gdate), toggle=False))
        doc.asis(listItem(keyval("Commit", gitLink(commit)), toggle=False))

    return doc.getvalue()


def formatLog(file):
    doc, tag, text = yattag.Doc().tagtext()
    try:
        with open(file, 'r') as f:
            loghtml = f.read()
            err = loghtml.count("<span class='level'>Error: </span>")
            warn = loghtml.count("<span class='level'>Warn: </span>")
            info = loghtml.count("<span class='level'>Info: </span>")
            short = "Error: {}, Warnings: {}, Information: {}".format(err, warn, info)

            def log(doc, tag, text):
                with tag('div', klass='log'):
                    doc.asis(loghtml)

            doc.asis(listItem(keyval("Log", short), log, status="ok" if err == 0 else "fail"))
    except FileNotFoundError as e:
        doc.asis(listItem(keyval("Log", "Logfile missing"), str(e), status="fail"))
    return doc.getvalue()


def image(path, **opts):
    doc, tag, text = yattag.Doc().tagtext()
    doc.stag('img', src=path, **opts)
    return doc.getvalue()


def imagesShort(report):
    doc, tag, text = yattag.Doc().tagtext()
    failures = safeget(report, 'failures', failure={})
    fail = next((v for k, v in failures if k == "images"), None)
    text("All OK" if fail is None else
         "{:d}/{:d} Images failed".format(len(fail), len(report["images"]["tests"])))
    return doc.getvalue()


def txtsShort(report):
    doc, tag, text = yattag.Doc().tagtext()
    failures = safeget(report, 'failures', failure={})
    fail = next((v for k, v in failures if k == "txts"), None)
    text("All OK" if fail is None else
         "{:d}/{:d} Txts failed".format(len(fail), len(report["txts"]["tests"])))
    return doc.getvalue()


def testImages(testimg, refimg, diffimg, maskimg):
    doc, tag, text = yattag.Doc().tagtext()

    with tag('div', klass="zoomset"):
        with tag('div', klass="slider"):
            with tag('div'):
                with tag('div', klass="imgtestlabel"):
                    text("Test")
                with tag('div', klass="zoom"):
                    doc.asis(image(testimg, alt="test image", klass="test"))
            with tag('div'):
                with tag('div', klass="imgtestlabel"):
                    text("Reference")
                with tag('div', klass="zoom"):
                    doc.asis(image(refimg, alt="reference image", klass="test"))
            if diffimg is not None:
                with tag('div'):
                    with tag('div', klass="imgtestlabel"):
                        text("Difference * 10")
                    with tag('div', klass="zoom"):
                        doc.asis(image(diffimg, alt="difference image", klass="diff"))
            if maskimg is not None:
                with tag('div'):
                    with tag('div', klass="imgtestlabel"):
                        text("Mask")
                    with tag('div', klass="zoom"):
                        doc.asis(image(maskimg, alt="mask image", klass="diff"))
    return doc.getvalue()


def dataToJsArray(data):
    return "[\n" + ",\n".join(["[" + str(x) + ", " + str(y) + "]" for x, y in data]) + "\n]"


difflibLegend = """
<table border=0>
      <tr>
         <th>Colors:</th>
          <td class="diff_add">Added</td>
          <td class="diff_chg">Changed</td>
          <td class="diff_sub">Deleted</td>
          <th>Links:</th>
          <td>(f)irst change</td>
          <td>(n)ext change</td>
          <td>(t)op</td>
      </tr>
</table>
"""


class TestRun:
    """Generate a html report for one Test Run"""

    def __init__(self, htmlReport, report):
        self.report = report
        self.db = htmlReport.db
        self.basedir = htmlReport.basedir
        self.created = htmlReport.created
        self.doc, self.tag, self.text = yattag.Doc().tagtext()

        self.name = report['name']
        self.module = report['module']
        self.history_days = 31

        testrun = self.db.getLastTestRun(self.report["module"], self.report["name"])
        lastSuccess, firstFailure = self.db.getLastSuccessFirstFailure(self.report["module"],
                                                                       self.report["name"])

        with self.item(self.head(), status=self.totalstatus()):
            with self.tag('ul'):
                if "images" in self.report.keys():
                    self.doc.asis(listItem(keyval("Images", imagesShort(self.report)),
                                           self.images(self.report["images"]["tests"],
                                                       self.report["outputdir"]),
                                           status=self.status('images')))
                    for key in [["images", "missing_imgs"], ["images", "missing_refs"]]:
                        self.simple(key)

                if "txts" in self.report.keys():
                    self.doc.asis(listItem(keyval("Txts", txtsShort(self.report)),
                                           self.txtsItem(self.report["txts"]["tests"],
                                                         self.report["path"],
                                                         self.report["outputdir"]),
                                           status=self.status('txts')))
                    for key in [["txts", "missing_txts"], ["txts", "missing_refs"]]:
                        self.simple(key)

                self.simple(["returncode"])
                self.doc.asis(formatLog(toPath(report['outputdir'], report['log'])))

                self.testRunInfo("Current Version", testrun)
                if self.totalstatus() != "ok":
                    self.testRunInfo("Last Success", lastSuccess)
                    self.testRunInfo("First Failure", firstFailure)
                    self.gitDiff(lastSuccess, firstFailure)

                self.doc.asis(self.paths())
                for key in [["command"], ["output"], ["errors"]]:
                    self.simple(key)

                self.doc.asis(self.plots())
                self.doc.asis(self.failures())

    def getvalue(self):
        return self.doc.getvalue()

    @contextlib.contextmanager
    def item(self, head, status="", hide=True, doc=None):
        if doc is None:
            doc = self.doc
        opts = {"style": "display: none;"} if hide else {}
        with doc.tag('li', klass='row'):
            with doc.tag('div', klass='lihead toggle ' + status):
                doc.asis(head)
            with doc.tag('div', klass='libody', **opts):
                yield None

    def totalstatus(self):
        if not safeget(self.report, "config", "enabled", failure=True):
            return "disabled"
        return ("ok" if len(self.report['failures']) == 0 else "fail")

    def status(self, key):
        status = ""
        if key in self.report['successes']:
            status = "ok"
        if key in (f for f, _ in self.report['failures']):
            status = "fail"
        return status

    def simple(self, key):
        value = toString(safeget(self.report, *key))
        short = abr(value)
        self.doc.asis(listItem(keyval(formatKey(key), html.escape(short)), html.escape(value),
                               status=self.status(key),
                               toggle=short != value))

    def paths(self):
        def pathList():
            doc, tag, text = yattag.Doc().tagtext()
            with tag('ol'):
                with tag('li'):
                    text(self.report["path"])
                    opts = {"data-clipboard-text": self.report["path"]}
                    with tag('button', klass="copylinkbtn", **opts):
                        text("copy")
                with tag('li'):
                    text(self.report["outputdir"])
                    opts = {"data-clipboard-text": self.report["outputdir"]}
                    with tag('button', klass="copylinkbtn", **opts):
                        text("copy")

            return doc.getvalue()

        value = toString(self.report["path"])
        short = abr(value)
        return listItem(keyval("Paths", short), pathList())

    def testRunInfo(self, key, testrun):
        if testrun is not None:
            date = testrun.commit.date.strftime('%Y-%m-%d %H:%M:%S')
            self.doc.asis(listItem(keyval(key,
                                          date + " " + html.escape(abr(testrun.commit.message, 50))
                                          ),
                                   commitInfo(testrun.commit)))
        else:
            self.doc.asis(listItem(keyval(key, "None"), toggle=False))

    def gitDiff(self, lastSuccess, firstFailure):
        if (lastSuccess is not None) and (firstFailure is not None):
            self.doc.asis(listItem(keyval("Diff",
                                          getDiffLink(lastSuccess.commit,
                                                      firstFailure.commit)), toggle=False))

    def sparkLine(self, series, klass, normalRange=True, valueRange=True):
        doc, tag, text = yattag.Doc().tagtext()
        data = self.db.getSeries(self.module, self.name, series)
        xmax = self.created.timestamp()

        xmin = xmax - 60 * 60 * 24 * self.history_days
        if xmin < data.created.timestamp():
            xmin = data.created.timestamp()

        mean, std = stats([x.value for x in data.measurements])

        values = [x for x in data.measurements if x.created.timestamp() > xmin]
        if len(values) == 0:
            values = data.measurements[-30:]
        minval = min((x.value for x in values))
        maxval = max((x.value for x in values))
        datastr = ", ".join((str(x.created.timestamp()) + ":" + str(x.value) for x in values))

        opts = {"sparkChartRangeMinX": str(xmin), "sparkChartRangeMaxX": str(xmax)}
        if valueRange:
            opts.update({
                "sparkChartRangeMin": str(min(values[-1].value, max(mean - 3 * std, minval))),
                "sparkChartRangeMax": str(max(values[-1].value, min(mean + 3 * std, maxval)))
            })
        if normalRange:
            opts.update({
                "sparkNormalRangeMin": str(mean - std),
                "sparkNormalRangeMax": str(mean + std)
            })

        with tag('span', klass=klass, **opts):
            doc.asis("<!-- " + datastr + " -->")
        return doc.getvalue()

    def imageShort(self, img):
        doc, tag, text = yattag.Doc().tagtext()

        with tag('div', klass="imagename"):
            text(img["image"])
        with tag('div', klass="imagediffpercent", title="Difference in percent"):
            text(f"Difference: {img['difference_percent']:3.2f}%")
        with tag('div', klass="imagespark"):
            doc.asis(self.sparkLine("image_test_diff." + img['image'], "sparkline_img_diff"))
        
        identicalSize = (img['ref_size'] == img['test_size'])
        identicalMode = (img['ref_mode'] == img['test_mode'])

        if identicalSize and identicalMode:
            with tag('div', klass="imagepixels", title="Number of different pixels"):
                if img['difference_pixels'] is not None:
                    text(f"# Pixels: {img['difference_pixels']}")
            with tag('div', klass="imagemaxdiff", title="Maximum difference per channel"):
                if img["max_differences"] is not None:
                    text(f"({', '.join([f'{x:0.3g}' for x in img['max_differences']])})")
        else:
            def resolutionToString(sizetuple):
                return f'({"x".join([str(x) for x in sizetuple])})'

            if not identicalSize:
                with tag('div', klass="imagenote", title="Image resolution mismatch (test / reference)"):
                    text(f"{resolutionToString(img['test_size'])} vs. {resolutionToString(img['ref_size'])}")
            if not identicalMode:
                with tag('div', klass="imagenote", title="Image format mismatch (test / reference)"):
                    text(f"{img['test_mode']} vs. {img['ref_mode']}")

        return doc.getvalue()

    def images(self, imgs, testdir):
        doc, tag, text = yattag.Doc().tagtext()

        def path(type, img):
            return os.path.relpath(toPath(testdir, type, img), self.basedir)

        def imgstatus(img, warnPercentage=10):
            # Return the status of the image. Either "fail", "ok", or "warning".
            # Warn if within warnPercentage % of the difference tolerance
            failures = safeget(self.report, 'failures', failure={})
            failedImgs = next((x for x in failures if x[0] == "images"), None)
            failed = (failedImgs is not None and img["image"] in failedImgs[1])

            if failed:
                return "fail"
            else:
                return "ok"

        with tag('ol'):
            for img in imgs:
                doc.asis(listItem(self.imageShort(img),
                                  testImages(path("imgtest", img["image"]),
                                             path("imgref", img["image"]),
                                             path("imgdiff", img["image"]),
                                             path("imgmask", img["image"])),
                                  status=imgstatus(img), hide=False))
        return doc.getvalue()

    def txtsItem(self, tests, regdir, testdir):
        doc, tag, text = yattag.Doc().tagtext()
        hd = difflib.HtmlDiff(tabsize=3)

        def status(key):
            failures = safeget(self.report, 'failures', failure={})
            fail = next((x for x in failures if x[0] == "txts"), None)
            return "ok" if fail is None else "fail"

        def diff(file):
            with open(toPath(testdir, "imgtest", file), 'r') as txtFile, \
                    open(toPath(regdir, file), 'r') as refFile:
                txtLines = txtFile.readlines()
                refLines = refFile.readlines()
            return hd.make_table(refLines, txtLines,
                                 fromdesc='reference', todesc='test',
                                 context=True, numlines=2)

        with tag('ol'):
            for test in tests:
                doc.asis(listItem(keyval(test["txt"], f"Differences {test['diff']}"),
                                  diff(test["txt"]),
                                  toggle=(test["diff"] > 0),
                                  status=("ok" if test["diff"] == 0 else "fail")))

        doc.asis(difflibLegend)
        return doc.getvalue()

    def timeSeries(self):
        doc, tag, text = yattag.Doc().tagtext()
        with tag('div'):
            with tag('span', klass="runtime"):
                text("{:3.2f}s".format(self.report["elapsed_time"]))
            doc.asis(self.sparkLine("elapsed_time", "sparkline_elapsed_time"))
        return doc.getvalue()

    def failureSeries(self, length=30):
        doc, tag, text = yattag.Doc().tagtext()
        with tag('div'):
            text("{:1d} ".format(len(self.report["failures"])) + " ")
            doc.asis(self.sparkLine("number_of_failures",
                                    "sparkline-failues", normalRange=False, valueRange=False))
        return doc.getvalue()

    def head(self):
        doc, tag, text = yattag.Doc().tagtext()
        with tag("div", klass="row"):
            with tag("div", klass="cell testmodule"):
                text(self.report["module"])
            with tag("div", klass="cell testname"):
                text(self.report["name"])
            with tag("div", klass="cell testfailures"):
                doc.asis(self.failureSeries())
            with tag("div", klass="cell testruntime"):
                doc.asis(self.timeSeries())
            with tag("div", klass="cell testdate"):
                text(stringToDate(self.report["date"]).strftime('%Y-%m-%d %H:%M:%S'))
        return doc.getvalue()

    def failureList(self):
        doc, tag, text = yattag.Doc().tagtext()
        with tag('ol'):
            for key, errors in self.report["failures"]:
                if isinstance(errors, dict):
                    items = errors.values()
                elif isinstance(errors, list):
                    items = errors
                elif isinstance(errors, str):
                    items = [errors]
                else:
                    raise RuntimeError("Invalid error type")

                for error in items:
                    toToggle, short = abrhtml(error)
                    doc.asis(listItem(keyval(formatKey(key), short),
                                      error, status="fail", toggle=toToggle))

        return doc.getvalue()

    def plots(self):
        def plotting(id):
            doc, tag, text = yattag.Doc().tagtext()
            doc.stag('div', klass='flot-plots', id=id)
            return doc.getvalue()

        return listItem(keyval("Plots", "..."), plotting(self.module + "/" + self.name))

    def failures(self):
        nfail = len(self.report["failures"])
        short = "No failues" if nfail == 0 else "{} failures".format(nfail)
        return listItem(keyval("Failures", short), self.failureList(),
                        status="ok" if nfail == 0 else "fail")


class HtmlReport:
    def __init__(self, basedir, reports, database, header=None, footer=None):
        self.doc, tag, text = yattag.Doc().tagtext()
        self.db = database
        self.basedir = basedir
        self.created = self.db.getLastRunDate()
        self.scriptDirname = "_scripts"
        self.scripts = ["jquery-3.6.0.min.js",
                        "clipboard.min.js",
                        "jquery.event.drag.js",
                        "jquery.canvaswrapper.js",
                        "jquery.colorhelpers.js",
                        "jquery.flot.js",
                        "jquery.flot.saturated.js",
                        "jquery.flot.browser.js",
                        "jquery.flot.drawSeries.js",
                        "jquery.flot.stack.js",
                        "jquery.flot.uiConstants.js",
                        "jquery.flot.axislabels.js",
                        "jquery.flot.time.js",
                        "jquery.flot.legend.js",
                        "jquery.flot.time.js",
                        "jquery.flot.hover.js",
                        "jquery.flot.selection.js",
                        "jquery.sparkline.min.js",
                        "jquery.zoom.js",
                        "list.min.js",
                        "main.js"]
        self.postScripts = ["make-list.js", "make-flot.js"]

        self.doc.asis("<!DOCTYPE html>")
        self.doc.stag("meta", charset="utf-8")

        with tag('html'):
            with tag('head'):
                self.doc.stag('link', rel='stylesheet', href="report.css")

                for script in self.scripts:
                    with tag('script', language="javascript",
                             src=self.scriptDirname + "/" + script):
                        text("")

            with tag('body'):
                if header is not None:
                    self.doc.asis(header)

                with tag('div', id='reportlist', klass='report'):
                    with tag("div"):
                        with tag('div', klass='titleimg'):
                            self.doc.stag('img', src="_images/inviwo.png")
                        with tag('div', klass='title'):
                            text("Inviwo Regressions")
                        self.doc.stag('input', klass='search', placeholder="Search")

                    timestamp = self.created.strftime('%Y-%m-%d %H:%M:%S')
                    with tag('div', klass='subtitle'):
                        with tag('div', klass="cell testdate"):
                            text(timestamp)

                        with tag('div', klass="cell"):
                            with tag('a', klass='version', href="report.html", title=timestamp):
                                text("latest")

                            oldreports = glob.glob(self.basedir + "/report-*.html")
                            oldreports.sort()
                            oldreports.reverse()

                            for i, old in enumerate(oldreports[:10]):
                                prev = os.path.relpath(old, self.basedir)
                                title = f'{old[-24:-14]} {old[-13:-5].replace("_", ":")}'
                                with tag('a', klass='version', href=prev, title=title):
                                    text("-" + str(i + 1))

                    with tag('div', klass='git'):
                        with tag('div', klass='box boxheader'):
                            with tag('span', klass='gitserver'):
                                text('Repository')
                            with tag('span', klass='gitdate'):
                                text('Date')
                            with tag('span', klass='gitcommit'):
                                text('Commit')
                            with tag('span', klass='gitauthor'):
                                text('Author')
                            with tag('span', klass='gitmsgheader'):
                                text('Message')

                        for commit in self.db.getLatestCommits():
                            commitHeader, _, _ = commitSplitMessage(commit.message)
                            with tag('div', klass='box'):
                                with tag('span', klass='gitserver'):
                                    self.doc.asis(gitServerName(commit))
                                with tag('span', klass='gitdate'):
                                    text(commit.date.strftime('%Y-%m-%d %H:%M:%S'))
                                with tag('span', klass='gitcommit'):
                                    self.doc.asis(gitLink(commit))
                                with tag('span', klass='gitauthor'):
                                    text(commit.author)
                                with tag('span', klass='gitmsgheader'):
                                    text(commitHeader)

                    with tag('div', klass='summary'):
                        self.doc.stag('div', style="width:800px;height:200px", id='flot-summary')

                    with tag("div", klass="head"):
                        with tag("div", klass="cell testmodule"):
                            with tag('button', ('data-sort', 'testmodule'), klass='sort'):
                                text("Module")
                        with tag("div", klass="cell testname"):
                            with tag('button', ('data-sort', 'testname'), klass='sort'):
                                text("Name")
                        with tag("div", klass="cell testfailures"):
                            with tag('button', ('data-sort', 'testfailures'), klass='sort'):
                                text("Failures")
                        with tag("div", klass="cell testruntime"):
                            with tag('button', ('data-sort', 'testruntime'), klass='sort'):
                                text("Run Time")
                        with tag("div", klass="cell testdate"):
                            with tag('button', ('data-sort', 'testdate'), klass='sort'):
                                text("Last Run")

                    with tag('ul', klass='list'):
                        for name, report in reports.items():
                            tr = TestRun(self, report)
                            self.doc.asis(tr.getvalue())

                if footer is not None:
                    self.doc.asis(footer)

                with tag('script', language="javascript",
                         src=self.scriptDirname + "/plotdata.js"):
                    text("")
                for script in self.postScripts:
                    with tag('script', language="javascript",
                             src=self.scriptDirname + "/" + script):
                        text("")

    def saveScripts(self):
        scriptdir = toPath(self.basedir, self.scriptDirname)
        mkdir(scriptdir)
        for script in (self.scripts + self.postScripts):
            scriptdata = pkgutil.get_data('ivwpy', 'regression/resources/' + script)
            with open(toPath(scriptdir, script), 'wb') as f:
                f.write(scriptdata)

        with open(toPath(scriptdir, "plotdata.js"), 'w') as f:
            runtimedata = elapsed_time_stats(self.db)
            print("var summarydata = " + dataToJsArray([[x.timestamp() * 1000, y]
                                                        for x, y in runtimedata]),
                  file=f)

            resulttimedata = result_time_stats(self.db)

            def makestep(a, b):
                x1 = a[0].timestamp() * 1000
                x2 = b[0].timestamp() * 1000
                xm = (x1 + x2) / 2
                y1 = a[1]
                y2 = b[1]
                return [[x1, y1], [xm, y1], [xm, y2]]

            resulttimedata = list(addMidSteps(makestep, iter(resulttimedata),
                                              transform=lambda x: [x[0].timestamp() * 1000, x[1]]))

            print("var passdata = " + dataToJsArray([[x, y[0]] for x, y in resulttimedata]), file=f)
            print("var faildata = " + dataToJsArray([[x, y[1]] for x, y in resulttimedata]), file=f)
            print("var skipdata = " + dataToJsArray([[x, y[2]] for x, y in resulttimedata]), file=f)

            plotdata = get_plot_data(self.db)
            print("var plotdata = " + json.dumps(plotdata), file=f)

    def saveHtml(self, filename):
        file = self.basedir + "/" + filename + ".html"

        self.saveScripts()

        imgdata = pkgutil.get_data('ivwpy', 'regression/resources/inviwo.png')
        imgdir = mkdir(self.basedir, "_images")
        with open(toPath(imgdir, "inviwo.png"), 'wb') as f:
            f.write(imgdata)

        cssdata = pkgutil.get_data('ivwpy', 'regression/resources/report.css')
        with open(toPath(self.basedir, "report.css"), 'w') as f:
            f.write(lesscpy.compile(io.StringIO(cssdata.decode("utf-8"))))

        with open(file, 'w') as f:
            f.write(yattag.indent(self.doc.getvalue()))

        return file

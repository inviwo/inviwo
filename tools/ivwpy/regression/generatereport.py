#*********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
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

import yattag # http://www.yattag.org
import sys
import os

from .. util import *

class HtmlReport:
	def __init__(self, reports, relto = None):
		self.doc, tag, text = yattag.Doc().tagtext()
		self.style = self.reportStyle()
		self.relto = relto

		self.doc.asis("<!DOCTYPE html>")
		self.doc.stag("meta", charset = "utf-8")
		
		with tag('html'):
			with tag('head'):
				with tag('style'):
					text(self.style)

				with tag('script',language="javascript", type="text/javascript", 
					src="https://code.jquery.com/jquery-2.2.0.min.js"): text("")

				with tag('script',language="javascript", type="text/javascript"):
					self.doc.asis(self.reportScrips())

			with tag('body'):
				with tag('h1'):
					text("Regressions")
				with tag('dl'):
					for report in reports:
						ok = len(report['failures']) == 0
						self.doc.asis(
							dd(("OK " if ok else "Fail ") + report['group'] + "/" + report['name'],
								self.reportToHtml(report),
								toggle = True,
								status = ("ok" if ok else "fail")
							))

	def reportStyle(self):
		css = {
		  "dl": {	   
		    "overflow": "hidden",
		    "padding": "0",
		    "margin": "0"
		  },
		  "dt": {
		    "font-weight": "bold"
		  },
		  "dt.test": {
		    "font-weight": "bold"
		  },
		  ".ok": {
		    "background-color": "#ddffdd"
		  },
		  ".fail": {
		    "background-color": "#ffdddd"
		  },
		  "dt.toggle": {
		    "cursor": "pointer"
		  },
		  "dt.toggle:hover" : {
		    "color": "#555555"
		  }
		}
		return dict2css(css)

	def reportToHtml(self, report):
		doc, tag, text = yattag.Doc().tagtext()

		with tag('dl'):
			keys = [
				["date"         , {"toggle" : False}],
				["failures"     , {"toggle" : False}],
				["path"         , {"toggle" : False}],
				["elapsed_time" , {"toggle" : False}],
				["command"      , {"toggle" : True}],
				["returncode"   , {"toggle" : False}],
				["missing_imgs" , {"toggle" : False}],
				["missing_refs" , {"toggle" : False}],
				["output"       , {"toggle" : True }],
				["errors"       , {"toggle" : True }]
			]

			for key, opts in keys:
				val = toString(report[key])
				if key in report['successes']: opts["status"] = "ok"
				if key in [x[0] for x in report['failures']]: opts["status"] = "fail"

				doc.asis(dd(formatKey(key), val, abr(val), **opts))
							
			with open(report['log'], 'r') as f:
				loghtml = f.read()
				err = loghtml.count("Error:")
				warn = loghtml.count("Warn:")
				info = loghtml.count("Info:")

				short = "Error: {}, Warnings: {}, Information: {}".format(err, warn, info)
				doc.asis(dd("Log", loghtml, short, toggle=True,
					status = "ok" if err == 0 else "fail")) 

			doc.asis(dd("Screenshot", image(report["screenshot"], "Screenshot"), "...", toggle=True))	

			ok = sum([1 if img["difference"] == 0.0 else 0 for img in report["image_tests"]])
			fail = sum([1 if img["difference"] != 0.0 else 0 for img in report["image_tests"]])
			short = (str(ok) + " ok images, " + str(fail) + " failed image tests")
			doc.asis(dd("Images", 
				genImages(report["image_tests"], report["outputdir"], report["path"]),
				short, toggle=True, status = "ok" if fail == 0 else "fail"))

		return doc.getvalue()

	def getHtml(self):
		return yattag.indent(self.doc.getvalue())


	def reportScrips(self):
		return """
$(document).ready(function() {
   	$('dt.toggle').click(function() {
   			if($(this).next().children("div.longform").is(':visible')) {
   				$(this).next().children("div.longform").hide()
   			}else {
           		$(this).next().children("div.longform").fadeToggle(500)
           	}

           	if($(this).next().children("div.shortform").is(':visible')) {
   				$(this).next().children("div.shortform").hide()
   			}else {
           		$(this).next().children("div.shortform").fadeToggle(500)
           	}
    });
	$('div.longform').hide()
	$('div.shortform').show()
 });
"""


def toString(val):
	if val is None:
		return "None"
	if isinstance(val, str):
		if len(val) == 0: return "None"
		else: return val
	elif isinstance(val, list):
		if len(val) == 0: return "None"
		else: return "(" + ", ".join(map(toString, val)) +")"
	elif isinstance(val, float):
		return "{:6f}".format(val)
	elif isinstance(val, int):
		return "{:}".format(val)

def abr(text):
	abr = text.split("\n")[0][:50]
	return abr + ("..." if len(text.split("\n")) > 1 or len(text) > 50 else "")

def formatKey(key):
	return key.capitalize().replace("_", " ")

def image(path, alt = ""):
	doc, tag, text = yattag.Doc().tagtext()
	doc.stag('img', src = "file://" + os.path.abspath(path), alt = alt)
	return doc.getvalue()

def dd(name, content, alt = "", status="", toggle = False):
	doc, tag, text = yattag.Doc().tagtext()
	tc = "toggle" if toggle else ""
	with tag('dt', klass = tc + " " + status): 
		text(name)
	with tag('dd'):
		if toggle:
			with tag('div', klass = "shortform"):
				doc.asis(alt)
			with tag('div', klass = "longform"):
				doc.asis(content)
		else:
			doc.asis(content)

	return doc.getvalue()

def testImages(testimg, refimg):
	doc, tag, text = yattag.Doc().tagtext()
	doc.asis(image(testimg, "test image"))
	doc.asis(image(refimg, "reference image"))
	return doc.getvalue()

def genImages(imgs, testdir, refdir):
	doc, tag, text = yattag.Doc().tagtext()
	with tag('dl'):
		for img in imgs:
			ok = img["difference"] == 0.0
			doc.asis(dd("{} Diff: {:3.3f}% {}".format("Ok" if ok else "Fail",img["difference"], img["image"]),
				testImages(toPath([testdir, img["image"]]), toPath([refdir, img["image"]])),
				toggle = True,
				status = "ok" if ok else "fail"
				))
	return doc.getvalue()
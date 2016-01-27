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

import sys
import os
import pkgutil
import glob
import datetime

# Yattag for HTML generation, http://www.yattag.org
import yattag 

from .. util import *
from . database import *

# Javascript packages
# jQuary			 http://jquery.com
# jQuery Zoom        http://www.jacklmoore.com/zoom/
# jQuery Sparklines  http://omnipotent.net/jquery.sparkline/
# List.js            http://www.listjs.com/

# Jenkins note: https://wiki.jenkins-ci.org/display/JENKINS/Configuring+Content+Security+Policy
# System.setProperty("hudson.model.DirectoryBrowserSupport.CSP", "default-src 'self';script-src 'self'")

class HtmlReport:
	def __init__(self, basedir, reports, dbfile):
		self.doc, tag, text = yattag.Doc().tagtext()
		self.db = Database(dbfile)
		self.basedir = basedir
		self.created = datetime.datetime.now()
		self.scriptDirname = "_scripts"
		self.scripts = ["jquery-2.2.0.min.js", 
						"jquery.sparkline.min.js", 
						"jquery.zoom.min.js", 
						"list.min.js",
						"make-list.js", 
						"main.js"]
		self.history_days = 1


		self.doc.asis("<!DOCTYPE html>")
		self.doc.stag("meta", charset = "utf-8")
		
		with tag('html'):
			with tag('head'):
				self.doc.stag('link', rel='stylesheet', href="report.css")

				for script in self.scripts:
					with tag('script', language="javascript", 
					src = self.scriptDirname + "/" + script): text("")

			with tag('body'):
				with tag('div', id='reportlist'):
					with tag("div"):
						with tag('div', klass='titleimg'):
							self.doc.stag('img', src= "_images/inviwo.png")
						with tag('div', klass='title'):
							text("Inviwo Regressions")
						self.doc.stag('input', klass='search', placeholder="Search")
					
					with tag('div', klass='subtitle'):
						with tag('div', klass="cell testdate"):
							text(self.created.strftime('%Y-%m-%d %H:%M:%S'))

						with tag('div', klass="cell"):
							with tag('a', klass='version', href="report.html"):
								text("latest")

							oldreports = glob.glob(self.basedir + "/report-*.html")
							oldreports.reverse()
					
							for i,old in enumerate(oldreports[:10]):
								prev = os.path.relpath(old, self.basedir)
								with tag('a', klass='version', href=prev):
									text("-"+str(i+1))

					with tag("div", klass = "head"):
						with tag("div", klass = "cell testmodule"):
							with tag('button', ('data-sort','testmodule'), klass='sort'):
								text("Module")
						with tag("div", klass = "cell testname"):
							with tag('button', ('data-sort', 'testname'), klass='sort'):
								text("Name")
						with tag("div", klass = "cell testfailures"):
							with tag('button', ('data-sort', 'testfailures'), klass='sort'):
								text("Failures")
						with tag("div", klass = "cell testruntime"):
							with tag('button', ('data-sort', 'testruntime'), klass='sort'):
								text("Run Time")
						with tag("div", klass = "cell testdate"):
							with tag('button', ('data-sort', 'testdate'), klass='sort'):
								text("Last Run")

					with tag('ul', klass='list'):
						for name, report in reports.items():
							ok = ("ok" if len(report['failures']) == 0 else "fail")
							self.doc.asis(li(self.testhead(report), self.reportToHtml(report), status = ok))

				with tag('script', language="javascript", 
					src = self.scriptDirname + "/make-list.js"): text("")
					

	def makeSparkLine(self, module, name, series, klass, normalRange = True):
		doc, tag, text = yattag.Doc().tagtext()
		data = self.db.getSeries(module, name, series)

		xmax = self.created.timestamp()
		xmin = xmax - 60*60*24*self.history_days
		mean, std = stats([x.value for x in data.measurements])

		datastr = ", ".join([str(x.created.timestamp()) + ":" + str(x.value) 
			for x in data.measurements if x.created.timestamp() > xmin])

		opts = {"sparkChartRangeMinX" : str(xmin), "sparkChartRangeMaxX" : str(xmax)}
		if normalRange:
			opts.update({
				"sparkNormalRangeMin" : str(mean-std), 
				"sparkNormalRangeMax" : str(mean+std)
			})
		with tag('span', klass=klass, **opts ): text(datastr)
		return doc.getvalue()

				
	def timeSeries(self, report):
		doc, tag, text = yattag.Doc().tagtext()
		with tag('div'):
			text("{:3.2f}s ".format(report["elapsed_time"]))
			doc.asis(self.makeSparkLine(report["module"], report["name"], "elapsed_time", 
						                "sparkline_elapsed_time"))
		return doc.getvalue()


	def failueSeries(self, report, length = 30):
		doc, tag, text = yattag.Doc().tagtext()
		nfail = len(report["failures"])	
		with tag('div'):
			text(str(nfail) + " ")
			doc.asis(self.makeSparkLine(report["module"], report["name"], "number_of_test_failures", 
						                "sparkline-failues", normalRange = False))
		return doc.getvalue()

	def testhead(self, report):
		doc, tag, text = yattag.Doc().tagtext()
		with tag("div", klass = "row"):
			with tag("div", klass = "cell testmodule"):
				text(report["module"])
			with tag("div", klass = "cell testname"):
				text(report["name"])
			with tag("div", klass = "cell testfailures"):
				doc.asis(self.failueSeries(report))
			with tag("div", klass = "cell testruntime"):
				doc.asis(self.timeSeries(report))
			with tag("div", klass = "cell testdate"):
				text(stringToDate(report["date"]).strftime('%Y-%m-%d %H:%M:%S'))
		return doc.getvalue()

	def imageShort(self, module, name, img):
		doc, tag, text = yattag.Doc().tagtext()
	
		with tag('div', klass="cell imagename"):
			text(img["image"])
		with tag('div', klass="cell imageinfo"):
			text("Diff: {:3.8f}%".format(img["difference"]))
		with tag('div', klass="cell imageinfo"):
			doc.asis(self.makeSparkLine(module, name , "image_test_diff." + img["image"], 
					                    "sparkline_img_diff"))
		return doc.getvalue()

	def genImages(self, module, name, imgs, testdir, refdir):
		doc, tag, text = yattag.Doc().tagtext()
		with tag('ol'):
			for img in imgs:
				ok = img["difference"] == 0.0					
				doc.asis(li(self.imageShort(module, name, img),
					testImages(os.path.relpath(toPath(testdir, "imgtest", img["image"]), self.basedir),
							   os.path.relpath(toPath(testdir, "imgref", img["image"]), self.basedir),
							   os.path.relpath(toPath(testdir, "imgdiff", img["image"]), self.basedir),
							   os.path.relpath(toPath(testdir, "imgmask", img["image"]), self.basedir)),
					status = "ok" if ok else "fail"
					))
		return doc.getvalue()

	def genGit(self, git):
		doc, tag, text = yattag.Doc().tagtext()
		with tag('ul'):
			doc.asis(li(keyval("Author", git["author"]), toggle=False))
			gdate = stringToDate(git["date"]).strftime('%Y-%m-%d %H:%M:%S')
			doc.asis(li(keyval("Date", gdate), toggle=False))
			doc.asis(li(keyval("Commit", git["commit"]), toggle=False))
			val = git["message"]
			vabr = abr(val)
			doc.asis(li(keyval("Message", vabr), val, toggle = vabr != val))
		return doc.getvalue()

	def formatLog(self, htmllog):
		doc, tag, text = yattag.Doc().tagtext()
		with tag('div', klass='log'):
			doc.asis(htmllog)
		return doc.getvalue()

	def reportToHtml(self, report):
		doc, tag, text = yattag.Doc().tagtext()

		with tag('ul'):
			keys = ["path", "command", "returncode", "missing_imgs", "missing_refs", "output", "errors"]

			for key in keys:
				val = toString(report[key])
				status = ""
				if key in report['successes']: status = "ok"
				if key in report['failures'].keys(): status = "fail"
				vabr = abr(val)
				doc.asis(li(keyval(formatKey(key), vabr), val, status = status, toggle = vabr != val))

			shortgit = safeget(report, "git", "message", failure ="")
			doc.asis(li(keyval("Git", shortgit), self.genGit(report["git"])))
			

			nfail = len(report["failures"])
			short = "No failues" if nfail == 0 else  "{} failures".format(nfail)
			doc.asis(li(keyval("Failures", short), genFailures(report["failures"]), 
				status = "ok" if nfail == 0 else "fail"))
							
			with open(toPath(report['outputdir'], report['log']), 'r') as f:
				loghtml = f.read()
				err = loghtml.count("Error:")
				warn = loghtml.count("Warn:")
				info = loghtml.count("Info:")

				short = "Error: {}, Warnings: {}, Information: {}".format(err, warn, info)
				doc.asis(li(keyval("Log", short), self.formatLog(loghtml), 
					status = "ok" if err == 0 else "fail")) 

			doc.asis(li(keyval("Screenshot", ""), 
				image(os.path.relpath(toPath(report['outputdir'], report["screenshot"]), self.basedir), 
					alt = "Screenshot", width="100%")))	

			ok = sum([1 if img["difference"] == 0.0 else 0 for img in report["image_tests"]])
			fail = sum([1 if img["difference"] != 0.0 else 0 for img in report["image_tests"]])
			short = (str(ok) + " ok images, " + str(fail) + " failed image tests")
			doc.asis(li(keyval("Images", short), 
				self.genImages(report["module"], report["name"], report["image_tests"], report["outputdir"], report["path"]),
				status = "ok" if fail == 0 else "fail"))

		return doc.getvalue()


	def saveScripts(self):
		scriptdir = toPath(self.basedir, self.scriptDirname)
		mkdir(scriptdir)
		for script in self.scripts:
			scriptdata = pkgutil.get_data('ivwpy', 'regression/resources/' + script)
			with open(toPath(scriptdir, script), 'wb') as f:
				f.write(scriptdata)

	def saveHtml(self, file):
		self.saveScripts()

		imgdata = pkgutil.get_data('ivwpy', 'regression/resources/inviwo.png')
		imgdir = mkdir(self.basedir, "_images")
		with open(toPath(imgdir, "inviwo.png"), 'wb') as f:
			f.write(imgdata)

		cssdata = pkgutil.get_data('ivwpy', 'regression/resources/report.css')
		with open(toPath(self.basedir, "report.css"), 'wb') as f:
			f.write(cssdata)

		with open(file, 'w') as f:
			f.write(yattag.indent(self.doc.getvalue())) 
			
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
	abr = text.split("\n")[0][:85]
	return abr + ("..." if len(text.split("\n")) > 1 or len(text) > 50 else "")

def formatKey(key):
	return key.capitalize().replace("_", " ")

def keyval(key, val):
	doc, tag, text = yattag.Doc().tagtext()
	with tag("div", klass = "row"):
		with tag("div", klass = "cell key"):
			text(key)
		with tag("div", klass = "cell"):
			doc.asis(val)
	return doc.getvalue()

def image(path, **opts):
	doc, tag, text = yattag.Doc().tagtext()
	with tag('div', klass='zoom'):
		doc.stag('img', src = path, **opts)
	return doc.getvalue()

def li(head, body="", status="", toggle = True):
	doc, tag, text = yattag.Doc().tagtext()
	toggleclass = "toggle" if toggle else ""
	with tag('li', klass='row'):
		with tag('div', klass='lihead ' + status + ' ' + toggleclass):
			doc.asis(head)
		if toggle:
			with tag('div', klass='libody'):
				doc.asis(body)

	return doc.getvalue()

def testImages(testimg, refimg, diffimg, maskimg):
	doc, tag, text = yattag.Doc().tagtext()
	with tag('table'):
		with tag('tr'):
			with tag('th'): text("Test")
			with tag('th'): text("Reference")
			with tag('th'): text("Difference * 10") 
			with tag('th'): text("Mask") 
		with tag('tr'):
			with tag('td'):
				doc.asis(image(testimg, alt = "test image", klass ="test"))
			with tag('td'):
				doc.asis(image(refimg, alt = "reference image", klass ="test"))
			with tag('td'):
				doc.asis(image(diffimg, alt = "difference image", klass ="diff"))
			with tag('td'):
				doc.asis(image(maskimg, alt = "mask image", klass ="diff"))
	return doc.getvalue()

def failureList(errors):
	doc, tag, text = yattag.Doc().tagtext()
	with tag('ul'):
		for error in errors:
			with tag('li'):
				text(error)
	return doc.getvalue()

def genFailures(failures):
	doc, tag, text = yattag.Doc().tagtext()
	with tag('ol'):
		for key, errors in failures.items():
			doc.asis(li(formatKey(key), failureList(errors), status = "fail"))
	return doc.getvalue()





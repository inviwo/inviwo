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
import io
import pkgutil
import glob
import datetime
import contextlib
import lesscpy

# Beautiful Soup 4 for dom manipulation
import bs4

# Yattag for HTML generation, http://www.yattag.org
import yattag 

from .. util import *
from . database import *

# Javascript packages
# jQuary			 http://jquery.com
# wheelzoom          http://www.jacklmoore.com/wheelzoom
# jQuery Sparklines  http://omnipotent.net/jquery.sparkline/
# List.js            http://www.listjs.com/

# Jenkins note: https://wiki.jenkins-ci.org/display/JENKINS/Configuring+Content+Security+Policy
# System.setProperty("hudson.model.DirectoryBrowserSupport.CSP", "default-src 'self';script-src 'self';style-src 'self' 'unsafe-inline';")
# added to /etc/default/jenkins


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
		if var is None or var is "" or not isinstance(var, str):
			return False
	return True

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

def listItem(head, body="", status="", toggle = True, hide = True):
	doc, tag, text = yattag.Doc().tagtext()
	toggleclass = "toggle" if toggle else ""
	opts = {"style" : "display: none;"} if hide else {}  
	with tag('li', klass='row'):
		with tag('div', klass='lihead ' + status + ' ' + toggleclass):
			doc.asis(head)
		if toggle:
			with tag('div', klass='libody', **opts):
				if callable(body): body(doc, tag, text)
				else: doc.asis(body)

	return doc.getvalue()

def gitLink(commit):
	doc, tag, text = yattag.Doc().tagtext()

	if isValidString(commit.server, commit.hash):
		with tag('a', href = commit.server + "/commit/"+ commit.hash):
			text(commit.server + "/commit/"+ commit.hash)
	else:
		text("None")
	return doc.getvalue()

def getDiffLink(start, stop):
	doc, tag, text = yattag.Doc().tagtext()
	if isValidString(start.server, start.hash, stop.hash):
		with tag('a', href = start.server + "/compare/"+ start.hash + "..." + stop.hash):
			text(start.server + "/compare/"+ start.hash + "..." + stop.hash)
	else:
		text("None")
	return doc.getvalue()

def commitInfo(commit):
	doc, tag, text = yattag.Doc().tagtext()
	with tag('ul'):
		val = commit.message
		vabr = abr(val)
		gdate = commit.date.strftime('%Y-%m-%d %H:%M:%S')

		doc.asis(listItem(keyval("Message", vabr), val, toggle = vabr != val))
		doc.asis(listItem(keyval("Author", commit.author), toggle=False))
		doc.asis(listItem(keyval("Date", gdate), toggle=False)) 
		doc.asis(listItem(keyval("Repository", gitLink(commit)), toggle=False)) 
	
	return doc.getvalue()


def formatLog(file):
	doc, tag, text = yattag.Doc().tagtext()
	with open(file, 'r') as f:
		loghtml = f.read()
		err  = loghtml.count("Error:")
		warn = loghtml.count("Warn:")
		info = loghtml.count("Info:")
		short = "Error: {}, Warnings: {}, Information: {}".format(err, warn, info)
				
		def log(doc, tag, text): 
			with tag('div', klass='log'): 
				doc.asis(loghtml)

		doc.asis(listItem(keyval("Log", short), log, status = "ok" if err == 0 else "fail")) 
	return doc.getvalue()

def image(path, **opts):
	doc, tag, text = yattag.Doc().tagtext()
	doc.stag('img', src = path, **opts)
	return doc.getvalue()

def testImages(testimg, refimg, diffimg, maskimg):
	doc, tag, text = yattag.Doc().tagtext()

	with tag('div', klass="zoomset"):
		with tag('div', klass="slider"):
			with tag('div', klass="imgtestlabel"): text("Test")
			with tag('div', klass="imgtestlabel"): text("Reference")
			with tag('div', klass="imgtestlabel"): text("Difference * 10") 
			with tag('div', klass="imgtestlabel"): text("Mask") 
		with tag('div', klass="slider"):
			with tag('div', klass ="zoom"):
				doc.asis(image(testimg, alt = "test image", klass ="test"))
			with tag('div', klass ="zoom"):
				doc.asis(image(refimg,  alt = "reference image", klass ="test"))
			with tag('div', klass ="zoom"):
				doc.asis(image(diffimg, alt = "difference image", klass ="diff"))
			with tag('div', klass ="zoom"):
				doc.asis(image(maskimg, alt = "mask image", klass ="diff"))
	return doc.getvalue()

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

		with self.item(self.head(), status = self.totalstatus()):
			with self.tag('ul'):

				failedImgs = safeget(self.report, 'failures', 'image_tests', failure = {})
				fail = len(failedImgs)
				short = "All OK" if fail == 0 else "{:d}/{:d} Images failed".format(fail, len(report["image_tests"]))
				self.doc.asis(listItem(keyval("Images", short), 
					self.images(report["image_tests"], report["outputdir"]),
					status = self.status('image_tests')))

				self.testRunInfo("Current Version", testrun)
				if self.totalstatus() != "ok":
					self.testRunInfo("Last Succsess", lastSuccess)
					self.testRunInfo("First Failure", firstFailure)
					self.gitDiff(lastSuccess, firstFailure)
				
				self.doc.asis(formatLog(toPath(report['outputdir'], report['log'])))
				self.doc.asis(self.screenshot())

				for key in ["path", "command", "returncode", "missing_imgs", 
							"missing_refs", "output", "errors"]:
					self.simple(key)

				self.doc.asis(self.failures())


	def getvalue(self):
		return self.doc.getvalue()

	@contextlib.contextmanager
	def item(self, head, status="", hide = True, doc = None):
		if doc is None: doc = self.doc
		opts = {"style" : "display: none;"} if hide else {}  
		with doc.tag('li', klass='row'):
			with doc.tag('div', klass='lihead toggle ' + status):
				doc.asis(head)
			with doc.tag('div', klass='libody', **opts):
				yield None

	def totalstatus(self):
		if not safeget(self.report, "config", "enabled", failure = True):
			return "disabled"
		return ("ok" if len(self.report['failures']) == 0 else "fail")

	def status(self, key):
		status = ""
		if key in self.report['successes']: status = "ok"
		if key in self.report['failures'].keys(): status = "fail"
		return status

	def simple(self, key):
		value = toString(self.report[key])
		short = abr(value)
		self.doc.asis(listItem(keyval(formatKey(key), short), value, 
			                   status = self.status(key), 
			             	   toggle = short != value))

	def testRunInfo(self, key, testrun):
		if testrun is not None:
			date = testrun.commit.date.strftime('%Y-%m-%d %H:%M:%S')
			self.doc.asis(listItem(keyval(key, date + " " + abr(testrun.commit.message, 50)),
				commitInfo(testrun.commit)))
		else:
			self.doc.asis(listItem(keyval(key, "None"), toggle=False)) 	

	def gitDiff(self, lastSuccess, firstFailure):
		if (lastSuccess is not None) and (firstFailure is not None):
			self.doc.asis(listItem(keyval("Diff", 
				getDiffLink(lastSuccess.commit, firstFailure.commit)), toggle = False))

	def sparkLine(self, series, klass, normalRange = True):
		doc, tag, text = yattag.Doc().tagtext()
		data = self.db.getSeries(self.module, self.name, series)
		xmax = self.created.timestamp()

		xmin = xmax - 60*60*24*self.history_days
		if xmin < data.created.timestamp():
			xmin = data.created.timestamp()

		mean, std = stats([x.value for x in data.measurements])

		datastr = ", ".join([str(x.created.timestamp()) + ":" + str(x.value) 
			for x in data.measurements if x.created.timestamp() > xmin])

		opts = { "sparkChartRangeMinX" : str(xmin), "sparkChartRangeMaxX" : str(xmax)}
		if normalRange:
			opts.update({
				"sparkNormalRangeMin" : str(mean-std), 
				"sparkNormalRangeMax" : str(mean+std)
			})

		with tag('span', klass=klass, **opts ): doc.asis("<!-- " + datastr + " -->")
		return doc.getvalue()

	def imageShort(self, img):
		doc, tag, text = yattag.Doc().tagtext()
	

		with tag('div', klass="imagename"):
			text(img["image"])
		with tag('div', klass="imagedifference"):
			text("Difference: {:2.8f}%".format(img["difference"]))
		with tag('div', klass="imagespark"):
			doc.asis(self.sparkLine("image_test_diff." + img["image"], "sparkline_img_diff"))
		with tag('div', klass="imagepixels"):
			if img["different_pixels"] is not None:
				text("Pixels: {:d}".format(img["different_pixels"]))
		with tag('div', klass="imagepercent"):
			if img["different_pixels"] is not None:
				text("({:2.4f}%)".format(100.0 * img["different_pixels"] /\
					(img['test_size'][0] * img['test_size'][1])))
		with tag('div', klass="imagedelta"):
			if img["max_difference"] is not None:
				text("Largest delta: {:1.8f}".format(img["max_difference"]))
		return doc.getvalue()

	def images(self, imgs, testdir):
		doc, tag, text = yattag.Doc().tagtext()

		def path(type, img):
			return os.path.relpath(toPath(testdir, type, img), self.basedir)

		def imgstatus(key):
			failedImgs = safeget(self.report, 'failures', 'image_tests', failure = {})
			return "fail" if key in failedImgs.keys() else "ok"

		with tag('ol'):
			for img in imgs:
				doc.asis(listItem(self.imageShort(img),
					testImages(path("imgtest", img["image"]), path("imgref", img["image"]),
							   path("imgdiff", img["image"]), path("imgmask", img["image"])),
					status = imgstatus(img['image']), hide=False))
		return doc.getvalue()

	def screenshot(self):
		return listItem(keyval("Screenshot", "..."), 
				image(os.path.relpath(toPath(self.report['outputdir'], self.report["screenshot"]), self.basedir), 
					alt = "Screenshot", width="100%"))	

	def timeSeries(self):
		doc, tag, text = yattag.Doc().tagtext()
		with tag('div'):
			with tag('span', klass="runtime"):
				text("{:3.2f}s".format(self.report["elapsed_time"]))
			doc.asis(self.sparkLine("elapsed_time", "sparkline_elapsed_time"))
		return doc.getvalue()


	def failueSeries(self, length = 30):
		doc, tag, text = yattag.Doc().tagtext()
		with tag('div'):
			text("{:1d} ".format(len(self.report["failures"])) + " ")
			doc.asis(self.sparkLine("number_of_test_failures", 
						            "sparkline-failues", normalRange = False))
		return doc.getvalue()

	def head(self):
		doc, tag, text = yattag.Doc().tagtext()
		with tag("div", klass = "row"):
			with tag("div", klass = "cell testmodule"):
				text(self.report["module"])
			with tag("div", klass = "cell testname"):
				text(self.report["name"])
			with tag("div", klass = "cell testfailures"):
				doc.asis(self.failueSeries())
			with tag("div", klass = "cell testruntime"):
				doc.asis(self.timeSeries())
			with tag("div", klass = "cell testdate"):
				text(stringToDate(self.report["date"]).strftime('%Y-%m-%d %H:%M:%S'))
		return doc.getvalue()


	def failureList(self):
		doc, tag, text = yattag.Doc().tagtext()
		with tag('ol'):
			for key, errors in self.report["failures"].items():
				if isinstance(errors, dict):
					items = errors.values()
				else:
					items = errors
				for error in items:
					toToggle, short = abrhtml(error)
					doc.asis(listItem(keyval(formatKey(key), short), error, status = "fail", toggle=toToggle))

		return doc.getvalue()

	def failures(self):
		nfail = len(self.report["failures"])
		short = "No failues" if nfail == 0 else  "{} failures".format(nfail)
		return listItem(keyval("Failures", short), self.failureList(), 
						status = "ok" if nfail == 0 else "fail")

class HtmlReport:
	def __init__(self, basedir, reports, database):
		self.doc, tag, text = yattag.Doc().tagtext()
		self.db = database
		self.basedir = basedir
		self.created = datetime.datetime.now()
		self.scriptDirname = "_scripts"
		self.scripts = ["jquery-2.2.0.min.js", 
						"jquery.sparkline.min.js", 
						"jquery.zoom.js", 
						"list.min.js",
						"make-list.js", 
						"main.js"]
		
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
							oldreports.sort()
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
							tr = TestRun(self, report)
							self.doc.asis(tr.getvalue())

				with tag('script', language="javascript", 
					src = self.scriptDirname + "/make-list.js"): text("")
					


	def saveScripts(self):
		scriptdir = toPath(self.basedir, self.scriptDirname)
		mkdir(scriptdir)
		for script in self.scripts:
			scriptdata = pkgutil.get_data('ivwpy', 'regression/resources/' + script)
			with open(toPath(scriptdir, script), 'wb') as f:
				f.write(scriptdata)

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
			



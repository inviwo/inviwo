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

# javascript http://omnipotent.net/jquery.sparkline/

import yattag # http://www.yattag.org
import sys
import os

from .. util import *
from . database import *

# Javascript packages
# jQuery Zoom        http://www.jacklmoore.com/zoom/
# jQuery Sparklines  http://omnipotent.net/jquery.sparkline/
# List.js            http://www.listjs.com/

class HtmlReport:
	def __init__(self, reports, dbfile):
		self.doc, tag, text = yattag.Doc().tagtext()
		self.style = self.reportStyle()
		self.db = Database(dbfile)

		self.doc.asis("<!DOCTYPE html>")
		self.doc.stag("meta", charset = "utf-8")
		
		with tag('html'):
			with tag('head'):
				with tag('style'):
					text(self.style)

				with tag('script',language="javascript", type="text/javascript", 
					src="https://code.jquery.com/jquery-2.2.0.min.js"): text("")

				with tag('script',language="javascript", type="text/javascript", 
					src="http://omnipotent.net/jquery.sparkline/2.1.2/jquery.sparkline.min.js"): text("")

				with tag('script',language="javascript", type="text/javascript", 
					src="https://raw.githubusercontent.com/jackmoore/zoom/master/jquery.zoom.min.js"): text("")

				with tag('script',language="javascript", type="text/javascript", 
					src="http://listjs.com/no-cdn/list.js"): text("")

				with tag('script',language="javascript", type="text/javascript"):
					self.doc.asis(self.reportScrips())

			with tag('body'):
				with tag('div', id='reportlist'):
					with tag("div"):
						with tag('div', klass='title') : text("Regressions")
						self.doc.stag('input', klass='search', placeholder="Search")
					 
					with tag("div", klass = "head"):
						with tag("div", klass = "cell testgroup"):
							with tag('button', ('data-sort','testgroup'), klass='sort'):
								text("Group")
						with tag("div", klass = "cell testname"):
							with tag('button', ('data-sort', 'testname'), klass='sort'):
								text("Name")
						with tag("div", klass = "cell testfailures"):
							with tag('button', ('data-sort', 'testfailures'), klass='sort'):
								text("Failures")
						with tag("div", klass = "cell testruntime"):
							with tag('button', ('data-sort', 'testruntime'), klass='sort'):
								text("Time")
						with tag("div", klass = "cell testdate"):
							with tag('button', ('data-sort', 'testdate'), klass='sort'):
								text("Date")

					with tag('ul', klass='list'):
						for report in reports:
							ok = ("ok" if len(report['failures']) == 0 else "fail")
							self.doc.asis(li(self.testhead(report), self.reportToHtml(report), status = ok))

				with tag('script'):
					self.doc.asis("var keys = ["+\
						"'testgroup', 'testname', 'testfailures', 'testruntime', 'testdate'];" + \
						"var userList = new List('reportlist', {valueNames: keys });")
					
	def reportStyle(self):
		css = [
		  ["ul" , {
		    "padding": "0px",
		    "margin": "0px",
		  	"list-style-type" : "none"
		  }],[
		  ".ok", {
		    "background-color": "#ddffdd"
		  }],[
		  ".fail" , {
		    "background-color": "#ffdddd"
		  }],[	
		  ".toggle", {
		    "cursor": "pointer"
		  }],[
		  ".toggle:hover" , {
		    "color": "#555555"
		  }],[
		  "div.title" , {
		  	"padding" :  "10px 10px 10px 10px",
		    "display" : "inline-block",
		    "font-size" : "250%",
            "vertical-align" : "middle"
		  }],[
		  "img.test" , {
   			"padding" : "1px",
   			"border" : "1px solid #000000",
    		"padding" : "0px",
    		"max-width" : "100%",
    		"max-height" : "100%",
    		"background-image" : ("linear-gradient(90deg, rgba(200,200,200,.5) 50%, transparent 50%)," 
    		                     + "linear-gradient(rgba(200,200,200,.5) 50%, transparent 50%)"),
    		"background-size" : "30px 30px,30px 30px",
    		"background-position" : "0, 0, 15 15px"
		  }],[
		  "img.diff" , {
   			"padding" : "1px",
   			"border" : "1px solid #000000",
   			"max-width" : "100%",
    		"max-height" : "100%",
    		"padding" : "0px",
		  }],[
		  "div.libody" , {
		    "margin-left" : "20px",
		    "padding-bottom" : "15px"
		  }],[
		  "div.head" , {
		    "border-bottom-style" : "solid",
		    "border-bottom-width" : "2px", 
		    "border-bottom-color" : "#dddddd"
		  }],[
		  "div.row" , {
		    "border-top-style" : "solid",
		    "border-top-width" : "1px", 
		    "border-top-color" : "#dddddd"
		  }],[		  
		  "div.cell" , {
		    "display" : "inline-block",
		  }],[
		  "div.testname" , {
		    "width" : "170px"
		  }],[
		  "div.testgroup" , {
		    "width" : "170px"
		  }],[
		  "div.testfailures" , {
		    "width" : "130px"
		  }],[
		  "div.testruntime" , {
		    "width" : "130px"
		  }],[
		  "div.testdate" , {
		    "width" : "150px"
		  }],[
		  "div.imageinfo" , {
		    "width" : "100px",
		  }],[
		  "div.imagename" , {
		    "width" : "200px",
		  }],[
		  "div.itemname" , {
		    "width" : "100px",
		  }],[
		  "div.key" , {
		    "width" : "150px",
		  }],[
		  "input" , {
		  	"font-size" : "100%",
		    "padding" :  "10px 10px 10px 10px",
		    "border" : "solid 1px #ccc",
		    "border-radius" : "5px",
			"vertical-align" : "middle"
		  }],[
		  "input:focus" , {
		    "outline" : "none",
		    "border-color" : "#aaa"
		  }],[
		  ".sort" , {
		    "font-size" : "100%",
		    "padding" : "0px 30px 0px 0px",
		    "display" : "inline-block",
		    "background" : "none",
    		"border" : "none",
    		"cursor" : "pointer"
		  }],[
		  ".sort:hover" , {
		    "background-color" : "#dddddd"
		  }],[
		  ".sort:active" , {
		    "background-color" : "#bbbbbb"
		  }],[
		  ".sort:after" , {
		    "width" : "0",
		    "height" : "0",
		    "border-left" : "5px solid transparent",
		    "border-right" : "5px solid transparent",
		    "border-bottom" : "5px solid transparent",
		    "content" : "\"\"",
		    "position" : "relative",
		    "top" : "-10px",
		    "right" : "-5px"
		  }],[
		  ".sort.asc:after" , {
		    "width" : "0",
		    "height" : "0",
		    "border-left" : "5px solid transparent",
		    "border-right" : "5px solid transparent",
		    "border-top" : "5px solid #000",
		    "content" : "\"\"",
		    "position" : "relative",
		    "top" : "13px",
		    "right" : "-5px"
		  }],[
		  ".sort.desc:after" , {
		    "width" : "0",
		    "height" : "0",
		    "border-left" : "5px solid transparent",
		    "border-right" : "5px solid transparent",
		    "border-bottom" : "5px solid #000",
		    "content" : "\"\"",
		    "position" : "relative",
		    "top" : "-10px",
		    "right" : "-5px"
		  }
		  ]
		]
		return dict2css(css)

	def timeSeries(self, report, length = 20):
		doc, tag, text = yattag.Doc().tagtext()
		data = self.db.getSeries(report["group"], report["name"], "elapsed_time")
		datastrShort = ", ".join([str(x.value) for x in data.measurements[:length]])
		
		with tag('div'):
			text("{:3.2f}s ".format(report["elapsed_time"]))
			with tag('span', klass="sparkline"): text(datastrShort)
		return doc.getvalue()


	def failueSeries(self, report, length = 30):
		doc, tag, text = yattag.Doc().tagtext()
		data = self.db.getSeries(report["group"], report["name"], "number_of_test_failures")
		datastr = ", ".join([str(x.value) for x in data.measurements[:length]])
		nfail = len(report["failures"])
		
		with tag('div'):
			text(str(nfail) + " ")
			with tag('span', klass="sparkline-failues"): text(datastr)
		return doc.getvalue()



	def testhead(self, report):
		doc, tag, text = yattag.Doc().tagtext()
		with tag("div", klass = "row"):
			with tag("div", klass = "cell testgroup"):
				text(report["group"])
			with tag("div", klass = "cell testname"):
				text(report["name"])
			with tag("div", klass = "cell testfailures"):
				doc.asis(self.failueSeries(report))
			with tag("div", klass = "cell testruntime"):
				doc.asis(self.timeSeries(report))
			with tag("div", klass = "cell testdate"):
				text(datetimeFromISO(report["date"]).strftime('%Y-%m-%d %H:%M:%S'))
		return doc.getvalue()


	def imageShort(self, group, name, img, length = 20):
		doc, tag, text = yattag.Doc().tagtext()

		data = self.db.getSeries(group, name , "image_test_diff." + img["image"])
		datastrShort = ", ".join([str(x.value) for x in data.measurements[:length]])
		
		with tag('div'):
			with tag('div', klass="cell imagename"):
				text(img["image"])
			with tag('div', klass="cell imageinfo"):
				text("Diff: {:3.3f}%".format(img["difference"]))
			with tag('div', klass="cell imageinfo"):
				with tag('span', klass="sparkline"): text(datastrShort)
		return doc.getvalue()


	def genImages(self, group, name, imgs, testdir, refdir):
		doc, tag, text = yattag.Doc().tagtext()
		with tag('ol'):
			for img in imgs:
				ok = img["difference"] == 0.0
						
				doc.asis(li(self.imageShort(group, name, img),
					testImages(toPath([testdir, img["image"]]), 
							   toPath([refdir, img["image"]]),
							   toPath([testdir, "imgdiff", img["image"]])),
					status = "ok" if ok else "fail"
					))
		return doc.getvalue()

	def reportToHtml(self, report):
		doc, tag, text = yattag.Doc().tagtext()

		with tag('ul'):
			keys = ["path", "command","returncode","missing_imgs", "missing_refs","output","errors"]

			for key in keys:
				val = toString(report[key])
				status = ""
				if key in report['successes']: status = "ok"
				if key in report['failures'].keys(): status = "fail"
				vabr = abr(val)
				doc.asis(li(keyval(formatKey(key), vabr), val, status = status, toggle = vabr != val))

			nfail = len(report["failures"])
			short = "No failues" if nfail == 0 else  "{} failures".format(nfail)
			doc.asis(li(keyval("Failures", short), genFailures(report["failures"]), status = "ok" if nfail == 0 else "fail"))

							
			with open(report['log'], 'r') as f:
				loghtml = f.read()
				err = loghtml.count("Error:")
				warn = loghtml.count("Warn:")
				info = loghtml.count("Info:")

				short = "Error: {}, Warnings: {}, Information: {}".format(err, warn, info)
				doc.asis(li(keyval("Log", short), loghtml, status = "ok" if err == 0 else "fail")) 

			doc.asis(li(keyval("Screenshot", ""), image(report["screenshot"], alt = "Screenshot", width="100%")))	

			ok = sum([1 if img["difference"] == 0.0 else 0 for img in report["image_tests"]])
			fail = sum([1 if img["difference"] != 0.0 else 0 for img in report["image_tests"]])
			short = (str(ok) + " ok images, " + str(fail) + " failed image tests")
			doc.asis(li(keyval("Images", short), 
				self.genImages(report["group"], report["name"], report["image_tests"], report["outputdir"], report["path"]),
				status = "ok" if fail == 0 else "fail"))

		return doc.getvalue()

	def getHtml(self):
		return yattag.indent(self.doc.getvalue())


	def reportScrips(self):
		return """
$(document).ready(function() {
   	$('div.lihead').click(function() {
   			body = $(this).next(".libody")
   			body.slideToggle(100);
    
           	$.sparkline_display_visible()
    });
	$('div.libody').hide();

	$('.sparkline').sparkline();
	$('.sparkline-box').sparkline('html', {type : 'box', showOutliers : false});
	$('.sparkline-failues').sparkline('html', {type : 'line', chartRangeMin : 0});

	$('div.zoom').zoom({magnify : 4, on : 'grab', duration : 400});
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
		doc.stag('img', src = "file://" + os.path.abspath(path), **opts)
	return doc.getvalue()

def li(head, body="", status="", toggle = True):
	doc, tag, text = yattag.Doc().tagtext()
	toggleclass = "toggle" if toggle else ""
	with tag('li'):
		with tag('div', klass='lihead ' + status + ' ' + toggleclass):
			doc.asis(head)
		if toggle:
			with tag('div', klass='libody'):
				doc.asis(body)

	return doc.getvalue()

def testImages(testimg, refimg, diffimg):
	doc, tag, text = yattag.Doc().tagtext()
	with tag('table', klass='imagetable'):
		with tag('tr'):
			with tag('th'): text("Test")
			with tag('th'): text("Reference")
			with tag('th'): text("Difference") 
		with tag('tr'):
			with tag('th'):
				doc.asis(image(testimg, alt = "test image", klass ="test"))
			with tag('th'):
				doc.asis(image(refimg, alt = "reference image", klass ="test"))
			with tag('th'):
				doc.asis(image(diffimg, alt = "difference image", klass ="diff"))
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





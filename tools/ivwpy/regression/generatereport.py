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
	def __init__(self, reports):
		self.doc, tag, text = yattag.Doc().tagtext()
		self.style = self.reportStyle()

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
						with tag('dt', klass=("toggle test " + ("ok" if ok else "fail"))):
							text(("OK " if ok else "Fail ") + report['module'] + "/" + report['name'])
						with tag('dd'):
							with tag('div', klass = "holder"):
								text("")
							with tag('div', klass = "content"):
								self.doc.asis(self.reportToHtml(report))

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
		  "dt.inline": {
		    "float": "left",
		    "width": "15%",
		    "padding": "0",
		    "margin": "0"
		  },
		  "dd.inline": {
		    "float": "left",
		    "width": "85%",
		    "padding": "0",
		    "margin": "0"
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

		def content(a):
			with tag('div', klass = "holder"):
				text(a.split("\n")[0][:50] + "...")
			with tag('div', klass = "content"):
				text(a)

		def dlpair(a,b, inline = False, toggle = False):
			ic = "inline" if inline else ""
			tc = "toggle" if toggle else ""
			
			with tag('dt', klass = ic + " " + tc): text(a)
			with tag('dd', klass = ic): content(b) if toggle else text(b)

		def kvpair(key, opts = {}):
			if isinstance(report[key], str):
				if len(report[key]) == 0: val = "None"
				else: val = report[key]
			elif isinstance(report[key], list):
				if len(report[key]) == 0: val = "None"
				else: val = ", ".join(report[key])
			elif isinstance(report[key], float):
				val = "{:6f}".format(report[key])
			elif isinstance(report[key], int):
				val = "{:}".format(report[key])

			dlpair(key.capitalize().replace("_", " "), val, **opts)


		def image(img):
			doc.stag('img', src = "file://" + report["outputdir"] + "/" + img["image"], alt = "img" )
			doc.stag('img', src = "file://" + report["path"] + "/" + img["image"] , alt = "ref")
			
		def screenshot():
			with tag('dt', klass = "toggle"): 
				text("Screenshot")
			with tag('dd'):
				with tag('div', klass = "holder"):
					text("...")
				with tag('div', klass = "content"):
					doc.stag('img', src = "file://" + report["screenshot"], alt = "img" )
			

		with tag('dl'):
			keys = [
				["date"         , {"toggle" : False , "inline" : True}],
				["failures"     , {"toggle" : False , "inline" : True}],
				["path"         , {"toggle" : False , "inline" : True}],
				["elapsed_time" , {"toggle" : False , "inline" : True}],
				["command"      , {"toggle" : True , "inline" : True}],
				["returncode"   , {"toggle" : False , "inline" : True}],
				["missing_imgs" , {"toggle" : False , "inline" : True}],
				["missing_refs" , {"toggle" : False , "inline" : True}],
				["output"       , {"toggle" : True ,  "inline" : True}],
				["errors"       , {"toggle" : True ,  "inline" : True}]
			]

			for key, opts in keys:
				kvpair(key, opts)
			
			with open(report['log'], 'r') as f:
				with tag('dt', klass = "toggle"): 
					text("Log")
				with tag('dd'):
					loghtml = f.read()
					with tag('div', klass = "holder"):
						doc.asis("Error: " + str(loghtml.count("Error:")) 
							      + ", Warnings: " + str(loghtml.count("Warn:"))
							      + ", Information: " + str(loghtml.count("Info:"))
							      )
					with tag('div', klass = "content"):
						doc.asis(loghtml)
				
			screenshot()

			with tag('dt', klass = "toggle"): text("Images")
			with tag('dd'):
				with tag('div', klass = "holder"):
					ok = sum([1 if img["difference"] == 0.0 else 0 for img in report["image_tests"]])
					fail = sum([1 if img["difference"] != 0.0 else 0 for img in report["image_tests"]])
					text(str(ok) + " ok images, " + str(fail) + " failed image tests")
				with tag('div', klass = "content"):
					with tag('dl'):
						for img in report["image_tests"]:
							ok = img["difference"] == 0.0
							with tag('dt', klass=("toggle image " + "ok" if ok else "fail")):
								text("{} Diff: {:3.3f}% {}".format("Ok" if ok else "Fail",img["difference"], img["image"]))
							with tag('dd'):
								with tag('div', klass = "holder"):
									text("")
								with tag('div', klass = "content"):
									image(img)

		return doc.getvalue()

	def getHtml(self):
		return yattag.indent(self.doc.getvalue())


	def reportScrips(self):
		return """
$(document).ready(function() {
   	$('dt.toggle').click(function() {
   			if($(this).next().children("div.content").is(':visible')) {
   				$(this).next().children("div.content").hide()
   			}else {
           		$(this).next().children("div.content").fadeToggle(500)
           	}

           	if($(this).next().children("div.holder").is(':visible')) {
   				$(this).next().children("div.holder").hide()
   			}else {
           		$(this).next().children("div.holder").fadeToggle(500)
           	}
    });
	$('div.content').hide()
	$('div.holder').show()
 });
"""
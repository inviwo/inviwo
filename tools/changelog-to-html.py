#*********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2019 Inviwo Foundation
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
import sys
import argparse
import pathlib
from distutils.version import StrictVersion

import ivwpy.util
from ivwpy.colorprint import *

# Requirements:
# python3
# markdown for parsing markdown files and encoding them in HTML
# gfm, a markdown extension for parsing github-flavored markdown (GFM)

missing_modules = {}
downgradeMarkdown = False

try:
	import markdown
	if hasattr(markdown, 'version'):
		# version check, py-gfm has not yet been updated to markdown >3.0.0
		if StrictVersion(markdown.version) > StrictVersion("3.0.0"):
			downgradeMarkdown = True
			raise ImportError # need to downgrade markdown module, do NOT check for gfm module!
except ImportError:
	missing_modules['"markdown<3.0"'] = "needed for markdown parsing (requires version prior 3 due to py-gfm dependency)"

if not downgradeMarkdown:
	try:
		import gfm
	except ImportError:
		missing_modules['py-gfm'] = "extension for markdown, needed for parsing github-flavored markdown (GFM)"


def makeCmdParser():
	parser = argparse.ArgumentParser(
		description="Convert a github-flavored markdown (GFM) changelog to HTML",
		formatter_class=argparse.ArgumentDefaultsHelpFormatter
	)
	parser.add_argument('-i', '--input', type=str, required=True, action="store", dest="input",
						help='Markdown input file (GFM)')
	parser.add_argument('-o', '--output', type=str, required=True, action="store", dest="output", help='Output HTML file')
		
	return parser.parse_args()


htmlHeader = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<style type="text/css">
	body {color: #9d9995;}
	a {color: #268bd2;}
	a:visited { color: #1E6A9E; }
	h1 { font-size:  xx-large; color: #268bd2; margin-bottom:1em; }
	h2 { font-size: large; color: #268bd2; margin-top:1em; margin-bottom:0em; }
	p { margin-bottom: 0.2em; margin-top: 0.1em; }
	div { margin: 0; padding: 0; }
	pre { margin-top: 0.3em; margin-bottom: 0.3em; }
	code { color: #E0E0DB; background-color: #4a4a4f; padding-left: 0.4em; padding-right: 0.4em; }
	div.highlight { background-color: #4a4a4f; padding-top: 0.1em; padding-bottom: 0.1em; }
	.hll { background-color: #49483e }
	.c { color: #75715e } /* Comment */
	.err { color: #960050; background-color: #1e0010 } /* Error */
	.k { color: #66d9ef } /* Keyword */
	.l { color: #ae81ff } /* Literal */
	.n { color: #f8f8f2 } /* Name */
	.o { color: #f92672 } /* Operator */
	.p { color: #f8f8f2 } /* Punctuation */
	.ch { color: #75715e } /* Comment.Hashbang */
	.cm { color: #75715e } /* Comment.Multiline */
	.cp { color: #E22453; font-weight: bold } /* Comment.Preproc */
	.cpf { color: #E6DB74; font-style: italic } /* Comment.PreprocFile */
	.c1 { color: #75715e } /* Comment.Single */
	.cs { color: #75715e } /* Comment.Special */
	.gd { color: #f92672 } /* Generic.Deleted */
	.ge { font-style: italic } /* Generic.Emph */
	.gi { color: #a6e22e } /* Generic.Inserted */
	.gs { font-weight: bold } /* Generic.Strong */
	.gu { color: #75715e } /* Generic.Subheading */
	.kc { color: #66d9ef } /* Keyword.Constant */
	.kd { color: #66d9ef } /* Keyword.Declaration */
	.kn { color: #f92672 } /* Keyword.Namespace */
	.kp { color: #66d9ef } /* Keyword.Pseudo */
	.kr { color: #66d9ef } /* Keyword.Reserved */
	.kt { color: #66d9ef } /* Keyword.Type */
	.ld { color: #e6db74 } /* Literal.Date */
	.m { color: #ae81ff } /* Literal.Number */
	.s { color: #e6db74 } /* Literal.String */
	.na { color: #a6e22e } /* Name.Attribute */
	.nb { color: #f8f8f2 } /* Name.Builtin */
	.nc { color: #a6e22e } /* Name.Class */
	.no { color: #66d9ef } /* Name.Constant */
	.nd { color: #a6e22e } /* Name.Decorator */
	.ni { color: #f8f8f2 } /* Name.Entity */
	.ne { color: #a6e22e } /* Name.Exception */
	.nf { color: #a6e22e } /* Name.Function */
	.nl { color: #f8f8f2 } /* Name.Label */
	.nn { color: #f8f8f2 } /* Name.Namespace */
	.nx { color: #a6e22e } /* Name.Other */
	.py { color: #f8f8f2 } /* Name.Property */
	.nt { color: #f92672 } /* Name.Tag */
	.nv { color: #f8f8f2 } /* Name.Variable */
	.ow { color: #f92672 } /* Operator.Word */
	.w { color: #f8f8f2 } /* Text.Whitespace */
	.mb { color: #ae81ff } /* Literal.Number.Bin */
	.mf { color: #ae81ff } /* Literal.Number.Float */
	.mh { color: #ae81ff } /* Literal.Number.Hex */
	.mi { color: #ae81ff } /* Literal.Number.Integer */
	.mo { color: #ae81ff } /* Literal.Number.Oct */
	.sa { color: #e6db74 } /* Literal.String.Affix */
	.sb { color: #e6db74 } /* Literal.String.Backtick */
	.sc { color: #e6db74 } /* Literal.String.Char */
	.dl { color: #e6db74 } /* Literal.String.Delimiter */
	.sd { color: #e6db74 } /* Literal.String.Doc */
	.s2 { color: #e6db74 } /* Literal.String.Double */
	.se { color: #ae81ff } /* Literal.String.Escape */
	.sh { color: #e6db74 } /* Literal.String.Heredoc */
	.si { color: #e6db74 } /* Literal.String.Interpol */
	.sx { color: #e6db74 } /* Literal.String.Other */
	.sr { color: #e6db74 } /* Literal.String.Regex */
	.s1 { color: #e6db74 } /* Literal.String.Single */
	.ss { color: #e6db74 } /* Literal.String.Symbol */
	.bp { color: #f8f8f2 } /* Name.Builtin.Pseudo */
	.fm { color: #a6e22e } /* Name.Function.Magic */
	.vc { color: #f8f8f2 } /* Name.Variable.Class */
	.vg { color: #f8f8f2 } /* Name.Variable.Global */
	.vi { color: #f8f8f2 } /* Name.Variable.Instance */
	.vm { color: #f8f8f2 } /* Name.Variable.Magic */
	.il { color: #ae81ff } /* Literal.Number.Integer.Long */
</style>
</head>
<body >
<h1>Latest Changes</h1>
"""
htmlBody = "</body></html>\n"
changelogBegin = "Here we document changes"

def main(args):
	args = makeCmdParser();

	if len(missing_modules)>0: 
		print_error("Warning: Cannot generate HTML changelog. Missing python modules:")
		for k,v in missing_modules.items():
			print_error("    {:20s} {}".format(k,v))
		print_info("    To install run: 'python -m pip install {}'".format(" ".join(missing_modules.keys())))

		# touch target file to update time stamp even though we could not update it
		pathlib.Path(args.output).touch()

		exit(0)

	if not os.path.exists(args.input):
		print_error("changelog-to-html.py was unable to locate the input file " + args.input)
		sys.exit(1)

	with open(args.input, mode="r", encoding="utf-8") as f:
		text = f.read();
	
	# remove first line starting with "Here we document changes..."
	if text.startswith(changelogBegin):
		text = text.split('\n', 2)[2]

	md = markdown.Markdown(extensions=['gfm'])
	body = md.convert(text);

	html = htmlHeader + body + htmlBody

	(path, filename)  = os.path.split(os.path.abspath(args.output))
	ivwpy.util.mkdir(path)
	with open(args.output, mode="w", encoding="utf-8", errors="xmlcharrefreplace") as f:
		f.write(html)

if __name__ == '__main__':
	main(sys.argv[1:])

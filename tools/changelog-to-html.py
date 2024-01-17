# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2019-2024 Inviwo Foundation
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
import urllib.request
import urllib.error
import json

import ivwpy.util
from ivwpy.colorprint import *


def makeCmdParser():
    parser = argparse.ArgumentParser(
        description="Convert a github-flavored markdown (GFM) changelog to HTML",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('-i', '--input', type=str, required=True, action="store", dest="input",
                        help='Markdown input file (GFM)')
    parser.add_argument('-o', '--output', type=str, required=True, action="store", dest="output",
                        help='Output HTML file')
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
    li { margin-bottom: 0.2em; }
    div { margin: 0; padding: 0; }
    pre { margin-top: 0.3em; margin-bottom: 0.3em; line-height:120%;}
    code { color: #f8f8f2; background-color: #4a4a4f; padding-left: 0.4em; padding-right: 0.4em; }
    div.highlight {
        color: #f8f8f2;
        background-color: #4a4a4f;
        padding-top: 0.1em;
        padding-bottom: 0.1em;
    }
    .pl-s, .pl-pds, .pl-sr { color: #e6db74; }
    .pl-k { color: #66d9ef; } /* keyword, storage, storage.type */
    .pl-s1 { color: #a6e22e; } /* string source */
    .pl-kos { color: #f8f8f2; }
    .pl-en, .pl-e{ color: #a6e22e; } /* entity.name */
    .pl-c1 { color: #f92672; }
    /* variable.parameter.function, storage.modifier.package,
       storage.modifier.import, storage.type.java, variable.other */
    .pl-smi { color: #f6f8fa; }
    .pl-v {color: #a6e22e; } /* variable */
    .pl-c { color: #f6f8fa; } /* comment, punctuation.definition.comment, string.comment */
</style>
</head>
<body>
"""

htmlBody = "</body></html>\n"
changelogBegin = "Here we document changes"


def main(args):
    args = makeCmdParser()

    if not os.path.exists(args.input):
        print_error("changelog-to-html.py was unable to locate the input file " + args.input)
        sys.exit(1)

    try:
        with open(args.input, mode="r", encoding="utf-8") as f:
            text = f.read()
    except FileNotFoundError:
        print_error("changelog-to-html.py was unable to locate the input file " + args.input)
        sys.exit(1)

    # remove first line starting with "Here we document changes..."
    if text.startswith(changelogBegin):
        text = text.split('\n', 2)[2]

    url = "https://api.github.com/markdown"
    headers = {
        'Accept': 'application/vnd.github.v3+json',
        'Content-type': 'application/json'
    }
    data = json.dumps({'text': text, 'mode': 'gfm'}).encode("utf-8")
    req = urllib.request.Request(url, data, headers)
    try:
        with urllib.request.urlopen(req) as f:
            body = f.read().decode('utf-8')
        html = htmlHeader + body + htmlBody

        (path, filename) = os.path.split(os.path.abspath(args.output))
        ivwpy.util.mkdir(path)
        with open(args.output, mode="w", encoding="utf-8", errors="xmlcharrefreplace") as f:
            f.write(html)
    except urllib.error.HTTPError as e:
        print_error(f"Error code: {e.code}")
        print_error(e.read().decode("utf-8"))
        sys.exit(0)


if __name__ == '__main__':
    main(sys.argv[1:])

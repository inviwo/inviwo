#!/usr/bin/env python
# -*- coding: utf-8 -*- 
#

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


# Copyright notice: 
# The MIT License (MIT)

# Copyright (C) 2012 Sebastian A. Schaefer
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import getopt          # get command-line options
import os.path         # getting extension from file
import string          # string manipulation
import sys             # output and stuff
import re              # for regular expressions

## extract doxygen-tag class
re_doxy_class = re.compile('(?<=[@]class\s)\w+', re.I | re.VERBOSE)
## extract doxygen-tag namespace
re_doxy_namespace = re.compile('(?<=[@]namespace\s)[^\s]+', re.I | re.VERBOSE)
re_blockcode_start = re.compile('(?<=[\*]class\s)\w+', re.I | re.VERBOSE)

##
# @package glslfilter
# @brief A Doxygen filter to document GLSL-Shader, based on a vb-filter from Basti Grembowietz
# @author Sebastian Schäfer
# @date 02/2012
# @version 0.1
# @copyright MIT License.
# 
# @details The shader file is wrapped into a class and namespace that can be set with 
# doxygen-tags.
# 
# Usage:
# - shader file:
#   - set doxygen name for class name -> defaults to filename
#   - set doxygen namespace for namespace (pseudo category) -> defaults to GLSL
# - doxygen file:
#   - add FILE_PATTERNS: *.frag, *.vert, *.geom, *.glsl
#   - add FILTER_PATTERNS: "*.frag=glslfilter.py", "*.vert=glslfilter.py", "*.geom=glslfilter.py", "*.glsl=glslfilter.py"
# latest version on <a href="http://www.grasmo.de">www.grasmo.de</a>

##run regex on a single line
# @returns either a found result or None
def getRegSearchLine(str, regex):
    search = regex.search(str)
    if search is not None:
        return search.group(0)
    return None

##run regex on an string array
# @returns either a found result or None
def getRegSearch(txt, regex):
    for str in txt:
        search = regex.search(str)
        if search is not None:
            return search.group(0)
    return None

# generate a class name from filename
# @return just the filename - no extension and no path
def generateName(filename):
    root, ext = os.path.splitext(filename)
    head, tail = os.path.split(root)
    return tail
    
def writeLine(txt):
    sys.stdout.write(txt)
    
## parse a shader and generate needed information along on the way
## - if comments contain a namespace move it the classname
def parseShader(filename, txt, type = None):
    # extract name from doxygen-tag or use generic GLSL namespace
    namespace = getRegSearch(txt, re_doxy_namespace)
    if namespace is None:
        namespace = "glsl"
    else:
        #remove namespace line from txt
        txt = [str for str in txt if getRegSearchLine(str, re_doxy_namespace) is None]

    # extract className from doxygen-tag or use filename
    className = getRegSearch(txt, re_doxy_class)
    if className is None:
        className = generateName(filename)
    else:
        #remove calssName line from txt
        txt = [str for str in txt if getRegSearchLine(str, re_doxy_class) is None]

    if len(txt) > 0:
        if txt[0].strip() == "/*********************************************************************************" \
            and txt[27].strip() ==  "*********************************************************************************/":
            showLines(txt[0:28])
            txt = txt[28:]
            
    if (len(txt)>0):
        while txt[0].isspace() or txt[0].find("#include") >= 0:
            writeLine(txt.pop(0))

    comment = []
    if len(txt) > 0:
        if txt[0].find("/*") >=0:
            line = txt.pop(0)
            while (line.find("*/") < 0) and (len(txt) > 0):
                comment.append(line)
                line = txt.pop(0)
            comment.append(line)
                
    # dump the file and pad it with namespace/name class information
    # 1st: namespace + class padding, also declare everything public
    writeLine("/** @namespace " + namespace + " */")
    writeLine("public class " + namespace+"::"+className + "{")
    writeLine("public:")
    # 2nd: dump original commentblock
    showLines(comment)
    # 3rd: add type-remark and classname
    if type is not None:
        writeLine("/** @remark <b>" + type + "</b> */")
    writeLine("/** @class " + namespace+"::"+className + " */")
    # 4th: dump remaining file 
    showLines(txt)
    # 5th: close dummy class
    writeLine("}\n")

## @returns the complete file content as an array of lines
def readFile(filename):
    f = open(filename)
    r = f.readlines()
    f.close()
    return r
    
## dump all lines to stdout
def showLines(r):
    for s in r:
        sys.stdout.write(s)
        
## main method - open a file and see what can be done
def filter(filename):
    try:
        root, ext = os.path.splitext(filename)
        txt = readFile(filename)
        if (ext.lower() == ".frag"):
            parseShader(filename, txt, "Fragment-Shader")
        elif (ext.lower() == ".vert"):
            parseShader(filename, txt, "Vertex-Shader")
        elif (ext.lower() == ".geom"):
            parseShader(filename, txt, "Geometry-Shader")
        elif (ext.lower() == ".glsl"):
            parseShader(filename, txt, "GLSL-Shader")
        else:
            showLines(txt)
    except IOError as e:
        sys.stderr.write(e[1]+"\n")

if len(sys.argv) != 2:
    print("usage: ", sys.argv[0], " filename")
    sys.exit(1)

filename = sys.argv[1] 
filter(filename)
sys.exit(0)
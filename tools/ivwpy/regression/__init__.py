# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2022 Inviwo Foundation
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

import importlib.util

from .. colorprint import *

requirements = {
    'yattag': ['yattag', "needed for html generation (http://www.yattag.org)"],
    'PIL': ['Pillow', "needed for image comparison "
                      "(Pillow is a fork of PIL https://python-pillow.github.io/)"],
    'sqlalchemy': ['sqlalchemy', "needed for database connection"],
    'bs4': ['beautifulsoup4', "needed for dom manipulation"],
    'lesscpy': ['lesscpy', "needed for css generation"],
}

missing_modules = {}
for m, [name, desc] in requirements.items():
    if importlib.util.find_spec(m) is None:
        missing_modules[name] = desc

if len(missing_modules) > 0:
    print_error("Error: Missing python modules:")
    for k, v in missing_modules.items():
        print_error("    {:20s} {}".format(k, v))
    print_info("    To install run: 'python -m pip install {}'".format(
        " ".join(missing_modules.keys())))
    exit()

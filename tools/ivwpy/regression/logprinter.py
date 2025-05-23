# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2025 Inviwo Foundation
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

import io
from .. colorprint import *


class LogPrinter:
    def __init__(self, logger):
        self.logger = logger
        self.success = True
        self.buffer = io.StringIO()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if self.success:
            self.logger.info(self.buffer.getvalue())
        else:
            self.logger.warning(self.buffer.getvalue())
        self.buffer.close()

    def text(self, mess, **kwargs):
        print(mess, file=self.buffer, **kwargs)

    def good(self, mess, **kwargs):
        cprint(Color.green, mess, file=self.buffer, **kwargs)

    def info(self, mess, **kwargs):
        cprint(Color.cyan, mess, file=self.buffer, **kwargs)

    def warn(self, mess, **kwargs):
        cprint(Color.yellow, mess, file=self.buffer, **kwargs)

    def error(self, mess, **kwargs):
        cprint(Color.red, mess, file=self.buffer, **kwargs)

    def pair(self, a, b, width=15, offset=1):
        if isinstance(b, dict):
            self.info("{padd:{offset}}{key:<{width}} : ".format(
                padd="", offset=offset, key=a, width=width - offset))
            for k, v in b.items():
                self.pair(k, v, width=width, offset=offset + 2)
        else:
            self.info("{padd:{offset}}{key:<{width}} : ".format(
                padd="", offset=offset, key=a, width=width - offset), end="")
            lines = str(b).strip().split('\n')
            if len(lines) > 0:
                self.text("{}".format(lines.pop(0)))
                for line in lines:
                    self.info("{padd:{offset}}{key:<{width}} : ".format(
                        padd="", offset=offset, key="", width=width - offset), end="")
                    self.text("{}".format(line))

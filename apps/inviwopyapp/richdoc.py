# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2023 Inviwo Foundation
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

import inviwopy
import rich
import rich.panel
import rich.console
import rich.table


def toText(document):
    stack = [rich.console.Group()]

    def before(elem, s):
        if elem.isText():
            stack.append(elem.content)
        if elem.isNode() and elem.name == "table":
            table = rich.table.Table()
            table.show_header = False
            table.show_edge = False
            table.box = None
            stack.append(table)

        if elem.isNode() and elem.name == "tr":
            stack.append([])
        if elem.isNode() and (elem.name == "td" or elem.name == "th"):
            stack.append(rich.console.Group())

    def after(elem, s):
        if elem.isText():
            text = stack.pop()
            stack[-1].renderables.append(text)
        if elem.isNode() and elem.name == "table":
            table = stack.pop()
            stack[-1].renderables.append(table)
        if elem.isNode() and elem.name == "tr":
            row = stack.pop()
            stack[-1].add_row(*row)

        if elem.isNode() and (elem.name == "td" or elem.name == "th"):
            group = stack.pop()
            stack[-1].append(group)

    document.visit(before, after)
    return rich.panel.Panel(stack[0])


rich.print(inviwopy.app.network.VolumeSource.filename.getDescription())

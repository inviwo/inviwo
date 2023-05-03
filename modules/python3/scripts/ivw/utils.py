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

import os
import os.path


def ensureDirectory(dir):
    if not os.path.exists(dir):
        os.makedirs(dir)


def update():
    try:
        from inviwopy import qt
        qt.update()
    except:
        pass


def getCanvases():
    import inviwopy as i
    return i.app.network.canvases


def snapshotAllCanvasesWithWorkspace(basePath: str, workspaceName, canvasFilenamePrefix="",
                                     canvasFilenameSufix="", filetype="png"):
    import inviwopy as i
    ensureDirectory(basePath)
    return snapshotWithWorkspace(basePath, i.app.network.canvases, workspaceName,
                                 canvasFilenamePrefix, canvasFilenameSufix, filetype)


def snapshotWithWorkspace(basePath: str, canvases, workspaceName,
                          canvasFilenamePrefix="", canvasFilenameSufix="", filetype="png"):

    import inviwopy

    from inviwopy import CanvasProcessor

    canvasList = []
    if isinstance(canvases, list):
        canvasList = canvases
    elif isinstance(canvases, CanvasProcessor):
        canvasList = [canvases]
    else:
        raise TypeError(
            "snapshotWithWorkspace expect parameter 2 to be either a"
            " CanvasProcessor or a list of strings of canvas ids")

    workspaceName = workspaceName.strip()
    canvasFilenamePrefix = canvasFilenamePrefix.strip()
    canvasFilenameSufix = canvasFilenameSufix.strip()

    if not workspaceName.endswith('.inv'):
        workspaceName += '.inv'

    workspacePath = basePath + "/" + workspaceName
    workspaceDir = os.path.dirname(workspacePath)

    ensureDirectory(basePath)
    ensureDirectory(workspaceDir)

    inviwopy.app.network.save(basePath + "/" + workspaceName)
    for c in canvasList:
        c.snapshot(basePath + "/" + canvasFilenamePrefix
                   + c.identifier + canvasFilenameSufix + "." + filetype)

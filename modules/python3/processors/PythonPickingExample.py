# Name: PythonPickingExample

# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2023-2025 Inviwo Foundation
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

import inviwopy as ivw
import numpy as np


class PythonPickingExample(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.outport = ivw.data.MeshOutport("outport")
        self.addOutport(self.outport, owner=False)

        self.count = ivw.properties.IntProperty("count", "count", 10, 0, 1000, 1)
        self.addProperty(self.count, owner=False)

        self.tf = ivw.properties.TransferFunctionProperty("tf", "TF", ivw.doc.Document(),
                                                          ivw.data.TransferFunction())
        self.addProperty(self.tf, owner=False)

        # initialize PickingMapper and set callback function
        self.pm = ivw.PickingMapper(self, 1, lambda x: self.callback(x))

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier="org.inviwo.PythonPickingExample",
            displayName="Python Picking Example",
            category="Python",
            codeState=ivw.CodeState.Stable,
            tags=ivw.Tags("PY, Example"),
            help=ivw.md2doc(r'''
Example processor in Python demonstrating picking and tooltips for random triangles with
`inviwopy.PickingMapper`.

See [python3/pythonpicking.inv](file:~modulePath~/data/workspaces/pythonpicking.inv)
workspace for example usage.
''')
        )

    def getProcessorInfo(self):
        return PythonPickingExample.processorInfo()

    def process(self):
        count = self.count.value

        self.positions = (np.random.normal(0, 1, (3 * count, 3))
                          + np.repeat(np.random.normal(0, 5, (count, 3)), 3, axis=0)
                          ).astype(np.float32)

        ctmp = np.asarray([self.tf.value.sample(i / count) for i in range(count)])
        color = np.repeat(ctmp, 3, axis=0)

        # resize picking manager to fit all triangles
        self.pm.resize(count)
        # generate picking IDs and make sure all vertices of a triangle share the same ID
        ptmp = np.asarray([self.pm.pickingId(i) for i in range(count)], dtype=np.uint32)
        picking = np.repeat(ptmp, 3)

        indices = np.arange(0, count * 3, dtype=np.uint32)

        mesh = ivw.data.Mesh(ivw.data.DrawType.Triangles, ivw.data.ConnectivityType.Unconnected)

        mesh.addBuffer(ivw.data.BufferType.PositionAttrib, ivw.data.Buffer(self.positions))
        mesh.addBuffer(ivw.data.BufferType.ColorAttrib, ivw.data.Buffer(color))
        mesh.addBuffer(ivw.data.BufferType.PickingAttrib, ivw.data.Buffer(picking))
        mesh.addBuffer(ivw.data.BufferType.IndexAttrib, ivw.data.Buffer(indices))

        self.outport.setData(mesh)

    def invokeEvent(self, event: ivw.Event) -> None:
        super().invokeEvent(event)
        if event.hasBeenUsed():
            return

        # catch context menu events that start with the processor identifier
        if event.hash() == ivw.ContextMenuEvent.chash:
            if event.id.startswith(self.getIdentifier()):
                ivw.logWarn(f"Context menu action triggered: '{event.id}'")
                event.markAsUsed()

    def callback(self, pickevent: ivw.PickingEvent) -> None:
        print("pick: ", pickevent.pickedId)

        if (not pickevent.getMovedSincePressed()
            and pickevent.pressItem == ivw.PickingPressItem.Secondary
                and pickevent.pressState == ivw.PickingPressState.Release):
            event: ivw.InteractionEvent = pickevent.getEvent()
            if event:
                i = pickevent.pickedId

                # show context menu with a single entry for the triangle
                entries: list = [
                    ivw.ContextMenuEntry(f"Triangle {i}", f"{self.getIdentifier()}.tri{i}")
                ]
                event.showContextMenu(entries)
                event.markAsUsed()

        if (pickevent.state == ivw.PickingState.Updated):
            i = pickevent.pickedId
            p1 = self.positions[i * 3 + 0]
            p2 = self.positions[i * 3 + 1]
            p3 = self.positions[i * 3 + 2]

            pickevent.setToolTip(f"id: {i} Pos: {p1}, {p2}, {p3}")
        else:
            pickevent.setToolTip("")

# Name: PythonPickingExample

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
Example processor in Python demonstrating picking and tooltips for random triangles with `inviwopy.PickingMapper`.

See [python3/pythonpicking.inv](file:~modulePath~/data/workspaces/pythonpicking.inv) workspace for example usage.
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

    def callback(self, pickevent):
        if (pickevent.state == ivw.PickingState.Updated):
            i = pickevent.pickedId
            p1 = self.positions[i * 3 + 0]
            p2 = self.positions[i * 3 + 1]
            p3 = self.positions[i * 3 + 2]

            pickevent.setToolTip(f"id: {i} Pos: {p1}, {p2}, {p3}")
        else:
            pickevent.setToolTip("")

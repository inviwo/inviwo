# Name: ChromatinDataFrameConverter

import inviwopy as ivw
import ivwdataframe as df
import ivwbnl
from typing import List

import numpy as np


class ChromatinData:

    def __init__(self, positions: np.ndarray, picking_ids: List[int],
                 indices: List[int], names: List[int], categories: List[str],
                 linear_pos: List[int]):
        self.count = len(positions)
        self.position_buffer: ivw.data.Buffer = ivw.data.Buffer(
            np.asarray(positions, dtype=np.float32))
        self.picking_buffer: ivw.data.Buffer = ivw.data.Buffer(
            np.asarray(picking_ids, dtype=np.uint32))
        self.indices_buffer: ivw.data.Buffer = ivw.data.Buffer(np.asarray(indices, dtype=np.uint32))
        self.names: List[int] = names
        self.categories: List[str] = categories
        self.linear_pos: ivw.data.Buffer = ivw.data.Buffer(np.asarray(linear_pos, dtype=np.float32))


class ChromatinDataFrameConverter(ivw.Processor):
    """
    Documentation of ChromatinDataFrameConverter
    """

    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)

        self.data: ChromatinData = None

        self.inport = df.DataFrameInport(
            "dataframe",
            help=ivw.md2doc('DataFrame representing chromatin data')
        )
        self.addInport(self.inport)

        self.outport = ivw.data.MeshOutport("mesh")
        self.addOutport(self.outport)

        self.bnlInport = ivwbnl.BrushingAndLinkingInport("bnl", [])
        self.addInport(self.bnlInport)

        self.tf = ivw.properties.TransferFunctionProperty("colormap", "Colormap",
                                                          ivw.doc.Document())

        self.addProperties((self.tf,))

        self.pm = ivw.PickingMapper(self, 1, lambda x: self.pickingCallback(x))

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier="org.inviwo.ChromatinDataFrameConverter",
            displayName="ChromatinDataFrameConverter",
            category="Python",
            codeState=ivw.CodeState.Stable,
            tags=ivw.Tags.PY,
            help=ivw.unindentMd2doc(ChromatinDataFrameConverter.__doc__)
        )

    def getProcessorInfo(self):
        return ChromatinDataFrameConverter.processorInfo()

    def initializeResources(self):
        pass

    def parseDataFrame(self, dataframe: df.DataFrame) -> ChromatinData:

        col_x = dataframe.getColumn(name='x')
        col_y = dataframe.getColumn(name='y')
        col_z = dataframe.getColumn(name='z')
        col_name = dataframe.getColumn(name='name')
        col_chromatin_pos = dataframe.getColumn(name='position')

        if not all((col_x, col_y, col_z, col_name, col_chromatin_pos)):
            raise ValueError(
                f'Missing column(s) in DataFrame input. Expected ["x", "y", "z", "name", "position"]')  # noqa: E501
        if not col_name.type == df.ColumnType.Categorical:
            raise ValueError(
                f'Expected column "name" to be of categorical type. Found "{col_name.type}".')

        if dataframe.rows == 0:
            self.pm.resize(1)
            return None

        pos_data = np.stack(tuple(col.buffer.data for col in (col_x, col_y, col_z)), axis=-1)
        # chromatin_pos_buf = col_chromatin_pos.buffer.data
        name_buf = list(col_name.buffer.data)

        self.pm.resize(dataframe.rows)
        picking = [self.pm.pickingId(i) for i in range(dataframe.rows)]
        indices = [i for i in range(dataframe.rows)]

        pos_buffer = col_chromatin_pos.buffer
        max_pos = np.max(pos_buffer.data)
        linear_pos = pos_buffer.data / max_pos
        ivw.logInfo(f'{max_pos= }  {np.min(pos_buffer.data)= }')

        return ChromatinData(positions=pos_data, picking_ids=picking,
                             indices=indices, names=name_buf,
                             categories=col_name.categories,
                             linear_pos=linear_pos)

    def process(self):
        if not self.data or self.inport.isChanged():
            self.data = self.parseDataFrame(self.inport.getData())

        mesh = ivw.data.Mesh(dt=ivw.data.DrawType.Lines, ct=ivw.data.ConnectivityType.Strip)
        mesh.modelMatrix = ivw.glm.mat4(1)

        # if current != name_buf[r] or r + 1 == dataframe.rows:
        #     mesh.addIndices(ivw.data.MeshInfo(dt=ivw.data.DrawType.Lines,
        #                                       ct=ivw.data.ConnectivityType.Strip),
        #                     ivw.data.IndexBufferUINT32(np.arange(start=start_index,
        #                                                          stop=r + 1, dtype=np.uint32)))

        #     current = name_buf[r]
        #     start_index = r

        if self.data:
            colors = []
            for i in range(self.data.count):
                color = self.tf.value.sample(self.data.names[i] / len(self.data.categories))

                # if self.bnlInport.isFiltered(i):
                #     color = ivw.glm.vec4(0.2, 0.2, 0.2, 0.0)
                # if self.bnlInport.isHighlighted(i):
                #     color = ivw.glm.vec4(1, 0, 0, 1)
                # elif self.bnlInport.isSelected(i):
                #     color = ivw.glm.vec4(1, 1, 0, 1)

                colors.append(color)
            # colors = [ivw.glm.vec4(1, 0, 0, 1) for _ in range(self.data.count)]

            mesh.addBuffer(ivw.data.BufferType.PositionAttrib, self.data.position_buffer)
            mesh.addBuffer(ivw.data.BufferType.PickingAttrib, self.data.picking_buffer)
            mesh.addBuffer(ivw.data.BufferType.IndexAttrib, self.data.indices_buffer)
            mesh.addBuffer(ivw.data.BufferType.ColorAttrib,
                           ivw.data.Buffer(np.asarray(colors, dtype=np.float32)))
            mesh.addBuffer(ivw.data.BufferType.ScalarMetaAttrib, self.data.linear_pos)

        # TODO: create index buffers based on selected segments!

        self.outport.setData(mesh)

    def pickingCallback(self, pickEvent):
        if pickEvent.pressState == ivw.PickingPressState.NoPress:
            if pickEvent.hoverState == ivw.PickingHoverState.Enter:
                i = pickEvent.pickedId
                self.bnlInport.highlight(ivw.data.BitSet([i]))

                pickEvent.setToolTip(
                    f"id: {i}\nname: "
                    f"{self.data.categories[self.data.names[i]] if self.data else '-'}")

            elif pickEvent.hoverState == ivw.PickingHoverState.Exit:
                self.bnlInport.highlight(ivw.data.BitSet())
                pickEvent.setToolTip("")

        if (pickEvent.pressState == ivw.PickingPressState.Release
            and pickEvent.pressItem == ivw.PickingPressItem.Primary
            and pickEvent.deltaPressedPosition.x == 0
                and pickEvent.deltaPressedPosition.y == 0):

            selection = self.bnlInport.getSelectedIndices()
            selection.flip(pickEvent.pickedId)
            self.bnlInport.select(selection)

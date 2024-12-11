# Name: ExportVolumeSlice

import inviwopy as ivw
import numpy as np
from pathlib import Path

class ExportVolumeSlice(ivw.Processor):
    """
    Documentation of ExportVolumeSlice
    """

    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.inport = ivw.data.VolumeInport("inport")
        self.addInport(self.inport)

        self.outputDirectory = ivw.properties.DirectoryProperty("outDir", "Output Directory")
        self.addProperty(self.outputDirectory)
        self.outputFile = ivw.properties.StringProperty("outFile", "Filename", "")
        self.addProperty(self.outputFile)
        self.override = ivw.properties.BoolProperty("override", "Override", False)
        self.addProperty(self.override)

        self.xAxis = ivw.properties.BoolCompositeProperty("xAxis", "X Axis", True)
        self.xAxis.addProperty(ivw.properties.IntProperty("xSlice", "Slice", 0, 0, 100, 1))
        self.addProperty(self.xAxis)

        self.yAxis = ivw.properties.BoolCompositeProperty("yAxis", "Y Axis", True)
        self.yAxis.addProperty(ivw.properties.IntProperty("ySlice", "Slice", 0, 0, 100, 1))
        self.addProperty(self.yAxis)

        self.zAxis = ivw.properties.BoolCompositeProperty("zAxis", "Z Axis", True)
        self.zAxis.addProperty(ivw.properties.IntProperty("zSlice", "Slice", 0, 0, 100, 1))
        self.addProperty(self.zAxis)

        self.exportButton = ivw.properties.ButtonProperty("exportButton", "Export")
        self.addProperty(self.exportButton)

        self.volume = None


    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier="org.inviwo.ExportVolumeSlice",
            displayName="ExportVolumeSlice",
            category="Python",
            codeState=ivw.CodeState.Stable,
            tags=ivw.Tags.PY,
            help=ivw.unindentMd2doc(ExportVolumeSlice.__doc__)
        )
    def export(self):
        if not self.inport.hasData():
            return

        # Check if output dir is empty
        if self.outputDirectory.value.samefile(''):
            print("No output directory specified")
            return

        # Check if output file name is empty
        if not self.outputFile.value:
            print("No output file specified")
            return

        data = self.volume.data

        # Concatenate directory and filname
        path = f"{self.outputDirectory.value}/{self.outputFile.value}"

        if self.xAxis.isChecked():
            xSlice = data[:, :, self.zAxis.zSlice.value]
            p = f"{path}_x.npy"
            if Path(p).exists() and not self.override.value:
                print(f"File {p} already exists")
            else:
                np.save(p, xSlice)
                print(f"Saving x slice to {p}")

        if self.yAxis.isChecked():
            ySlice = data[:, self.yAxis.ySlice.value, :]
            p = f"{path}_y.npy"
            if Path(p).exists() and not self.override.value:
                print(f"File {p} already exists")
            else:
                np.save(p, ySlice)
                print(f"Saving y slice to {p}")
        if self.zAxis.isChecked():
            zSlice = data[self.xAxis.xSlice.value, :, :]
            p = f"{path}_z.npy"
            if Path(p).exists() and not self.override.value:
                print(f"File {p} already exists")
            else:
                np.save(p, zSlice)
                print(f"Saving z slice to {p}")


    def getProcessorInfo(self):
        return ExportVolumeSlice.processorInfo()

    def initializeResources(self):
        pass

    def process(self):
        if not self.inport.hasData():
            return

        self.volume = self.inport.getData()

        # OpenGL and Qt have their own context and as of now we cant use the exportButton
        # onChange function to export the data because the context is not set with Qt at
        # the time of the call. Temp fix is to check if the buttonas clicked which is
        # indicated by the isModified flag
        if self.exportButton.isModified:
            self.export()
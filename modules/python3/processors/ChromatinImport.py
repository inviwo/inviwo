# Name: ChromatinImport

import inviwopy as ivw
import ivwdataframe as df

from pathlib import Path
import pandas as pd
import numpy as np
import itertools
import os
import natsort


class ChromatinImport(ivw.Processor):
    """
    Import tab-separated chromatin data from a directory
    """

    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.outport = df.DataFrameOutport(
            "dataframe",
            help=ivw.md2doc('DataFrame representing chromatin data')
        )
        self.addOutport(self.outport)

        self.filepath = ivw.properties.DirectoryProperty("filepath", "Path", "", "text")
        self.haploidColumn = ivw.properties.BoolProperty("haploidColumn",
                                                         "Separate Haploid Column", False)

        self.addProperties([self.filepath, self.haploidColumn])

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier="org.inviwo.ChromatinImport",
            displayName="Chromatin Import",
            category="Python",
            codeState=ivw.CodeState.Stable,
            tags=ivw.Tags.PY,
            help=ivw.unindentMd2doc(ChromatinImport.__doc__)
        )

    def getProcessorInfo(self):
        return ChromatinImport.processorInfo()

    def initializeResources(self):
        pass

    @staticmethod
    def importTextData(path: Path, haploidColumn: bool=False) -> pd.DataFrame:
        files = [f for f in path.iterdir() if f.is_file()
                 and (f.suffix == '.txt' or f.suffix == '.tsv')]
        sorted_files = natsort.natsorted(files)

        # def convert(source):
        #     return int(source) if source.isdigit() else ord(source)
        # sorted_files = sorted(files, key=lambda path: convert(path.stem.rsplit('_', 1)[1]))

        dataframe = pd.DataFrame()
        dataframe = pd.concat([pd.read_table(p, header=None, usecols=[0, 1, 2, 3, 4])
                               for p in sorted_files], ignore_index=True)
        dataframe.columns = ('name', 'position', 'x', 'y', 'z')

        if haploidColumn:
            dataframe[['name', 'haploid']] = dataframe['name'].str.split('hap', n=1, expand=True)

        positions = dataframe[['x', 'y', 'z']].to_numpy()
        euclidean_dist = np.linalg.norm(positions[1:, :] - positions[0:-1, :], axis=1)
        forward_backward_dist = [a + b for a, b in itertools.pairwise(euclidean_dist)]
        forward_backward_dist = np.pad(forward_backward_dist, pad_width=((1, 1)),
                                       mode='constant', constant_values=0)
        s = pd.Series(forward_backward_dist, name='pairdistance')

        return pd.concat([dataframe, s], axis=1)

    @staticmethod
    def convertToInviwoDataFrame(source: pd.DataFrame) -> df.DataFrame:
        dataframe = df.DataFrame()

        for colname in source:
            col = source[colname]
            if col.dtype.name == 'object':
                dataframe.addCategoricalColumn(colname, col.to_list())
            else:
                # ensure 32bit instead of 64
                dtype = col.dtype.name.replace('64', '32')
                dataframe.addColumnFromBuffer(colname, ivw.data.Buffer(col.to_numpy(dtype=dtype)))

        dataframe.updateIndex()

        return dataframe

    def process(self):
        if not self.filepath.value or not os.path.isdir(self.filepath.value):
            return

        dataframe = ChromatinImport.importTextData(self.filepath.value, self.haploidColumn.value)
        ivw_dataframe = ChromatinImport.convertToInviwoDataFrame(dataframe)

        self.outport.setData(ivw_dataframe)

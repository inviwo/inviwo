import numpy as np # requiers version 1.12+
import os
import math

dat = """RawFile: {file}
Resolution: {x} {y} {z}
Format: {format}
ByteOrder:  {byteorder}
"""

ivf = """<?xml version="1.0" ?>
<InviwoTreeData version="1.0">
    <RawFile content="{file}" />
    <Format content="{format}" />
    <Dimension x="{x}" y="{y}" z="{z}" />
    <MetaDataMap>
        <MetaDataItem type="org.inviwo.BoolMetaData" key="LittleEndian">
            <MetaData content="{byteorder}" />
        </MetaDataItem>
    </MetaDataMap>
</InviwoTreeData>
"""

types = [np.dtype(t + str(1)) for t in ('i', 'u')] + \
    [np.dtype(e + t + str(s)) for e in ('<','>') for t in ('i', 'u', 'f') for s in (2, 4, 8)]

# values in each voxel should be: 
# x+y+z

for t in types:
    # use column major as Inviwo instead of numpys defult Row Major
    dims = (3,7,13)
    arr = np.zeros(dims , dtype=t)

    for x, y, z in np.ndindex(arr.shape):
        arr[x, y, z] = x + y + z

    if t.byteorder == '|':
        order = ""
    elif t.byteorder in ('=', '<'):
        order = ".LittleEndian"
    else:
        order = ".BigEndian"

    def toString(t):
        if t.kind == 'f':
            return "FLOAT" + str(t.itemsize*8)
        elif t.kind == 'u':
            return "UINT" + str(t.itemsize*8)
        elif t.kind == 'i':
            return "INT" + str(t.itemsize*8)

    filename = "testdata."+ toString(t) + order
    arr.tofile(filename + ".raw")

    with open(filename + ".dat", 'w') as f:
        f.write(dat.format(
            file = filename +".raw", 
            format = toString(t), 
            byteorder = "LittleEndian" if not t.byteorder == '>' else "BigEndian",
            x = dims[2],
            y = dims[1],
            z = dims[0]
        ))

    with open(filename + ".ivf", 'w') as f:
        f.write(ivf.format(
            file = filename +".raw", 
            format = toString(t), 
            byteorder = "1" if not t.byteorder == '>' else "0",
            x = dims[2],
            y = dims[1],
            z = dims[0]
        ))

    print(filename)
    f = "  {:>12s}: {: 6.8e}" if t.kind == 'f' else "  {:>12s}: {: f}"
    print(f.format("Data Min", arr.min()))
    print(f.format("Data Max", arr.max()))
    print()
    

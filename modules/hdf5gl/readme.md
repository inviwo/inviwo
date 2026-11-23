# HDF5GL Module

Provides converters for uploading HDF5 backed representations (`VolumeHDF`, `LayerHDF`) directly
into OpenGL textures (`VolumeGL`, `LayerGL`). The data is read from the HDF5 file straight into a
mapped pixel buffer object, bypassing an intermediate RAM representation.

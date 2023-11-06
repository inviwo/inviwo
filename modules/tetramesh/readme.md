# TetraMesh Module

This module adds basic rendering support for unstructured grids using tetrahedra like the `TetraMeshVolumeRaycaster`. The `TetraMesh` provides a common interface for arbitrary tetrahedral grids. The data upload to the GPU with the necessary data required for rendering is managed by `TetraMeshBuffers`. See for example `VolumeTetraMesh` and `VTKTetraMesh` in the topovis/ttk module.

Enable the topovis/ttk module for supporting and rendering VTK unstructured grids.

Data structures for tetrahedra indexing and face enumeration based on
    M. Lage, T. Lewiner, H. Lopes, and L. Velho.
	CHF: A scalable topological data structure for tetrahedral meshes.
    In Brazilian Symposium on Computer Graphics and Image Processing
    (SIBGRAPI'05), pp. 349-356, 2005, doi: 10.1109/SIBGRAPI.2005.18

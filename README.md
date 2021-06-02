## Inviwo - Interactive Visualization Workshop

[![Build Status](http://jenkins.inviwo.org:8080/buildStatus/icon?job=inviwo/master)](http://jenkins.inviwo.org:8080/job/inviwo/job/master/) [![BSD licensed](https://img.shields.io/badge/license-BSD-blue.svg?maxAge=2592000)](https://github.com/inviwo/inviwo/blob/master/LICENSE)
[![Release version](https://img.shields.io/github/release/inviwo/inviwo.svg?maxAge=86400)](https://github.com/inviwo/inviwo/releases/latest)

<div class="inviwo-screenshot">

![](docs/images/inviwo-screenshot.png)

</div>

Inviwo is a software framework for rapid visualization prototyping.

Package releases and information is available at www.inviwo.org

Below follow some general information about the framework:

 - Freely available under the Simplified BSD License.
 - Cross-platform and runs on Windows, Linux and Mac OS X.
 - Easily extendable through inclusion of external modules and projects.

#### Core
 - The core is written in C/C++ only, with minor dependencies.

#### Modules
 - Modern graphics programming through OpenGL (>= 3.3 Core) related modules.
 - Parallel computing on multiple platforms (GPU/CPU) through OpenCL (>= 1.0) related modules.
 - Python (> 3.2) scripting and computation is supported through provided modules.

#### GUI
 - The primary GUI is based on Qt (Supported >= 5.12).
 - A minimal application is available, utilizing GLFW 3 for multiple window and context management.

#### Build system
 - The project and module configuration/generation is performed through CMake (>= 3.12.0).
 - Inviwo has been compiled in Visual Studio 2019, Clang 8, GCC 9, XCode 11
 - C++17 Required

#### Documentation
  - [Manual](https://inviwo.org/manual_index.html)
  - [API](https://inviwo.org/inviwo/doc)
  - [Changelog](/CHANGELOG.md)

For help and general discussion join us on our [Slack](https://join.slack.com/t/inviwo/shared_invite/enQtNTc2Nzc2NDQwNzIxLTRiMWM1ZWJiYjljZjkwNWE3OTk3MzYxODZlMDUyMzRmZjUzMzBiZjVhNTM3NWUyNzU1MjI4OWJjMzdkODViMzM) server

### External modules and applications
There are a large number of modules developed and maintained in other repositories.
These can be added through the CMake option `IVW_EXTERNAL_MODULES`, see [Inviwo modules](https://github.com/inviwo/modules) for more details.
The following modules add a variety of functionalities to Inviwo. Please refer to the respective repository for possible issues related to them.

- [Dicom reader](https://github.com/inviwo/modules/tree/master/medvis/dicom) Adds support for reading DICOM image/volume files (.dcm file ending)
- [Molecular visualization](https://github.com/inviwo/modules/tree/master/molvis) Adds support for molecular data structures along with reading PDB and mmCIF files. Molecules are rendered in van-der-Waals, Licorice, and Ball and Stick representations.
-  [Tensor visualization](https://github.com/inviwo/modules/tree/master/tensorvis) Adds support for reading/writing tensor fields. Includes algorithms such as HyperLIC and Anisotropy raycasting.
-  [Topology visualization](https://github.com/inviwo/modules/tree/master/topovis) Integrates the [Topology Toolkit](https://topology-tool-kit.github.io/) into Inviwo. Includes algorithms for critical points, integral lines, persistence diagrams, persistence curves, merge trees, contour trees, Reeb graphs, Morse-Smale complexes, topological simplification, topology-aware compression, harmonic design, fiber surfaces, continuous scatterplots, Jacobi sets, Reeb spaces, bottleneck and Wasserstein distances between persistence diagrams etc.
-  [Clustering](https://github.com/inviwo/modules/tree/master/misc/dataframeclustering) Cluster rows of a DataFrame. Supported clustering methods are k-means, DBSCAN, agglomerative, and spectral clustering
- [Vector Graphics](https://github.com/inviwo/modules/tree/master/misc/nanovgutils) Integrates [NanoVG](https://github.com/memononen/nanovg), which is a small antialiased vector graphics rendering library for OpenGL.
- [NetCDF](https://github.com/inviwo/modules/tree/master/misc/netcdf) Adds support for reading NetCDF files
- [OpenMesh](https://github.com/inviwo/modules/tree/master/misc/openmesh) Integrats the [OpenMesh](https://www.graphics.rwth-aachen.de/software/openmesh/) library, which is a data structure for polygonal meshes. Includes for example mesh decimation and vertex normal generation.
- [Spring-mass system](https://github.com/inviwo/modules/tree/master/misc/springsystem) Simulation of spring-mass system.
- [Visualization Toolkit](https://github.com/inviwo/modules/tree/master/misc/vtk) Integrates support for [VTK](https://gitlab.kitware.com/vtk/vtk), which has algorithms for surface reconstruction, implicit modeling, decimation and much more.
- [Photon mapping](https://github.com/ResearchDaniel/Correlated-Photon-Mapping-for-Interactive-Global-Illumination-of-Time-Varying-Volumetric-Data) Volume illumination algorithm for time-varying heterogenous media.
- [Temporal Tree Maps](https://github.com/Wiebke/TemporalTreeMaps) Temporal treemaps for visualizing trees whose topology and data change over time.
- [Developer tools](https://github.com/inviwo/modules/tree/master/misc/devtools) Log inviwo events, useful for debugging interaction.
- 
#### Applications 
- [Visual Neuro](https://github.com/SciVis/VisualNeuro) A visual analysis tool understanding cohorts of brain imaging and clinical data. Also includes algorithms for statistical computation.
- [ENVISIoN](https://github.com/rartino/ENVISIoN) Visualization of electronic structure quantities from ab-initio calculations.


Is your repository missing above? Just add it and make a pull request!

### Sponsors
This work is supported by Linköping University, Ulm University, and KTH Royal Institute of Technology as well as grants from the Swedish e-Science Research Centre (SeRC), the Excellence Center at Linköping - Lund in Information Technology (ELLIIT), the Swedish Research Council (Vetenskapsrådet), DFG (German Research Foundation), and the BMBF.

<a href='https://www.liu.se'><img src="docs/images/liu-600x600.png" width="150" style="margin-right:10px;"></a>
<a href='https://www.uni-ulm.de/en/'><img src="docs/images/uulm-600x600.png" width="150" style="margin-right:10px;"></a>
<a href='https://www.kth.se'><img src="docs/images/kth-600x600.png" width="150" style="margin-right:10px;"></a>
<a href='https://e-science.se'><img src="docs/images/serc-600x600.png" width="150" style="margin-right:10px;"></a>
<a href='https://old.liu.se/elliit?l=en'><img src="docs/images/elliit-600x600.png" width="150" style="margin-right:10px;"></a>
<a href='https://www.vr.se/english.html'><img src='docs/images/vr-600x600.png' width='150' style='margin-right:10px;'></a>
<a href='https://www.dfg.de/en/index.jsp'><img src='docs/images/dfg-600x600.png' width='150' style='margin-right:10px;'></a>
<a href='https://www.bmbf.de'><img src='docs/images/bmbf-600x600.png' width='150' style='margin-right:10px;'></a>

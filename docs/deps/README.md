# Overview of inviow dependencies:

## the dependencies of inviwo core:
![](core-deps.png)

## the dependencies most big parts
![](vcpkg-deps.png)


# howto
Create by running 

enable all features:
  "default-features": [
    "hdf5",
    "ffmpeg",
    "qt",
    "ttk",
    "vtk",
    "dicom",
    "dome"
  ],
and add python3 to the deps, in vcpkg.json

../vcpkg/vcpkg depend-info --x-install-root=../build/inviwo-vcpkg-dynamic/vcpkg_installed  --overlay-ports=. --overlay-ports=../modules/tools/vcpkg --format dot inviwo

Then remove all the vcpkg_* stuff and collapse all the boost_* to boost in the dot file.

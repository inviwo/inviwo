# Create python module
set(HEADER_FILES
    include/inviwopy/properties/pyminmaxproperties.h
    include/inviwopy/properties/pyoptionproperties.h
    include/inviwopy/properties/pyordinalproperties.h
    include/inviwopy/properties/pyordinalrefproperties.h
    include/inviwopy/pybitset.h
    include/inviwopy/pybuffer.h
    include/inviwopy/pycamera.h
    include/inviwopy/pycameraproperty.h
    include/inviwopy/pycompositeproperties.h
    include/inviwopy/pydataformat.h
    include/inviwopy/pydatamapper.h
    include/inviwopy/pydatareaders.h
    include/inviwopy/pydatawriters.h
    include/inviwopy/pydocument.h
    include/inviwopy/pyevent.h
    include/inviwopy/pyflags.h
    include/inviwopy/pyglmmatports.h
    include/inviwopy/pyglmmattypes.h
    include/inviwopy/pyglmmattypesdouble.h
    include/inviwopy/pyglmmattypesfloat.h
    include/inviwopy/pyglmmattypesint.h
    include/inviwopy/pyglmmattypesuint.h
    include/inviwopy/pyglmports.h
    include/inviwopy/pyglmportsdouble.h
    include/inviwopy/pyglmportsfloat.h
    include/inviwopy/pyglmportsint.h
    include/inviwopy/pyglmportsuint.h
    include/inviwopy/pyglmtypes.h
    include/inviwopy/pyimage.h
    include/inviwopy/pyinviwoapplication.h
    include/inviwopy/pyinviwomodule.h
    include/inviwopy/pylogging.h
    include/inviwopy/pymesh.h
    include/inviwopy/pynetwork.h
    include/inviwopy/pypickingmapper.h
    include/inviwopy/pyport.h
    include/inviwopy/pyprocessors.h
    include/inviwopy/pyproperties.h
    include/inviwopy/pypropertyowner.h
    include/inviwopy/pytfprimitiveset.h
    include/inviwopy/pyvolume.h
    include/inviwopy/util/pyglmhelper.h
    include/inviwopy/util/pypropertyhelper.h
    include/inviwopy/vectoridentifierwrapper.h
)
ivw_group("Header Files" BASE ${CMAKE_CURRENT_SOURCE_DIR}/include/inviwopy ${HEADER_FILES})

set(SOURCE_FILES
    src/inviwopy.cpp
    src/properties/pyminmaxproperties.cpp
    src/properties/pyoptionproperties.cpp
    src/properties/pyordinalproperties.cpp
    src/properties/pyordinalrefproperties.cpp
    src/pybitset.cpp
    src/pybuffer.cpp
    src/pycamera.cpp
    src/pycameraproperty.cpp
    src/pycompositeproperties.cpp
    src/pydataformat.cpp
    src/pydatamapper.cpp
    src/pydatareaders.cpp
    src/pydatawriters.cpp
    src/pydocument.cpp
    src/pyevent.cpp
    src/pyflags.cpp
    src/pyglmmatports.cpp
    src/pyglmmattypes.cpp
    src/pyglmmattypesdouble.cpp
    src/pyglmmattypesfloat.cpp
    src/pyglmmattypesint.cpp
    src/pyglmmattypesuint.cpp
    src/pyglmports.cpp
    src/pyglmportsdouble.cpp
    src/pyglmportsfloat.cpp
    src/pyglmportsint.cpp
    src/pyglmportsuint.cpp
    src/pyglmtypes.cpp
    src/pyimage.cpp
    src/pyinviwoapplication.cpp
    src/pyinviwomodule.cpp
    src/pylogging.cpp
    src/pymesh.cpp
    src/pynetwork.cpp
    src/pypickingmapper.cpp
    src/pyport.cpp
    src/pyprocessors.cpp
    src/pyproperties.cpp
    src/pypropertyowner.cpp
    src/pytfprimitiveset.cpp
    src/pyvolume.cpp
    src/util/pyglmhelper.cpp
    src/util/pypropertyhelper.cpp
    src/vectoridentifierwrapper.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_add_py_wrapper(inviwopy)
target_sources(inviwopy
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS include
    FILES
        ${HEADER_FILES}
    PRIVATE
        ${SOURCE_FILES}
)
target_link_libraries(inviwopy PUBLIC inviwo::core inviwo::module::python3)


ivw_install_helper(TARGET inviwopy
    NAMESPACE inviwo
    DESTINATION inviwo
)


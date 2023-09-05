#--------------------------------------------------------------------
# Inviwo OpenCL Module
ivw_module(OpenCL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/opencl/buffer/buffercl.h
    include/modules/opencl/buffer/bufferclbase.h
    include/modules/opencl/buffer/bufferclconverter.h
    include/modules/opencl/buffer/bufferclgl.h
    include/modules/opencl/buffer/bufferclglconverter.h
    include/modules/opencl/cl.hpp
    include/modules/opencl/clformats.h
    include/modules/opencl/clockcl.h
    include/modules/opencl/glmcl.h
    include/modules/opencl/image/imagecl.h
    include/modules/opencl/image/imageclgl.h
    include/modules/opencl/image/layercl.h
    include/modules/opencl/image/layerclbase.h
    include/modules/opencl/image/layerclconverter.h
    include/modules/opencl/image/layerclgl.h
    include/modules/opencl/image/layerclglconverter.h
    include/modules/opencl/image/layerclresizer.h
    include/modules/opencl/inviwoopencl.h
    include/modules/opencl/kernelmanager.h
    include/modules/opencl/kernelowner.h
    include/modules/opencl/light/packedlightsource.h
    include/modules/opencl/openclcapabilities.h
    include/modules/opencl/openclexception.h
    include/modules/opencl/openclformatexception.h
    include/modules/opencl/openclmodule.h
    include/modules/opencl/openclmoduledefine.h
    include/modules/opencl/settings/openclsettings.h
    include/modules/opencl/syncclgl.h
    include/modules/opencl/utilcl.h
    include/modules/opencl/volume/volumecl.h
    include/modules/opencl/volume/volumeclbase.h
    include/modules/opencl/volume/volumeclconverter.h
    include/modules/opencl/volume/volumeclgl.h
    include/modules/opencl/volume/volumeclglconverter.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/buffer/buffercl.cpp
    src/buffer/bufferclbase.cpp
    src/buffer/bufferclconverter.cpp
    src/buffer/bufferclgl.cpp
    src/buffer/bufferclglconverter.cpp
    src/cl.cpp
    src/clformats.cpp
    src/clockcl.cpp
    src/glmcl.cpp
    src/image/imagecl.cpp
    src/image/imageclgl.cpp
    src/image/layercl.cpp
    src/image/layerclbase.cpp
    src/image/layerclconverter.cpp
    src/image/layerclgl.cpp
    src/image/layerclglconverter.cpp
    src/image/layerclresizer.cpp
    src/inviwoopencl.cpp
    src/kernelmanager.cpp
    src/kernelowner.cpp
    src/light/packedlightsource.cpp
    src/openclcapabilities.cpp
    src/openclexception.cpp
    src/openclformatexception.cpp
    src/openclmodule.cpp
    src/settings/openclsettings.cpp
    src/syncclgl.cpp
    src/utilcl.cpp
    src/volume/volumecl.cpp
    src/volume/volumeclbase.cpp
    src/volume/volumeclconverter.cpp
    src/volume/volumeclgl.cpp
    src/volume/volumeclglconverter.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add OpenCL files
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/datastructures/bbox.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/datastructures/lightsource.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/intersection/rayboxintersection.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/intersection/raydiscintersection.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/intersection/rayplaneintersection.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/intersection/raysphereintersection.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/intersection/raytriangleintersection.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/shading/abcbrdf.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/shading/ashikiminbrdf.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/shading/blinnphongbrdf.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/shading/cooktorrancebrdf.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/shading/microfacet.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/shading/phasefunctions.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/shading/shading.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/shading/shadingmath.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/shading/wardbrdf.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/colorconversion.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/gradients.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/image3d_write.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/img_resize.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/samplers.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/transformations.cl
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add kernel directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/cl)

find_package(OpenCL QUIET REQUIRED)
target_link_libraries(inviwo-module-opencl PUBLIC ${OpenCL_LIBRARY})
target_include_directories(inviwo-module-opencl PUBLIC 
    $<BUILD_INTERFACE:${OpenCL_INCLUDE_DIR}>
)
target_compile_definitions(inviwo-module-opencl PUBLIC __CL_ENABLE_EXCEPTIONS)
if(APPLE)
    target_compile_definitions(inviwo-module-opencl PUBLIC CL_SILENCE_DEPRECATION)
endif()

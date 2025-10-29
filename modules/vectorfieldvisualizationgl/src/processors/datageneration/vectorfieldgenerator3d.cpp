/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/vectorfieldvisualizationgl/processors/datageneration/vectorfieldgenerator3d.h>

#include <inviwo/core/datastructures/datamapper.h>                      // for DataMapper
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume
#include <inviwo/core/ports/volumeport.h>                               // for VolumeOutport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/minmaxproperty.h>                      // for FloatMinMaxProperty
#include <inviwo/core/properties/ordinalproperty.h>                     // for OrdinalProperty
#include <inviwo/core/properties/stringproperty.h>                      // for StringProperty
#include <inviwo/core/util/formats.h>                                   // for DataVec3Float32
#include <inviwo/core/util/glmmat.h>                                    // for mat3
#include <inviwo/core/util/glmvec.h>                                    // for size3_t, vec3, dvec2
#include <modules/opengl/buffer/framebufferobject.h>                    // for FrameBufferObject
#include <modules/opengl/inviwoopengl.h>                                // for glViewport, GLsizei
#include <modules/opengl/shader/shader.h>                               // for Shader, Shader::B...
#include <modules/opengl/shader/shaderobject.h>                         // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>                          // for setUniforms
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>                        // for multiDrawImagePla...
#include <modules/opengl/volume/volumegl.h>                             // for VolumeGL
#include <modules/opengl/volume/volumeutils.h>                          // for bindAndSetUniforms

#include <functional>     // for __base
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set

#include <glm/detail/qualifier.hpp>  // for tvec2
#include <glm/mat3x3.hpp>            // for mat<>::col_type
#include <glm/vec2.hpp>              // for vec<>::(anonymous)
#include <glm/vec3.hpp>              // for operator-, vec

namespace inviwo {

const ProcessorInfo VectorFieldGenerator3D::processorInfo_{
    "org.inviwo.VectorFieldGenerator3D",  // Class identifier
    "Vector Field Generator 3D",          // Display name
    "Data Creation",                      // Category
    CodeState::Stable,                    // Code state
    "GL, Generator",                      // Tags
};
const ProcessorInfo& VectorFieldGenerator3D::getProcessorInfo() const { return processorInfo_; }

VectorFieldGenerator3D::VectorFieldGenerator3D()
    : VolumeGLProcessor("vectorfieldgenerator3d.frag",
                        {.dimensions = size3_t{16},
                         .format = DataVec3Float32::get(),
                         .dataRange = dvec2(0, 1),
                         .valueRange = dvec2(-1, 1)},
                        VolumeGLProcessor::UseInport::No)
    , size_("size", "Volume size", size3_t(16), size3_t(1), size3_t(1024))
    , xRange_("xRange", "X Range", -1, 1, -10, 10)
    , yRange_("yRange", "Y Range", -1, 1, -10, 10)
    , zRange_("zRange", "Z Range", -1, 1, -10, 10)
    , xValue_("x", "X", "-y", InvalidationLevel::InvalidResources)
    , yValue_("y", "Y", "x", InvalidationLevel::InvalidResources)
    , zValue_("z", "Z", "(1-sqrt(x*x+y*y))*0.4", InvalidationLevel::InvalidResources) {

    addProperties(size_, xValue_, yValue_, zValue_, xRange_, yRange_, zRange_);

    outport_.setIdentifier("outport");
}

VectorFieldGenerator3D::~VectorFieldGenerator3D() {}

void VectorFieldGenerator3D::initializeShader(Shader& shader) {
    shader.getFragmentShaderObject()->addShaderDefine("X_VALUE(x,y,z)", xValue_.get());
    shader.getFragmentShaderObject()->addShaderDefine("Y_VALUE(x,y,z)", yValue_.get());
    shader.getFragmentShaderObject()->addShaderDefine("Z_VALUE(x,y,z)", zValue_.get());
}
void VectorFieldGenerator3D::preProcess(TextureUnitContainer&, Shader& shader,
                                        VolumeConfig& config) {
    utilgl::setUniforms(shader, xRange_, yRange_, zRange_);
    config.dimensions = size_.get();
}
void VectorFieldGenerator3D::postProcess(Volume& volume) {
    vec3 corners[4];
    corners[0] = vec3(xRange_.get().x, yRange_.get().x, zRange_.get().x);
    corners[1] = vec3(xRange_.get().y, yRange_.get().x, zRange_.get().x);
    corners[2] = vec3(xRange_.get().x, yRange_.get().y, zRange_.get().x);
    corners[3] = vec3(xRange_.get().x, yRange_.get().x, zRange_.get().y);

    mat3 basis;
    basis[0] = corners[1] - corners[0];
    basis[1] = corners[2] - corners[0];
    basis[2] = corners[3] - corners[0];

    volume.setBasis(basis);
    volume.setOffset(corners[0]);
}

}  // namespace inviwo

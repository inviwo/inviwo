/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

#include <modules/base/processors/cubeproxygeometryprocessor.h>

#include <inviwo/core/algorithm/markdown.h>            // for operator""_help, operator""_uninde...
#include <inviwo/core/network/networklock.h>           // for NetworkLock
#include <inviwo/core/ports/meshport.h>                // for MeshOutport
#include <inviwo/core/ports/volumeport.h>              // for VolumeInport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>      // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>       // for BoolProperty
#include <inviwo/core/properties/minmaxproperty.h>     // for IntMinMaxProperty, MinMaxProperty
#include <inviwo/core/properties/valuewrapper.h>       // for PropertySerializationMode, Propert...
#include <inviwo/core/util/glmvec.h>                   // for ivec2, size3_t
#include <inviwo/core/util/volumeutils.h>              // for getVolumeDimensions
#include <modules/base/algorithm/cubeproxygeometry.h>  // for createCubeProxyGeometry
#include <modules/base/algorithm/meshutils.h>          // for IncludeNormals, IncludeNormals::No

#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view

#include <glm/detail/qualifier.hpp>  // for tvec2
#include <glm/vec2.hpp>              // for vec<>::(anonymous)
#include <glm/vec3.hpp>              // for vec, operator!=, vec<>::(anonymous)

namespace inviwo {
class Mesh;

const ProcessorInfo CubeProxyGeometry::processorInfo_{
    "org.inviwo.CubeProxyGeometry",  // Class identifier
    "Cube Proxy Geometry",           // Display name
    "Mesh Creation",                 // Category
    CodeState::Stable,               // Code state
    Tags::CPU,                       // Tags
    R"(Constructs a proxy geometry based on the model and world matrix of the input volume.
    The geometry will be shaped as a parallelepiped. If clipping is enabled the geometry
    will be cut along the corresponding axes.)"_unindentHelp};

const ProcessorInfo CubeProxyGeometry::getProcessorInfo() const { return processorInfo_; }

CubeProxyGeometry::CubeProxyGeometry()
    : Processor()
    , inport_("volume", "Input Volume"_help)
    , outport_("proxyGeometry", "Output proxy geometry"_help)
    , addFaceNormals_("addFaceNormals", "Add Face Normals",
                      "Face normals are included in the mesh"_help, true)
    , clippingEnabled_("clippingEnabled", "Enable Clipping",
                       "Enable axis aligned clipping of the mesh"_help, true)
    , clipX_("clipX", "Clip X Slices", "Clip X axis"_help, 0, 256, 0, 256, 1, 1)
    , clipY_("clipY", "Clip Y Slices", "Clip Y axis"_help, 0, 256, 0, 256, 1, 1)
    , clipZ_("clipZ", "Clip Z Slices", "Clip Z axis"_help, 0, 256, 0, 256, 1, 1) {

    addPorts(inport_, outport_);
    addProperties(addFaceNormals_, clippingEnabled_, clipX_, clipY_, clipZ_);

    // Since the clips depend on the input volume dimensions, we make sure to always
    // serialize them so we can do a proper renormalization when we load new data.
    clipX_.setSerializationMode(PropertySerializationMode::All);
    clipY_.setSerializationMode(PropertySerializationMode::All);
    clipZ_.setSerializationMode(PropertySerializationMode::All);

    inport_.onChange([this]() {
        const auto volume = inport_.getData();
        // Update to the new dimensions.
        const auto dims = util::getVolumeDimensions(volume);

        if (dims !=
            size3_t(clipX_.getRangeMax() - 1, clipY_.getRangeMax() - 1, clipZ_.getRangeMax() - 1)) {
            NetworkLock lock(this);

            clipX_.setRangeNormalized(ivec2(0, dims.x - 1));
            clipY_.setRangeNormalized(ivec2(0, dims.y - 1));
            clipZ_.setRangeNormalized(ivec2(0, dims.z - 1));

            // set the new dimensions to default if we were to press reset
            clipX_.setCurrentStateAsDefault();
            clipY_.setCurrentStateAsDefault();
            clipZ_.setCurrentStateAsDefault();
        }
    });
}

CubeProxyGeometry::~CubeProxyGeometry() = default;

void CubeProxyGeometry::process() {
    auto normals = addFaceNormals_ ? meshutil::IncludeNormals::Yes : meshutil::IncludeNormals::No;
    std::shared_ptr<Mesh> mesh;
    if (clippingEnabled_.get()) {
        const size3_t clipMin(clipX_->x, clipY_->x, clipZ_->x);
        const size3_t clipMax(clipX_->y, clipY_->y, clipZ_->y);
        mesh = algorithm::createCubeProxyGeometry(inport_.getData(), clipMin, clipMax, normals);
    } else {
        mesh = algorithm::createCubeProxyGeometry(inport_.getData(), normals);
    }
    outport_.setData(mesh);
}

}  // namespace inviwo

/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include "cubeproxygeometry.h"
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <inviwo/core/network/networklock.h>

namespace inviwo {

const ProcessorInfo CubeProxyGeometry::processorInfo_{
    "org.inviwo.CubeProxyGeometry",  // Class identifier
    "Cube Proxy Geometry",           // Display name
    "Geometry Creation",             // Category
    CodeState::Stable,               // Code state
    Tags::CPU,                       // Tags
};
const ProcessorInfo CubeProxyGeometry::getProcessorInfo() const {
    return processorInfo_;
}

CubeProxyGeometry::CubeProxyGeometry()
    : Processor()
    , inport_("volume.inport")
    , outport_("geometry.outport")
    , clippingEnabled_("clippingEnabled", "Enable Clipping", true)
    , clipX_("clipX", "Clip X Slices", 0, 256, 0, 256, 1, 1)
    , clipY_("clipY", "Clip Y Slices", 0, 256, 0, 256, 1, 1)
    , clipZ_("clipZ", "Clip Z Slices", 0, 256, 0, 256, 1, 1) {

    addPort(inport_);
    addPort(outport_);
    addProperty(clippingEnabled_);
    addProperty(clipX_);
    addProperty(clipY_);
    addProperty(clipZ_);
    

    // Since the clips depend on the input volume dimensions, we make sure to always
    // serialize them so we can do a proper renormalization when we load new data.
    clipX_.setSerializationMode(PropertySerializationMode::All);
    clipY_.setSerializationMode(PropertySerializationMode::All);
    clipZ_.setSerializationMode(PropertySerializationMode::All);

    inport_.onChange(this, &CubeProxyGeometry::onVolumeChange);
}

CubeProxyGeometry::~CubeProxyGeometry() {}

void CubeProxyGeometry::process() {
    vec3 startDataTexCoord = vec3(0.0f);
    vec3 extent = vec3(1.0f); //!< will be added to the origin in the parallelepiped() yielding the end position

    if (inport_.getData()->getMetaData<BoolMetaData>("marginsEnabled", false)) {
        // volume has margins enabled
        // adjust start and end texture coordinate accordingly
        auto marginsBottomLeft = inport_.getData()->getMetaData<FloatVec3MetaData>("marginsBottomLeft", vec3(0.0f));
        auto marginsTopRight = inport_.getData()->getMetaData<FloatVec3MetaData>("marginsTopRight", vec3(0.0f));

        startDataTexCoord += marginsBottomLeft;
        // extent needs to be adjusted for both margins
        extent -= marginsBottomLeft + marginsTopRight;
    }

    glm::vec3 pos(0.0f);
    glm::vec3 p1(1.0f, 0.0f, 0.0f);
    glm::vec3 p2(0.0f, 1.0f, 0.0f);
    glm::vec3 p3(0.0f, 0.0f, 1.0f);
    glm::vec3 tex(startDataTexCoord);
    glm::vec3 t1(extent.x, 0.0f, 0.0f);
    glm::vec3 t2(0.0f, extent.y, 0.0f);
    glm::vec3 t3(0.0f, 0.0f, extent.z);
    glm::vec4 col(startDataTexCoord, 1.0f);
    glm::vec4 c1(t1, 0.0f);
    glm::vec4 c2(t2, 0.0f);
    glm::vec4 c3(t3, 0.0f);
    auto dims = inport_.getData()->getDimensions();

    if (clippingEnabled_.get()) {
        pos = pos + p1*static_cast<float>(clipX_.get().x)/static_cast<float>(dims.x)
                  + p2*static_cast<float>(clipY_.get().x)/static_cast<float>(dims.y)
                  + p3*static_cast<float>(clipZ_.get().x)/static_cast<float>(dims.z);
        p1 = p1*(static_cast<float>(clipX_.get().y)-static_cast<float>(clipX_.get().x))/static_cast<float>(dims.x);
        p2 = p2*(static_cast<float>(clipY_.get().y)-static_cast<float>(clipY_.get().x))/static_cast<float>(dims.y);
        p3 = p3*(static_cast<float>(clipZ_.get().y)-static_cast<float>(clipZ_.get().x))/static_cast<float>(dims.z);
        tex = tex + t1*static_cast<float>(clipX_.get().x)/static_cast<float>(dims.x)
                  + t2*static_cast<float>(clipY_.get().x)/static_cast<float>(dims.y)
                  + t3*static_cast<float>(clipZ_.get().x)/static_cast<float>(dims.z);
        t1 = t1*(static_cast<float>(clipX_.get().y)-static_cast<float>(clipX_.get().x))/static_cast<float>(dims.x);
        t2 = t2*(static_cast<float>(clipY_.get().y)-static_cast<float>(clipY_.get().x))/static_cast<float>(dims.y);
        t3 = t3*(static_cast<float>(clipZ_.get().y)-static_cast<float>(clipZ_.get().x))/static_cast<float>(dims.z);
        col = col + c1*static_cast<float>(clipX_.get().x)/static_cast<float>(dims.x)
                  + c2*static_cast<float>(clipY_.get().x)/static_cast<float>(dims.y)
                  + c3*static_cast<float>(clipZ_.get().x)/static_cast<float>(dims.z);
        c1 = c1*(static_cast<float>(clipX_.get().y)-static_cast<float>(clipX_.get().x))/static_cast<float>(dims.x);
        c2 = c2*(static_cast<float>(clipY_.get().y)-static_cast<float>(clipY_.get().x))/static_cast<float>(dims.y);
        c3 = c3*(static_cast<float>(clipZ_.get().y)-static_cast<float>(clipZ_.get().x))/static_cast<float>(dims.z);
    }

    // Create parallelepiped and set it to the outport. The outport will own the data.
    auto geom = SimpleMeshCreator::parallelepiped(pos, p1, p2, p3,
                                                  tex, t1, t2, t3,
                                                  col, c1, c2, c3);
    geom->setModelMatrix(inport_.getData()->getModelMatrix());
    geom->setWorldMatrix(inport_.getData()->getWorldMatrix());
    outport_.setData(geom);
}

void CubeProxyGeometry::onVolumeChange() {
    // Update to the new dimensions.
    auto dims = inport_.getData()->getDimensions();
    if (dims != size3_t(clipX_.getRangeMax(), clipY_.getRangeMax(), clipZ_.getRangeMax())) {
        NetworkLock lock(this);

        clipX_.setRangeNormalized(ivec2(0, dims.x));
        clipY_.setRangeNormalized(ivec2(0, dims.y));
        clipZ_.setRangeNormalized(ivec2(0, dims.z));

        // set the new dimensions to default if we were to press reset
        clipX_.setCurrentStateAsDefault();
        clipY_.setCurrentStateAsDefault();
        clipZ_.setCurrentStateAsDefault();
    }

}

} // namespace


/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2016 Inviwo Foundation
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
    "Mesh Creation",             // Category
    CodeState::Stable,               // Code state
    Tags::CPU,                       // Tags
};
const ProcessorInfo CubeProxyGeometry::getProcessorInfo() const {
    return processorInfo_;
}

CubeProxyGeometry::CubeProxyGeometry()
    : Processor()
    , inport_("volume")
    , outport_("proxyGeometry")
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

    glm::vec3 origin(0.0f);
    glm::vec3 e1(1.0f, 0.0f, 0.0f);
    glm::vec3 e2(0.0f, 1.0f, 0.0f);
    glm::vec3 e3(0.0f, 0.0f, 1.0f);
    glm::vec3 texOrigin(startDataTexCoord);
    glm::vec3 t1(extent.x, 0.0f, 0.0f);
    glm::vec3 t2(0.0f, extent.y, 0.0f);
    glm::vec3 t3(0.0f, 0.0f, extent.z);
    glm::vec4 colOrigin(startDataTexCoord, 1.0f);
    glm::vec4 c1(t1, 0.0f);
    glm::vec4 c2(t2, 0.0f);
    glm::vec4 c3(t3, 0.0f);

    if (clippingEnabled_.get()) {

        vec3 clipRange(clipX_.getRangeMax(), clipY_.getRangeMax(), clipZ_.getRangeMax());

        vec3 clipMin(clipX_.get().x, clipY_.get().x, clipZ_.get().x);
        vec3 clipMax(clipX_.get().y, clipY_.get().y, clipZ_.get().y);

        vec3 min(clipMin / clipRange);
        vec3 clipextent((clipMax - clipMin) / clipRange);

        origin = origin + e1 * min.x + e2 * min.y + e3 * min.z;
        e1 *= clipextent.x;
        e2 *= clipextent.y;
        e3 *= clipextent.z;

        texOrigin = texOrigin + t1 * min.x + t2 * min.y + t3 * min.z;
        t1 *= clipextent.x;
        t2 *= clipextent.y;
        t3 *= clipextent.z;

        colOrigin += c1 * min.x + c2 * min.y + c3 * min.z;
        c1 *= clipextent.x;
        c2 *= clipextent.y;
        c3 *= clipextent.z;
    }

    // Create parallelepiped and set it to the outport. The outport will own the data.
    auto geom = SimpleMeshCreator::parallelepiped(origin, e1, e2, e3,
                                                  texOrigin, t1, t2, t3,
                                                  colOrigin, c1, c2, c3);
    geom->setModelMatrix(inport_.getData()->getModelMatrix());
    geom->setWorldMatrix(inport_.getData()->getWorldMatrix());
    outport_.setData(geom);
}

void CubeProxyGeometry::onVolumeChange() {
    // Update to the new dimensions.
    auto dims = inport_.getData()->getDimensions();
    
    if (inport_.getData()->getMetaData<BoolMetaData>("brickedVolume", false)) {
        // adjust dimensions for bricked volumes

        // volume dimensions refer only to the size of the index volume, multiply it by brick dimension
        auto brickDim = inport_.getData()->getMetaData<IntVec3MetaData>("brickDim", ivec3(1, 1, 1));
        dims *= size3_t(brickDim);
    }
    // re-adjust slider ranges by considering margins
    // the clip range should not cover the area within the margins
    if (inport_.getData()->getMetaData<BoolMetaData>("marginsEnabled", false)) {
        // volume has margins enabled
        // adjust start and end texture coordinate accordingly
        auto marginsBottomLeft = inport_.getData()->getMetaData<FloatVec3MetaData>("marginsBottomLeft", vec3(0.0f));
        auto marginsTopRight = inport_.getData()->getMetaData<FloatVec3MetaData>("marginsTopRight", vec3(0.0f));
        
        dims = size3_t(vec3(dims) * (vec3(1.0f) - (marginsBottomLeft + marginsTopRight)));
    }

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


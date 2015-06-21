/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "surfaceextraction.h"

#include <inviwo/core/properties/propertysemantics.h>
#include <modules/base/algorithm/volume/marchingtetrahedron.h>

#define TETRA 1

namespace inviwo {

ProcessorClassIdentifier(SurfaceExtraction, "org.inviwo.SurfaceExtraction");
ProcessorDisplayName(SurfaceExtraction, "Surface Extraction");
ProcessorTags(SurfaceExtraction, Tags::CPU);
ProcessorCategory(SurfaceExtraction, "Geometry Creation");
ProcessorCodeState(SurfaceExtraction, CODE_STATE_EXPERIMENTAL);

// TODO make changing color not rerun extraction but only change the color, (and run only
// extraction when volume change or iso change)

SurfaceExtraction::SurfaceExtraction()
    : Processor()
    , volume_("volume")
    , mesh_("mesh")
    , isoValue_("iso", "ISO Value", 0.5f, 0.0f, 1.0f)
    , method_("method", "Method")
    , color_("color", "Triangle Color", vec4(1, 1, 1, 1)) {
    addPort(volume_);
    addPort(mesh_);

    addProperty(method_);
    addProperty(isoValue_);
    addProperty(color_);

    method_.addOption("marchingtetrahedra", "Marching Tetrahedra", TETRA);

    color_.setSemantics(PropertySemantics::Color);

    volume_.onChange(this, &SurfaceExtraction::setMinMax);

    method_.setCurrentStateAsDefault();
    color_.setCurrentStateAsDefault();
}
SurfaceExtraction::~SurfaceExtraction() {}

void SurfaceExtraction::process() {
    const VolumeRAM* vol = volume_.getData()->getRepresentation<VolumeRAM>();

    switch (method_.get()) {
        case TETRA:
            mesh_.setData(MarchingTetrahedron::apply(vol, isoValue_.get(), color_.get()));
            break;
        default:
            break;
    }

    mesh_.getData()->setModelMatrix(volume_.getData()->getModelMatrix());
    mesh_.getData()->setWorldMatrix(volume_.getData()->getWorldMatrix());
}

void SurfaceExtraction::setMinMax() {
    if (volume_.hasData()) {
        isoValue_.setMinValue(static_cast<const float>(volume_.getData()->dataMap_.dataRange.x));
        isoValue_.setMaxValue(static_cast<const float>(volume_.getData()->dataMap_.dataRange.y));
    }
}

}  // namespace

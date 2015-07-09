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
#include <inviwo/core/common/inviwoapplication.h>

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
    , colors_("meshColors", "Mesh Colors") {
    addPort(volume_);
    addPort(mesh_);

    addProperty(method_);
    addProperty(isoValue_);
    addProperty(colors_);

    getProgressBar().hide();

    method_.addOption("marchingtetrahedra", "Marching Tetrahedra", TETRA);

    volume_.onChange(this, &SurfaceExtraction::setMinMax);
    volume_.onChange(this, &SurfaceExtraction::updateColors);

    method_.setCurrentStateAsDefault();
}
SurfaceExtraction::~SurfaceExtraction() {}

void SurfaceExtraction::process() {
    auto meshList = mesh_.getData();
    if (!meshList) {
        meshList = new std::vector<std::unique_ptr<Mesh>>();
        mesh_.setData(meshList);
    }

    auto data = volume_.getSourceVectorData();
    auto changed = volume_.getChangedOutports();
    result_.resize(data.size());
    meshList->resize(data.size());

    for (size_t i = 0; i < data.size(); ++i) {
        const VolumeRAM* vol = data[i].second->getRepresentation<VolumeRAM>();

        switch (method_.get()) {
            case TETRA: {
                if (result_[i].result.valid() &&
                    result_[i].result.wait_for(std::chrono::duration<int, std::milli>(0)) ==
                        std::future_status::ready) {
                    (*meshList)[i] = std::move(result_[i].result.get());
                    result_[i].status = 1.0f;
                }

                float iso = isoValue_.get();
                vec4 color = static_cast<FloatVec4Property*>(colors_.getProperties()[i])->get();
                if (!result_[i].result.valid() && (util::contains(changed, data[i].first) ||
                                            result_[i].iso != iso || result_[i].color != color)) {
                    result_[i].iso = iso;
                    result_[i].color = color;
                    result_[i].status = 0.0f;
                    result_[i].result =
                        dispatchPoolAndInvalidate(this, [this, vol, iso, color, i]() -> std::unique_ptr<Mesh> {
                            auto m = std::unique_ptr<Mesh>(
                                MarchingTetrahedron::apply(vol, iso, color, [this, i](float s) {
                                    this->result_[i].status = s;
                                    float status = 0;
                                    for (const auto& e : this->result_) status += e.status;
                                    status /= result_.size();
                                    dispatchFront([status](ProgressBar& pb) {
                                        if (status < 1.0f) pb.show();
                                        else pb.hide();
                                        pb.updateProgress(status);
                                    }, std::ref(this->getProgressBar()));

                                }));
                            return std::move(m);
                        });
                }
                break;
            }
            default:
                break;
        }
    }
}

void SurfaceExtraction::setMinMax() {
    if (volume_.hasData()) {
        isoValue_.setMinValue(static_cast<const float>(volume_.getData()->dataMap_.dataRange.x));
        isoValue_.setMaxValue(static_cast<const float>(volume_.getData()->dataMap_.dataRange.y));
    }
}

void SurfaceExtraction::updateColors() {
    const static vec4 defaultColor[11] = {vec4(1.0f),
                                          vec4(0x1f, 0x77, 0xb4, 255) / vec4(255, 255, 255, 255),
                                          vec4(0xff, 0x7f, 0x0e, 255) / vec4(255, 255, 255, 255),
                                          vec4(0x2c, 0xa0, 0x2c, 255) / vec4(255, 255, 255, 255),
                                          vec4(0xd6, 0x27, 0x28, 255) / vec4(255, 255, 255, 255),
                                          vec4(0x94, 0x67, 0xbd, 255) / vec4(255, 255, 255, 255),
                                          vec4(0x8c, 0x56, 0x4b, 255) / vec4(255, 255, 255, 255),
                                          vec4(0xe3, 0x77, 0xc2, 255) / vec4(255, 255, 255, 255),
                                          vec4(0x7f, 0x7f, 0x7f, 255) / vec4(255, 255, 255, 255),
                                          vec4(0xbc, 0xbd, 0x22, 255) / vec4(255, 255, 255, 255),
                                          vec4(0x17, 0xbe, 0xcf, 255) / vec4(255, 255, 255, 255)};

    auto properties = colors_.getProperties();
    auto numConnections = volume_.getNumberOfConnections();

    for (size_t i = 0; i < properties.size(); i++) {
        properties[i]->setVisible(i < numConnections);
    }

    for (size_t i = properties.size(); i < numConnections; i++) {
        const static std::string color = "color";
        const static std::string dispName = "Color for Volume ";
        FloatVec4Property* colorProp = new FloatVec4Property(
            color + toString(i), dispName + toString(i + 1), defaultColor[i % 11]);
        colorProp->setCurrentStateAsDefault();
        colorProp->setSemantics(PropertySemantics::Color);
        colorProp->setSerializationMode(PropertySerializationMode::ALL);
        colors_.addProperty(colorProp);
    }
}

SurfaceExtraction::task::task(task&& rhs)
    : result(std::move(rhs.result))
    , iso(rhs.iso)
    , color(std::move(rhs.color))
    , status(rhs.status) {
}

SurfaceExtraction::task& SurfaceExtraction::task::operator=(task&& that) {
    if (this != &that) {
        result = std::move(that.result);
        iso = that.iso;
        color = std::move(that.color);
        status = that.status;
    }

    return *this;
}

}  // namespace

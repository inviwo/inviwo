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
#include <inviwo/core/util/stdextensions.h>
#include <numeric>

#define TETRA 1

namespace inviwo {

const ProcessorInfo SurfaceExtraction::processorInfo_{
    "org.inviwo.SurfaceExtraction",  // Class identifier
    "Surface Extraction",            // Display name
    "Geometry Creation",             // Category
    CodeState::Experimental,         // Code state
    Tags::CPU,                       // Tags
};
const ProcessorInfo SurfaceExtraction::getProcessorInfo() const {
    return processorInfo_;
}

// TODO make changing color not rerun extraction but only change the color, (and run only
// extraction when volume change or iso change)

SurfaceExtraction::SurfaceExtraction()
    : Processor()
    , volume_("volume")
    , outport_("mesh")
    , isoValue_("iso", "ISO Value", 0.5f, 0.0f, 1.0f, 0.01f)
    , method_("method", "Method")
    , colors_("meshColors", "Mesh Colors")
    , dirty_(false) {
    addPort(volume_);
    addPort(outport_);

    addProperty(method_);
    addProperty(isoValue_);
    addProperty(colors_);

    getProgressBar().hide();

    method_.addOption("marchingtetrahedra", "Marching Tetrahedra", TETRA);
    volume_.onChange(this, &SurfaceExtraction::updateColors);
    volume_.onChange(this, &SurfaceExtraction::setMinMax);
    method_.setCurrentStateAsDefault();
}

SurfaceExtraction::~SurfaceExtraction() {}

void SurfaceExtraction::process() {
    if (!meshes_) {
        meshes_ = std::make_shared<std::vector<std::shared_ptr<Mesh>>>();
        outport_.setData(meshes_);
    }

    auto data = volume_.getSourceVectorData();
    auto changed = volume_.getChangedOutports();
    result_.resize(data.size());
    meshes_->resize(data.size());

    for (size_t i = 0; i < data.size(); ++i) {
        auto vol = data[i].second;

        switch (method_.get()) {
            case TETRA: {
                if (util::is_future_ready(result_[i].result)) {
                    (*meshes_)[i] = std::move(result_[i].result.get());
                    result_[i].status = 1.0f;
                    dirty_ = false;
                }

                float iso = isoValue_.get();
                vec4 color = static_cast<FloatVec4Property*>(colors_[i])->get();
                if (!result_[i].result.valid() &&
                    (util::contains(changed, data[i].first) || !result_[i].isSame(iso, color))) {
                    result_[i].set(
                        iso, color, 0.0f,
                        dispatchPool([this, vol, iso, color, i]() -> std::shared_ptr<Mesh> {
                            auto m =
                                MarchingTetrahedron::apply(vol, iso, color, [this, i](float s) {
                                    this->result_[i].status = s;
                                    float status = 0;
                                    for (const auto& e : this->result_) status += e.status;
                                    status /= result_.size();
                                    dispatchFront([status](ProgressBar& pb) {
                                        pb.updateProgress(status);
                                        if (status < 1.0f)
                                            pb.show();
                                        else
                                            pb.hide();
                                    }, std::ref(this->getProgressBar()));
                                });

                            dispatchFront([this]() {
                                dirty_ = true;
                                invalidate(InvalidationLevel::InvalidOutput);
                            });

                            return m;
                        }));
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
        auto minmax = std::make_pair(std::numeric_limits<double>::max(),
                                     std::numeric_limits<double>::lowest());
        minmax =
            std::accumulate(volume_.begin(), volume_.end(), minmax,
                            [](decltype(minmax) mm, std::shared_ptr<const Volume> v) {
                                return std::make_pair(std::min(mm.first, v->dataMap_.dataRange.x),
                                                      std::max(mm.second, v->dataMap_.dataRange.y));
                            });

        isoValue_.setMinValue(static_cast<const float>(minmax.first));
        isoValue_.setMaxValue(static_cast<const float>(minmax.second));
    }
}

#include <warn/push>
#include <warn/ignore/unused-variable>
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

    size_t count = 0;
    for (const auto& data : volume_) {
        count++;
        if (colors_.size() < count) {
            const static std::string color = "color";
            const static std::string dispName = "Color for Volume ";
            FloatVec4Property* colorProp =
                new FloatVec4Property(color + toString(count - 1), dispName + toString(count),
                                      defaultColor[(count - 1) % 11]);
            colorProp->setCurrentStateAsDefault();
            colorProp->setSemantics(PropertySemantics::Color);
            colorProp->setSerializationMode(PropertySerializationMode::All);
            colors_.addProperty(colorProp);
        }
        colors_[count - 1]->setVisible(true);
    }

    for (size_t i = count; i < colors_.size(); i++) {
        colors_[i]->setVisible(false);
    }
}
#include <warn/pop>


// This will stop the invalidation of the network unless the dirty flag is set.
void SurfaceExtraction::invalidate(InvalidationLevel invalidationLevel,
                                   Property* modifiedProperty) {
    notifyObserversInvalidationBegin(this);
    PropertyOwner::invalidate(invalidationLevel, modifiedProperty);

    if (dirty_ || volume_.isChanged()) outport_.invalidate(InvalidationLevel::InvalidOutput);

    notifyObserversInvalidationEnd(this);
}

SurfaceExtraction::task::task(task&& rhs)
    : result(std::move(rhs.result))
    , iso(rhs.iso)
    , color(std::move(rhs.color))
    , status(rhs.status) {}

bool SurfaceExtraction::task::isSame(float i, vec4 c) const { return iso == i && color == c; }

void SurfaceExtraction::task::set(float i, vec4 c, float s,
                                  std::future<std::shared_ptr<Mesh>>&& r) {
    iso = i;
    color = c;
    status = s;
    result = std::move(r);
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


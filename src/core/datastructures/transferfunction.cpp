/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>

#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/interpolation.h>

#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/zip.h>

#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datawriterfactory.h>

#include <cmath>
#include <streambuf>

#include <fmt/std.h>

namespace inviwo {

TransferFunction::TransferFunction() : TransferFunction({}, TFPrimitiveSetType::Relative) {}

TransferFunction::TransferFunction(const std::vector<TFPrimitiveData>& values)
    : TransferFunction(values, TFPrimitiveSetType::Relative) {}

TransferFunction::TransferFunction(const std::vector<TFPrimitiveData>& values,
                                   TFPrimitiveSetType type)
    : TFPrimitiveSet(values, type)
    , maskMin_{type == TFPrimitiveSetType::Relative ? 0.0 : std::numeric_limits<double>::lowest()}
    , maskMax_{type == TFPrimitiveSetType::Relative ? 1.0 : std::numeric_limits<double>::max()} {}

TransferFunction::TransferFunction(const TransferFunction&) = default;
TransferFunction& TransferFunction::operator=(const TransferFunction& rhs) = default;

TransferFunction::~TransferFunction() = default;

void TransferFunction::setMask(dvec2 mask) {
    if (maskMin_ != mask.x || maskMax_ != mask.y) {
        maskMin_ = mask.x;
        maskMax_ = mask.y;
        notifyTFMaskChanged(*this, mask);
    }
}

dvec2 TransferFunction::getMask() const { return dvec2{maskMin_, maskMax_}; }

void TransferFunction::setMaskMin(double maskMin) {
    if (maskMin_ != maskMin) {
        maskMin_ = maskMin;
        notifyTFMaskChanged(*this, dvec2{maskMin_, maskMax_});
    }
}

double TransferFunction::getMaskMin() const { return maskMin_; }

void TransferFunction::setMaskMax(double maskMax) {
    if (maskMax_ != maskMax) {
        maskMax_ = maskMax;
        notifyTFMaskChanged(*this, dvec2{maskMin_, maskMax_});
    }
}

double TransferFunction::getMaskMax() const { return maskMax_; }

void TransferFunction::clearMask() {
    if (getType() == TFPrimitiveSetType::Relative) {
        setMask(dvec2{0.0, 1.0});
    } else {
        setMask(dvec2{std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max()});
    }
}

void TransferFunction::serialize(Serializer& s) const {
    s.serialize("maskMin", maskMin_);
    s.serialize("maskMax", maskMax_);
    TFPrimitiveSet::serialize(s);
}

void TransferFunction::deserialize(Deserializer& d) {
    d.deserialize("maskMin", maskMin_);
    d.deserialize("maskMax", maskMax_);
    TFPrimitiveSet::deserialize(d);
}

vec4 TransferFunction::sample(double v) const { return interpolateColor(v); }

vec4 TransferFunction::sample(float v) const { return interpolateColor(v); }

std::vector<TFPrimitiveData> TransferFunction::simplify(const std::vector<TFPrimitiveData>& points,
                                                        double delta) {
    if (points.size() < 3) return points;
    std::vector<TFPrimitiveData> simple{points};

    // Calculate the error resulting from using a linear interpolation between the prev and next
    // point instead of including the current one
    const auto error = [&](size_t i) {
        const auto& prev = simple[i - 1];
        const auto& curr = simple[i];
        const auto& next = simple[i + 1];

        const double x = (curr.pos - prev.pos) / (next.pos - prev.pos);
        return glm::compMax(glm::abs(glm::mix(prev.color, next.color, x) - curr.color));
    };

    // Find the point which will result in the smallest error when removed.
    const auto nextToRemove = [&]() {
        const auto index = util::make_sequence<size_t>(1, simple.size() - 1, 1);
        return *std::min_element(index.begin(), index.end(),
                                 [&](size_t a, size_t b) { return error(a) < error(b); });
    };

    // Iteratively remove the point with the smallest error until the error gets larger then delta
    // or only 2 points are left
    auto toRemove = nextToRemove();
    while (error(toRemove) < delta && simple.size() > 2) {
        simple.erase(simple.begin() + toRemove);
        toRemove = nextToRemove();
    }

    return simple;
}

void TransferFunction::interpolateAndStoreColors(std::span<vec4> data) const {
    TFPrimitiveSet::interpolateAndStoreColors(data);
    for (size_t i = 0; i < size_t(maskMin_ * data.size()); i++) {
        data[i].a = 0.0;
    }
    for (size_t i = size_t(maskMax_ * data.size()); i < data.size(); i++) {
        data[i].a = 0.0;
    }
}

std::string_view TransferFunction::serializationKey() const { return "Points"; }

std::string_view TransferFunction::serializationItemKey() const { return "Point"; }

bool operator==(const TransferFunction& lhs, const TransferFunction& rhs) {
    if (lhs.maskMin_ != rhs.maskMin_) return false;
    if (lhs.maskMax_ != rhs.maskMax_) return false;

    return static_cast<const TFPrimitiveSet&>(lhs) == static_cast<const TFPrimitiveSet&>(rhs);
}

bool operator!=(const TransferFunction& lhs, const TransferFunction& rhs) {
    return !operator==(lhs, rhs);
}

TransferFunction TransferFunction::load(const std::filesystem::path& path) {
    auto factory = util::getDataReaderFactory();

    if (auto tf = factory->readDataForTypeAndExtension<TransferFunction>(path)) {
        return *tf;
    } else {
        throw Exception(IVW_CONTEXT_CUSTOM("TransferFunction"),
                        "Unable to load TransferFunction from {}", path);
    }
}
void TransferFunction::save(const TransferFunction& tf, const std::filesystem::path& path) {
    auto factory = util::getDataWriterFactory();

    if (!factory->writeDataForTypeAndExtension(&tf, path)) {
        throw Exception(IVW_CONTEXT_CUSTOM("TransferFunction"),
                        "Unable to save TransferFunction to {}", path);
    }
}

}  // namespace inviwo

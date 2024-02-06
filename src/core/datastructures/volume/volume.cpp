/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/util/document.h>

#include <fmt/format.h>

namespace inviwo {

Volume::Volume(size3_t defaultDimensions, const DataFormatBase* defaultFormat,
               const SwizzleMask& defaultSwizzleMask, InterpolationType interpolation,
               const Wrapping3D& wrapping)
    : Data<Volume, VolumeRepresentation>{}
    , StructuredGridEntity<3>{}
    , MetaDataOwner{}
    , HistogramSupplier{}
    , dataMap_{defaultFormat}
    , axes{util::defaultAxes<3>()}
    , defaultDimensions_{defaultDimensions}
    , defaultDataFormat_{defaultFormat}
    , defaultSwizzleMask_{defaultSwizzleMask}
    , defaultInterpolation_{interpolation}
    , defaultWrapping_{wrapping} {}

Volume::Volume(VolumeConfig config)
    : Data<Volume, VolumeRepresentation>{}
    , StructuredGridEntity<3>{config.model.value_or(VolumeConfig::defaultModel),
                              config.world.value_or(VolumeConfig::defaultWorld)}
    , MetaDataOwner{}
    , HistogramSupplier{}
    , dataMap_{config.dataMap()}
    , axes{config.xAxis.value_or(VolumeConfig::defaultXAxis),
           config.yAxis.value_or(VolumeConfig::defaultYAxis),
           config.zAxis.value_or(VolumeConfig::defaultZAxis)}
    , defaultDimensions_{config.dimensions.value_or(VolumeConfig::defaultDimensions)}
    , defaultDataFormat_{config.format ? config.format : VolumeConfig::defaultFormat}
    , defaultSwizzleMask_{config.swizzleMask.value_or(VolumeConfig::defaultSwizzleMask)}
    , defaultInterpolation_{config.interpolation.value_or(VolumeConfig::defaultInterpolation)}
    , defaultWrapping_{config.wrapping.value_or(VolumeConfig::defaultWrapping)} {}

Volume::Volume(std::shared_ptr<VolumeRepresentation> in)
    : Data<Volume, VolumeRepresentation>{}
    , StructuredGridEntity<3>{}
    , MetaDataOwner{}
    , HistogramSupplier{}
    , dataMap_{in->getDataFormat()}
    , axes{util::defaultAxes<3>()}
    , defaultDimensions_{in->getDimensions()}
    , defaultDataFormat_{in->getDataFormat()}
    , defaultSwizzleMask_{in->getSwizzleMask()}
    , defaultInterpolation_{in->getInterpolation()}
    , defaultWrapping_{in->getWrapping()} {

    addRepresentation(in);
}

Volume::Volume(const Volume& rhs, NoData, VolumeConfig config)
    : Data<Volume, VolumeRepresentation>{}
    , StructuredGridEntity<3>{config.model.value_or(rhs.getModelMatrix()),
                              config.world.value_or(rhs.getWorldMatrix())}
    , MetaDataOwner{rhs}
    , HistogramSupplier{}
    , dataMap_{config.dataRange.value_or(rhs.dataMap_.dataRange),
               config.valueRange.value_or(rhs.dataMap_.valueRange),
               config.valueAxis.value_or(rhs.dataMap_.valueAxis)}
    , axes{config.xAxis.value_or(rhs.axes[0]), config.yAxis.value_or(rhs.axes[1]),
           config.zAxis.value_or(rhs.axes[2])}
    , defaultDimensions_{config.dimensions.value_or(rhs.defaultDimensions_)}
    , defaultDataFormat_{config.format ? config.format : rhs.getDataFormat()}
    , defaultSwizzleMask_{config.swizzleMask.value_or(rhs.defaultSwizzleMask_)}
    , defaultInterpolation_{config.interpolation.value_or(rhs.defaultInterpolation_)}
    , defaultWrapping_{config.wrapping.value_or(rhs.defaultWrapping_)} {}

Volume* Volume::clone() const { return new Volume(*this); }
Volume::~Volume() = default;

void Volume::setDimensions(const size3_t& dim) {
    defaultDimensions_ = dim;
    setLastAndInvalidateOther(&VolumeRepresentation::setDimensions, dim);
}

size3_t Volume::getDimensions() const {
    return getLastOr(&VolumeRepresentation::getDimensions, defaultDimensions_);
}

void Volume::setDataFormat(const DataFormatBase* format) { defaultDataFormat_ = format; }

const DataFormatBase* Volume::getDataFormat() const {
    return getLastOr(&VolumeRepresentation::getDataFormat, defaultDataFormat_);
}

void Volume::setSwizzleMask(const SwizzleMask& mask) {
    defaultSwizzleMask_ = mask;
    setLastAndInvalidateOther(&VolumeRepresentation::setSwizzleMask, mask);
}

SwizzleMask Volume::getSwizzleMask() const {
    return getLastOr(&VolumeRepresentation::getSwizzleMask, defaultSwizzleMask_);
}

void Volume::setInterpolation(InterpolationType interpolation) {
    defaultInterpolation_ = interpolation;
    setLastAndInvalidateOther(&VolumeRepresentation::setInterpolation, interpolation);
}

InterpolationType Volume::getInterpolation() const {
    return getLastOr(&VolumeRepresentation::getInterpolation, defaultInterpolation_);
}

void Volume::setWrapping(const Wrapping3D& wrapping) {
    defaultWrapping_ = wrapping;
    setLastAndInvalidateOther(&VolumeRepresentation::setWrapping, wrapping);
}

Wrapping3D Volume::getWrapping() const {
    return getLastOr(&VolumeRepresentation::getWrapping, defaultWrapping_);
}

Document Volume::getInfo() const {
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    Document doc;
    doc.append("b", "Volume", {{"style", "color:white;"}});
    utildoc::TableBuilder tb(doc.handle(), P::end());

    tb(H("Format"), getDataFormat()->getString());
    tb(H("Dimension"), getDimensions());
    tb(H("SwizzleMask"), getSwizzleMask());
    tb(H("Interpolation"), getInterpolation());
    tb(H("Wrapping"), getWrapping());
    tb(H("Data Range"), dataMap_.dataRange);
    tb(H("Value Range"), dataMap_.valueRange);
    tb(H("Value"), fmt::format("{}{: [}", dataMap_.valueAxis.name, dataMap_.valueAxis.unit));
    tb(H("Axis 1"), fmt::format("{}{: [}", axes[0].name, axes[0].unit));
    tb(H("Axis 2"), fmt::format("{}{: [}", axes[1].name, axes[1].unit));
    tb(H("Axis 3"), fmt::format("{}{: [}", axes[2].name, axes[2].unit));

    tb(H("Basis"), getBasis());
    tb(H("Offset"), getOffset());

    if (hasRepresentation<VolumeRAM>()) {
        if (hasHistograms()) {
            const auto& histograms = getHistograms();
            for (size_t i = 0; i < histograms.size(); ++i) {
                std::stringstream ss;
                ss << "Channel " << i << " Min: " << histograms[i].stats_.min
                   << " Mean: " << histograms[i].stats_.mean << " Max: " << histograms[i].stats_.max
                   << " Std: " << histograms[i].stats_.standardDeviation;
                tb(H("Stats"), ss.str());

                std::stringstream ss2;
                ss2 << "(1: " << histograms[i].stats_.percentiles[1]
                    << ", 25: " << histograms[i].stats_.percentiles[25]
                    << ", 50: " << histograms[i].stats_.percentiles[50]
                    << ", 75: " << histograms[i].stats_.percentiles[75]
                    << ", 99: " << histograms[i].stats_.percentiles[99] << ")";
                tb(H("Percentiles"), ss2.str());
            }
        }
    }
    return doc;
}

vec3 Volume::getWorldSpaceGradientSpacing() const {
    mat3 textureToWorld = mat3(getCoordinateTransformer().getTextureToWorldMatrix());
    // Basis vectors with a length of one voxel.
    // Basis vectors may be non-orthogonal
    auto dimensions = getDimensions();
    vec3 a = textureToWorld[0] / static_cast<float>(dimensions[0]);
    vec3 b = textureToWorld[1] / static_cast<float>(dimensions[1]);
    vec3 c = textureToWorld[2] / static_cast<float>(dimensions[2]);
    // Project the voxel basis vectors
    // onto the world space x/y/z axes,
    // and choose the longest projected vector
    // for each axis.
    // Using the fact that
    // vec3 x{ 1.f, 0, 0 };
    // vec3 y{ 0, 1.f, 0 };
    // vec3 z{ 0, 0, 1.f };
    // such that
    // ax' = dot(x, a) = a.x
    // bx' = dot(x, b) = b.x
    // cx' = dot(x, c) = c.x
    // and so on.
    auto signedMax = [](const float& x1, const float& x2) {
        return (std::abs(x1) >= std::abs(x2)) ? x1 : x2;
    };

    vec3 ds{signedMax(a.x, signedMax(b.x, c.x)), signedMax(a.y, signedMax(b.y, c.y)),
            signedMax(a.z, signedMax(b.z, c.z))};

    // Return the spacing in world space,
    // actually given by:
    // { gradientSpacing.x         0                     0
    //         0             gradientSpacing.y           0
    //         0                   0               gradientSpacing.z }
    return ds;
}

const Axis* Volume::getAxis(size_t index) const {
    if (index >= 3) {
        return nullptr;
    }
    return &axes[index];
}

VolumeConfig Volume::config() const {
    return {.dimensions = getDimensions(),
            .format = getDataFormat(),
            .swizzleMask = getSwizzleMask(),
            .interpolation = getInterpolation(),
            .wrapping = getWrapping(),
            .xAxis = axes[0],
            .yAxis = axes[1],
            .zAxis = axes[2],
            .valueAxis = dataMap_.valueAxis,
            .dataRange = dataMap_.dataRange,
            .valueRange = dataMap_.valueRange,
            .model = getModelMatrix(),
            .world = getWorldMatrix()};
}

uvec3 Volume::colorCode = uvec3(188, 101, 101);
const std::string Volume::classIdentifier = "org.inviwo.Volume";
const std::string Volume::dataName = "Volume";

std::shared_ptr<HistogramCalculationState> Volume::calculateHistograms(size_t bins) const {
    return HistogramSupplier::startCalculation(getRepresentationShared<VolumeRAM>(),
                                               dataMap_.dataRange, bins);
}

template class IVW_CORE_TMPL_INST DataReaderType<Volume>;
template class IVW_CORE_TMPL_INST DataWriterType<Volume>;
template class IVW_CORE_TMPL_INST DataReaderType<VolumeSequence>;

template class IVW_CORE_TMPL_INST DataInport<Volume>;
template class IVW_CORE_TMPL_INST DataOutport<Volume>;
template class IVW_CORE_TMPL_INST DataInport<VolumeSequence>;
template class IVW_CORE_TMPL_INST DataOutport<VolumeSequence>;

}  // namespace inviwo

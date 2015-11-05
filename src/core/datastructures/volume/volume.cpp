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

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/util/tooltiphelper.h>

namespace inviwo {

Volume::Volume(size3_t dimensions, const DataFormatBase* format)
    : Data<VolumeRepresentation>(format), StructuredGridEntity<3>(dimensions), dataMap_(format) {}

Volume::Volume(const Volume& rhs)
    : Data<VolumeRepresentation>(rhs), StructuredGridEntity<3>(rhs), dataMap_(rhs.dataMap_) {}

Volume::Volume(std::shared_ptr<VolumeRepresentation> in)
    : Data<VolumeRepresentation>(in->getDataFormat())
    , StructuredGridEntity<3>(in->getDimensions())
    , dataMap_(in->getDataFormat()) {
    addRepresentation(in);
}

Volume& Volume::operator=(const Volume& that) {
    if (this != &that) {
        Data<VolumeRepresentation>::operator=(that);
        StructuredGridEntity<3>::operator=(that);
        dataMap_ = that.dataMap_;
    }

    return *this;
}
Volume* Volume::clone() const { return new Volume(*this); }
Volume::~Volume() {}

std::string Volume::getDataInfo() const {
    ToolTipHelper t("Volume");
    t.tableTop();

    t.row("Format", getDataFormat()->getString());
    t.row("Dimension", toString(getDimensions()));
    t.row("Data Range", toString(dataMap_.dataRange));
    t.row("Value Range", toString(dataMap_.valueRange));

    if (hasRepresentation<VolumeRAM>()) {
        auto volumeRAM = getRepresentation<VolumeRAM>();
        if (volumeRAM->hasHistograms()) {
            auto histograms = volumeRAM->getHistograms();
            for (size_t i = 0; i < histograms->size(); ++i) {
                std::stringstream ss;
                ss << "Channel " << i << " Min: " << (*histograms)[i].stats_.min
                   << " Mean: " << (*histograms)[i].stats_.mean
                   << " Max: " << (*histograms)[i].stats_.max
                   << " Std: " << (*histograms)[i].stats_.standardDeviation;
                t.row("Stats", ss.str());

                std::stringstream ss2;
                ss2 << "(1: " << (*histograms)[i].stats_.percentiles[1]
                   << ", 25: " << (*histograms)[i].stats_.percentiles[25]
                   << ", 50: " << (*histograms)[i].stats_.percentiles[50]
                   << ", 75: " << (*histograms)[i].stats_.percentiles[75]
                   << ", 99: " << (*histograms)[i].stats_.percentiles[99] << ")";
                t.row("Percentiles", ss2.str());
            }
        }
    }

    t.tableBottom();
    return t;
}

size3_t Volume::getDimensions() const {
    if (hasRepresentations() && lastValidRepresentation_) {
        return lastValidRepresentation_->getDimensions();
    }
    return StructuredGridEntity<3>::getDimensions();
}

void Volume::setDimensions(const size3_t& dim) {
    StructuredGridEntity<3>::setDimensions(dim);

    if (lastValidRepresentation_) {
        // Resize last valid representation
        lastValidRepresentation_->setDimensions(dim);
        removeOtherRepresentations(lastValidRepresentation_.get());
    }
}

void Volume::setOffset(const vec3& offset) {
    SpatialEntity<3>::setOffset(Vector<3, float>(offset));
}
vec3 Volume::getOffset() const { return SpatialEntity<3>::getOffset(); }

mat3 Volume::getBasis() const { return SpatialEntity<3>::getBasis(); }
void Volume::setBasis(const mat3& basis) { SpatialEntity<3>::setBasis(Matrix<3, float>(basis)); }

mat4 Volume::getModelMatrix() const { return SpatialEntity<3>::getModelMatrix(); }
void Volume::setModelMatrix(const mat4& mat) {
    SpatialEntity<3>::setModelMatrix(Matrix<4, float>(mat));
}

mat4 Volume::getWorldMatrix() const { return SpatialEntity<3>::getWorldMatrix(); }
void Volume::setWorldMatrix(const mat4& mat) {
    SpatialEntity<3>::setWorldMatrix(Matrix<4, float>(mat));
}

std::shared_ptr<VolumeRepresentation> Volume::createDefaultRepresentation() const {
    return createVolumeRAM(getDimensions(), getDataFormat());
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
    vec3 ds{ 
        std::max(a.x, std::max(b.x, c.x)),
        std::max(a.y, std::max(b.y, c.y)),
        std::max(a.z, std::max(b.z, c.z)) };

    // Return the spacing in world space,
    // actually given by:
    // { gradientSpacing.x         0                     0
    //         0             gradientSpacing.y           0
    //         0                   0               gradientSpacing.z }
    return ds;
}

inviwo::uvec3 Volume::COLOR_CODE = uvec3(188, 101, 101);
const std::string Volume::CLASS_IDENTIFIER = "org.inviwo.Volume";

const StructuredCameraCoordinateTransformer<3>& Volume::getCoordinateTransformer(
    const Camera& camera) const {
    return StructuredGridEntity<3>::getCoordinateTransformer(camera);
}

}  // namespace
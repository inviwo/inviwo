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

#include <modules/opencl/volume/volumeclbase.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/datamapper.h>

namespace inviwo {

VolumeCLBase::VolumeCLBase()
    : clImage_(NULL), volumeStruct_(sizeof(VolumeParameters), DataUINT8::get())
{
    volumeStruct_.addRepresentation(new BufferRAMPrecision<glm::u8>(sizeof(VolumeParameters), DataUINT8::get()));
    ivwAssert(volumeStruct_.getSize() == 512, "VolumeParameters must have a size that is power of two, currently " << volumeStruct_.getSize())
}

VolumeCLBase::VolumeCLBase(const VolumeCLBase& rhs): volumeStruct_(sizeof(VolumeParameters), DataUINT8::get()) {
    volumeStruct_.addRepresentation(new BufferRAMPrecision<glm::u8>(sizeof(VolumeParameters), DataUINT8::get()));
}

VolumeCLBase::~VolumeCLBase() { }

vec2 VolumeCLBase::getVolumeDataOffsetAndScaling(const Volume* volume) const
{
    // Note: The basically the same code is used in VolumeCLGL and VolumeGL as well.
    // Changes here should also be done there.
    // Compute data scaling based on volume data range

    dvec2 dataRange = volume->dataMap_.dataRange;
    DataMapper defaultRange(volume->getDataFormat());
    double typescale = getCLFormats()->getCLFormat(volume->getDataFormat()->getId()).scaling;
    defaultRange.dataRange *= typescale;

    double scalingFactor = 1.0;
    double signedScalingFactor = 1.0;
    double offset = 0.0;
    double signedOffset = 0.0;

    double invRange = 1.0 / (dataRange.y - dataRange.x);
    double defaultToDataRange = (defaultRange.dataRange.y - defaultRange.dataRange.x) * invRange;
    double defaultToDataOffset = (dataRange.x - defaultRange.dataRange.x) /
        (defaultRange.dataRange.y - defaultRange.dataRange.x);

    switch (getCLFormats()->getCLFormat(volume->getDataFormat()->getId()).normalization) {
    case CLFormats::NONE:
        scalingFactor = invRange;
        offset = -dataRange.x;
        signedScalingFactor = scalingFactor;
        signedOffset = offset;
        break;
    case CLFormats::NORMALIZED:
        scalingFactor = defaultToDataRange;
        offset = -defaultToDataOffset;
        signedScalingFactor = scalingFactor;
        signedOffset = offset;
        break;
    case CLFormats::SIGN_NORMALIZED:
        scalingFactor = 0.5 * defaultToDataRange;
        offset = 1.0 - 2 * defaultToDataOffset;
        signedScalingFactor = defaultToDataRange;
        signedOffset = -defaultToDataOffset;
        break;
    }
    return vec2(offset, scalingFactor);
}

const Buffer& VolumeCLBase::getVolumeStruct(const Volume* volume) const {
    // Update data before returning it
    VolumeParameters* volumeStruct = static_cast<VolumeParameters*>(const_cast<Buffer&>(volumeStruct_).getEditableRepresentation<BufferRAM>()->getData());

    volumeStruct->modelToWorld = volume->getCoordinateTransformer().getModelToWorldMatrix();
    volumeStruct->worldToModel = volume->getCoordinateTransformer().getWorldToModelMatrix();
    volumeStruct->worldToTexture = volume->getCoordinateTransformer().getWorldToTextureMatrix();
    volumeStruct->textureToWorld = volume->getCoordinateTransformer().getTextureToWorldMatrix();
    volumeStruct->textureToIndex = volume->getCoordinateTransformer().getTextureToIndexMatrix();
    volumeStruct->indexToTexture = volume->getCoordinateTransformer().getIndexToTextureMatrix();
    float gradientSpacing = volume->getWorldSpaceGradientSpacing();
    // Scale the world matrix by the gradient spacing and the transform it to texture space.
    // Note that since we are dealing with real values we can multiply the scalar after the transform as well
    volumeStruct->textureSpaceGradientSpacing = glm::scale(volumeStruct->worldToTexture, vec3(gradientSpacing));
    volumeStruct->worldSpaceGradientSampleSpacing = gradientSpacing;

    // Compute scaling and offset for datatypes that will be sampled 
    // (for instance 12-bit data)
    // Note: Basically the same code is used in VolumeGL as well.
    // Changes here should also be done there.
    // Compute data scaling based on volume data range

    dvec2 dataRange = volume->dataMap_.dataRange;
    DataMapper defaultRange(volume->getDataFormat());
    double typescale = getCLFormats()->getCLFormat(volume->getDataFormat()->getId()).scaling;
    defaultRange.dataRange *= typescale;

   
    double formatScalingFactor = 1.0;
    double signedFormatScalingFactor = 1.0;
    double formatOffset = 0.0;
    double signedFormatOffset = 0.0;

    double invRange = 1.0 / (dataRange.y - dataRange.x);
    double defaultToDataRange = (defaultRange.dataRange.y - defaultRange.dataRange.x) * invRange;
    double defaultToDataOffset = (dataRange.x - defaultRange.dataRange.x) /
        (defaultRange.dataRange.y - defaultRange.dataRange.x);

    switch (getCLFormats()->getCLFormat(volume->getDataFormat()->getId()).normalization) {
    case CLFormats::NONE:
        formatScalingFactor = invRange;
        formatOffset = -dataRange.x;
        signedFormatScalingFactor = formatScalingFactor;
        signedFormatOffset = formatOffset;
        break;
    case CLFormats::NORMALIZED:
        formatScalingFactor = defaultToDataRange;
        formatOffset = -defaultToDataOffset;
        signedFormatScalingFactor = formatScalingFactor;
        signedFormatOffset = formatOffset;
        break;
    case CLFormats::SIGN_NORMALIZED:
        formatScalingFactor = 0.5 * defaultToDataRange;
        formatOffset = 1.0 - 2 * defaultToDataOffset;
        signedFormatScalingFactor = defaultToDataRange;
        signedFormatOffset = -defaultToDataOffset;
        break;
    }


    volumeStruct->formatScaling = static_cast<float>(formatScalingFactor);
    volumeStruct->formatOffset = static_cast<float>(formatOffset);
    volumeStruct->signedFormatScaling = static_cast<float>(signedFormatScalingFactor);
    volumeStruct->signedFormatOffset = static_cast<float>(signedFormatOffset);

    return volumeStruct_;
}

} // namespace

namespace cl {

template <>
cl_int Kernel::setArg(cl_uint index, const inviwo::VolumeCLBase& value) {
    return setArg(index, value.get());
}

} // namespace cl

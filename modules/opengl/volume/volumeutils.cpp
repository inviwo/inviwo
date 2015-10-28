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

#include <modules/opengl/glformats.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>

namespace inviwo {

namespace utilgl {

void setShaderUniforms(Shader& shader, const Volume& volume, const std::string& samplerID) {
    const StructuredCoordinateTransformer<3>& ct = volume.getCoordinateTransformer();

    shader.setUniform(samplerID + ".dataToModel", ct.getDataToModelMatrix());
    shader.setUniform(samplerID + ".modelToData", ct.getModelToDataMatrix());

    shader.setUniform(samplerID + ".dataToWorld", ct.getDataToWorldMatrix());
    shader.setUniform(samplerID + ".worldToData", ct.getWorldToDataMatrix());

    shader.setUniform(samplerID + ".modelToWorld", ct.getModelToWorldMatrix());
    shader.setUniform(samplerID + ".worldToModel", ct.getWorldToModelMatrix());

    shader.setUniform(samplerID + ".worldToTexture", ct.getWorldToTextureMatrix());
    shader.setUniform(samplerID + ".textureToWorld", ct.getTextureToWorldMatrix());

    auto textureToWorldNormalMatrix = glm::inverseTranspose(ct.getTextureToWorldMatrix());
    shader.setUniform(samplerID + ".textureToWorldNormalMatrix", textureToWorldNormalMatrix);

    shader.setUniform(samplerID + ".textureToIndex", ct.getTextureToIndexMatrix());
    shader.setUniform(samplerID + ".indexToTexture", ct.getIndexToTextureMatrix());

    float gradientSpacing = volume.getWorldSpaceGradientSpacing();
    // Scale the world matrix by the gradient spacing and the transform it to texture space.
    // Note that since we are dealing with real values we can multiply the scalar after the
    // transform as well
    shader.setUniform(
        samplerID + ".textureSpaceGradientSpacing",
        gradientSpacing * mat3(ct.getWorldToTextureMatrix()));

    vec3 dimF = static_cast<vec3>(volume.getDimensions());
    shader.setUniform(samplerID + ".dimensions", dimF);
    shader.setUniform(samplerID + ".reciprocalDimensions", vec3(1.f) / dimF);

    shader.setUniform(samplerID + ".worldSpaceGradientSpacing", gradientSpacing);

    dvec2 dataRange = volume.dataMap_.dataRange;
    DataMapper defaultRange(volume.getDataFormat());

    double typescale = 1.0 - getGLFormats()->getGLFormat(volume.getDataFormat()->getId()).scaling;
    defaultRange.dataRange = defaultRange.dataRange * typescale;

    double scalingFactor = 1.0;
    double signedScalingFactor = 1.0;
    double offset = 0.0;
    double signedOffset = 0.0;

    double invRange = 1.0 / (dataRange.y - dataRange.x);
    double defaultToDataRange = (defaultRange.dataRange.y - defaultRange.dataRange.x) * invRange;
    double defaultToDataOffset = (dataRange.x - defaultRange.dataRange.x) /
                                 (defaultRange.dataRange.y - defaultRange.dataRange.x);

    switch (getGLFormats()->getGLFormat(volume.getDataFormat()->getId()).normalization) {
        case GLFormats::Normalization::None:
            scalingFactor = invRange;
            offset = -dataRange.x;
            signedScalingFactor = scalingFactor;
            signedOffset = offset;
            break;
        case GLFormats::Normalization::Normalized:
            scalingFactor = defaultToDataRange;
            offset = -defaultToDataOffset;
            signedScalingFactor = scalingFactor;
            signedOffset = offset;
            break;
        case GLFormats::Normalization::SignNormalized:
            scalingFactor = 0.5 * defaultToDataRange;
            offset = 1.0 - 2 * defaultToDataOffset;
            signedScalingFactor = defaultToDataRange;
            signedOffset = -defaultToDataOffset;
            break;
    }
    // offset scaling because of reversed scaling in the shader, i.e. (1 - formatScaling_)
    shader.setUniform(samplerID + ".formatScaling", static_cast<float>(1.0 - scalingFactor));
    shader.setUniform(samplerID + ".formatOffset", static_cast<float>(offset));

    shader.setUniform(samplerID + ".signedFormatScaling",
                       static_cast<float>(1.0 - signedScalingFactor));
    shader.setUniform(samplerID + ".signedFormatOffset", static_cast<float>(signedOffset));
}

void setShaderUniforms(Shader& shader, const VolumeInport& port, const std::string& samplerID) {
    setShaderUniforms(shader, *port.getData(), samplerID);
}

void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont, VolumeInport& volumePort) {
    TextureUnit unit;
    utilgl::bindTexture(volumePort, unit);
    shader.setUniform(volumePort.getIdentifier(), unit.getUnitNumber());
    utilgl::setShaderUniforms(shader, volumePort, volumePort.getIdentifier() + "Parameters");
    cont.push_back(std::move(unit));
}

IVW_MODULE_OPENGL_API void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                                              const Volume& volume, const std::string& samplerID) {
    TextureUnit unit;
    utilgl::bindTexture(volume, unit);
    shader.setUniform(samplerID, unit.getUnitNumber());
    utilgl::setShaderUniforms(shader, volume, samplerID + "Parameters");
    cont.push_back(std::move(unit));
}

}  // namespace

}  // namespace

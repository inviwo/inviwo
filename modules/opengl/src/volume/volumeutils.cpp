/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <modules/opengl/volume/volumeutils.h>

#include <inviwo/core/datastructures/coordinatetransformer.h>  // for StructuredCoordinateTransf...
#include <inviwo/core/datastructures/datamapper.h>             // for DataMapper
#include <inviwo/core/datastructures/volume/volume.h>          // for Volume
#include <inviwo/core/ports/datainport.h>                      // for DataInport
#include <inviwo/core/util/formats.h>                          // for DataFormatBase
#include <inviwo/core/util/glmmat.h>                           // for mat3
#include <inviwo/core/util/glmvec.h>                           // for dvec2, vec3
#include <inviwo/core/util/stringconversion.h>                 // for StrBuffer
#include <modules/opengl/glformats.h>                          // for Normalization, GLFormat
#include <modules/opengl/shader/shader.h>                      // for Shader
#include <modules/opengl/texture/textureunit.h>                // for TextureUnit, TextureUnitCo...
#include <modules/opengl/texture/textureutils.h>               // for VolumeInport, bindTexture

#include <memory>   // for shared_ptr
#include <string>   // for string
#include <utility>  // for move

#include <fmt/core.h>                    // for basic_string_view, format_to
#include <glm/ext/matrix_transform.hpp>  // for scale
#include <glm/gtc/matrix_inverse.hpp>    // for inverseTranspose
#include <glm/mat4x4.hpp>                // for mat
#include <glm/vec2.hpp>                  // for vec<>::(anonymous)
#include <glm/vec3.hpp>                  // for operator/
#include <glm/vec4.hpp>                  // for operator*

#include <inviwo/tracy/tracy.h>

namespace inviwo {

namespace utilgl {

namespace {}  // namespace

void setShaderUniforms(Shader& shader, const Volume& volume, std::string_view samplerID) {
    TRACY_ZONE_SCOPED_C(0x000088);
    const StructuredCoordinateTransformer<3>& ct = volume.getCoordinateTransformer();

    StrBuffer buff;

    shader.setUniform(buff.replace("{}.dataToModel", samplerID), ct.getDataToModelMatrix());
    shader.setUniform(buff.replace("{}.modelToData", samplerID), ct.getModelToDataMatrix());

    shader.setUniform(buff.replace("{}.dataToWorld", samplerID), ct.getDataToWorldMatrix());
    shader.setUniform(buff.replace("{}.worldToData", samplerID), ct.getWorldToDataMatrix());

    shader.setUniform(buff.replace("{}.modelToWorld", samplerID), ct.getModelToWorldMatrix());
    shader.setUniform(buff.replace("{}.worldToModel", samplerID), ct.getWorldToModelMatrix());

    shader.setUniform(buff.replace("{}.worldToTexture", samplerID), ct.getWorldToTextureMatrix());
    shader.setUniform(buff.replace("{}.textureToWorld", samplerID), ct.getTextureToWorldMatrix());

    const auto textureToWorldNormalMatrix = glm::inverseTranspose(ct.getTextureToWorldMatrix());
    shader.setUniform(buff.replace("{}.textureToWorldNormalMatrix", samplerID),
                      textureToWorldNormalMatrix);

    shader.setUniform(buff.replace("{}.textureToIndex", samplerID), ct.getTextureToIndexMatrix());
    shader.setUniform(buff.replace("{}.indexToTexture", samplerID), ct.getIndexToTextureMatrix());

    const auto gradientSpacing = volume.getWorldSpaceGradientSpacing();
    // Transform the world space gradient spacing to texture space.
    // Wold space gradient spacing is given by:
    // mat3{ gradientSpacing.x         0                     0
    //             0             gradientSpacing.y           0
    //             0                   0               gradientSpacing.z }
    // which means that the transformation is equal to scaling
    // the world to texture matrix.
    shader.setUniform(buff.replace("{}.textureSpaceGradientSpacing", samplerID),
                      mat3(glm::scale(ct.getWorldToTextureMatrix(), gradientSpacing)));

    const vec3 dimF = static_cast<vec3>(volume.getDimensions());
    shader.setUniform(buff.replace("{}.dimensions", samplerID), dimF);
    shader.setUniform(buff.replace("{}.reciprocalDimensions", samplerID), vec3(1.f) / dimF);

    shader.setUniform(buff.replace("{}.worldSpaceGradientSpacing", samplerID), gradientSpacing);

    const dvec2 dataRange = volume.dataMap_.dataRange;
    const DataMapper defaultRange(volume.getDataFormat());

    const double invRange = 1.0 / (dataRange.y - dataRange.x);
    const double defaultToDataRange =
        (defaultRange.dataRange.y - defaultRange.dataRange.x) * invRange;
    const double defaultToDataOffset = (dataRange.x - defaultRange.dataRange.x) /
                                       (defaultRange.dataRange.y - defaultRange.dataRange.x);

    double scalingFactor = 1.0;
    double signedScalingFactor = 1.0;
    double offset = 0.0;
    double signedOffset = 0.0;

    switch (GLFormats::get(volume.getDataFormat()->getId()).normalization) {
        case utilgl::Normalization::None:
            scalingFactor = invRange;
            offset = -dataRange.x;
            signedScalingFactor = scalingFactor;
            signedOffset = offset;
            break;
        case utilgl::Normalization::Normalized:
            scalingFactor = defaultToDataRange;
            offset = -defaultToDataOffset;
            signedScalingFactor = scalingFactor;
            signedOffset = offset;
            break;
        case utilgl::Normalization::SignNormalized:
            scalingFactor = 0.5 * defaultToDataRange;
            offset = 1.0 - 2 * defaultToDataOffset;
            signedScalingFactor = defaultToDataRange;
            signedOffset = -defaultToDataOffset;
            break;
    }
    // offset scaling because of reversed scaling in the shader, i.e. (1 - formatScaling_)
    shader.setUniform(buff.replace("{}.formatScaling", samplerID),
                      static_cast<float>(1.0 - scalingFactor));
    shader.setUniform(buff.replace("{}.formatOffset", samplerID), static_cast<float>(offset));

    shader.setUniform(buff.replace("{}.signedFormatScaling", samplerID),
                      static_cast<float>(1.0 - signedScalingFactor));
    shader.setUniform(buff.replace("{}.signedFormatOffset", samplerID),
                      static_cast<float>(signedOffset));
}

void setShaderUniforms(Shader& shader, const VolumeInport& port, std::string_view samplerID) {
    setShaderUniforms(shader, *port.getData(), samplerID);
}

void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont,
                        const VolumeInport& volumePort) {
    TRACY_ZONE_SCOPED_C(0x000088);
    TextureUnit unit;
    utilgl::bindTexture(volumePort, unit);
    shader.setUniform(volumePort.getIdentifier(), unit.getUnitNumber());

    utilgl::setShaderUniforms(shader, volumePort,
                              StrBuffer{"{}Parameters", volumePort.getIdentifier()});
    cont.push_back(std::move(unit));
}

void bindAndSetUniforms(Shader& shader, TextureUnitContainer& cont, const Volume& volume,
                        std::string_view samplerID) {
    TRACY_ZONE_SCOPED_C(0x000088);
    TextureUnit unit;
    utilgl::bindTexture(volume, unit);
    shader.setUniform(samplerID, unit.getUnitNumber());

    utilgl::setShaderUniforms(shader, volume, StrBuffer{"{}Parameters", samplerID});
    cont.push_back(std::move(unit));
}

}  // namespace utilgl

}  // namespace inviwo

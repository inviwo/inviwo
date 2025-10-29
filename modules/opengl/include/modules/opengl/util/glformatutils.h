/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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
#pragma once

#include <modules/opengl/openglmoduledefine.h>

#include <inviwo/core/util/glmvec.h>

namespace inviwo {

class DataMapper;
class DataFormatBase;

namespace utilgl {

struct IVW_MODULE_OPENGL_API GLFormatConversion {
    double toValueScaling = 1.0;
    double toValueOffset = 0.0;
    double outputValueOffset = 0.0;
    double outputScaling = 1.0;
    double outputOffset = 0.0;
};

struct IVW_MODULE_OPENGL_API GLFormatRenormalization {
    double formatScaling = 1.0;
    double signedFormatScaling = 1.0;
    double formatOffset = 0.0;
    double signedFormatOffset = 0.0;
};

/**
 * Map the value @p glValue from the OpenGL texture input range (regular, normalize, or sign
 * normalized) to a normalized range [0,1] using the [min,max] data range of the DataMapper.
 *
 * @tparam scalar or glm vector type
 * @param glValue          value in OpenGL data range (regular, normalize, or sign normalized)
 * @param renormalization  parameters used for the range conversion
 * @return @p glValue renormalized from the OpenGL data range to [0,1]
 *
 * * @see getNormalizedTexel (sampler2d.glsl), getNormalizedVoxel (sampler3d.glsl)
 */
template <typename T>
T mapFromGLInputToNormalized(T glValue, const GLFormatRenormalization& renormalization) {
    return (glValue + renormalization.formatOffset) * (1.0 - renormalization.formatScaling);
}

/**
 * Map the value @p glValue from the OpenGL texture input range (regular, normalize, or sign
 * normalized) to a sign normalized range [-1,1] using the [min,max] data range of the DataMapper.
 *
 * @tparam scalar or glm vector type
 * @param glValue          value in OpenGL data range (regular, normalize, or sign normalized)
 * @param renormalization  parameters used for the range conversion
 * @return @p glValue renormalized from the OpenGL data range to [-1,1]
 *
 * @see getSignNormalizedTexel (sampler2d.glsl), getSignNormalizedVoxel (sampler3d.glsl)
 */
template <typename T>
T mapFromGLInputToSignNormalized(T glValue, const GLFormatRenormalization& renormalization) {
    return (glValue + renormalization.signedFormatOffset) *
           (1.0 - renormalization.signedFormatScaling);
}

/**
 * Map a normalized OpenGL value @p normalizedValue to value space as specified by @p conversion.
 *
 * @tparam scalar or glm vector type
 * @param normalizedValue  normalized value [0,1] as obtained by getNormalizedTexel() in
 *                         sampler2d.glsl
 * @param conversion       parameters used for the range conversion
 * @return converted value in value space
 */
template <typename T>
T mapFromNormalizedGLToValue(T normalizedValue, const GLFormatConversion& conversion) {
    return normalizedValue * conversion.toValueScaling + conversion.toValueOffset;
}

/**
 * Map the value @p value from value space to the OpenGL output range as defined by the underlying
 * data format and specified by @p conversion as follows:
 *   + map from value range to dataRange for float and non-normalized GL data formats
 *   + normalized dataRange [0,1] for normalized, unsigned GL data formats
 *   + or sign normalized dataRange [-1,1] for normalized, signed GL data formats
 *
 * @tparam scalar or glm vector type
 * @param value        value in value space
 * @param conversion   parameters used for the range conversion
 * @return @p value converted to the OpenGL output range
 */
template <typename T>
T mapFromValueToGLOutput(T value, const GLFormatConversion& conversion) {
    return ((value - conversion.outputValueOffset) * conversion.outputScaling) +
           conversion.outputOffset;
}

/**
 * Calculate the parameters for a format conversion from normalized values to value range of
 * @p dataMapIn and the conversion from value range to the OpenGL output range based on the value
 * and data range of @p dataMapOut and the underlying format @p formatOut as follows:
 *   + map from value range to dataRange for float and non-normalized GL data formats
 *   + normalized dataRange [0,1] for normalized, unsigned GL data formats
 *   + or sign normalized dataRange [-1,1] for normalized, signed GL data formats
 *
 * @param dataMapIn   used for mapping normalized GL values to the corresponding value space
 * @param dataMapOut  used for mapping from value space to the GL output range of @p formatOut
 * @param formatOut   destination data format
 *
 * @return struct containing the parameters for the format conversion
 *
 * @see mapFromNormalizedGLToValue, mapFromValueToGLOutput
 */
IVW_MODULE_OPENGL_API GLFormatConversion createGLFormatConversion(const DataMapper& dataMapIn,
                                                                  const DataMapper& dataMapOut,
                                                                  const DataFormatBase* formatOut);
/**
 * Calculate the parameters for a OpenGL data range re-normalization based on @p dataMapIn and
 * source format @p format. Used in getNormalizedTexel/Voxel and getSignNormalizedTexel/Voxel in
 * sampler2d.glsl and sampler3d.glsl
 *
 * @param dataMap
 * @param format
 * @return struct containing the parameters for the data range normalization for @p format
 *
 * @see setShaderUniforms(Shader&, const DataMapper&, const DataFormatBase*, std::string_view)
 */
IVW_MODULE_OPENGL_API GLFormatRenormalization
createGLFormatRenormalization(const DataMapper& dataMap, const DataFormatBase* format);

}  // namespace utilgl

}  // namespace inviwo

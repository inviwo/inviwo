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

#include <modules/opengl/util/glformatutils.h>

#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/util/formats.h>

#include <modules/opengl/openglcapabilities.h>
#include <modules/opengl/glformats.h>

namespace inviwo::utilgl {

GLFormatConversion createGLFormatConversion(const DataMapper& dataMapIn,
                                            const DataMapper& dataMapOut,
                                            const DataFormatBase* formatOut) {
    using enum DataMapper::SignedNormalization;

    const auto& glFormatOut = GLFormats::get(formatOut->getId());

    const DataMapper defaultRange(formatOut, Symmetric);

    // map normalized input [0,1] to valueRange
    const dvec2 valueRangeIn = dataMapIn.valueRange;
    const double toValueScaling = valueRangeIn.y - valueRangeIn.x;
    const double toValueOffset = valueRangeIn.x;
    // map from value range to dataRange (float, non-normalized),
    // normalized dataRange [0,1] (normalized, unsigned),
    // or sign normalized dataRange [-1,1] (normalized, signed)
    const dvec2 dataRangeOut = dataMapOut.dataRange;
    const dvec2 valueRangeOut = dataMapOut.valueRange;
    const double dataOutToDefault =
        (dataRangeOut.y - dataRangeOut.x) / (defaultRange.dataRange.y - defaultRange.dataRange.x);
    const double dataOutToDefaultOffset = (dataRangeOut.x - defaultRange.dataRange.x) /
                                          (defaultRange.dataRange.y - defaultRange.dataRange.x);
    double outputValueOffset = 0.0;
    double outputScaling = 1.0;
    double outputOffset = 0.0;
    switch (glFormatOut.normalization) {
        case utilgl::Normalization::None:  // map to data range
            outputValueOffset = valueRangeOut.x;
            outputScaling = (dataRangeOut.y - dataRangeOut.x) / (valueRangeOut.y - valueRangeOut.x);
            outputOffset = dataRangeOut.x;
            break;
        case utilgl::Normalization::Normalized:  // map to [0,1]
            outputValueOffset = valueRangeOut.x;
            outputScaling = dataOutToDefault / (valueRangeOut.y - valueRangeOut.x);
            outputOffset = dataOutToDefaultOffset;
            break;
        case utilgl::Normalization::SignNormalized:  // map to [-1,1]
            outputValueOffset = valueRangeOut.x;
            outputScaling = dataOutToDefault / (valueRangeOut.y - valueRangeOut.x) * 2.0;
            outputOffset = dataOutToDefaultOffset * 2.0 - 1.0;
            break;
    }

    return GLFormatConversion{
        .toValueScaling = toValueScaling,
        .toValueOffset = toValueOffset,
        .outputValueOffset = outputValueOffset,
        .outputScaling = outputScaling,
        .outputOffset = outputOffset,
    };
}

GLFormatRenormalization createGLFormatRenormalization(const DataMapper& dataMap,
                                                      const DataFormatBase* format) {
    using enum DataMapper::SignedNormalization;

    const dvec2 dataRange = dataMap.dataRange;
    const DataMapper defaultRange(
        format, OpenGLCapabilities::isSignedIntNormalizationSymmetric() ? Symmetric : Asymmetric);

    const double invRange = 1.0 / (dataRange.y - dataRange.x);
    const double defaultToDataRange =
        (defaultRange.dataRange.y - defaultRange.dataRange.x) * invRange;
    const double defaultToDataOffset = (dataRange.x - defaultRange.dataRange.x) /
                                       (defaultRange.dataRange.y - defaultRange.dataRange.x);

    double formatScaling = 1.0;
    double signedFormatScaling = 1.0;
    double formatOffset = 0.0;
    double signedFormatOffset = 0.0;

    switch (GLFormats::get(format->getId()).normalization) {
        case utilgl::Normalization::None:
            formatScaling = invRange;
            formatOffset = -dataRange.x;
            signedFormatScaling = formatScaling;
            signedFormatOffset = formatOffset;
            break;
        case utilgl::Normalization::Normalized:
            formatScaling = defaultToDataRange;
            formatOffset = -defaultToDataOffset;
            signedFormatScaling = formatScaling;
            signedFormatOffset = formatOffset;
            break;
        case utilgl::Normalization::SignNormalized:
            formatScaling = 0.5 * defaultToDataRange;
            formatOffset = 1.0 - 2 * defaultToDataOffset;
            signedFormatScaling = defaultToDataRange;
            signedFormatOffset = -defaultToDataOffset;
            break;
    }

    return GLFormatRenormalization{
        .formatScaling = formatScaling,
        .signedFormatScaling = signedFormatScaling,
        .formatOffset = formatOffset,
        .signedFormatOffset = signedFormatOffset,
    };
}

}  // namespace inviwo::utilgl

/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2024 Inviwo Foundation
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

#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/util/formats.h>

namespace inviwo {

DataMapper::DataMapper() : DataMapper(DataUInt8::get(), SignedNormalization::Asymmetric) {}
DataMapper::DataMapper(const DataFormatBase* format, SignedNormalization normalization,
                       Axis aValueAxis)
    : dataRange{defaultDataRangeFor(format, normalization)}
    , valueRange{dataRange}
    , valueAxis{std::move(aValueAxis)} {}
DataMapper::DataMapper(dvec2 aDataRange, Axis aValueAxis)
    : dataRange{aDataRange}, valueRange{dataRange}, valueAxis{std::move(aValueAxis)} {}
DataMapper::DataMapper(dvec2 aDataRange, dvec2 aValueRange, Axis aValueAxis)
    : dataRange{aDataRange}, valueRange{aValueRange}, valueAxis{std::move(aValueAxis)} {}

DataMapper::DataMapper(const DataMapper& rhs) = default;

DataMapper& DataMapper::operator=(const DataMapper& that) = default;

dvec2 DataMapper::defaultDataRangeFor(const DataFormatBase* format,
                                      SignedNormalization normalization) {
    switch (format->getNumericType()) {
        case NumericType::Float:
            return {0.0, 1.0};
        case NumericType::UnsignedInteger:
            return {0.0, format->getMax()};
        case NumericType::SignedInteger:
            if (normalization == SignedNormalization::Symmetric) {
                return {-format->getMax(), format->getMax()};
            } else {
                return {format->getMin(), format->getMax()};
            }
        case NumericType::NotSpecialized:
            return {format->getMin(), format->getMax()};
    }
    return {format->getMin(), format->getMax()};
}

void DataMapper::initWithFormat(const DataFormatBase* format, SignedNormalization normalization) {

    dataRange = defaultDataRangeFor(format, normalization);
    valueRange = dataRange;
    valueAxis = {"", Unit{}};
}

}  // namespace inviwo

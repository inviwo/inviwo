/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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

#include <modules/hdf5/hdf5types.h>
#include <inviwo/core/util/logcentral.h>

namespace inviwo::hdf5 {

#include <warn/push>
#include <warn/ignore/switch-enum>
std::optional<DataFormatId> util::getDataFormatFromDataSet(const H5::DataSet& dataset) {
    switch (dataset.getTypeClass()) {
        case H5T_NO_CLASS: break;

        case H5T_INTEGER:  {
            const auto type = dataset.getIntType();
            switch (type.getSign()) {
                case H5T_SGN_ERROR: break;

                case H5T_SGN_NONE:  {
                    return DataFormatBase::find(NumericType::UnsignedInteger, 1,
                                                type.getPrecision());
                }
                case H5T_SGN_2: {
                    return DataFormatBase::find(NumericType::SignedInteger, 1, type.getPrecision());
                }
                case H5T_NSGN: break;
            }
            break;
        }
        case H5T_FLOAT: {
            const auto type = dataset.getFloatType();
            return DataFormatBase::find(NumericType::Float, 1, type.getPrecision());
        }
        case H5T_TIME:      break;
        case H5T_STRING:    break;
        case H5T_BITFIELD:  break;
        case H5T_OPAQUE:    break;
        case H5T_COMPOUND:  break;
        case H5T_REFERENCE: break;
        case H5T_ENUM:      break;
        case H5T_VLEN:      break;
        case H5T_ARRAY:     break;
        case H5T_COMPLEX:   break;
        case H5T_NCLASSES:  break;
    }

    log::warn("HDF type not supported");
    return std::nullopt;
}
#include <warn/pop>

H5::PredType util::getPredType(const DataFormatBase* format) {
    if (!format) {
        throw Exception{SourceContext{}, "No data format given"};
    }
    if (format->getComponents() != 1) {
        throw Exception{SourceContext{}, "HDF5 only supports scalar data formats, got '{}'",
                        format->getString()};
    }

    switch (format->getNumericType()) {
        case NumericType::Float:
            switch (format->getSizeInBytes()) {
                case 4:  return H5::PredType::NATIVE_FLOAT;
                case 8:  return H5::PredType::NATIVE_DOUBLE;
                default: break;
            }
            break;
        case NumericType::SignedInteger:
            switch (format->getSizeInBytes()) {
                case 1:  return H5::PredType::NATIVE_INT8;
                case 2:  return H5::PredType::NATIVE_INT16;
                case 4:  return H5::PredType::NATIVE_INT32;
                case 8:  return H5::PredType::NATIVE_INT64;
                default: break;
            }
            break;
        case NumericType::UnsignedInteger:
            switch (format->getSizeInBytes()) {
                case 1:  return H5::PredType::NATIVE_UINT8;
                case 2:  return H5::PredType::NATIVE_UINT16;
                case 4:  return H5::PredType::NATIVE_UINT32;
                case 8:  return H5::PredType::NATIVE_UINT64;
                default: break;
            }
            break;
        case NumericType::NotSpecialized: break;
    }
    throw Exception{SourceContext{}, "Data format '{}' is not supported by HDF5",
                    format->getString()};
}

}  // namespace inviwo::hdf5

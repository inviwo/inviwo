/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

namespace inviwo {

namespace hdf5 {

#include <warn/push>
#include <warn/ignore/switch-enum>

IVW_MODULE_HDF5_API const DataFormatBase* util::getDataFormatFromDataSet(
    const H5::DataSet& dataset) {
    NumericType numerictype;
    const int components = 1;
    size_t presision = 8;

    switch (dataset.getTypeClass()) {
        case H5T_INTEGER: {
            H5::IntType type = dataset.getIntType();
            presision = type.getPrecision();

            switch (type.getSign()) {
                case H5T_SGN_NONE: {
                    numerictype = NumericType::UnsignedInteger;
                    break;
                }
                case H5T_SGN_2: {
                    numerictype = NumericType::SignedInteger;
                    break;
                }
                default: {
                    LogWarnCustom("HDFType", "HDF type not supported");
                    return nullptr;
                }
            }

            break;
        }
        case H5T_FLOAT: {
            H5::FloatType type = dataset.getFloatType();
            numerictype = NumericType::Float;
            presision = type.getPrecision();
            break;
        }
        case H5T_ARRAY: {
            H5::ArrayType type = dataset.getArrayType();
            LogWarnCustom("HDFType", "HDF type not supported");
            return nullptr;
        }
        default: {
            LogWarnCustom("HDFType", "HDF type not supported");
            return nullptr;
        }
    }

    return DataFormatBase::get(numerictype, components, presision);
}

#include <warn/pop>

}  // namespace hdf5

}  // namespace inviwo

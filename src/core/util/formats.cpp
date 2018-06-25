/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include <inviwo/core/util/formats.h>
#include <inviwo/core/util/stringconversion.h>

#include <unordered_map>

namespace inviwo {

DataFormatException::DataFormatException(const std::string& message, ExceptionContext context)
    : Exception(message, context) {}

DataFormatBase::DataFormatBase(DataFormatId t, size_t c, size_t size, double max, double min,
                               double lowest, NumericType nt, const std::string& s)
    : formatId_(t)
    , components_(c)
    , size_(size)
    , numericType_(nt)
    , max_(max)
    , min_(min)
    , lowest_(lowest)
    , formatStr_(s) {}

DataFormatBase::~DataFormatBase() = default;

size_t DataFormatBase::getSize() const { return size_; }

NumericType DataFormatBase::getNumericType() const { return numericType_; }

size_t DataFormatBase::getComponents() const { return components_; }

size_t DataFormatBase::getPrecision() const { return size_ / components_ * 8; }

double DataFormatBase::getMax() const { return max_; }

double DataFormatBase::getMin() const { return min_; }

double DataFormatBase::getLowest() const { return lowest_; }

const char* DataFormatBase::getString() const { return formatStr_.c_str(); }

DataFormatId DataFormatBase::getId() const { return formatId_; }

double DataFormatBase::valueToDouble(void*) const { return 0.0; }
dvec2 DataFormatBase::valueToVec2Double(void*) const { return dvec2(0.0); }
dvec3 DataFormatBase::valueToVec3Double(void*) const { return dvec3(0.0); }
dvec4 DataFormatBase::valueToVec4Double(void*) const { return dvec4(0.0); }
double DataFormatBase::valueToNormalizedDouble(void*) const { return 0.0; }
dvec2 DataFormatBase::valueToNormalizedVec2Double(void*) const { return dvec2(0.0); }
dvec3 DataFormatBase::valueToNormalizedVec3Double(void*) const { return dvec3(0.0); }
dvec4 DataFormatBase::valueToNormalizedVec4Double(void*) const { return dvec4(0.0); }
void DataFormatBase::doubleToValue(double val, void* loc) const {
    *static_cast<double*>(loc) = val;
}
void DataFormatBase::vec2DoubleToValue(dvec2 val, void* loc) const {
    *static_cast<dvec2*>(loc) = val;
}
void DataFormatBase::vec3DoubleToValue(dvec3 val, void* loc) const {
    *static_cast<dvec3*>(loc) = val;
}
void DataFormatBase::vec4DoubleToValue(dvec4 val, void* loc) const {
    *static_cast<dvec4*>(loc) = val;
}

const DataFormatBase* DataFormatBase::get() {
    static DataFormatBase instance{DataFormatId::NotSpecialized, 0, 0, 0.0, 0.0, 0.0,
                                   NumericType::NotSpecialized, "NotSpecialized"};
    return &instance;
}

// clang-format off
const DataFormatBase* DataFormatBase::get(DataFormatId id) {
    switch(id) {
        case DataFormatId::Float16: return DataFloat16::get();
        case DataFormatId::Float32: return DataFloat32::get();
        case DataFormatId::Float64: return DataFloat64::get();
        case DataFormatId::Int8: return DataInt8::get();
        case DataFormatId::Int16: return DataInt16::get();
        case DataFormatId::Int32: return DataInt32::get();
        case DataFormatId::Int64: return DataInt64::get();
        case DataFormatId::UInt8: return DataUInt8::get();
        case DataFormatId::UInt16: return DataUInt16::get();
        case DataFormatId::UInt32: return DataUInt32::get();
        case DataFormatId::UInt64: return DataUInt64::get();
        case DataFormatId::Vec2Float16: return DataVec2Float16::get();
        case DataFormatId::Vec2Float32: return DataVec2Float32::get();
        case DataFormatId::Vec2Float64: return DataVec2Float64::get();
        case DataFormatId::Vec2Int8: return DataVec2Int8::get();
        case DataFormatId::Vec2Int16: return DataVec2Int16::get();
        case DataFormatId::Vec2Int32: return DataVec2Int32::get();
        case DataFormatId::Vec2Int64: return DataVec2Int64::get();
        case DataFormatId::Vec2UInt8: return DataVec2UInt8::get();
        case DataFormatId::Vec2UInt16: return DataVec2UInt16::get();
        case DataFormatId::Vec2UInt32: return DataVec2UInt32::get();
        case DataFormatId::Vec2UInt64: return DataVec2UInt64::get();
        case DataFormatId::Vec3Float16: return DataVec3Float16::get();
        case DataFormatId::Vec3Float32: return DataVec3Float32::get();
        case DataFormatId::Vec3Float64: return DataVec3Float64::get();
        case DataFormatId::Vec3Int8: return DataVec3Int8::get();
        case DataFormatId::Vec3Int16: return DataVec3Int16::get();
        case DataFormatId::Vec3Int32: return DataVec3Int32::get();
        case DataFormatId::Vec3Int64: return DataVec3Int64::get();
        case DataFormatId::Vec3UInt8: return DataVec3UInt8::get();
        case DataFormatId::Vec3UInt16: return DataVec3UInt16::get();
        case DataFormatId::Vec3UInt32: return DataVec3UInt32::get();
        case DataFormatId::Vec3UInt64: return DataVec3UInt64::get();
        case DataFormatId::Vec4Float16: return DataVec4Float16::get();
        case DataFormatId::Vec4Float32: return DataVec4Float32::get();
        case DataFormatId::Vec4Float64: return DataVec4Float64::get();
        case DataFormatId::Vec4Int8: return DataVec4Int8::get();
        case DataFormatId::Vec4Int16: return DataVec4Int16::get();
        case DataFormatId::Vec4Int32: return DataVec4Int32::get();
        case DataFormatId::Vec4Int64: return DataVec4Int64::get();
        case DataFormatId::Vec4UInt8: return DataVec4UInt8::get();
        case DataFormatId::Vec4UInt16: return DataVec4UInt16::get();
        case DataFormatId::Vec4UInt32: return DataVec4UInt32::get();
        case DataFormatId::Vec4UInt64: return DataVec4UInt64::get();
        case DataFormatId::NotSpecialized: return DataFormatBase::get();
        case DataFormatId::NumberOfFormats:
        default:
            throw DataFormatException("Invalid format id", IvwContextCustom("DataFormat"));
    }
}
// clang-format on

const DataFormatBase* DataFormatBase::get(const std::string& name) {
    static std::unordered_map<std::string, const DataFormatBase*> nameMap = []() {
        std::unordered_map<std::string, const DataFormatBase*> res;
        res[toLower(DataFloat16::str())] = DataFloat16::get();
        res[toLower(DataFloat32::str())] = DataFloat32::get();
        res[toLower(DataFloat64::str())] = DataFloat64::get();
        res[toLower(DataInt8::str())] = DataInt8::get();
        res[toLower(DataInt16::str())] = DataInt16::get();
        res[toLower(DataInt32::str())] = DataInt32::get();
        res[toLower(DataInt64::str())] = DataInt64::get();
        res[toLower(DataUInt8::str())] = DataUInt8::get();
        res[toLower(DataUInt16::str())] = DataUInt16::get();
        res[toLower(DataUInt32::str())] = DataUInt32::get();
        res[toLower(DataUInt64::str())] = DataUInt64::get();
        res[toLower(DataVec2Float16::str())] = DataVec2Float16::get();
        res[toLower(DataVec2Float32::str())] = DataVec2Float32::get();
        res[toLower(DataVec2Float64::str())] = DataVec2Float64::get();
        res[toLower(DataVec2Int8::str())] = DataVec2Int8::get();
        res[toLower(DataVec2Int16::str())] = DataVec2Int16::get();
        res[toLower(DataVec2Int32::str())] = DataVec2Int32::get();
        res[toLower(DataVec2Int64::str())] = DataVec2Int64::get();
        res[toLower(DataVec2UInt8::str())] = DataVec2UInt8::get();
        res[toLower(DataVec2UInt16::str())] = DataVec2UInt16::get();
        res[toLower(DataVec2UInt32::str())] = DataVec2UInt32::get();
        res[toLower(DataVec2UInt64::str())] = DataVec2UInt64::get();
        res[toLower(DataVec3Float16::str())] = DataVec3Float16::get();
        res[toLower(DataVec3Float32::str())] = DataVec3Float32::get();
        res[toLower(DataVec3Float64::str())] = DataVec3Float64::get();
        res[toLower(DataVec3Int8::str())] = DataVec3Int8::get();
        res[toLower(DataVec3Int16::str())] = DataVec3Int16::get();
        res[toLower(DataVec3Int32::str())] = DataVec3Int32::get();
        res[toLower(DataVec3Int64::str())] = DataVec3Int64::get();
        res[toLower(DataVec3UInt8::str())] = DataVec3UInt8::get();
        res[toLower(DataVec3UInt16::str())] = DataVec3UInt16::get();
        res[toLower(DataVec3UInt32::str())] = DataVec3UInt32::get();
        res[toLower(DataVec3UInt64::str())] = DataVec3UInt64::get();
        res[toLower(DataVec4Float16::str())] = DataVec4Float16::get();
        res[toLower(DataVec4Float32::str())] = DataVec4Float32::get();
        res[toLower(DataVec4Float64::str())] = DataVec4Float64::get();
        res[toLower(DataVec4Int8::str())] = DataVec4Int8::get();
        res[toLower(DataVec4Int16::str())] = DataVec4Int16::get();
        res[toLower(DataVec4Int32::str())] = DataVec4Int32::get();
        res[toLower(DataVec4Int64::str())] = DataVec4Int64::get();
        res[toLower(DataVec4UInt8::str())] = DataVec4UInt8::get();
        res[toLower(DataVec4UInt16::str())] = DataVec4UInt16::get();
        res[toLower(DataVec4UInt32::str())] = DataVec4UInt32::get();
        res[toLower(DataVec4UInt64::str())] = DataVec4UInt64::get();
        res["uchar"] = DataUInt8::get();
        res["char"] = DataInt8::get();
        res["ushort"] = DataUInt16::get();
        res["short"] = DataInt16::get();
        res["uint"] = DataUInt32::get();
        res["int"] = DataInt32::get();
        res["float"] = DataFloat32::get();
        res["double"] = DataFloat64::get();
        return res;
    }();

    auto it = nameMap.find(toLower(name));
    if (it != nameMap.end()) {
        return it->second;
    } else {
        throw DataFormatException("Invalid format string: '" + name + "'",
                                  IvwContextCustom("DataFormat"));
    }
}

const DataFormatBase* DataFormatBase::get(NumericType type, size_t components, size_t precision) {
    switch (type) {
        case NumericType::Float:
            switch (components) {
                case 1:
                    switch (precision) {
                        case 16:
                            return DataFloat16::get();
                        case 32:
                            return DataFloat32::get();
                        case 64:
                            return DataFloat64::get();
                    }
                    break;
                case 2:
                    switch (precision) {
                        case 16:
                            return DataVec2Float16::get();
                        case 32:
                            return DataVec2Float32::get();
                        case 64:
                            return DataVec2Float64::get();
                    }
                    break;
                case 3:
                    switch (precision) {
                        case 16:
                            return DataVec3Float16::get();
                        case 32:
                            return DataVec3Float32::get();
                        case 64:
                            return DataVec3Float64::get();
                    }
                    break;
                case 4:
                    switch (precision) {
                        case 16:
                            return DataVec4Float16::get();
                        case 32:
                            return DataVec4Float32::get();
                        case 64:
                            return DataVec4Float64::get();
                    }
                    break;
            }
            break;
        case NumericType::SignedInteger:
            switch (components) {
                case 1:
                    switch (precision) {
                        case 8:
                            return DataInt8::get();
                        case 16:
                            return DataInt16::get();
                        case 32:
                            return DataInt32::get();
                        case 64:
                            return DataInt64::get();
                    }
                    break;
                case 2:
                    switch (precision) {
                        case 8:
                            return DataVec2Int8::get();
                        case 16:
                            return DataVec2Int16::get();
                        case 32:
                            return DataVec2Int32::get();
                        case 64:
                            return DataVec2Int64::get();
                    }
                    break;
                case 3:
                    switch (precision) {
                        case 8:
                            return DataVec3Int8::get();
                        case 16:
                            return DataVec3Int16::get();
                        case 32:
                            return DataVec3Int32::get();
                        case 64:
                            return DataVec3Int64::get();
                    }
                    break;
                case 4:
                    switch (precision) {
                        case 8:
                            return DataVec4Int8::get();
                        case 16:
                            return DataVec4Int16::get();
                        case 32:
                            return DataVec4Int32::get();
                        case 64:
                            return DataVec4Int64::get();
                    }
                    break;
            }
            break;
        case NumericType::UnsignedInteger:
            switch (components) {
                case 1:
                    switch (precision) {
                        case 8:
                            return DataUInt8::get();
                        case 16:
                            return DataUInt16::get();
                        case 32:
                            return DataUInt32::get();
                        case 64:
                            return DataUInt64::get();
                    }
                    break;
                case 2:
                    switch (precision) {
                        case 8:
                            return DataVec2UInt8::get();
                        case 16:
                            return DataVec2UInt16::get();
                        case 32:
                            return DataVec2UInt32::get();
                        case 64:
                            return DataVec2UInt64::get();
                    }
                    break;
                case 3:
                    switch (precision) {
                        case 8:
                            return DataVec3UInt8::get();
                        case 16:
                            return DataVec3UInt16::get();
                        case 32:
                            return DataVec3UInt32::get();
                        case 64:
                            return DataVec3UInt64::get();
                    }
                    break;
                case 4:
                    switch (precision) {
                        case 8:
                            return DataVec4UInt8::get();
                        case 16:
                            return DataVec4UInt16::get();
                        case 32:
                            return DataVec4UInt32::get();
                        case 64:
                            return DataVec4UInt64::get();
                    }
                    break;
            }
            break;
        case NumericType::NotSpecialized:
        default:
            break;
    }
    return nullptr;
}

}  // namespace inviwo

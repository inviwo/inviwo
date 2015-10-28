/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

namespace inviwo {

DataFormatBase* DataFormatBase::instance_[] = {nullptr};

DataFormatBase::DataFormatBase() : formatId_(id()), components_(components()), size_(size()), formatStr_("") {}

DataFormatBase::DataFormatBase(DataFormatId t, size_t c, size_t size, double max, double min,
                               NumericType nt, std::string s)
    : formatId_(t), components_(c), size_(size), numericType_(nt), max_(max), min_(min), formatStr_(s) {}

DataFormatBase::~DataFormatBase() {}

const DataFormatBase* DataFormatBase::get() {
    if (!instance_[static_cast<size_t>(DataFormatId::NotSpecialized)])
        instance_[static_cast<size_t>(DataFormatId::NotSpecialized)] = new DataFormatBase();

    return instance_[static_cast<size_t>(DataFormatId::NotSpecialized)];
}

const DataFormatBase* DataFormatBase::get(DataFormatId id) {
    if (!instance_[static_cast<size_t>(id)]){
        if (id == DataFormatId::NotSpecialized) return DataFormatBase::get();
#define DataFormatIdMacro(i) else if(id == DataFormatId::i) return Data##i::get();
#include <inviwo/core/util/formatsdefinefunc.h>
    }

    return instance_[static_cast<size_t>(id)];
}

const DataFormatBase* DataFormatBase::get(std::string name) {
    name = toLower(name);
    if (name == "") return DataFormatBase::get();
#define DataFormatIdMacro(i) else if(name == toLower(#i)) return Data##i::get();
#include <inviwo/core/util/formatsdefinefunc.h>
    else if (name == "uchar") return DataUInt8::get();
    else if (name == "char") return DataInt8::get();
    else if (name == "ushort") return DataUInt16::get();
    else if (name == "short") return DataInt16::get();
    else if (name == "uint") return DataUInt32::get();
    else if (name == "int") return DataInt32::get();
    else if (name == "float") return DataFloat32::get();
    else if (name == "double") return DataFloat64::get();
    else return DataFormatBase::get();
}

const DataFormatBase* DataFormatBase::get(NumericType type, size_t components,
                                          size_t precision) {
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

void DataFormatBase::cleanDataFormatBases() {
    for (auto& elem : instance_) {
        if (elem) {
            delete elem;
            elem = nullptr;
        }
    }
}

double DataFormatBase::valueToDouble(void*) const {
    return 0.0;
}

dvec2 DataFormatBase::valueToVec2Double(void*) const {
    return dvec2(0.0);
}

dvec3 DataFormatBase::valueToVec3Double(void*) const {
    return dvec3(0.0);
}

dvec4 DataFormatBase::valueToVec4Double(void*) const {
    return dvec4(0.0);
}

double DataFormatBase::valueToNormalizedDouble(void*) const {
    return 0.0;
}

dvec2 DataFormatBase::valueToNormalizedVec2Double(void*) const {
    return dvec2(0.0);
}

dvec3 DataFormatBase::valueToNormalizedVec3Double(void*) const {
    return dvec3(0.0);
}

dvec4 DataFormatBase::valueToNormalizedVec4Double(void*) const {
    return dvec4(0.0);
}

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

size_t DataFormatBase::getSize() const {
    return size_;
}

NumericType DataFormatBase::getNumericType() const {
    return numericType_;
}

size_t DataFormatBase::getComponents() const {
    return components_;
}

double DataFormatBase::getMax() const {
    return max_;
}

double DataFormatBase::getMin() const {
    return min_;
}

const char* DataFormatBase::getString() const {
    return formatStr_.c_str();
}

DataFormatId DataFormatBase::getId() const {
    return formatId_;
}

} // namespace

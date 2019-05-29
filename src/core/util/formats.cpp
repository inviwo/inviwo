/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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
#include <inviwo/core/util/foreacharg.h>

#include <unordered_map>

namespace inviwo {

namespace {
struct AddInstance {
    template <typename T>
    void operator()(std::array<std::unique_ptr<DataFormatBase>,
                               static_cast<int>(DataFormatId::NumberOfFormats)>& res) {
        res[++i] = std::make_unique<T>();
    }
    int i = static_cast<int>(DataFormatId::NotSpecialized);
};

struct AddName {
    template <typename T>
    void operator()(std::unordered_map<std::string, const DataFormatBase*>& res) {
        res[toLower(T::str())] = T::get();
    }
};
}  // namespace

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

const DataFormatBase* DataFormatBase::get() { return getPointer(DataFormatId::NotSpecialized); }

const DataFormatBase* DataFormatBase::getPointer(DataFormatId id) {
    using DataFormatArray = std::array<std::unique_ptr<DataFormatBase>,
                                       static_cast<int>(DataFormatId::NumberOfFormats)>;
    static const DataFormatArray instances = []() {
        DataFormatArray res;
        res[static_cast<int>(DataFormatId::NotSpecialized)] =
            std::make_unique<DataFormatBase>(DataFormatId::NotSpecialized, 0, 0, 0.0, 0.0, 0.0,
                                             NumericType::NotSpecialized, "NotSpecialized");
        util::for_each_type<DefaultDataFormats>{}(AddInstance{}, res);
        return res;
    }();

    return instances[static_cast<int>(id)].get();
}

const DataFormatBase* DataFormatBase::get(DataFormatId id) {
    if (static_cast<int>(id) < static_cast<int>(DataFormatId::NumberOfFormats) &&
        static_cast<int>(id) >= 0) {
        return getPointer(id);
    } else {
        throw DataFormatException("Invalid format id", IVW_CONTEXT_CUSTOM("DataFormat"));
    }
}

const DataFormatBase* DataFormatBase::get(const std::string& name) {
    static std::unordered_map<std::string, const DataFormatBase*> nameMap = []() {
        std::unordered_map<std::string, const DataFormatBase*> res;
        res["uchar"] = DataUInt8::get();
        res["char"] = DataInt8::get();
        res["ushort"] = DataUInt16::get();
        res["short"] = DataInt16::get();
        res["uint"] = DataUInt32::get();
        res["int"] = DataInt32::get();
        res["float"] = DataFloat32::get();
        res["double"] = DataFloat64::get();
        res["notspecialized"] = DataFormatBase::get();
        util::for_each_type<DefaultDataFormats>{}(AddName{}, res);
        return res;
    }();

    auto it = nameMap.find(toLower(name));
    if (it != nameMap.end()) {
        return it->second;
    } else {
        throw DataFormatException("Invalid format string: '" + name + "'",
                                  IVW_CONTEXT_CUSTOM("DataFormat"));
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

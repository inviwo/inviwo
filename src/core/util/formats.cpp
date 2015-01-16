/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

DataFormatBase* DataFormatBase::instance_[] = {NULL};

DataFormatBase::DataFormatBase()
    : formatId_(id())
    , bitsAllocated_(bitsAllocated())
    , bitsStored_(bitsStored())
    , components_(components()) {
    formatStr_ = new std::string(str());
}

DataFormatBase::DataFormatBase(DataFormatEnums::Id t, size_t bA, size_t bS, int c, double max, double min, DataFormatEnums::NumericType nt, std::string s)
    : formatId_(t)
    , bitsAllocated_(bA)
    , bitsStored_(bS)
    , components_(c)
    , numericType_(nt)
    , max_(max)
    , min_(min) {
    formatStr_ = new std::string(s);
}

DataFormatBase::~DataFormatBase() {
    delete formatStr_;
}

const DataFormatBase* DataFormatBase::get() {
    if (!instance_[DataFormatEnums::NOT_SPECIALIZED])
        instance_[DataFormatEnums::NOT_SPECIALIZED] = new DataFormatBase();

    return instance_[DataFormatEnums::NOT_SPECIALIZED];
}

const DataFormatBase* DataFormatBase::get(DataFormatEnums::Id id) {
    if (!instance_[id]){
        if (id == DataFormatEnums::NOT_SPECIALIZED) return DataFormatBase::get();
#define DataFormatIdMacro(i) else if(id == DataFormatEnums::i) return Data##i::get();
#include <inviwo/core/util/formatsdefinefunc.h>
    }

    return instance_[id];
}

const DataFormatBase* DataFormatBase::get(std::string name) {
    if (name == "") return DataFormatBase::get();

#define DataFormatIdMacro(i) else if(name == #i) return Data##i::get();
#include <inviwo/core/util/formatsdefinefunc.h>
    else if (name == "UCHAR") return DataUINT8::get();
    else if (name == "CHAR") return DataINT8::get();
    else if (name == "USHORT") return DataUINT16::get();
    else if (name == "USHORT_12") return DataUINT12::get();
    else if (name == "SHORT") return DataINT16::get();
    else if (name == "UINT") return DataUINT32::get();
    else if (name == "INT") return DataINT32::get();
    else if (name == "FLOAT") return DataFLOAT32::get();
    else if (name == "DOUBLE") return DataFLOAT64::get();
    else return DataFormatBase::get();
}

const DataFormatBase* DataFormatBase::get(DataFormatEnums::NumericType type, int components, int precision) {
    switch (type) {
        case DataFormatEnums::FLOAT_TYPE:
            switch (components) {
        case 1:
            switch (precision) {
        case 16:
            return DataFLOAT16::get();
        case 32:
            return DataFLOAT32::get();
        case 64:
            return DataFLOAT64::get();
            }
            break;
        case 2:
            switch (precision) {
        case 16:
            return DataVec2FLOAT16::get();
        case 32:
            return DataVec2FLOAT32::get();
        case 64:
            return DataVec2FLOAT64::get();
            }
            break;
        case 3:
            switch (precision) {
        case 16:
            return DataVec3FLOAT16::get();
        case 32:
            return DataVec3FLOAT32::get();
        case 64:
            return DataVec3FLOAT64::get();
            }
            break;
        case 4:
            switch (precision) {
        case 16:
            return DataVec4FLOAT16::get();
        case 32:
            return DataVec4FLOAT32::get();
        case 64:
            return DataVec4FLOAT64::get();
            }
            break;
            }
            break;
        case DataFormatEnums::SIGNED_INTEGER_TYPE:
            switch (components) {
        case 1:
            switch (precision) {
        case 8:
            return DataINT8::get();
        case 12:
            return DataINT12::get();
        case 16:
            return DataINT16::get();
        case 32:
            return DataINT32::get();
        case 64:
            return DataINT64::get();
            }
            break;
        case 2:
            switch (precision) {
        case 8:
            return DataVec2INT8::get();
        case 12:
            return DataVec2INT12::get();
        case 16:
            return DataVec2INT16::get();
        case 32:
            return DataVec2INT32::get();
        case 64:
            return DataVec2INT64::get();
            }
            break;
        case 3:
            switch (precision) {
        case 8:
            return DataVec3INT8::get();
        case 12:
            return DataVec3INT12::get();
        case 16:
            return DataVec3INT16::get();
        case 32:
            return DataVec3INT32::get();
        case 64:
            return DataVec3INT64::get();
            }
            break;
        case 4:
            switch (precision) {
        case 8:
            return DataVec4INT8::get();
        case 12:
            return DataVec4INT12::get();
        case 16:
            return DataVec4INT16::get();
        case 32:
            return DataVec4INT32::get();
        case 64:
            return DataVec4INT64::get();
            }
            break;
            }
            break;
        case DataFormatEnums::UNSIGNED_INTEGER_TYPE:
            switch (components) {
        case 1:
            switch (precision) {
        case 8:
            return DataUINT8::get();
        case 12:
            return DataUINT12::get();
        case 16:
            return DataUINT16::get();
        case 32:
            return DataUINT32::get();
        case 64:
            return DataUINT64::get();
            }
            break;
        case 2:
            switch (precision) {
        case 8:
            return DataVec2UINT8::get();
        case 12:
            return DataVec2UINT12::get();
        case 16:
            return DataVec2UINT16::get();
        case 32:
            return DataVec2UINT32::get();
        case 64:
            return DataVec2UINT64::get();
            }
            break;
        case 3:
            switch (precision) {
        case 8:
            return DataVec3UINT8::get();
        case 12:
            return DataVec3UINT12::get();
        case 16:
            return DataVec3UINT16::get();
        case 32:
            return DataVec3UINT32::get();
        case 64:
            return DataVec3UINT64::get();
            }
            break;
        case 4:
            switch (precision) {
        case 8:
            return DataVec4UINT8::get();
        case 12:
            return DataVec4UINT12::get();
        case 16:
            return DataVec4UINT16::get();
        case 32:
            return DataVec4UINT32::get();
        case 64:
            return DataVec4UINT64::get();
            }
            break;
            }
            break;
        default:
            break;

    }
    return NULL;
}

void DataFormatBase::cleanDataFormatBases() {
    for (int i = 0; i<DataFormatEnums::NUMBER_OF_FORMATS; i++) {
        if (instance_[i]) {
            delete instance_[i];
            instance_[i] = 0;
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
    loc = &val;
}

void DataFormatBase::vec2DoubleToValue(dvec2 val, void* loc) const {
    loc = &val;
}

void DataFormatBase::vec3DoubleToValue(dvec3 val, void* loc) const {
    loc = &val;
}

void DataFormatBase::vec4DoubleToValue(dvec4 val, void* loc) const {
    loc = &val;
}

size_t DataFormatBase::getBitsAllocated() const {
    return bitsAllocated_;
}

DataFormatEnums::NumericType DataFormatBase::getNumericType() const {
    return numericType_;
}

size_t DataFormatBase::getBitsStored() const {
    return bitsStored_;
}

size_t DataFormatBase::getBytesAllocated() const {
    return static_cast<size_t>(glm::ceil(BITS_TO_BYTES(static_cast<float>(getBitsAllocated()))));
}

size_t DataFormatBase::getBytesStored() const {
    return static_cast<size_t>(glm::ceil(BITS_TO_BYTES(static_cast<float>(getBitsStored()))));
}

int DataFormatBase::getComponents() const {
    return components_;
}

double DataFormatBase::getMax() const {
    return max_;
}

double DataFormatBase::getMin() const {
    return min_;
}

const char* DataFormatBase::getString() const {
    return formatStr_->c_str();
}

DataFormatEnums::Id DataFormatBase::getId() const {
    return formatId_;
}

} // namespace

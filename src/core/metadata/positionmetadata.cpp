/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/core/metadata/positionmetadata.h>

namespace inviwo {

PositionMetaData::PositionMetaData()
    : IntVec2MetaData(ivec2(0,0))
{}

PositionMetaData::PositionMetaData(int x, int y)
    : IntVec2MetaData(ivec2(x,y))
{}

PositionMetaData::~PositionMetaData() {}

PositionMetaData* PositionMetaData::clone() const {
    return new PositionMetaData(*this);
}

ivec2 PositionMetaData::getXY() {
    return get();
}

void PositionMetaData::setXY(const ivec2& xy) {
    set(xy);
}

int PositionMetaData::getX() {
    return get().x;
}

void PositionMetaData::setX(const int& x) {
    ivec2 value = get();
    value.x = x;
    set(value);
}

int PositionMetaData::getY() {
    return get().y;
}

void PositionMetaData::setY(const int& y) {
    ivec2 value = get();
    value.y = y;
    set(value);
}

void PositionMetaData::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("position", value_);
}

void PositionMetaData::deserialize(Deserializer& d) {
    std::string className;
    d.deserialize("type", className, SerializationTarget::Attribute);
    d.deserialize("position", value_);
}

const std::string PositionMetaData::CLASS_IDENTIFIER = "org.inviwo.PositionMetaData";

} // namespace

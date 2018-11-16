/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

#include <modules/discretedata/discretedatamoduledefine.h>
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/discretedata/channels/datachannel.h>

namespace inviwo {
namespace discretedata {

Channel::Channel(ind numComponents, const std::string& name, DataFormatId format,
                 GridPrimitive prim) {
    // Create and set name.
    StringMetaData* nameData = createMetaData<StringMetaData>("Name");
    *nameData = name;

    // Create and set format identifier.
    IntMetaData* formatData = createMetaData<IntMetaData>("DataFormatId");
    *formatData = (int)format;

    // Create and set GridPrimitive type.
    IntMetaData* dimData = createMetaData<IntMetaData>("GridPrimitiveType");
    *dimData = (int)prim;

    // Create and set NumComponents type.
    IntMetaData* compData = createMetaData<IntMetaData>("NumComponents");
    *compData = (int)numComponents;
}

const std::string Channel::getName() const { return getMetaData<StringMetaData>("Name")->get(); }

void Channel::setName(const std::string& name) { setMetaData<StringMetaData>("Name", name); }

GridPrimitive Channel::getGridPrimitiveType() const {
    return (GridPrimitive)getMetaData<IntMetaData>("GridPrimitiveType")->get();
}

DataFormatId Channel::getDataFormatId() const {
    return (DataFormatId)getMetaData<IntMetaData>("DataFormatId")->get();
}

ind Channel::getNumComponents() const {
    return (ind)getMetaData<IntMetaData>("NumComponents")->get();
}

void Channel::setGridPrimitiveType(GridPrimitive prim) {
    setMetaData<IntMetaData>("GridPrimitiveType", (int)prim);
}

void Channel::setDataFormatId(DataFormatId format) {
    setMetaData<IntMetaData>("DataFormatId", (int)format);
}

void Channel::setNumComponents(ind numComp) {
    setMetaData<IntMetaData>("NumComponents", (int)numComp);
}

}  // namespace discretedata
}  // namespace inviwo

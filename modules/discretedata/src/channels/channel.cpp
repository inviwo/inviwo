/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/discretedata/channels/channel.h>

namespace inviwo {

namespace discretedata {

Channel::Channel(ind numComponents, const std::string& name, DataFormatId format,
                 GridPrimitive prim)
    : MetaDataOwner()
    , name_{name}
    , format_{DataFormatBase::get(format)}
    , grid_{prim}
    , numComponents_{numComponents} {}

const std::string Channel::getName() const { return name_; }

void Channel::setName(const std::string& name) { name_ = name; }

GridPrimitive Channel::getGridPrimitiveType() const { return grid_; }

DataFormatId Channel::getDataFormatId() const { return format_->getId(); }

ind Channel::getNumComponents() const { return numComponents_; }

void Channel::setGridPrimitiveType(GridPrimitive prim) { grid_ = prim; }

void Channel::setDataFormatId(DataFormatId format) { format_ = DataFormatBase::get(format); }

void Channel::setNumComponents(ind numComp) { numComponents_ = numComp; }

}  // namespace discretedata

}  // namespace inviwo

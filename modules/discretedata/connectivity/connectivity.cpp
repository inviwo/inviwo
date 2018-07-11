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

#include <discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include "discretedata/connectivity/connectivity.h"
#include "discretedata/connectivity/elementiterator.h"

namespace inviwo {
namespace dd {

ind Connectivity::getNumElements(GridPrimitive elementType) const {
    ivwAssert((ind)numGridPrimitives_.size() == (ind)gridDimension_ + 1,
              "GridPrimitive count vector has the wrong size: " +
                  std::to_string(numGridPrimitives_.size()) +
                  " != " + std::to_string((ind)gridDimension_ + 1));
    ivwAssert(numGridPrimitives_[elementType] != -1, "No size stored.");
    return numGridPrimitives_[elementType];
}

ElementRange Connectivity::all(GridPrimitive dim) const { return ElementRange(dim, this); }

double Connectivity::getPrimitiveMeasure(ElementIterator& element) const {
    return getPrimitiveMeasure(element.getType(), element.getIndex());
}

}  // namespace
}

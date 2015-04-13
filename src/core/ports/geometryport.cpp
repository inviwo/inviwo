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

#include <inviwo/core/ports/geometryport.h>
#include <inviwo/core/datastructures/geometry/simplemesh.h>

namespace inviwo {

uvec3 GeometryInport::colorCode = uvec3(188, 188, 101);

// Geometry Inport
GeometryInport::GeometryInport(std::string identifier, InvalidationLevel invalidationLevel)
    : DataInport<Geometry>(identifier, invalidationLevel) {}

GeometryInport::~GeometryInport() {}

uvec3 GeometryInport::getColorCode() const { return GeometryInport::colorCode; }

GeometryMultiInport::GeometryMultiInport(std::string identifier)
    : MultiDataInport<Geometry>(identifier) {}

GeometryMultiInport::~GeometryMultiInport() {}

uvec3 GeometryMultiInport::getColorCode() const { return GeometryInport::colorCode; }

// Geometry Outport
GeometryOutport::GeometryOutport(std::string identifier, InvalidationLevel invalidationLevel)
    : DataOutport<Geometry>(identifier, invalidationLevel) {}

GeometryOutport::~GeometryOutport() {}

uvec3 GeometryOutport::getColorCode() const { return GeometryInport::colorCode; }

}  // namespace
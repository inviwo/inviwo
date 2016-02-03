/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include "pathlineproperties.h"

namespace inviwo {

PropertyClassIdentifier(PathLineProperties, "org.inviwo.PathLineProperties");

PathLineProperties::PathLineProperties(std::string identifier, std::string displayName)
    : IntegralLineProperties(identifier, displayName)

    , startT_("startT", "Start at timestep", 0, 0, 1)

{
    setUpProperties();
}

PathLineProperties::PathLineProperties(const PathLineProperties& rhs)
    : IntegralLineProperties(rhs), startT_(rhs.startT_) {
    setUpProperties();
}

PathLineProperties& PathLineProperties::operator=(const PathLineProperties& that) {
    if (this != &that) {
        PathLineProperties::operator=(that);
        startT_ = that.startT_;
    }
    return *this;
}

PathLineProperties* PathLineProperties::clone() const { return new PathLineProperties(*this); }

PathLineProperties::~PathLineProperties() {}

void PathLineProperties::deserialize(Deserializer& d) {
    IntegralLineProperties::deserialize(d);
}

void PathLineProperties::setUpProperties() {
    addProperty(startT_);
}

}  // namespace

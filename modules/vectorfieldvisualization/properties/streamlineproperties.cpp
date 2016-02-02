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

#include "streamlineproperties.h"

namespace inviwo {

PropertyClassIdentifier(StreamLineProperties, "org.inviwo.StreamLineProperties");

StreamLineProperties::StreamLineProperties(std::string identifier, std::string displayName)
    : IntegralLineProperties(identifier, displayName)
    , normalizeSamples_("normalizeSamples", "Normalize Samples", true) {
    setUpProperties();
}

StreamLineProperties::StreamLineProperties(const StreamLineProperties& rhs)
    : IntegralLineProperties(rhs), normalizeSamples_(rhs.normalizeSamples_) {
    setUpProperties();
}

void StreamLineProperties::setUpProperties() { addProperty(normalizeSamples_); }

bool StreamLineProperties::getNormalizeSamples() const { return normalizeSamples_; }

StreamLineProperties* StreamLineProperties::clone() const {
    return new StreamLineProperties(*this);
}

StreamLineProperties::~StreamLineProperties() {}

StreamLineProperties& StreamLineProperties::operator=(const StreamLineProperties& that) {
    if (this != &that) {
        IntegralLineProperties::operator=(that);
        normalizeSamples_ = that.normalizeSamples_;
    }
    return *this;
}

}  // namespace

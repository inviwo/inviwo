/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <inviwo/core/properties/propertyobserver.h>

namespace inviwo {

void PropertyObservable::notifyObserversOnSetIdentifier(Property* property,
                                                        const std::string& identifier) {
    forEachObserver([&](PropertyObserver* o) { o->onSetIdentifier(property, identifier); });
}

void PropertyObservable::notifyObserversOnSetDisplayName(Property* property,
                                                         const std::string& displayName) {
    forEachObserver([&](PropertyObserver* o) { o->onSetDisplayName(property, displayName); });
}

void PropertyObservable::notifyObserversOnSetSemantics(Property* property,
                                                       const PropertySemantics& semantics) {
    forEachObserver([&](PropertyObserver* o) { o->onSetSemantics(property, semantics); });
}

void PropertyObservable::notifyObserversOnSetReadOnly(Property* property, bool readonly) {
    forEachObserver([&](PropertyObserver* o) { o->onSetReadOnly(property, readonly); });
}

void PropertyObservable::notifyObserversOnSetVisible(Property* property, bool visible) {
    forEachObserver([&](PropertyObserver* o) { o->onSetVisible(property, visible); });
}

void PropertyObservable::notifyObserversOnSetUsageMode(Property* property, UsageMode usageMode) {
    forEachObserver([&](PropertyObserver* o) { o->onSetUsageMode(property, usageMode); });
}

void PropertyObserver::onSetIdentifier(Property*, const std::string&) {}

void PropertyObserver::onSetDisplayName(Property*, const std::string&) {}

void PropertyObserver::onSetSemantics(Property*, const PropertySemantics&) {}

void PropertyObserver::onSetReadOnly(Property*, bool) {}

void PropertyObserver::onSetVisible(Property*, bool) {}

void PropertyObserver::onSetUsageMode(Property*, UsageMode) {}

}  // namespace inviwo

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

#include <inviwo/core/properties/propertyobserver.h>

namespace inviwo {

void PropertyObservable::notifyObserversOnSetIdentifier(const std::string& identifier) const {
    for (auto it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<PropertyObserver*>(*it)->onSetIdentifier(identifier);
    }
}

void PropertyObservable::notifyObserversOnSetDisplayName(const std::string& displayName) const {
    for (auto it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<PropertyObserver*>(*it)->onSetDisplayName(displayName);
    }
}

void PropertyObservable::notifyObserversOnSetSemantics(const PropertySemantics& semantics) const {
    for (auto it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<PropertyObserver*>(*it)->onSetSemantics(semantics);
    }
}

void PropertyObservable::notifyObserversOnSetReadOnly(bool readonly) const {
    for (auto it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<PropertyObserver*>(*it)->onSetReadOnly(readonly);
    }
}

void PropertyObservable::notifyObserversOnSetVisible(bool visible) const {
    for (auto it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<PropertyObserver*>(*it)->onSetVisible(visible);
    }
}

void PropertyObservable::notifyObserversOnSetUsageMode(UsageMode usageMode) const {
    for (auto it = observers_->rbegin(); it != observers_->rend(); ++it) {
        static_cast<PropertyObserver*>(*it)->onSetUsageMode(usageMode);
    }
}

}  // namespace

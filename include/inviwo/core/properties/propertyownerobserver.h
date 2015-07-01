/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_PROPERTYOWNEROBSERVER_H
#define IVW_PROPERTYOWNEROBSERVER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

class Property;

class IVW_CORE_API PropertyOwnerObserver : public Observer {
public:
    PropertyOwnerObserver() = default;
    virtual ~PropertyOwnerObserver() = default;

    /**
    * This methods will be called when observed object changes.
    * Override it to add behavior.
    */
    virtual void onWillAddProperty(Property* property, size_t index){};
    virtual void onDidAddProperty(Property* property, size_t index){};

    virtual void onWillRemoveProperty(Property* property, size_t index){};
    virtual void onDidRemoveProperty(Property* property, size_t index){};
};

class IVW_CORE_API PropertyOwnerObservable : public Observable<PropertyOwnerObserver> {
public:
    PropertyOwnerObservable() = default;

    void notifyObserversWillAddProperty(Property* property, size_t index) const;
    void notifyObserversDidAddProperty(Property* property, size_t index) const;

    void notifyObserversWillRemoveProperty(Property* property, size_t index) const;
    void notifyObserversDidRemoveProperty(Property* property, size_t index) const;
};

}  // namespace

#endif  // IVW_PROPERTYOWNEROBSERVER_H

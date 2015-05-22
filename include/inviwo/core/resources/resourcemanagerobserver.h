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

#ifndef IVW_RESOURCE_MANAGER_OBSERVER_H
#define IVW_RESOURCE_MANAGER_OBSERVER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/resources/resource.h>

namespace inviwo {


/** \class ResourceManagerObserver
 *
 * Observer for list type objects, such as std::vector.
 *
 * @see ListObservable
 */
class IVW_CORE_API ResourceManagerObserver: public Observer {
public:
    ResourceManagerObserver(): Observer() {};

    /**
    * This method will be called when an item has been added to the observed object.
    * Override it to add behavior.
    */
    virtual void resourceAdded(const Resource*) {};

    /**
    * This method will be called when an item has been removed from the observed object.
    * Override it to add behavior.
    */
    virtual void resourceRemoved(const Resource*) {};
};


/** \class ResourceManagerObservable
 *
 * Observable for list type objects. Should call notifyResourceAdded as soon as an item
 * has been added to the list and notifyResourceRemoved when an item has been removed.
 *
 * @see Observable
 */
class IVW_CORE_API ResourceManagerObservable: public Observable<ResourceManagerObserver> {
public:
    ResourceManagerObservable(): Observable<ResourceManagerObserver>() {};


    void notifyResourceAdded(const Resource* item) const {
        // Notify observers
        ObserverSet::iterator endIt = observers_->end();

        for (ObserverSet::iterator it = observers_->begin(); it != endIt; ++it) {
            // static_cast can be used since only template class objects can be added
            static_cast<ResourceManagerObserver*>(*it)->resourceAdded(item);
        }
    }

    void notifyResourceRemoved(const Resource* item) const {
        // Notify observers
        ObserverSet::iterator endIt = observers_->end();

        for (ObserverSet::iterator it = observers_->begin(); it != endIt; ++it) {
            // static_cast can be used since only template class objects can be added
            static_cast<ResourceManagerObserver*>(*it)->resourceRemoved(item);
        }
    }
};

} // namespace

#endif // IVW_RESOURCE_MANAGER_OBSERVER_H

/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_RESOURCE_MANAGER_H
#define IVW_RESOURCE_MANAGER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/resources/resource.h>
#include <inviwo/core/resources/resourcemanagerobserver.h>
#include <inviwo/core/util/singleton.h>
#include <vector>

namespace inviwo {
/** \class ResourceManager
 * Manager for resources.
 * Resources added are owned by the ResourceManager and they will be deleted when the ResourceManager is destroyed.
 * Use an identifier to find resources added to the ResourceManager. The identifier can be the file name if it is a file resource.
 * @see ResourceTemplate
 */
// TODO: Should we add resource counting?
// TODO: How do we generate identifiers for different resources?
class IVW_CORE_API ResourceManager: public Singleton<ResourceManager>, public ResourceManagerObservable {
friend class Singleton<ResourceManager>; // Allow access to constructor
public:
    ~ResourceManager();


    /**
     * Add resource to ResourceManager.
     * Will take ownership of the resource.
     *
     * @param resource Resource to add.
     */
    void addResource(Resource* resource) { resources_->push_back(resource); notifyResourceAdded(resource); }

    /**
     * Remove resource from ResourceManager.
     * This will delete the resource.
     *
     * @param resource Resource to remove.
     */
    void removeResource(Resource* resource);

    /**
     * Remove resource from ResourceManager using the identifier.
     * This will delete the resource.
     *
     * @param identifier Identifier of resource to remove.
     */
    void removeResource(const std::string& identifier);

    /**
     * Clear all resource in ResourceManager.
     */
    void clearAllResources();

    /**
     * Get Resource using identifier.
     *
     * @param identifier Resource identifier.
     * @return NULL if the resource was not found, otherwise pointer to resource.
     */
    Resource* getResource(const std::string& identifier);

    /**
     * Get Resource using identifier and cast it to type.
     *
     * @param identifier Resource identifier.
     * @return NULL if the resource was not found or dymanic cast failed, otherwise pointer to resource.
     */
    template<typename T> T* getResourceAs(const std::string& identifier);

    /**
     * Check if ResourceManager has resource.
     *
     * @param identifier Resource identifier.
     * @return True if it exists, false otherwise.
     */
    bool hasResource(const std::string& identifier) { return getResource(identifier) != NULL; };
    /**
     * Returns a vector of Resources which are of type T. In case no Resources match T
     * an empty vector is returned.
     *
     */
    template<typename T> std::vector<T*> getResourcesByType() const {
        std::vector<T*> typedResources;

        for (std::vector<Resource*>::const_iterator it = resources_->begin(); it != resources_->end(); ++it) {
            if (dynamic_cast<T*>(*it))
                typedResources.push_back(*it);
        }

        return typedResources;
    }

    std::vector<Resource*>* getResources() { return resources_; }

protected:
    // This would preferably be private 
    // but it is not possible with current 
    // Singleton implementation
    ResourceManager(): ResourceManagerObservable() {
        resources_ = new std::vector<Resource*>();
    };
private:
    // Do not allow the Resource manager to be
    // copied
    ResourceManager(ResourceManager const&) {
        resources_ = new std::vector<Resource*>();
    };
    void operator=(ResourceManager const&) {};

    std::vector<Resource*>* resources_;
};

template<typename T>
T* inviwo::ResourceManager::getResourceAs(const std::string& identifier)
{
    return dynamic_cast<T*>(getResource(identifier));
}

} // namespace

#endif // IVW_RESOURCE_MANAGER_H
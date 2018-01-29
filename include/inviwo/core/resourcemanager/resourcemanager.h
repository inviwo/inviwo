/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#ifndef IVW_RESOURCEMANAGER_H
#define IVW_RESOURCEMANAGER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/resourcemanager/resource.h>
#include <inviwo/core/resourcemanager/resourcemanagerobserver.h>

#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

/**
 * \class ResourceManager
 * \brief A resource manager to store data to avoid creating/loading the same dataset twice.
 *
 * To avoid loading the same data twice, resources are stored in a unordered\_map are hashed by type
 * and key. Key should uniquely describe the dataset, for example, when loading a file from disk the
 * filename could be used.
 *
 * Example Usage:
 * \code{.cpp}
 * std::shared_ptr<T> loadData(std::string filename){
 *     auto rm = InviwoApplication::getPtr()->getResourceManager();
 *     if(rm->hasResource<T>(filename)){
 *        return rm->getResource<T>(filename);
 *     } else {
 *        auto data = std::make_shared<T>();
 *        ... // load/create data
 *        rm->addResource<T>(filename,data);
 *     }
 * }
 * \endcode
 *
 */
class IVW_CORE_API ResourceManager : public ResourceManagerObservable {
public:
    ResourceManager();
    virtual ~ResourceManager() = default;

    /**
     * \brief Finds and returns the resource with given key and type.
     *
     * @param key key of the resource to find
     * @return the found resource
     * @throw inviwo::Exception if resource with key and type T could not be found
     */
    template <typename T>
    TypedResource<T> &getResource(const std::string &key);

    /**
     * \brief Adds a resource to the manager
     *
     * @param key key of the resource to add
     * @param resource a shared_ptr to the data to store
     * @param overwrite a flag to indicate if overwriting existing resources is allowed.
     * @throw inviwo::Exception if resource with key and type T exists and overwrite is set to false
     */
    template <typename T>
    void addResource(const std::string &key, std::shared_ptr<T> resource, bool overwrite = false);

    /**
     * \brief Checks if a resource of type T with given key exists
     *
     * @param key the key to look for
     * @return bool true if such a resource exists, otherwise false.
     */
    template <typename T>
    bool hasResource(const std::string &key) const;

    /**
     * \brief Removes a resource from the manager.
     *
     * Does not release the memory if the data is used somewhere else.
     *
     * @param key the key of the resource to remove
     */
    template <typename T>
    void removeResource(const std::string &key);

    /**
     * \brief Removes a resource from the manager.
     *
     * Does not release the memory if the data is used somewhere else.
     *
     * This method is called from the ResourceManagerDockWidget when removing a resource.
     *
     * @param key the key of the resource to remove
     * @param type the type as a string of the resource to remove
     */
    void removeResource(const std::string &key, const std::string &type);

    /**
     * \brief Clears the resource manager.
     *
     * Does not release the memory if the data is used somewhere else.
     */
    void clear();

private:
    /**
    * \brief Convenience function to converts a type T to string.
    *
    * Uses DataTraits::dataName() 
    */
    template <typename T>
    static std::string typeToString();


    /**
    * \brief Convenience function to create a std::pair of strings for a key and a type T.
    *
    * Used for look up into the resources map. 
    * @see typeToString
    *
    * @param key the key to be used in the pair
    */
    template <typename T>
    static std::pair<std::string, std::string> keyTypePair(const std::string &key);

    std::unordered_map<std::pair<std::string, std::string>, std::shared_ptr<Resource>> resources_;
};

template <typename T>
TypedResource<T> &ResourceManager::getResource(const std::string &key) {
    IVW_ASSERT(!key.empty(), "Key should not be empty string");
    auto it = resources_.find(keyTypePair<T>(key));
    if (it == resources_.end()) {
        throw inviwo::Exception("No resource with " + key + " registered", IvwContext);
    }
    auto resourceTyped = dynamic_cast<TypedResource<T> *>(it->second.get());
    if (!resourceTyped) {
        throw inviwo::Exception("Wrong type given for resource " + key, IvwContext);
    }
    return *resourceTyped;
}

template <typename T>
void ResourceManager::addResource(const std::string &key, std::shared_ptr<T> resource,
                                  bool overwrite) {
    IVW_ASSERT(!key.empty(), "Key should not be empty string");
    auto tk = keyTypePair<T>(key);
    if (hasResource<T>(key)) {
        if (overwrite) {
            removeResource<T>(key);
        } else {
            throw inviwo::Exception("Resource with " + key + " already registered", IvwContext);
        }
    }
    auto typedResource = std::make_shared<TypedResource<T>>(resource);
    resources_[tk] = typedResource;
    notifyResourceAdded(key, tk.second, typedResource.get());
}

template <typename T>
bool ResourceManager::hasResource(const std::string &key) const {
    IVW_ASSERT(!key.empty(), "Key should not be empty string");
    auto tk = keyTypePair<T>(key);
    auto it = resources_.find(tk);
    if (it == resources_.end()) {
        return false;
    }
    auto resourceTyped = dynamic_cast<TypedResource<T> *>(it->second.get());
    if (!resourceTyped) {
        throw inviwo::Exception("Wrong type given for resource " + key, IvwContext);
    }
    return true;
}

template <typename T>
void ResourceManager::removeResource(const std::string &key) {
    removeResource(key, typeToString<T>());
}

template <typename T>
std::string ResourceManager::typeToString() {
    return DataTraits<T>::dataName();
}

template <typename T>
std::pair<std::string, std::string> ResourceManager::keyTypePair(const std::string &key) {
    return {key, typeToString<T>()};
}

}  // namespace inviwo

#endif  // IVW_RESOURCEMANAGER_H

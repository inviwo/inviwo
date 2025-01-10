/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/representationfactory.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/datastructures/representationfactorymanager.h>
#include <inviwo/core/datastructures/nodata.h>
#include <inviwo/core/resourcemanager/resource.h>

#include <inviwo/core/util/demangle.h>

#include <typeindex>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <type_traits>

namespace inviwo {

/**
 * \defgroup datastructures Datastructures
 */

/**
 * \ingroup datastructures
 *
 * \brief The base class for all data objects.
 *
 * Data is a handle class and, by it self does not have any data, it only stores metadata, and a
 * list of representations. The representations is were the actually data is stored. A Volume can
 * have a set of different representations, for example a VolumeRAM representation and a VolumeGL
 * representation. At any time at least one of the representations should be in a valid state. When
 * ever we want to access the data of a volume we will ask the volume for a representation of some
 * kind, and the handle is then responsible to try and provide that to us. If the requested
 * representation is valid the handle can just return the that representation. If not, it will have
 * to find a valid representation and try to either create the representation we wanted from the
 * valid representation, if there was no representation of the kind we asked for around. Or if there
 * is a invalid representation around, update that representation with the valid representation.
 *
 * Call getRepresentation to retrieve the data:
 * \code{.cpp}
 *      // Create a Buffer with a BufferRAM representation
 *      auto buffer = util::makeBuffer<vec2>(
 *      { {0.0f, 1.0f}, {-1.0f, -1.0f}, {1.0f, -1.0f} });
 *      // Retrieve a read-only representation of the data
 *      auto bufferRAM = buffer.getRepresentation<BufferRAM>();
 *      // The data will be transferred to the GPU if not already there.
 *      auto bufferGL = buffer.getRepresentation<BufferGL>();
 * \endcode
 *
 * Requirements:
 *      1. Copy constructor for deep copy
 *      2. Assignment operator for deep copy
 *      3. Clone pattern
 *
 * 1 and 2 are needed to be a vaild member type of std::vector.
 * 3 is needed for the factory pattern, 3 should be implemented using 1.
 *
 *
 *
 * @note Do not use the same representation in different Data objects.
 * This can cause inconsistencies since the Data objects cannot know if
 * another one has edited the representation.
 * @see Representation and RepresentationConverter
 */
template <typename Self, typename Repr>
class Data {
public:
    using self = Self;
    using repr = Repr;

    virtual Data<Self, Repr>* clone() const = 0;
    virtual ~Data() = default;

    /**
     * Get a representation of type T. If there already is a valid representation of type T, just
     * return it, if it's invalid use the last valid representation to update it so it will be
     * valid. It there is no representation of type T, create it from the last valid representation.
     * If there are no representations create a default representation and from that create a
     * representation of type T.
     */
    template <typename T>
    const T* getRepresentation() const;

    template <typename T>
    std::shared_ptr<const T> getRepresentationShared() const;

    /**
     * Get an editable representation. This will invalidate all other representations.
     * They will now have to be updated from this one before use.
     * @see getRepresentation and invalidateAllOther
     */
    template <typename T>
    T* getEditableRepresentation();

    /**
     * Check if a specific representation type exists.
     * Example:
     * \code{.cpp}
     *     hasRepresentation<BufferRAM>();
     * \endcode
     * @return true if existing, false otherwise.
     */
    template <typename T>
    bool hasRepresentation() const;

    /**
     * Check if the Data object has any representation.
     * @return true if any representation exist, false otherwise.
     */
    bool hasRepresentations() const;

    /**
     * Add the representation and set it as last valid.
     * The owner of the representation will be set to this object.
     * @note A representation can only be added to one Data object.
     * @param representation The representation to add
     */
    void addRepresentation(std::shared_ptr<Repr> representation);

    /**
     * Remove representation from data object.
     * This will delete the representation, thus rendering the representation pointer invalid.
     * @param representation The representation to remove
     */
    void removeRepresentation(const Repr* representation);

    /**
     * Remove all other representations.
     * This will delete all representations except the one passed in.
     * @param representation The representation to keep
     */
    void removeOtherRepresentations(const Repr* representation);

    /**
     * Delete all representations.
     */
    void clearRepresentations();

    /**
     * This call will make all other representations invalid. You need to call this function if
     * you are modifying a representation directly without calling getEditableRepresentation.
     * getEditableRepresentation will automatically invalidate all other representations.
     * @see getEditableRepresentation
     */
    void invalidateAllOther(const Repr* repr);

    void updateResource(const ResourceMeta& meta) const;

protected:
    Data() = default;
    Data(const Data<Self, Repr>& rhs);
    Data(Data<Self, Repr>&& rhs) = default;
    Data<Self, Repr>& operator=(const Data<Self, Repr>& that);
    Data<Self, Repr>& operator=(Data<Self, Repr>&& that) = default;

    template <typename F, typename T>
    decltype(auto) getLastOr(F&& f, T&& fallback) const {
        std::scoped_lock lock(mutex_);
        return lastValidRepresentation_ ? std::invoke(std::forward<F>(f), *lastValidRepresentation_)
                                        : std::forward<T>(fallback);
    }
    template <typename F, typename T>
    void setLastAndInvalidateOther(F&& f, T&& value) {
        std::scoped_lock lock(mutex_);
        if (lastValidRepresentation_) {
            std::invoke(std::forward<F>(f), *lastValidRepresentation_, std::forward<T>(value));
            invalidateAllOtherInternal(lastValidRepresentation_.get());
        }
    }

private:
    void copyRepresentationsTo(Data<Self, Repr>* targetData) const;
    std::shared_ptr<Repr> addRepresentationInternal(std::shared_ptr<Repr> representation) const;
    void invalidateAllOtherInternal(const Repr* repr);
    template <typename T, typename D>
    static std::shared_ptr<T> getReprInternal(D& data);

    std::shared_ptr<Repr> findRepr(std::type_index idx) const {
        if (auto it = representations_.find(idx); it != representations_.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }

    mutable std::recursive_mutex mutex_;
    mutable std::unordered_map<std::type_index, std::shared_ptr<Repr>> representations_;
    // A pointer to the the most recently updated representation. Makes updates and creation faster.
    mutable std::shared_ptr<Repr> lastValidRepresentation_;

    mutable std::optional<ResourceMeta> meta_;
};

template <typename Self, typename Repr>
Data<Self, Repr>::Data(const Data<Self, Repr>& rhs)
    : mutex_{}, representations_{}, lastValidRepresentation_{nullptr} {
    rhs.copyRepresentationsTo(this);
}

template <typename Self, typename Repr>
Data<Self, Repr>& Data<Self, Repr>::operator=(const Data<Self, Repr>& that) {
    if (this != &that) {
        that.copyRepresentationsTo(this);
    }
    return *this;
}

template <typename Self, typename Repr>
template <typename T, typename D>
std::shared_ptr<T> Data<Self, Repr>::getReprInternal(D& data) {
    const auto requestedType = std::type_index{typeid(T)};
    if (data.representations_.empty()) {
        auto factory = RepresentationFactoryManager::getRepresentationFactory<Repr>();
        auto repr = std::shared_ptr<Repr>{factory->createOrDefault(requestedType, &data)};
        if (!repr) {
            throw Exception("Failed to create default representation");
        }
        data.lastValidRepresentation_ = data.addRepresentationInternal(repr);
    }

    if (auto repr = data.findRepr(requestedType); repr && repr->isValid()) {
        data.lastValidRepresentation_ = repr;
        return std::dynamic_pointer_cast<T>(repr);
    } else {
        auto factory = RepresentationFactoryManager::getRepresentationConverterFactory<Repr>();

        const auto lastValidType = data.lastValidRepresentation_->getTypeIndex();
        if (auto package = factory->getRepresentationConverter(lastValidType, requestedType)) {
            for (auto converter : package->getConverters()) {
                const auto dstType = converter->getConverterID().second;
                const auto srcRepr = data.lastValidRepresentation_;

                if (auto dstRepr = data.findRepr(dstType)) {
                    converter->update(srcRepr, dstRepr);
                    data.lastValidRepresentation_ = dstRepr;
                    data.lastValidRepresentation_->setValid(true);
                } else {  // No representation found, create it
                    dstRepr = converter->createFrom(srcRepr);
                    if (!dstRepr) {
                        throw ConverterException("Converter failed to create");
                    }
                    data.lastValidRepresentation_ = data.addRepresentationInternal(dstRepr);
                }
            }
            return std::dynamic_pointer_cast<T>(data.lastValidRepresentation_);
        } else {
            auto buff = fmt::memory_buffer();
            for (const auto& [converterId, converter] : factory->getConverters()) {
                fmt::format_to(
                    std::back_inserter(buff), "{}({}) -> {}({})\n",
                    util::demangle(converterId.first.name()), converterId.first.hash_code(),
                    util::demangle(converterId.second.name()), converterId.second.hash_code());
            }
            throw ConverterException(
                SourceContext{},
                "Found no converters, Source {}({}),  Destination {}({})\nConverters:\n{}",
                util::demangle(data.lastValidRepresentation_->getTypeIndex().name()),
                data.lastValidRepresentation_->getTypeIndex().hash_code(),
                util::demangle(typeid(T).name()), std::type_index{typeid(T)}.hash_code(),
                fmt::string_view(buff.data(), buff.size()));
        }
    }
};

template <typename Self, typename Repr>
template <typename T>
std::shared_ptr<const T> Data<Self, Repr>::getRepresentationShared() const {
    std::scoped_lock lock(mutex_);
    return getReprInternal<const T>(*static_cast<const Self*>(this));
}

template <typename Self, typename Repr>
template <typename T>
const T* Data<Self, Repr>::getRepresentation() const {
    std::scoped_lock lock(mutex_);
    return getReprInternal<const T>(*static_cast<const Self*>(this)).get();
}

template <typename Self, typename Repr>
template <typename T>
T* Data<Self, Repr>::getEditableRepresentation() {
    std::scoped_lock lock(mutex_);
    auto repr = getReprInternal<T>(*static_cast<const Self*>(this)).get();
    invalidateAllOtherInternal(repr);
    return repr;
}

template <typename Self, typename Repr>
template <typename T>
bool Data<Self, Repr>::hasRepresentation() const {
    std::scoped_lock lock(mutex_);
    return util::has_key(representations_, std::type_index{typeid(T)});
}

template <typename Self, typename Repr>
void Data<Self, Repr>::invalidateAllOther(const Repr* repr) {
    std::scoped_lock lock(mutex_);
    invalidateAllOtherInternal(repr);
}
template <typename Self, typename Repr>
void Data<Self, Repr>::invalidateAllOtherInternal(const Repr* repr) {
    bool found = false;
    for (auto& elem : representations_) {
        if (elem.second.get() != repr) {
            elem.second->setValid(false);
        } else {
            found = true;
            elem.second->setValid(true);
            lastValidRepresentation_ = elem.second;
        }
    }
    if (!found) {
        throw Exception("Called with representation not in representations.");
    }
}

template <typename Self, typename Repr>
void Data<Self, Repr>::clearRepresentations() {
    std::scoped_lock lock(mutex_);
    representations_.clear();
}

template <typename Self, typename Repr>
void Data<Self, Repr>::copyRepresentationsTo(Data<Self, Repr>* target) const {
    std::scoped_lock targetLock(mutex_, target->mutex_);
    target->representations_.clear();

    if (lastValidRepresentation_) {
        auto rep = std::shared_ptr<Repr>(lastValidRepresentation_->clone());
        target->lastValidRepresentation_ = target->addRepresentationInternal(rep);
    }
}

template <typename Self, typename Repr>
std::shared_ptr<Repr> Data<Self, Repr>::addRepresentationInternal(
    std::shared_ptr<Repr> repr) const {
    repr->setValid(true);
    repr->setOwner(static_cast<const Self*>(this));
    representations_[repr->getTypeIndex()] = repr;
    if (meta_) {
        repr->updateResource(*meta_);
    }
    return repr;
}

template <typename Self, typename Repr>
void Data<Self, Repr>::addRepresentation(std::shared_ptr<Repr> representation) {
    std::scoped_lock lock(mutex_);
    lastValidRepresentation_ = addRepresentationInternal(std::move(representation));
}

template <typename Self, typename Repr>
void Data<Self, Repr>::removeRepresentation(const Repr* representation) {
    std::scoped_lock lock(mutex_);

    for (auto& elem : representations_) {
        if (elem.second.get() == representation) {
            representations_.erase(elem.first);
            break;
        }
    }

    if (lastValidRepresentation_.get() == representation) {
        lastValidRepresentation_.reset();

        for (auto& elem : representations_) {
            if (elem.second->isValid()) {
                lastValidRepresentation_ = elem.second;
                break;
            }
        }
    }
}

template <typename Self, typename Repr>
void Data<Self, Repr>::removeOtherRepresentations(const Repr* representation) {
    std::scoped_lock lock(mutex_);

    std::unordered_map<std::type_index, std::shared_ptr<Repr>> repr;
    for (auto& elem : representations_) {
        if (elem.second.get() == representation) {
            repr.insert(elem);
            if (lastValidRepresentation_.get() != representation) {
                if (elem.second->isValid()) {
                    lastValidRepresentation_ = elem.second;
                } else {
                    lastValidRepresentation_.reset();
                }
            }
            break;
        }
    }
    std::swap(repr, representations_);
}

template <typename Self, typename Repr>
bool Data<Self, Repr>::hasRepresentations() const {
    std::scoped_lock lock(mutex_);
    return !representations_.empty();
}

template <typename Self, typename Repr>
void Data<Self, Repr>::updateResource(const ResourceMeta& meta) const {
    meta_ = meta;
    for (auto& elem : representations_) {
        elem.second->updateResource(meta);
    }
}

template <template <typename...> class F>
struct conversion_tester {
    template <typename... Ts>
    conversion_tester(const F<Ts...>&);  // NOLINT(google-explicit-constructor)
};

template <class From, template <typename...> class To>
constexpr bool is_instance_of = std::is_convertible_v<From, conversion_tester<To>>;

/**
 * Concept for ensuring that @p T is an instance of the Data class.
 */
template <typename T>
concept DataType = is_instance_of<T, Data>;

/**
 * Concept for ensuring that @p D is an instance of the Data class that @p TRAMrep is a RAM
 * representation of @p D as well.
 *
 * Usage:
 *
 *     template <DataType T, RepresentationOf<T> TRAMrep>
 *     std::shared_ptr<T> createFromRAMRepresentation(std::shared_ptr<TRAMrep> ramRep) {
 *         return std::make_shared<T>(ramRep);
 *     }
 */
template <typename D, typename Rep>
concept RepresentationOf = is_instance_of<D, Data> && std::is_base_of_v<typename D::repr, Rep>;

}  // namespace inviwo

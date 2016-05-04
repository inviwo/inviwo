/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_SERIALIZE_BASE_H
#define IVW_SERIALIZE_BASE_H

#pragma warning(push)
#pragma warning(disable: 4263)
#include <inviwo/core/io/serialization/ticpp.h>
#pragma warning(pop)

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/io/serialization/serializeconstants.h>
#include <inviwo/core/io/serialization/serializationexception.h>
#include <inviwo/core/util/factory.h>
#include <map>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// include glm
#include <inviwo/core/util/glm.h>

namespace inviwo {

template <typename T>
struct ElementIdentifier {
    virtual ~ElementIdentifier() = default;
    virtual void setKey(TxElement*) = 0;
    virtual bool operator()(const T* elem) const = 0;
};

template <typename T>
struct StandardIdentifier : public ElementIdentifier<T> {
    typedef std::string (T::*funcPtr)() const;

    StandardIdentifier(std::string key = "identifier", funcPtr ptr = &T::getIdentifier)
        : ptr_(ptr), key_(key) {}

    virtual void setKey(TxElement* node) { identifier_ = node->GetAttributeOrDefault(key_, ""); }
    virtual bool operator()(const T* elem) const { return identifier_ == (*elem.*ptr_)(); }

private:
    funcPtr ptr_;
    std::string key_;
    std::string identifier_;
};

enum class IVW_CORE_API SerializationTarget {Node, Attribute};

class NodeSwitch;
class Serializable;

class IVW_CORE_API SerializeBase {
public:
    /**
     * \brief Base class for Serializer and Deserializer.
     *
     * This class consists of features that are common to both serializer
     * and de-serializer. Some of them are reference data manager,
     * (ticpp::Node) node switch and factory registration.
     *
     * @param allowReference disables or enables reference management schemes.
     */
    SerializeBase(bool allowReference=true);

    /**
     * \brief Base class for Serializer and Deserializer.
     *
     * This class consists of features that are common to both serializer
     * and de-serializer. Some of them are reference data manager,
     * (ticpp::Node) node switch and factory registration.
     *
     * @param s object of similar type.
     * @param allowReference disables or enables reference management schemes.
     */
    SerializeBase(SerializeBase& s, bool allowReference=true);
    /**
     * \brief Base class for Serializer and Deserializer.
     *
     * This class consists of features that are common to both serializer
     * and de-serializer. Some of them are reference data manager,
     * (ticpp::Node) node switch and factory registration.
     *
     * @param fileName full path to xml file (for reading or writing).
     * @param allowReference disables or enables reference management schemes.
     */
    SerializeBase(std::string fileName, bool allowReference=true);
    /**
     * \brief Base class for Serializer and Deserializer.
     *
     * This class consists of features that are common to both serializer
     * and de-serializer. Some of them are reference data manager,
     * (ticpp::Node) node switch and factory registration.
     *
     * @param stream containing all xml data (for reading).
     * @param path A path that will be used to decode the location of data during deserialization. 
     * @param allowReference disables or enables reference management schemes.
     */
    SerializeBase(std::istream& stream, const std::string& path, bool allowReference=true);
    
    /**
     * \brief Destructor
     */
    virtual ~SerializeBase();
    
    /**
     * \brief gets the xml file name.
     */
    virtual const std::string& getFileName() const;

    /**
     * \brief Checks whether the given type is a primitive type.
     *
     * return true if type is one of following type:
     * bool, char, signed int, unsigned int, float, double, long double, std::string
     *
     * @param type can be one of  bool, char, signed int, unsigned int, float, double, long double, std::string
     * @return true or false
     */
    bool isPrimitiveType(const std::type_info& type) const;

    struct IVW_CORE_API ReferenceData {
        TxElement* node_; //Ticpp Node element.
        bool isPointer_; //Used to differentiate pointer and object.
    };

    typedef std::pair<const void*, SerializeBase::ReferenceData> RefDataPair;
    typedef std::multimap<const void*,ReferenceData> RefMap;
    typedef std::vector<SerializeBase::ReferenceData> RefDataList;

    class IVW_CORE_API ReferenceDataContainer {
    public:
        ReferenceDataContainer();
        ~ReferenceDataContainer();
        size_t insert(const void* data, TxElement* node, bool isPointer=true);
        size_t find(const void* data);
        void* find(const std::string& key, const std::string& reference_or_id);
        TxElement* nodeCopy(const void* data);
        void setReferenceAttributes();

    private:
        RefMap referenceMap_;
        int referenceCount_;
    };


    static std::string nodeToString(const TxElement& node);


protected:
    friend class NodeSwitch;

    std::string fileName_;
    TxDocument doc_;
    TxElement* rootElement_;
    bool allowRef_;
    bool retrieveChild_;
    ReferenceDataContainer refDataContainer_;
};

class IVW_CORE_API NodeSwitch {
public:
    /**
     * \brief NodeSwitch helps track parent node during recursive/nested function calls.
     *
     * @param serializer reference to serializer or deserializer
     * @param node //Parent (Ticpp Node) element.
     */
    NodeSwitch(SerializeBase& serializer, TxElement* node, bool retrieveChild = true);

    /**
     * \brief NodeSwitch helps track parent node during recursive/nested function calls.
     *
     * @param serializer reference to serializer or deserializer
     * @param key the child to switch to.
     */
    NodeSwitch(SerializeBase& serializer, const std::string& key, bool retrieveChild = true);

    /**
     * \brief Destructor
     */
    ~NodeSwitch();

    operator bool() const;

private:
    SerializeBase& serializer_;  // reference to serializer or deserializer
    TxElement* storedNode_;  // Parent (Ticpp Node) element.
    bool storedRetrieveChild_;
};

} //namespace
#endif
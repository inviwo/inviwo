/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_DESERIALIZER_H
#define IVW_DESERIALIZER_H

#pragma warning(disable: 4290)

#include <inviwo/core/io/serialization/ivwserializebase.h>
#include <inviwo/core/util/exception.h>


namespace inviwo {

class IvwSerializable;
class VersionConverter;
    
class IVW_CORE_API IvwDeserializer : public  IvwSerializeBase {
public:
    /**
     * \brief Deserializer constructor
     *
     * @param IvwDeserializer & s optional reference to existing deserializer.
     * @param bool allowReference flag to manage references to avoid multiple object creation.
     */
    IvwDeserializer(IvwDeserializer& s, bool allowReference=true);
    /**
     * \brief Deserializer constructor
     *
     * @param std::string fileName path to file that is to be deserialized.
     * @param bool allowReference flag to manage references to avoid multiple object creation.
     */
    IvwDeserializer(std::string fileName, bool allowReference=true);
    /**
     * \brief Deserializes content from the stream using path to calculate relative paths to data.
     *
     * @param std::iostream& stream Stream with content that is to be deserialized.
     * @param std::string path A path that will be used to decode the location of data during deserialization.
     * @param bool allowReference flag to manage references to avoid multiple object creation.
     */
    IvwDeserializer(std::istream& stream, const std::string& path, bool allowReference=true);

    virtual ~IvwDeserializer();

    // std containers
    /**
     * \brief Deserialize a vector
     *
     * Deserialize the vector that has pre-allocated objects of type T or allocated by deserializer.
     * A vector is identified by key and vector items are identified by itemKey
     *
     * eg. xml tree with key=Properties and itemKey=Property
     *
     * <Properties>
     *      <Property identifier="enableMIP" displayName="MIP">
     *          <value content="0" />
     *      </Property>
     *      <Property identifier="enableShading" displayName="Shading">
     *          <value content="0" />
     *      </Property>
     * <Properties>
     *
     * @param const std::string & key vector key.
     * @param std::vector<T> & sVector vector to be deserialized.
     * @param const std::string & itemKey vector item key
     */
    template <typename T>
    void deserialize(const std::string& key,
                     std::vector<T*>& sVector,
                     const std::string& itemKey);
    template <typename T>
    void deserialize(const std::string& key,
                     std::vector<T>& sVector,
                     const std::string& itemKey);
    /**
     * \brief  Deserialize a map
     *
     * Deserialize a map, which can have
     * keys of type K,
     * values of type V* (pointers)
     * and compare function C ( optional if
     * K primitive type, i.e., std::string, int, etc.,)
     * eg., std::map<std::string, Property*>
     *
     * eg. xml tree
     *
     * <Properties>
     *      <Property identifier="enableMIP" displayName="MIP">
     *          <value content="0" />
     *      </Property>
     *      <Property identifier="enableShading" displayName="Shading">
     *          <value content="0" />
     *      </Property>
     * <Properties>
     *
     * In the above xml tree,
     *
     * key                   = "Properties"
     * itemKey               = "Property"
     * param comparisionAttribute  = "identifier"
     * param sMap["enableMIP"]     = address of a property
     *         sMap["enableShading"] = address of a property
     *         where, "enableMIP" & "enableShading" are keys.
     *         address of a property is a value
     *
     * Note: If children has attribute "type", then comparisionAttribute becomes meaningless.
     *       Because deserializer always allocates a new instance of type using registered factories.
     *
     *       eg., <Processor type="EntryExitPoints" identifier="EntryExitPoints" reference="ref2" />
     *
     * @param const std::string & key Map key or parent node of itemKey.
     * @param std::map<K
     * @param V *
     * @param C> & sMap  map to be deserialized - source / input map.
     * @param const std::string & itemKey map item key of childeren nodes.
     * @param const std::string & comparisionAttribute  - forced comparison attribute.
     */
    template <typename K, typename V, typename C, typename A>
    void deserialize(const std::string& key,
                     std::map<K,V,C,A>& sMap,
                     const std::string& itemKey,
                     const std::string& comparisionAttribute = IvwSerializeConstants::KEY_ATTRIBUTE);

    /**
     * \brief Deserialize string data.
     *
     * @param key Parent node key e.g, "Property"
     * @param data string data to be deserialized
     * @param asAttribute if attribute is true the xml node is formatted as <Key data="this is an attribute"\>,
     * otherwise <Key> <data="this is non-attribute"> <Key\>
     */
    void deserialize(const std::string& key,
                     std::string& data,
                     const bool asAttribute=false);

    // primitive types
    void deserialize(const std::string& key, bool& data);
    void deserialize(const std::string& key, float& data);
    void deserialize(const std::string& key, double& data);
    void deserialize(const std::string& key, signed char& data);
    void deserialize(const std::string& key, unsigned char& data);
    void deserialize(const std::string& key, char& data);
    void deserialize(const std::string& key, short& data);
    void deserialize(const std::string& key, unsigned short& data);
    void deserialize(const std::string& key, int& data);
    void deserialize(const std::string& key, unsigned int& data);
    void deserialize(const std::string& key, long& data);
    void deserialize(const std::string& key, long long& data);
    void deserialize(const std::string& key, unsigned long long& data);

    // glm vector types
    template<class T>
    void deserialize(const std::string& key, glm::detail::tvec4<T, glm::defaultp>& data);
    template<class T>
    void deserialize(const std::string& key, glm::detail::tvec3<T, glm::defaultp>& data);
    template<class T>
    void deserialize(const std::string& key, glm::detail::tvec2<T, glm::defaultp>& data);
    // glm matrix types
    template<class T>
    void deserialize(const std::string& key, glm::detail::tmat4x4<T, glm::defaultp>& data);
    template<class T>
    void deserialize(const std::string& key, glm::detail::tmat3x3<T, glm::defaultp>& data);
    template<class T>
    void deserialize(const std::string& key, glm::detail::tmat2x2<T, glm::defaultp>& data);

    /**
     * \brief  Deserialize any Serializable object
     */
    void deserialize(const std::string& key, IvwSerializable& sObj);
    /**
     * \brief  Deserialize pointer data of type T, which is of type
     *         serializeble object or primitive data
     */
    template <class T>
    void deserialize(const std::string& key, T*& data);

    void convertVersion(VersionConverter* converter);

protected:
    friend class NodeSwitch;
    /** 
     * \brief Read xml data and set the root element.
     */
    virtual void readXMLData();
private:

    /**
     * \brief Deserialize primitive string data type which is not an attribute
     *        that is formatted as <Key> <data="this is non-attribute"> <Key\>
     *
     * @param key  Parent node key e.g, "Property"
     * @param data Object to be deserialized
     */
    void deserializePrimitive(const std::string& key, std::string& data);

    /**
     * \brief Deserialize primitive data type which string data which is an
     *        attribute that is formatted as <Key data="this is an attribute"\>
     *
     * @param key Parent node key e.g, "Property"
     * @param data Object to be deserialized
     */
    void deserializeAttributes(const std::string& key, std::string& data);

    /**
     * \brief Deserialize primitive data type such as int, long, float, etc.,
     *        (except string data) which is not an attribute that is formatted
     *        as <Key> <stepValue=1.0> <Key\>
     *
     * @param key Parent node key e.g, "Property"
     * @param data Object to be deserialized of type int, long, float, etc.,
     *        (except string)
     */
    template <typename T>
    void deserializePrimitive(const std::string& key, T& data);

    /**
     * \brief Deserialize vector data structure vec2, ive2, vec3, ivec3, etc.,
     *
     * @param const std::string & key Parent node key e.g, "Property"
     * @param T & vector Glm data structures such as vec2, ive2, vec3, ivec3, etc.,
     */
    template <class T>
    void deserializeVector(const std::string& key,
                           T& vector);
};

template<class T>
void IvwDeserializer::deserialize(const std::string& key, glm::detail::tvec4<T, glm::defaultp>& data) {
    deserializeVector(key, data);
}
template<class T>
void IvwDeserializer::deserialize(const std::string& key, glm::detail::tvec3<T, glm::defaultp>& data) {
    deserializeVector(key, data);
}
template<class T>
void IvwDeserializer::deserialize(const std::string& key, glm::detail::tvec2<T, glm::defaultp>& data) {
    deserializeVector(key, data);
}

template<class T>
void IvwDeserializer::deserialize(const std::string& key, glm::detail::tmat4x4<T, glm::defaultp>& data) {
    try {
        TxElement* keyNode = rootElement_->FirstChildElement(key, false);
        if(!keyNode) return;
        NodeSwitch tempNodeSwitch(*this, keyNode);
        glm::detail::tvec4<T, glm::defaultp> rowVec;

        for (glm::length_t i = 0; i < 4; i++) {
            std::stringstream key;
            key << "row" << i;
            deserializeVector(key.str(), rowVec);
            data[i][0] = rowVec[0];
            data[i][1] = rowVec[1];
            data[i][2] = rowVec[2];
            data[i][3] = rowVec[3];
        }
    } catch (TxException&) {}
}
template<class T>
void IvwDeserializer::deserialize(const std::string& key, glm::detail::tmat3x3<T, glm::defaultp>& data) {
    try {
        TxElement* keyNode = rootElement_->FirstChildElement(key, false);
        if(!keyNode) return;
        NodeSwitch tempNodeSwitch(*this, keyNode);
        glm::detail::tvec3<T, glm::defaultp> rowVec;

        for (glm::length_t i = 0; i < 3; i++) {
            std::stringstream key;
            key << "row" << i;
            deserializeVector(key.str(), rowVec);
            data[i][0] = rowVec[0];
            data[i][1] = rowVec[1];
            data[i][2] = rowVec[2];
        }
    } catch (TxException&) {}
}
template<class T>
void IvwDeserializer::deserialize(const std::string& key, glm::detail::tmat2x2<T, glm::defaultp>& data) {
    try {
        TxElement* keyNode = rootElement_->FirstChildElement(key, false);
        if(!keyNode) return;
        NodeSwitch tempNodeSwitch(*this, keyNode);
        glm::detail::tvec2<T, glm::defaultp> rowVec;

        for (glm::length_t i = 0; i < 2; i++) {
            std::stringstream key;
            key << "row" << i;
            deserializeVector(key.str(), rowVec);
            data[i][0] = rowVec[0];
            data[i][1] = rowVec[1];
        }
    } catch (TxException&) {}
}

template <typename T>
void IvwDeserializer::deserialize(const std::string& key,
                                  std::vector<T*>& sVector,
                                  const std::string& itemKey) {
    try {
        TxElement* keyNode = rootElement_->FirstChildElement(key, false);
        if(!keyNode) return;
        NodeSwitch tempNodeSwitch(*this, keyNode);
        unsigned int i=0;
        TxEIt child(itemKey);

        for (child = child.begin(rootElement_); child != child.end(); ++child) {
            NodeSwitch tempNodeSwitch(*this, &(*child), false);

            if (sVector.size()<=i) {
                T* item = NULL;
                sVector.push_back(item);
            }

            deserialize(itemKey, sVector[i]);
            i++;
        }
    } catch (TxException&) {}
}
template <typename T>
void IvwDeserializer::deserialize(const std::string& key,
                                  std::vector<T>& sVector,
                                  const std::string& itemKey) {
    try {
        TxElement* keyNode = rootElement_->FirstChildElement(key, false);
        if(!keyNode) return;
        NodeSwitch tempNodeSwitch(*this, keyNode);
        unsigned int i = 0;
        TxEIt child(itemKey);

        for (child = child.begin(rootElement_); child != child.end(); ++child) {
            NodeSwitch tempNodeSwitch(*this, &(*child), false);

            if (sVector.size() <= i) {
                T item;
                sVector.push_back(item);
            }

            deserialize(itemKey, sVector[i]);
            i++;
        }
    } catch (TxException&) {}
}

template <typename K, typename V, typename C, typename A>
void IvwDeserializer::deserialize(const std::string& key,
                                  std::map<K, V, C, A>& map,
                                  const std::string& itemKey,
                                  const std::string& comparisionAttribute) {
    if (!isPrimitiveType(typeid(K)) || comparisionAttribute.empty())
        throw SerializationException("Error: map key has to be a primitive type");

    try {
        TxElement* keyNode = rootElement_->FirstChildElement(key, false);
        if(!keyNode) return;
        NodeSwitch tempNodeSwitch(*this, keyNode);
        TxEIt child(itemKey);

        for (child = child.begin(rootElement_); child != child.end(); ++child) {
            NodeSwitch tempNodeSwitch(*this, &(*child), false);
            K key;
            child->GetAttribute(comparisionAttribute, &key);
            V value;
            typename std::map<K, V, C, A>::iterator it = map.find(key);

            if (it != map.end())
                value = it->second;
            else
                value = NULL;

            deserialize(itemKey, value);
            map[key] = value;
        }
    } catch (TxException&) {}
}

template<class T>
inline void IvwDeserializer::deserialize(const std::string& key, T*& data) {
    TxElement* keyNode;

    if (retrieveChild_) {
        try {
            keyNode = rootElement_->FirstChildElement(key);
        } catch (TxException&) {
            return;
        }
    } else
        keyNode = rootElement_;

    std::string type_attr("");
    std::string ref_attr("");
    std::string id_attr("");
    keyNode->GetAttribute(IvwSerializeConstants::TYPE_ATTRIBUTE, &type_attr, false);

    if (allowRef_) {
        keyNode->GetAttribute(IvwSerializeConstants::REF_ATTRIBUTE, &ref_attr, false);
        keyNode->GetAttribute(IvwSerializeConstants::ID_ATTRIBUTE, &id_attr, false);
    }

    if (!data) {
        if (!ref_attr.empty()) {
            // Has reference identifier, data should already be allocated and we only have to find
            // and set the pointer to it.
            data=static_cast<T*>(refDataContainer_.find(type_attr, ref_attr));
            return;
        } else if (!type_attr.empty())
            data = IvwSerializeBase::getRegisteredType<T>(type_attr);
        else
            data = IvwSerializeBase::getNonRegisteredType<T>();
    }

    if (data) {
        deserialize(key, *data);

        if (!id_attr.empty())
            refDataContainer_.insert(data, keyNode);
    }
}

template<class T>
inline void IvwDeserializer::deserializePrimitive(const std::string& key, T& data) {
    try {
        if (retrieveChild_) {
            rootElement_->FirstChildElement(key)->GetAttribute(
                IvwSerializeConstants::CONTENT_ATTRIBUTE, &data);
        } else {
            rootElement_->GetAttribute(
                IvwSerializeConstants::CONTENT_ATTRIBUTE, &data);
        }
    } catch (TxException&) {
        try {
            rootElement_->FirstChildElement(key)->GetAttribute(
                IvwSerializeConstants::CONTENT_ATTRIBUTE, &data);
        } catch (TxException&) {}
    }
}

template<class T>
inline void IvwDeserializer::deserializeVector(const std::string& key, T& vector) {
    TxElement* keyNode = rootElement_->FirstChildElement(key, false);

    if (!keyNode) {
        //Try to finding key in the current node before exit. If not, let the exception be thrown.
        try {
            T tempVec;
            rootElement_->GetAttribute(IvwSerializeConstants::VECTOR_X_ATTRIBUTE, &tempVec[0]);
            keyNode = rootElement_;
        }
        catch (TxException&) {
            return;
        }
    }

    std::string attr;
    keyNode->GetAttribute(IvwSerializeConstants::VECTOR_X_ATTRIBUTE, &vector[0]);

    if (vector.length() >= 2) {
        keyNode->GetAttribute(IvwSerializeConstants::VECTOR_Y_ATTRIBUTE, &vector[1]);
    }

    if (vector.length() >= 3) {
        keyNode->GetAttribute(IvwSerializeConstants::VECTOR_Z_ATTRIBUTE, &vector[2]);
    }

    if (vector.length() >= 4) {
        keyNode->GetAttribute(IvwSerializeConstants::VECTOR_W_ATTRIBUTE, &vector[3]);
    }
}



} //namespace
#endif
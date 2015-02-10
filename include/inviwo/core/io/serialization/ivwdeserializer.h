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
#include <inviwo/core/io/serialization/deserializationerrorhandler.h>
#include <inviwo/core/io/serialization/nodedebugger.h>
#include <inviwo/core/util/stringconversion.h>

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

    void pushErrorHandler(BaseDeserializationErrorHandler*);
    BaseDeserializationErrorHandler* popErrorHandler();

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

    template <typename T, typename C>
    void deserialize(const std::string& key,
                     std::vector<T*>& sVector,
                     const std::string& itemKey,
                     C identifier);


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
    void deserializeVector(const std::string& key, T& vector);
    void storeReferences(TxElement* node);

    void handleError(SerializationException&);

    std::vector<BaseDeserializationErrorHandler*> errorHandlers_;
    std::map<std::string, TxElement*> referenceLookup_;
};

template <typename T>
class DeserializationErrorHandle {
public:
    typedef void (T::*Callback)(SerializationException&);
    DeserializationErrorHandle(IvwDeserializer&, std::string type, T* obj, Callback callback);
    virtual ~DeserializationErrorHandle();

private:
    IvwDeserializer& d_;
};

template <typename T>
inviwo::DeserializationErrorHandle<T>::DeserializationErrorHandle(IvwDeserializer& d,
                                                                  std::string type, T* obj,
                                                                  Callback callback)
    : d_(d) {
    d_.pushErrorHandler(new DeserializationErrorHandler<T>(type, obj, callback));
}

template <typename T>
inviwo::DeserializationErrorHandle<T>::~DeserializationErrorHandle() {
    delete d_.popErrorHandler();
}

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
        NodeSwitch ns(*this, key);
    
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
        NodeSwitch ns(*this, key);
        
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
        NodeSwitch ns(*this, key);
        
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
void IvwDeserializer::deserialize(const std::string& key, std::vector<T*>& vector,
                                  const std::string& itemKey) {
    try {
        NodeSwitch vectorNodeSwitch(*this, key);
        
        unsigned int i = 0;
        TxEIt child(itemKey);
        for (child = child.begin(rootElement_); child != child.end(); ++child) {
            // In the next deserialization call do net fetch the "child" since we are looping...
            // hence the "false" as the last arg.
            NodeSwitch elementNodeSwitch(*this, &(*child), false);
            try {
                if (vector.size() <= i) {
                    T* item = NULL;
                    deserialize(itemKey, item);
                    vector.push_back(item);
                } else {
                    deserialize(itemKey, vector[i]);
                }
            } catch (SerializationException& e) {
                handleError(e);
            }
            i++;
        }
    } catch (TxException&) {}
}


template <typename T, typename C>
void IvwDeserializer::deserialize(const std::string& key, std::vector<T*>& vector,
                                  const std::string& itemKey, C identifier) {
    try {
        NodeSwitch vectorNodeSwitch(*this, key);

        unsigned int i = 0;
        TxEIt child(itemKey);
        for (child = child.begin(rootElement_); child != child.end(); ++child) {

            identifier.setKey(child.Get());
            typename std::vector<T*>::iterator it =
                std::find_if(vector.begin(), vector.end(), identifier);

            try {
                if (it != vector.end()) {
                    NodeSwitch elementNodeSwitch(*this, &(*child), false);
                    deserialize(itemKey, *it);
                } else {
                    T* item = NULL;
                    NodeSwitch elementNodeSwitch(*this, &(*child), false);
                    deserialize(itemKey, item);
                    vector.push_back(item);
                }
            } catch (SerializationException& e) {
                handleError(e);
            }
        }
    } catch (TxException&) {
    }
}


template <typename T>
void IvwDeserializer::deserialize(const std::string& key, std::vector<T>& vector,
                                  const std::string& itemKey) {
    try {
        NodeSwitch vectorNodeSwitch(*this, key);
        unsigned int i = 0;
        TxEIt child(itemKey);

        for (child = child.begin(rootElement_); child != child.end(); ++child) {
            // In the next deserialization call do net fetch the "child" since we are looping...
            // hence the "false" as the last arg.
            NodeSwitch elementNodeSwitch(*this, &(*child), false);

            try {
                if (vector.size() <= i) {
                    T item;
                    deserialize(itemKey, item);
                    vector.push_back(item);
                } else {
                    deserialize(itemKey, vector[i]);
                }
            } catch (SerializationException& e) {
                handleError(e);
            }
            i++;
        }
    } catch (TxException&) {}
}

template <typename K, typename V, typename C, typename A>
void IvwDeserializer::deserialize(const std::string& key, std::map<K, V, C, A>& map,
                                  const std::string& itemKey,
                                  const std::string& comparisionAttribute) {
    if (!isPrimitiveType(typeid(K)) || comparisionAttribute.empty())
        throw SerializationException("Error: map key has to be a primitive type");

    try {
        NodeSwitch mapNodeSwitch(*this, key);
        TxEIt child(itemKey);

        for (child = child.begin(rootElement_); child != child.end(); ++child) {
            // In the next deserialization call do net fetch the "child" since we are looping...
            // hence the "false" as the last arg.
            NodeSwitch elementNodeSwitch(*this, &(*child), false);
            K key;
            child->GetAttribute(comparisionAttribute, &key);
            
            typename std::map<K, V, C, A>::iterator it = map.find(key);
            V value = (it != map.end() ? it->second : NULL);
 
            try {
                deserialize(itemKey, value);
                map[key] = value;
            } catch (SerializationException& e) {
                handleError(e);
            }
        }
    } catch (TxException&) {}
}

template <class T>
inline void IvwDeserializer::deserialize(const std::string& key, T*& data) {
    TxElement* keyNode = retrieveChild_?rootElement_->FirstChildElement(key,false):rootElement_;
    if(!keyNode)return;
        
    const std::string type_attr(keyNode->GetAttribute(IvwSerializeConstants::TYPE_ATTRIBUTE));
    const std::string ref_attr(keyNode->GetAttribute(IvwSerializeConstants::REF_ATTRIBUTE));
    const std::string id_attr(keyNode->GetAttribute(IvwSerializeConstants::ID_ATTRIBUTE));

    if (!data) {
        if (allowRef_ && !ref_attr.empty()) {
            // Has reference identifier, data should already be allocated and we only have to find
            // and set the pointer to it.
            data = static_cast<T*>(refDataContainer_.find(type_attr, ref_attr));
            if (!data) {
                std::map<std::string, TxElement*>::iterator it = referenceLookup_.find(ref_attr);
                if (it != referenceLookup_.end()) {
                    NodeDebugger error(keyNode);
                    throw SerializationException(
                        "Reference to " + error[0].key + " not instantiated: \"" +
                            error[0].identifier + "\" of class \"" + error[0].type + "\" at line " +
                            toString(error[0].line),
                        error[0].key, error[0].type, error[0].identifier, it->second);
                } else {
                    throw SerializationException(
                        "Could not find reference to " + key + ": " + type_attr, key, type_attr);
                }
            }
            return;
            
        } else if (!type_attr.empty()) {
            data = IvwSerializeBase::getRegisteredType<T>(type_attr);
            if (!data) {
                NodeDebugger error(keyNode);
                throw SerializationException(
                    "Could not create " + key + ": \"" + error[0].identifier + "\" of class \"" +
                        error[0].type + "\" at line: " + toString(error[0].line),
                    key, type_attr, error[0].identifier, keyNode);
            }

        } else {
            data = IvwSerializeBase::getNonRegisteredType<T>();
            if (!data) {
                NodeDebugger error(keyNode);
                throw SerializationException(
                    "Could not create " + key + ": \"" + error[0].identifier + "\" of class \"" +
                        error[0].type + "\" at line: " + toString(error[0].line),
                    key, type_attr, error[0].identifier, keyNode);
            }
        }
    }

    if (data) {
        deserialize(key, *data);
        if (allowRef_ && !id_attr.empty()) refDataContainer_.insert(data, keyNode);
    }
}

template <class T>
inline void IvwDeserializer::deserializePrimitive(const std::string& key, T& data) {
    try {
        NodeSwitch ns(*this, key);
        rootElement_->GetAttribute(IvwSerializeConstants::CONTENT_ATTRIBUTE, &data);
    } catch (TxException&) {
        try {
            NodeSwitch ns(*this, key, true);
            rootElement_->GetAttribute(IvwSerializeConstants::CONTENT_ATTRIBUTE, &data);
        } catch (TxException&) {
        }
    }
}

template <class T>
inline void IvwDeserializer::deserializeVector(const std::string& key, T& vector) {
    try {
        NodeSwitch ns(*this, key);
        switch (vector.length()) {
            case 4:
                rootElement_->GetAttribute(IvwSerializeConstants::VECTOR_W_ATTRIBUTE, &vector[3]);
            case 3:
                rootElement_->GetAttribute(IvwSerializeConstants::VECTOR_Z_ATTRIBUTE, &vector[2]);
            case 2:
                rootElement_->GetAttribute(IvwSerializeConstants::VECTOR_Y_ATTRIBUTE, &vector[1]);
            case 1:
                rootElement_->GetAttribute(IvwSerializeConstants::VECTOR_X_ATTRIBUTE, &vector[0]);
            default: break;
        }
    } catch (TxException&) {
    }
}



} //namespace
#endif
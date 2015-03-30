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

#ifndef IVW_SERIALIZER_H
#define IVW_SERIALIZER_H

#include <inviwo/core/io/serialization/ivwserializebase.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/io/serialization/serializationexception.h>
#include <type_traits>
#include <list>
namespace inviwo {

class IvwSerializable;

class IVW_CORE_API IvwSerializer : public IvwSerializeBase {
public:
    /**
     * Copies parameters from other serializer.
     *
     * @param IvwSerializeBase & s object of similar type.
     * @param bool allowReference disables or enables reference management schemes.
     * @throws SerializationException
     */
    IvwSerializer(IvwSerializer& s, bool allowReference = true);
    /**
     * \brief Initializes serializer with a file name that will be used to set relative paths to
     *data.
     * The specified file name will not be used to write any content until writeFile() is called.
     *
     * @param std::string fileName full path to xml file.
     * @param bool allowReference disables or enables reference management schemes.
     * @throws SerializationException
     */
    IvwSerializer(const std::string& fileName, bool allowReference = true);

    virtual ~IvwSerializer();

    /**
     * \brief Writes serialized data to the file specified by the currently set file name.
     *
     * @note File name needs to be set before calling this method.
     * @throws SerializationException
     */
    virtual void writeFile();
    /**
     * \brief Writes serialized data to stream.
     *
     * @param std::ostream & stream Stream to be written to.
     * @throws SerializationException
     */
    virtual void writeFile(std::ostream& stream);

    // std containers
    template <typename T>
    void serialize(const std::string& key, const std::vector<T>& sVector,
                   const std::string& itemKey);

    template <typename T>
    void serialize(const std::string& key, const std::list<T>& container,
                   const std::string& itemKey);

    template <typename K, typename V, typename C, typename A>
    void serialize(const std::string& key, const std::map<K, V, C, A>& sMap,
                   const std::string& itemKey);

    // Specializations for chars
    void serialize(const std::string& key, const signed char& data, const bool asAttribute = false);
    void serialize(const std::string& key, const char& data, const bool asAttribute = false);
    void serialize(const std::string& key, const unsigned char& data,
                   const bool asAttribute = false);

    // integers, reals, strings
    template <typename T, typename std::enable_if<std::is_integral<T>::value ||
                                                      std::is_floating_point<T>::value ||
                                                      util::is_string<T>::value,
                                                  int>::type = 0>
    void serialize(const std::string& key, const T& data, const bool asAttribute = false);

    // Enum types
    template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
    void serialize(const std::string& key, const T& data, const bool asAttribute = false);

    // glm vector types
    template <typename Vec, typename std::enable_if<util::rank<Vec>::value == 1, int>::type = 0>
    void serialize(const std::string& key, const Vec& data);

    // glm matrix types
    template <typename Mat, typename std::enable_if<util::rank<Mat>::value == 2, int>::type = 0>
    void serialize(const std::string& key, const Mat& data);

    // serializable classes
    void serialize(const std::string& key, const IvwSerializable& sObj);

    // pointers to something of the above.
    template <class T>
    void serialize(const std::string& key, const T* const& data);

protected:
    friend class NodeSwitch;

private:
    template <class T>
    void serializeVector(const std::string& key, const T& vector);

    /**
     * \brief Creates xml documents and initializes factories. Does not open files or streams.
     *
     * @throws SerializationException
     */
    void initialize();
};

template <typename T>
void IvwSerializer::serialize(const std::string& key, const std::vector<T>& vector,
                              const std::string& itemKey) {
    if (vector.empty()) return;

    auto node = util::make_unique<TxElement>(key);
    rootElement_->LinkEndChild(node.get());
    NodeSwitch nodeSwitch(*this, node.get());

    for (typename std::vector<T>::const_iterator it = vector.begin(); it != vector.end(); ++it)
        serialize(itemKey, (*it));
}

template <typename T>
void IvwSerializer::serialize(const std::string& key, const std::list<T>& container,
                              const std::string& itemKey) {
    if (container.empty()) return;

    auto node = util::make_unique<TxElement>(key);
    rootElement_->LinkEndChild(node.get());

    NodeSwitch nodeSwitch(*this, node.get());
    for (typename std::list<T>::const_iterator it = container.begin(); it != container.end(); ++it)
        serialize(itemKey, (*it));
}

template <typename K, typename V, typename C, typename A>
void IvwSerializer::serialize(const std::string& key, const std::map<K, V, C, A>& map,
                              const std::string& itemKey) {
    if (!isPrimitiveType(typeid(K)))
        throw SerializationException("Error: map key has to be a primitive type");

    if (map.empty()) return;

    auto node = util::make_unique<TxElement>(key);
    rootElement_->LinkEndChild(node.get());
    NodeSwitch nodeSwitch(*this, node.get());

    for (typename std::map<K, V, C, A>::const_iterator it = map.begin(); it != map.end(); ++it) {
        serialize(itemKey, it->second);
        rootElement_->LastChild()->ToElement()->SetAttribute(IvwSerializeConstants::KEY_ATTRIBUTE,
                                                             it->first);
    }
}

template <class T>
inline void IvwSerializer::serialize(const std::string& key, const T* const& data) {
    if (!allowRef_)
        serialize(key, *data);
    else {
        if (refDataContainer_.find(data)) {
            TxElement* newNode = refDataContainer_.nodeCopy(data);
            newNode->SetValue(key);
            rootElement_->LinkEndChild(newNode);
            refDataContainer_.insert(data, newNode);
        } else {
            serialize(key, *data);
            refDataContainer_.insert(data, rootElement_->LastChild()->ToElement(), false);
        }
    }
}

// integers, reals, strings
template <typename T,
          typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value ||
                                      util::is_string<T>::value,
                                  int>::type>
void IvwSerializer::serialize(const std::string& key, const T& data, const bool asAttribute) {
    if (asAttribute) {
        rootElement_->SetAttribute(key, data);
    } else {
        auto node = util::make_unique<TxElement>(key);
        rootElement_->LinkEndChild(node.get());
        node->SetAttribute(IvwSerializeConstants::CONTENT_ATTRIBUTE, data);
    }
}

// enum types
template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type>
void IvwSerializer::serialize(const std::string& key, const T& data, const bool asAttribute) {
    using ET = typename std::underlying_type<T>::type;
    const ET tmpdata{static_cast<const ET>(data)};
    serialize(key, tmpdata, asAttribute);
}

// glm vector types
template <typename Vec, typename std::enable_if<util::rank<Vec>::value == 1, int>::type>
void IvwSerializer::serialize(const std::string& key, const Vec& data) {
    auto node = util::make_unique<TxElement>(key);
    rootElement_->LinkEndChild(node.get());
    for (size_t i = 0; i < util::extent<Vec, 0>::value; ++i) {
        node->SetAttribute(IvwSerializeConstants::VECTOR_ATTRIBUTES[i], data[i]);
    }
}

// glm matrix types
template <typename Mat, typename std::enable_if<util::rank<Mat>::value == 2, int>::type>
void IvwSerializer::serialize(const std::string& key, const Mat& data) {
    auto node = util::make_unique<TxElement>(key);
    rootElement_->LinkEndChild(node.get());

    NodeSwitch nodeSwitch(*this, node.get());
    for (size_t i = 0; i < util::extent<Mat, 0>::value; ++i) {
        serialize("row" + toString(i), data[i]);
    }
}

}  // namespace
#endif

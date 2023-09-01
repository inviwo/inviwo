/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#include <inviwo/core/io/serialization/serializebase.h>
#include <inviwo/core/io/serialization/ticpp.h>
#include <inviwo/core/io/serialization/serializationexception.h>
#include <inviwo/core/io/serialization/serializeconstants.h>

#include <charconv>
#include <sstream>

namespace inviwo {

namespace config {
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
constexpr bool charconv = true;
#else
constexpr bool charconv = false;
#endif
}  // namespace config

SerializeBase::SerializeBase()
    : doc_{std::make_unique<TxDocument>()}, rootElement_{nullptr}, retrieveChild_{true} {}

SerializeBase::SerializeBase(const std::filesystem::path& fileName)
    : fileName_{fileName}
    , doc_{std::make_unique<TxDocument>(fileName.string())}
    , rootElement_{nullptr}
    , retrieveChild_{true} {}

SerializeBase::SerializeBase(std::istream& stream, const std::filesystem::path& path)
    : fileName_{path}
    , doc_{std::make_unique<TxDocument>()}
    , rootElement_{nullptr}
    , retrieveChild_{true} {
    stream >> *doc_;
}

SerializeBase::~SerializeBase() = default;
SerializeBase::SerializeBase(SerializeBase&&) noexcept = default;
SerializeBase& SerializeBase::operator=(SerializeBase&&) noexcept = default;

const std::filesystem::path& SerializeBase::getFileName() const { return fileName_; }

std::string SerializeBase::nodeToString(const TxElement& node) {
    try {
        TiXmlPrinter printer;
        printer.SetIndent("    ");
        node.Accept(&printer);
        return printer.Str();
    } catch (TxException&) {
        return "No valid root node";
    }
}

NodeSwitch::NodeSwitch(NodeSwitch&& rhs) noexcept
    : node_{std::move(rhs.node_)}
    , serializer_{rhs.serializer_}
    , storedNode_{nullptr}
    , storedRetrieveChild_{rhs.storedRetrieveChild_} {

    std::swap(storedNode_, rhs.storedNode_);
}

NodeSwitch& NodeSwitch::operator=(NodeSwitch&& rhs) noexcept {
    if (this != &rhs) {
        if (storedNode_) {
            serializer_->rootElement_ = storedNode_;
            serializer_->retrieveChild_ = storedRetrieveChild_;
        }
        node_ = std::move(rhs.node_);
        serializer_ = rhs.serializer_;
        storedNode_ = rhs.storedNode_;
        storedRetrieveChild_ = rhs.storedRetrieveChild_;

        rhs.storedNode_ = nullptr;
    }
    return *this;
}

NodeSwitch::NodeSwitch(SerializeBase& serializer, TxElement* node, bool retrieveChild)
    : serializer_(&serializer)
    , storedNode_(serializer_->rootElement_)
    , storedRetrieveChild_(serializer_->retrieveChild_) {

    serializer_->rootElement_ = node;
    serializer_->retrieveChild_ = retrieveChild;
}

NodeSwitch::NodeSwitch(SerializeBase& serializer, std::unique_ptr<TxElement> node,
                       bool retrieveChild)
    : node_(std::move(node))
    , serializer_(&serializer)
    , storedNode_(serializer_->rootElement_)
    , storedRetrieveChild_(serializer_->retrieveChild_) {

    serializer_->rootElement_ = node_.get();
    serializer_->retrieveChild_ = retrieveChild;
}
NodeSwitch::NodeSwitch(SerializeBase& serializer, std::string_view key, bool retrieveChild)
    : serializer_(&serializer)
    , storedNode_(serializer_->rootElement_)
    , storedRetrieveChild_(serializer_->retrieveChild_) {

    serializer_->rootElement_ =
        serializer_->retrieveChild_
            ? serializer_->rootElement_->FirstChildElement(key.data(), false)
            : serializer_->rootElement_;

    serializer_->retrieveChild_ = retrieveChild;
}

NodeSwitch::~NodeSwitch() {
    if (storedNode_) {
        serializer_->rootElement_ = storedNode_;
        serializer_->retrieveChild_ = storedRetrieveChild_;
    }
}

NodeSwitch::operator bool() const { return serializer_->rootElement_ != nullptr; }

std::string detail::getNodeAttributeOrDefault(TxElement* node, const std::string& key,
                                              const std::string& defaultValue) {
    return node->GetAttributeOrDefault(key, defaultValue);
}

namespace {

template <class T>
void fromStrInternal(std::string_view value, T& dest) {
    if constexpr (config::charconv && (std::is_same_v<double, T> || std::is_same_v<float, T> ||
                                       (!std::is_same_v<bool, T> && std::is_integral_v<T>))) {
        const auto end = value.data() + value.size();
        if (auto [p, ec] = std::from_chars(value.data(), end, dest);
            ec != std::errc() || p != end) {
            throw SerializationException("Error parsing number", IVW_CONTEXT_CUSTOM("fromStr"));
        }
    } else {
        std::istringstream stream{std::string{value}};
        stream >> dest;
    }
}

}  // namespace

void detail::numericalFromStr(std::string_view value, double& dest) {
    fromStrInternal(value, dest);
}
void detail::numericalFromStr(std::string_view value, float& dest) { fromStrInternal(value, dest); }
void detail::numericalFromStr(std::string_view value, char& dest) { fromStrInternal(value, dest); }
void detail::numericalFromStr(std::string_view value, signed char& dest) {
    fromStrInternal(value, dest);
}
void detail::numericalFromStr(std::string_view value, unsigned char& dest) {
    fromStrInternal(value, dest);
}
void detail::numericalFromStr(std::string_view value, short& dest) { fromStrInternal(value, dest); }
void detail::numericalFromStr(std::string_view value, unsigned short& dest) {
    fromStrInternal(value, dest);
}
void detail::numericalFromStr(std::string_view value, int& dest) { fromStrInternal(value, dest); }
void detail::numericalFromStr(std::string_view value, unsigned int& dest) {
    fromStrInternal(value, dest);
}
void detail::numericalFromStr(std::string_view value, long& dest) { fromStrInternal(value, dest); }
void detail::numericalFromStr(std::string_view value, unsigned long& dest) {
    fromStrInternal(value, dest);
}
void detail::numericalFromStr(std::string_view value, long long& dest) {
    fromStrInternal(value, dest);
}
void detail::numericalFromStr(std::string_view value, unsigned long long& dest) {
    fromStrInternal(value, dest);
}

}  // namespace inviwo

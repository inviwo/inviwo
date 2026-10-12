/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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
#include <inviwo/core/io/serialization/nodedebugger.h>
#include <inviwo/core/util/charconv.h>

#include <fmt/format.h>
#include <fmt/std.h>

namespace inviwo {

SerializeBase::SerializeBase(const std::filesystem::path& fileName, allocator_type alloc)
    : fileName_{fileName}
    , fileDir_{fileName.parent_path()}
    , doc_{std::make_unique<TiXmlDocument>(fileName.string(), alloc)}
    , rootElement_{nullptr}
    , retrieveChild_{true} {}

SerializeBase::~SerializeBase() = default;
SerializeBase::SerializeBase(SerializeBase&&) noexcept = default;
SerializeBase& SerializeBase::operator=(SerializeBase&&) noexcept = default;

auto SerializeBase::getAllocator() const -> allocator_type { return doc_->getAllocator(); }

TiXmlDocument& SerializeBase::doc() { return *doc_; }

NodeSwitch::NodeSwitch(NodeSwitch&& rhs) noexcept
    : serializer_{rhs.serializer_}
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
        serializer_ = rhs.serializer_;
        storedNode_ = rhs.storedNode_;
        storedRetrieveChild_ = rhs.storedRetrieveChild_;

        rhs.storedNode_ = nullptr;
    }
    return *this;
}

NodeSwitch::NodeSwitch(SerializeBase& serializer, TiXmlElement* node, bool retrieveChild)
    : serializer_(&serializer)
    , storedNode_(serializer_->rootElement_)
    , storedRetrieveChild_(serializer_->retrieveChild_) {

    serializer_->rootElement_ = node;
    serializer_->retrieveChild_ = retrieveChild;
}

NodeSwitch::NodeSwitch(SerializeBase& serializer, TiXmlElement& node, bool retrieveChild)
    : serializer_(&serializer)
    , storedNode_(serializer_->rootElement_)
    , storedRetrieveChild_(serializer_->retrieveChild_) {

    serializer_->rootElement_ = &node;
    serializer_->retrieveChild_ = retrieveChild;
}

NodeSwitch::NodeSwitch(SerializeBase& serializer, std::string_view key, bool retrieveChild)
    : serializer_(&serializer)
    , storedNode_(serializer_->rootElement_)
    , storedRetrieveChild_(serializer_->retrieveChild_) {

    if (serializer_->retrieveChild_) {
        serializer_->rootElement_ = serializer_->rootElement_->FirstChildElement(key);
    }

    serializer_->retrieveChild_ = retrieveChild;
}

NodeSwitch::~NodeSwitch() {
    if (storedNode_) {
        serializer_->rootElement_ = storedNode_;
        serializer_->retrieveChild_ = storedRetrieveChild_;
    }
}

NodeSwitch::operator bool() const { return serializer_->rootElement_ != nullptr; }

namespace serialization {

// NOLINTBEGIN(google-runtime-int)
void fromStr(std::string_view value, double& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, float& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, char& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, signed char& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, unsigned char& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, short& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, unsigned short& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, int& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, unsigned int& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, long& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, unsigned long& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, long long& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, unsigned long long& dest) { util::fromStr(value, dest); }
void fromStr(std::string_view value, bool& dest) {
    static constexpr std::string_view trueVal = "1";
    static constexpr std::string_view falseVal = "0";
    if (value == trueVal) {
        dest = true;
    } else if (value == falseVal) {
        dest = false;
    } else {
        throw SerializationException(SourceContext{}, "Error parsing boolean value ({})", value);
    }
}

void formatTo(double value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}
void formatTo(float value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}
void formatTo(char value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}
void formatTo(signed char value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}
void formatTo(unsigned char value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}
void formatTo(short value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}
void formatTo(unsigned short value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}
void formatTo(int value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}
void formatTo(unsigned int value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}
void formatTo(long value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}
void formatTo(unsigned long value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}
void formatTo(long long value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}
void formatTo(unsigned long long value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}

void formatTo(const std::filesystem::path& value, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{}", value);
}

void formatToBinary(unsigned long long value, size_t bits, std::pmr::string& out) {
    fmt::format_to(std::back_inserter(out), "{:0{}b}", value, bits);
}
// NOLINTEND(google-runtime-int)

std::pmr::string& addAttribute(TiXmlElement* node, std::string_view key) {
    return node->AddAttribute(key);
}

std::optional<std::string_view> attribute(const TiXmlElement* node, std::string_view key) {
    return node->Attribute(key);
}

std::string_view attributeOrThrow(const TiXmlElement* node, std::string_view key) {
    const auto attr = serialization::attribute(node, key);
    if (!attr) {
        throw SerializationException{SourceContext{}, serialization::getStack(node), "Missing '{}'",
                                     key};
    }
    return *attr;
}

std::string_view getAttribute(const TiXmlElement& node, std::string_view key) {
    static const std::string empty;

    if (auto str = node.Attribute(key)) {
        return *str;
    } else {
        return empty;
    }
}

TiXmlElement* firstChild(TiXmlElement* parent, std::string_view key) {
    return parent->FirstChildElement(key);
}
TiXmlElement* nextChild(TiXmlElement* child, std::string_view key) {
    return child->NextSiblingElement(key);
}

}  // namespace serialization

}  // namespace inviwo

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

#include <inviwo/core/ports/port.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/io/serialization/serialization.h>

#include <fmt/format.h>

namespace inviwo {

Port::Port(std::string_view identifier, Document help)
    : identifier_(identifier), processor_(nullptr), help_{std::move(help)} {}

Processor* Port::getProcessor() const { return processor_; }

std::string Port::getPath() const {
    if (!processor_) return fmt::format("<not in any processor>.{}", identifier_);
    return fmt::format("{}.{}", processor_->getIdentifier(), identifier_);
}

void Port::getPath(std::pmr::string& out) const {
    if (!processor_) {
        out.append("<not in any processor>.");
        out.append(identifier_);
    } else {
        out.append(processor_->getIdentifier());
        out.push_back('.');
        out.append(identifier_);
    }
}

const std::string& Port::getIdentifier() const { return identifier_; }

void Port::setIdentifier(const std::string& name) { identifier_ = name; }

void Port::setProcessor(Processor* processor) { processor_ = processor; }

const Document& Port::getHelp() const { return help_; }
Document& Port::getHelp() { return help_; }
Port& Port::setHelp(Document help) {
    help_ = std::move(help);
    return *this;
}

void Port::serialize(Serializer& s) const {
    s.serialize("type", getClassIdentifier(), SerializationTarget::Attribute);
    s.serialize("identifier", identifier_, SerializationTarget::Attribute);
}

void Port::deserialize(Deserializer& d) {
    d.deserialize("identifier", identifier_, SerializationTarget::Attribute);
}

}  // namespace inviwo

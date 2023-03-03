/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2023 Inviwo Foundation
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

#include <inviwo/core/util/docbuilder.h>
#include <fmt/format.h>

namespace inviwo {

namespace help {

void HelpInport::serialize(Serializer& s) const {
    s.serialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    s.serialize("displayName", displayName, SerializationTarget::Attribute);
    s.serialize("help", help);
}
void HelpInport::deserialize(Deserializer& d) {
    d.deserialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    d.deserialize("displayName", displayName, SerializationTarget::Attribute);
    d.deserialize("help", help);
}

void HelpOutport::serialize(Serializer& s) const {
    s.serialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    s.serialize("displayName", displayName, SerializationTarget::Attribute);
    s.serialize("help", help);
}
void HelpOutport::deserialize(Deserializer& d) {
    d.deserialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    d.deserialize("displayName", displayName, SerializationTarget::Attribute);
    d.deserialize("help", help);
}

void HelpProperty::serialize(Serializer& s) const {
    s.serialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    s.serialize("displayName", displayName, SerializationTarget::Attribute);
    s.serialize("help", help);
    s.serialize("properties", properties);
}
void HelpProperty::deserialize(Deserializer& d) {
    d.deserialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    d.deserialize("displayName", displayName, SerializationTarget::Attribute);
    d.deserialize("help", help);
    d.deserialize("properties", properties);
}

void HelpProcessor::serialize(Serializer& s) const {
    s.serialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    s.serialize("displayName", displayName, SerializationTarget::Attribute);
    s.serialize("help", help);
    s.serialize("inports", inports);
    s.serialize("outports", outports);
    s.serialize("properties", properties);
}
void HelpProcessor::deserialize(Deserializer& d) {
    d.deserialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    d.deserialize("displayName", displayName, SerializationTarget::Attribute);
    d.deserialize("help", help);
    d.deserialize("inports", inports);
    d.deserialize("outports", outports);
    d.deserialize("properties", properties);
}

}  // namespace help

help::HelpProcessor help::buildProcessorHelp(Processor& processor) {

    std::vector<HelpInport> inports;
    for (auto inport : processor.getInports()) {
        inports.push_back(
            HelpInport{inport->getClassIdentifier(), inport->getIdentifier(), inport->getHelp()});
    }

    std::vector<HelpOutport> outports;
    for (auto outport : processor.getOutports()) {
        outports.push_back(HelpOutport{outport->getClassIdentifier(), outport->getIdentifier(),
                                       outport->getHelp()});
    }

    std::vector<HelpProperty> properties;
    std::vector<std::vector<HelpProperty>*> stack{&properties};

    LambdaNetworkVisitor visitor{
        [&](Property& property) {
            stack.back()->push_back(HelpProperty{
                property.getClassIdentifier(), property.getDisplayName(), property.getHelp(), {}});
        },
        [&](CompositeProperty& property, NetworkVisitorEnter) {
            stack.back()->push_back(HelpProperty{
                property.getClassIdentifier(), property.getDisplayName(), property.getHelp(), {}});
            stack.push_back(&stack.back()->back().properties);

            return true;
        },
        [&](CompositeProperty&, NetworkVisitorExit) { stack.pop_back(); }};

    processor.accept(visitor);

    return HelpProcessor{processor.getClassIdentifier(),
                         processor.getDisplayName(),
                         processor.getProcessorInfo().help,
                         std::move(inports),
                         std::move(outports),
                         std::move(properties)};
}

}  // namespace inviwo

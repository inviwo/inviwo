/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {
VersionConverter::VersionConverter() {}

bool util::xmlCopyMatchingSubPropsIntoComposite(TxElement* node, const CompositeProperty& prop) {
    TxElement propitem("Property");
    propitem.SetAttribute("type", prop.getClassIdentifier());
    propitem.SetAttribute("identifier", prop.getIdentifier());
    propitem.SetAttribute("displayName", prop.getDisplayName());
    propitem.SetAttribute("key", prop.getIdentifier());
    TxElement list("Properties");

    propitem.InsertEndChild(list);
    node->InsertEndChild(propitem);

    std::vector<Property*> props = prop.getProperties();

    bool res = false;

    for (auto& p : props) {
        bool match = false;

        ticpp::Iterator<ticpp::Element> child;
        for (child = child.begin(node); child != child.end(); child++) {
            std::string name;
            child->GetValue(&name);
            std::string type = child->GetAttributeOrDefault("type", "");
            std::string id = child->GetAttributeOrDefault("identifier", "");

            if (p->getIdentifier() == id &&
                (p->getClassIdentifier() == type ||
                 p->getClassIdentifier() == splitString(type, '.').back())) {
                LogInfoCustom("VersionConverter", "    Match for sub property: " +
                                                          joinString(p->getPath(), ".") +
                                                          " found in type: "
                                                      << type << " id: " << id);

                list.InsertEndChild(*(child.Get()));
                match = true;
            }
        }
        res = res && match;
    }
    return res;
}

bool util::xmlHasProp(TxElement* node, const Property& prop) {
    bool result = false;
    ticpp::Iterator<ticpp::Element> child;
    for (child = child.begin(node); child != child.end(); child++) {
        if (prop.getClassIdentifier() == child->GetAttributeOrDefault("type", "")
            && prop.getIdentifier() == child->GetAttributeOrDefault("identifier", "")) {
            result = true;
        }
    }
    return result;
}

std::vector<TxElement*> util::xmlGetMatchingElements(TxElement* node, std::string key) {
    std::vector<TxElement*> res;
    ticpp::Iterator<ticpp::Element> child;
    for (child = child.begin(node); child != child.end(); child++) {
        std::string childkey;
        child->GetValue(&childkey);

        if(childkey == key){
            res.push_back(child.Get());
        }
    }
    return res;
}

bool util::xmlFindMatchingSubPropertiesForComposites(
    TxElement* processornode, const std::vector<const CompositeProperty*>& props) {
    std::vector<TxElement*> pelm = util::xmlGetMatchingElements(processornode, "Properties");

    bool res = true;

    for (auto& prop : props) {
        if (!util::xmlHasProp(pelm[0], *prop)) {

            bool foundMatchingComposite = util::xmlCopyMatchingCompositeProperty(pelm[0], *prop);
            bool foundSubProp = false;
            if (!foundMatchingComposite) {
                foundSubProp = util::xmlCopyMatchingSubPropsIntoComposite(pelm[0], *prop);
            }
            res = res && (foundSubProp || foundMatchingComposite);
        }
    }
    return res;
}

TxElement* util::xmlGetElement(TxElement* node, std::string path) {
    std::vector<std::string> parts = splitString(path, '/');
    if (parts.size() > 0) {
        std::vector<std::string> components = splitString(parts[0], '&');
        std::string name = components[0];

        ticpp::Iterator<ticpp::Element> child;
        for (child = child.begin(node); child != child.end(); child++) {
            bool match = true;
            std::string childname;
            child->GetValue(&childname);
            if (childname == name) {
                for (size_t i = 1; i < components.size(); ++i) {
                    std::vector<std::string> pair = splitString(components[i], '=');
                    match = match && child->GetAttributeOrDefault(pair[0], "") == pair[1];
                }
            } else {
                match = false;
            }
            if (match) {
                if (parts.size() > 1) {
                    return xmlGetElement(child.Get(),
                                         joinString(parts.begin() + 1, parts.end(), "/"));
                } else {
                    return child.Get();
                }
                break;
            }
        }
    }
    return nullptr;
}

bool util::xmlCopyMatchingCompositeProperty(TxElement* node, const CompositeProperty& prop) {
    ticpp::Iterator<ticpp::Element> child;
    for (child = child.begin(node); child != child.end(); child++) {
        std::string name;
        child->GetValue(&name);
        std::string type = child->GetAttributeOrDefault("type", "");
        std::string id = child->GetAttributeOrDefault("identifier", "");

        if ((type == "CompositeProperty" || type == "org.inviwo.CompositeProperty") && prop.getIdentifier() == id) {
            LogInfoCustom("VersionConverter", "    Found Composite with same identifier");

            TxElement* newChild = node->InsertEndChild(*(child.Get()))->ToElement();
            newChild->SetAttribute("type", prop.getClassIdentifier());
            return true;
        }
    }

    return false;
}

}  // namespace

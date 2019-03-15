/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>

namespace inviwo {
VersionConverter::VersionConverter() {}

NodeVersionConverter::NodeVersionConverter(std::function<bool(TxElement*)> fun)
    : VersionConverter(), fun_(fun) {}

bool NodeVersionConverter::convert(TxElement* root) { return fun_(root); }

TraversingVersionConverter::TraversingVersionConverter(std::function<bool(TxElement*)> fun)
    : VersionConverter(), fun_(fun) {}

bool TraversingVersionConverter::convert(TxElement* root) { return traverseNodes(root); }

bool TraversingVersionConverter::traverseNodes(TxElement* node) {
    bool res = true;
    res = res && fun_(node);
    ticpp::Iterator<ticpp::Element> child;
    for (child = child.begin(node); child != child.end(); child++) {
        res = res && traverseNodes(child.Get());
    }
    return res;
}

bool xml::copyMatchingSubPropsIntoComposite(TxElement* node, const CompositeProperty& prop) {
    TxElement propitem("Property");
    propitem.SetAttribute("type", prop.getClassIdentifier());
    propitem.SetAttribute("identifier", prop.getIdentifier());
    propitem.SetAttribute("displayName", prop.getDisplayName());
    propitem.SetAttribute("key", prop.getIdentifier());
    TxElement list("Properties");

    std::vector<Property*> props = prop.getProperties();

    bool res = false;

    // temp list
    std::vector<TxElement*> toBeDeleted;

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
                LogInfoCustom("VersionConverter",
                              "    Match for sub property: " + joinString(p->getPath(), ".") +
                                      " found in type: "
                                  << type << " id: " << id);

                list.InsertEndChild(*(child->Clone()));
                toBeDeleted.push_back(child.Get());
                match = true;
            }
        }
        res = res || match;
    }

    for (auto& elem : toBeDeleted) {
        node->RemoveChild(elem);
    }

    propitem.InsertEndChild(list);
    node->InsertEndChild(propitem);

    return res;
}

bool xml::hasProp(TxElement* node, const Property& prop) {
    bool result = false;
    ticpp::Iterator<ticpp::Element> child;
    for (child = child.begin(node); child != child.end(); child++) {
        if (prop.getClassIdentifier() == child->GetAttributeOrDefault("type", "") &&
            prop.getIdentifier() == child->GetAttributeOrDefault("identifier", "")) {
            result = true;
        }
    }
    return result;
}

std::vector<TxElement*> xml::getMatchingElements(TxElement* node, std::string key) {
    std::vector<TxElement*> res;
    ticpp::Iterator<ticpp::Element> child;
    for (child = child.begin(node); child != child.end(); child++) {
        std::string childkey;
        child->GetValue(&childkey);

        if (childkey == key) {
            res.push_back(child.Get());
        }
    }
    return res;
}

bool xml::findMatchingSubPropertiesForComposites(
    TxElement* processornode, const std::vector<const CompositeProperty*>& props) {
    auto pelm = xml::getMatchingElements(processornode, "Properties");
    if (pelm.empty()) return false;

    bool res = false;
    for (auto& prop : props) {
        if (!xml::hasProp(pelm[0], *prop)) {
            bool foundMatchingComposite = xml::copyMatchingCompositeProperty(pelm[0], *prop);
            bool foundSubProp = false;
            if (!foundMatchingComposite) {
                foundSubProp = xml::copyMatchingSubPropsIntoComposite(pelm[0], *prop);
            }
            res = res || foundSubProp || foundMatchingComposite;
        }
    }
    return res;
}

TxElement* xml::getElement(TxElement* node, std::string path) {
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
                    auto val = child->GetAttributeOrDefault(pair[0], "");
                    match = match && val == pair[1];
                }
            } else {
                match = false;
            }
            if (match) {
                if (parts.size() > 1) {
                    return getElement(child.Get(), joinString(parts.begin() + 1, parts.end(), "/"));
                } else {
                    return child.Get();
                }
                break;
            }
        }
    }
    return nullptr;
}

bool xml::copyMatchingCompositeProperty(TxElement* node, const CompositeProperty& prop) {
    ticpp::Iterator<ticpp::Element> child;
    for (child = child.begin(node); child != child.end(); child++) {
        std::string name;
        child->GetValue(&name);
        std::string type = child->GetAttributeOrDefault("type", "");
        std::string id = child->GetAttributeOrDefault("identifier", "");

        if ((type == "CompositeProperty" || type == "org.inviwo.CompositeProperty") &&
            prop.getIdentifier() == id) {
            LogInfoCustom("VersionConverter", "    Found Composite with same identifier");

            TxElement* newChild = node->InsertEndChild(*(child.Get()))->ToElement();
            newChild->SetAttribute("type", prop.getClassIdentifier());
            return true;
        }
    }

    return false;
}

void util::renamePort(Deserializer& d, std::vector<std::pair<const Port*, std::string>> rules) {
    NodeVersionConverter vc([&rules](TxElement* node) {
        bool didChanges = false;
        for (auto rule : rules) {
            TxElement* elem = nullptr;
            if (auto p1 = dynamic_cast<const Outport*>(rule.first)) {
                elem = xml::getElement(node, "OutPorts/OutPort&type=" + p1->getClassIdentifier() +
                                                 "&identifier=" + rule.second);
            } else if (auto p2 = dynamic_cast<const Inport*>(rule.first)) {
                elem = xml::getElement(node, "InPorts/InPort&type=" + p2->getClassIdentifier() +
                                                 "&identifier=" + rule.second);
            }
            if (elem) {
                elem->SetAttribute("identifier", rule.first->getIdentifier());
                didChanges = true;
            }
        }
        return didChanges;
    });
    d.convertVersion(&vc);
}

void util::renameProperty(Deserializer& d,
                          std::vector<std::pair<const Property*, std::string>> rules,
                          std::string path) {
    NodeVersionConverter vc([&rules, &path](TxElement* node) {
        bool didChanges = false;
        for (auto rule : rules) {
            TxElement* p =
                xml::getElement(node, path + "/Property&type=" + rule.first->getClassIdentifier() +
                                          "&identifier=" + rule.second);
            if (p) {
                p->SetAttribute("identifier", rule.first->getIdentifier());
                didChanges = true;
            }
        }
        return didChanges;
    });
    d.convertVersion(&vc);
}

void util::changePropertyType(Deserializer& d,
                              std::vector<std::pair<const Property*, std::string>> rules) {
    NodeVersionConverter vc([&rules](TxElement* node) {
        bool didChanges = false;
        for (auto rule : rules) {
            TxElement* p = xml::getElement(
                node, "Properties/Property&type=" + rule.first->getClassIdentifier() +
                          "&identifier=" + rule.first->getIdentifier());
            if (p) {
                p->SetAttribute("type", rule.second);
                didChanges = true;
            }
        }
        return didChanges;
    });
    d.convertVersion(&vc);
}

bool xml::changeTag(TxElement* node, const std::vector<Kind>& path, const std::string& oldName,
                    const std::string& newName) {
    if (path.empty()) return false;

    std::vector<xml::ElementMatcher> selector;
    for (const auto& kind : path) {
        selector.insert(selector.end(), kind.getMatchers().begin(), kind.getMatchers().end());
    }
    // selector.back().name = oldName;
    selector.insert(selector.end(), {oldName, {}});

    bool res = false;
    xml::visitMatchingNodes(node, selector, [&res, &newName](TxElement* n) {
        n->SetValue(newName);
        res |= true;
    });
    return res;
}

bool xml::changeIdentifier(TxElement* root, const std::vector<Kind>& path, const std::string& oldId,
                           const std::string& newId) {
    return changeAttribute(root, path, "identifier", oldId, newId);
}

IVW_CORE_API bool xml::changeIdentifiers(TxElement* root,
                                         const std::vector<IdentifierReplacement>& replacements) {
    bool res = false;
    for (const auto& repl : replacements) {
        res |= changeIdentifier(root, repl.path, repl.oldId, repl.newId);
    }
    return res;
}

bool xml::changeAttribute(TxElement* node, const std::vector<Kind>& path,
                          const std::string& attribute, const std::string& oldValue,
                          const std::string& newValue) {

    if (path.empty()) return false;

    std::vector<xml::ElementMatcher> selector;
    for (const auto& kind : path) {
        selector.insert(selector.end(), kind.getMatchers().begin(), kind.getMatchers().end());
    }
    selector.back().attributes.push_back({attribute, oldValue});

    bool res = false;
    xml::visitMatchingNodes(node, selector, [&res, &attribute, &newValue](TxElement* n) {
        n->SetAttribute(attribute, newValue);
        res |= true;
    });
    return res;
}

xml::Kind::Kind(const std::string& name, const std::string& list, const std::string& type)
    : name_(name), list_(list), type_(type) {

    ElementMatcher m1 = {list_, {}};
    ElementMatcher m2 = {name_, {{"type", type_}}};

    matchers_.push_back(m1);
    matchers_.push_back(m2);
}

xml::Kind xml::Kind::processor(const std::string& type) {
    Kind kind("Processor", "Processors", type);
    ElementMatcher m = {"ProcessorNetwork", {}};
    kind.matchers_.insert(kind.matchers_.begin(), m);
    return kind;
}

xml::Kind xml::Kind::inport(const std::string& type) { return Kind("InPort", "InPorts", type); }

xml::Kind xml::Kind::outport(const std::string& type) { return Kind("OutPort", "OutPorts", type); }

xml::Kind xml::Kind::portgroup(const std::string& type) {
    return Kind("PortGroup", "PortGroups", type);
}

xml::Kind xml::Kind::property(const std::string& type) {
    return Kind("Property", "Properties", type);
}

xml::Kind xml::Kind::propertyLinkSource(const std::string& type, const std::string& identifier) {
    Kind kind("SourceProperty", "PropertyLink", type);
    kind.matchers_.back().attributes.push_back({"identifier", identifier});
    ElementMatcher m = {"ProcessorNetwork", {}};
    ElementMatcher m2 = {"PropertyLinks", {}};
    kind.matchers_.insert(kind.matchers_.begin(), m2);
    kind.matchers_.insert(kind.matchers_.begin(), m);
    return kind;
}

xml::Kind xml::Kind::propertyLinkDestination(const std::string& type,
                                             const std::string& identifier) {
    Kind kind("DestinationProperty", "PropertyLink", type);
    kind.matchers_.back().attributes.push_back({"identifier", identifier});
    ElementMatcher m = {"ProcessorNetwork", {}};
    ElementMatcher m2 = {"PropertyLinks", {}};
    kind.matchers_.insert(kind.matchers_.begin(), m2);
    kind.matchers_.insert(kind.matchers_.begin(), m);
    return kind;
}

const std::string& xml::Kind::name() const { return name_; }

const std::string& xml::Kind::list() const { return list_; }

const std::string& xml::Kind::type() const { return type_; }

const std::vector<xml::ElementMatcher>& xml::Kind::getMatchers() const { return matchers_; }

xml::IdentifierReplacement::IdentifierReplacement(const std::vector<xml::Kind>& p,
                                                  const std::string& oi, const std::string& ni)
    : path(p), oldId(oi), newId(ni) {}

xml::IdentifierReplacement::IdentifierReplacement(IdentifierReplacement&& rhs)
    : path(std::move(rhs.path)), oldId(std::move(rhs.oldId)), newId(std::move(rhs.newId)) {}
xml::IdentifierReplacement& xml::IdentifierReplacement::operator=(IdentifierReplacement&& that) {
    if (this != &that) {
        path = std::move(that.path);
        oldId = std::move(that.oldId);
        newId = std::move(that.newId);
    }
    return *this;
}

}  // namespace inviwo

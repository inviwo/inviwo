/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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
    bool res = fun_(node);
    if (res) {
        for (TiXmlElement* child = node->FirstChildElement(); child;
             child = child->NextSiblingElement()) {
            res = res && traverseNodes(child);
        }
    }
    return res;
}

bool xml::copyMatchingSubPropsIntoComposite(TxElement* node, const CompositeProperty& prop) {

    TxElement list("Properties");
    bool res = false;
    std::vector<TxElement*> toBeDeleted;

    for (auto& p : prop.getProperties()) {
        bool match = false;

        for (TiXmlElement* child = node->FirstChildElement(); child;
             child = child->NextSiblingElement()) {

            const auto type = child->Attribute("type");
            const auto id = child->Attribute("identifier");

            if (type && id) {
                if (p->getIdentifier() == *id &&
                    (p->getClassIdentifier() == *type ||
                     p->getClassIdentifier() == util::splitByLast(*type, '.').second)) {
                    log::report(LogLevel::Info, SourceContext("VersionConverter"_sl),
                                "    Match for sub property: {} found in type: {}  id: {}",
                                p->getPath(), *type, *id);

                    list.LinkEndChild(child->Clone());
                    toBeDeleted.push_back(child);
                    match = true;
                }
            }
        }
        res = res || match;
    }

    for (auto& elem : toBeDeleted) {
        node->RemoveChild(elem);
    }

    if (res) {
        TxElement propitem("Property");
        propitem.SetAttribute("type", prop.getClassIdentifier());
        propitem.SetAttribute("identifier", prop.getIdentifier());
        propitem.SetAttribute("displayName", prop.getDisplayName());
        propitem.SetAttribute("key", prop.getIdentifier());
        propitem.InsertEndChild(list);
        node->InsertEndChild(propitem);
    }

    return res;
}

bool xml::hasProp(TxElement* node, const Property& prop) {
    bool result = false;
    for (TiXmlElement* child = node->FirstChildElement(); child;
         child = child->NextSiblingElement()) {

        const auto type = child->Attribute("type");
        const auto id = child->Attribute("identifier");

        if (type && id && prop.getClassIdentifier() == *type && prop.getIdentifier() == *id) {
            result = true;
        }
    }
    return result;
}

std::vector<TxElement*> xml::getMatchingElements(TxElement* node, std::string_view key) {
    std::vector<TxElement*> res;
    for (TiXmlElement* child = node->FirstChildElement(key); child;
         child = child->NextSiblingElement(key)) {
        res.push_back(child);
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

TxElement* xml::getElement(TxElement* node, std::string_view path) {
    if (path.empty()) return nullptr;

    const auto [first, rest] = util::splitByFirst(path, '/');
    const auto [name, attrs] = util::splitByFirst(first, '&');

    for (TiXmlElement* child = node->FirstChildElement(name); child;
         child = child->NextSiblingElement(name)) {
        bool match = true;

        util::forEachStringPart(attrs, "&", [&](std::string_view attrValue) {
            const auto [attr, value] = util::splitByFirst(attrValue, '=');
            const auto val = child->Attribute(attr);
            match = match && val && *val == value;
        });

        if (match) {
            if (!rest.empty()) {
                return getElement(child, rest);
            } else {
                return child;
            }
            break;
        }
    }

    return nullptr;
}

bool xml::copyMatchingCompositeProperty(TxElement* node, const CompositeProperty& prop) {
    for (TiXmlElement* child = node->FirstChildElement(); child;
         child = child->NextSiblingElement()) {

        const auto& type = child->GetAttribute("type");
        const auto& id = child->GetAttribute("identifier");

        if ((type == "CompositeProperty" || type == "org.inviwo.CompositeProperty") &&
            prop.getIdentifier() == id) {
            log::report(LogLevel::Info, SourceContext("VersionConverter"_sl),
                        "    Found Composite with same identifier");

            TxElement* newChild = node->InsertEndChild(*child)->ToElement();
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
                elem = xml::getElement(node, fmt::format("OutPorts/OutPort&type={}&identifier={}",
                                                         p1->getClassIdentifier(), rule.second));
            } else if (const auto* p2 = dynamic_cast<const Inport*>(rule.first)) {
                elem = xml::getElement(node, fmt::format("InPorts/InPort&type={}&identifier={}",
                                                         p2->getClassIdentifier(), rule.second));
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
                xml::getElement(node, fmt::format("{}/Property&type={}&identifier={}", path,
                                                  rule.first->getClassIdentifier(), rule.second));
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
                node, fmt::format("Properties/Property&type={}&identifier={}",
                                  rule.first->getClassIdentifier(), rule.first->getIdentifier()));
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

bool xml::changeAttributeRecursive(TxElement* node, const std::vector<Kind>& path,
                                   const std::string& attribute, const std::string& oldValue,
                                   const std::string& newValue) {

    if (path.empty()) return false;

    std::vector<xml::ElementMatcher> selector;
    for (const auto& kind : path) {
        selector.insert(selector.end(), kind.getMatchers().begin(), kind.getMatchers().end());
    }
    selector.back().attributes.push_back({attribute, oldValue});

    bool res = false;
    xml::visitMatchingNodesRecursive(node, selector, [&res, &attribute, &newValue](TxElement* n) {
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

xml::IdentifierReplacement::IdentifierReplacement(IdentifierReplacement&& rhs) noexcept
    : path(std::move(rhs.path)), oldId(std::move(rhs.oldId)), newId(std::move(rhs.newId)) {}
xml::IdentifierReplacement& xml::IdentifierReplacement::operator=(
    IdentifierReplacement&& that) noexcept {
    if (this != &that) {
        path = std::move(that.path);
        oldId = std::move(that.oldId);
        newId = std::move(that.newId);
    }
    return *this;
}

TxElement* xml::createNode(std::string_view desc, TxElement* parent) {
    util::forEachStringPart(desc, "/", [&](std::string_view part) {
        const auto [name, attrs] = util::splitByFirst(part, "&");

        TxElement::allocator_type alloc = parent->Allocator();
        auto* node = alloc.new_object<TxElement>(name);
        util::forEachStringPart(attrs, "&", [node](std::string_view attr) {
            const auto [key, value] = util::splitByFirst(attr, "=");
            node->SetAttribute(key, value);
        });
        if (parent) {
            parent->LinkEndChild(node);
        }
        parent = node;
    });
    return parent;
}

void xml::logNode(TxElement* node) {
    std::pmr::string xml;
    xml.reserve(4096);
    TiXmlPrinter printer{xml, TiXmlStreamPrint::No};
    node->Accept(&printer);
    log::report(LogLevel::Info, xml);
}

bool xml::renamePortIdentifier(TxElement* root, std::string_view processorClassId,
                               std::string_view portIdentifier, std::string_view newIdentifier) {
    bool res = false;
    std::vector<std::string> processorIds;
    xml::visitMatchingNodesRecursive(
        root,
        {
            {"ProcessorNetwork", {}},
            {"Processors", {}},
            {"Processor", {{"type", std::string{processorClassId}}}},
        },
        [&](TxElement* elem) {
            processorIds.push_back(elem->GetAttribute("identifier"));

            std::vector<xml::ElementMatcher> portGroupSelector;
            portGroupSelector.push_back({"PortGroups", {}});
            portGroupSelector.push_back({"PortGroup", {{"key", std::string(portIdentifier)}}});
            xml::visitMatchingNodes(elem, portGroupSelector, [&](TxElement* prop) {
                prop->SetAttribute("key", newIdentifier);
                res |= true;
            });

            std::vector<xml::ElementMatcher> inportSelector;
            inportSelector.push_back({"InPorts", {}});
            inportSelector.push_back({"InPort", {{"identifier", std::string(portIdentifier)}}});
            xml::visitMatchingNodes(elem, inportSelector, [&](TxElement* prop) {
                prop->SetAttribute("identifier", newIdentifier);
                res |= true;
            });

            std::vector<xml::ElementMatcher> outportSelector;
            outportSelector.push_back({"OutPorts", {}});
            outportSelector.push_back({"OutPort", {{"identifier", std::string(portIdentifier)}}});
            xml::visitMatchingNodes(elem, outportSelector, [&](TxElement* prop) {
                prop->SetAttribute("identifier", newIdentifier);
                res |= true;
            });
        });

    // Update any connections
    StrBuffer buf;
    xml::visitMatchingNodesRecursive(
        root,
        {
            {"ProcessorNetwork", {}},
            {"Connections", {}},
            {"Connection", {}},
        },
        [&](TxElement* elem) {
            auto src = elem->GetAttribute("src");
            auto dst = elem->GetAttribute("dst");
            for (const auto& pid : processorIds) {
                if (src == buf.replace("{}.{}", pid, portIdentifier).view()) {
                    elem->SetAttribute("src", buf.replace("{}.{}", pid, newIdentifier).view());
                }
                if (dst == buf.replace("{}.{}", pid, portIdentifier).view()) {
                    elem->SetAttribute("dst", buf.replace("{}.{}", pid, newIdentifier).view());
                }
            }
        });

    return res;
}

bool xml::renamePropertyIdentifier(TxElement* root, std::string_view processorClassId,
                                   std::string_view propertyPath, std::string_view newIdentifier) {
    bool res = false;

    std::vector<std::string> processorIds;
    auto path = util::splitStringView(propertyPath, '.');
    xml::visitMatchingNodesRecursive(
        root,
        {
            {"ProcessorNetwork", {}},
            {"Processors", {}},
            {"Processor", {{"type", std::string{processorClassId}}}},
        },
        [&](TxElement* elem) {
            processorIds.push_back(elem->GetAttribute("identifier"));

            std::vector<xml::ElementMatcher> selector;
            for (auto item : path) {
                selector.push_back({"Properties", {}});
                selector.push_back({"Property", {{"identifier", std::string(item)}}});
            }

            xml::visitMatchingNodes(elem, selector, [&](TxElement* prop) {
                prop->SetAttribute("identifier", newIdentifier);
                res |= true;
            });
        });

    // Update any links
    auto newPathParts = path;
    newPathParts.back() = newIdentifier;
    auto newPath = joinString(newPathParts, ".");

    StrBuffer buf;
    xml::visitMatchingNodesRecursive(
        root,
        {
            {"ProcessorNetwork", {}},
            {"PropertyLinks", {}},
            {"PropertyLink", {}},
        },
        [&](TxElement* elem) {
            auto src = elem->GetAttribute("src");
            auto dst = elem->GetAttribute("dst");
            for (const auto& pid : processorIds) {
                if (src.starts_with(buf.replace("{}.{}", pid, propertyPath))) {
                    elem->SetAttribute(
                        "src", buf.replace("{}.{}{}", pid, newPath,
                                           src.substr(pid.length() + propertyPath.length() + 1))
                                   .view());
                }
                if (dst.starts_with(buf.replace("{}.{}", pid, propertyPath))) {
                    elem->SetAttribute(
                        "dst", buf.replace("{}.{}{}", pid, newPath,
                                           dst.substr(pid.length() + propertyPath.length() + 1))
                                   .view());
                }
            }
        });

    return res;
}

}  // namespace inviwo

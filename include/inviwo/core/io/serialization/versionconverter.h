/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/io/serialization/ticpp.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/ports/port.h>

namespace inviwo {

class IVW_CORE_API VersionConverter {
public:
    VersionConverter();
    virtual ~VersionConverter() = default;
    virtual bool convert(TxElement* root) = 0;
};

/**
 * A version converter for a single node that calls a function for the given node.
 */
class IVW_CORE_API NodeVersionConverter : public VersionConverter {
public:
    template <typename T>
    NodeVersionConverter(T* obj, bool (T::*fPtr)(TxElement*));
    NodeVersionConverter(std::function<bool(TxElement*)> fun);

    virtual ~NodeVersionConverter() = default;
    virtual bool convert(TxElement* root);

private:
    std::function<bool(TxElement*)> fun_;
};

template <typename T>
inviwo::NodeVersionConverter::NodeVersionConverter(T* obj, bool (T::*fPtr)(TxElement*))
    : VersionConverter(), fun_(std::bind(fPtr, obj, std::placeholders::_1)) {}

/**
 * A version converter that traverses all child nodes calling a function for each node. The
 * traversal stops immediately when the function returns false.
 */
class IVW_CORE_API TraversingVersionConverter : public VersionConverter {
public:
    template <typename T>
    TraversingVersionConverter(T* obj, bool (T::*fPtr)(TxElement*));
    TraversingVersionConverter(std::function<bool(TxElement*)> fun);
    virtual ~TraversingVersionConverter() {}

    /**
     * Traverse all child nodes starting at @p root. The function provided in the constructor will
     * be called for each traversed node. If the function returns false, the traversal is stopped.
     * @param root  root node of the traversal
     * @return true if all children of @p root were traversed, false if the traversal was stopped
     */
    virtual bool convert(TxElement* root);

private:
    bool traverseNodes(TxElement* node);
    std::function<bool(TxElement*)> fun_;
};

template <typename T>
TraversingVersionConverter::TraversingVersionConverter(T* obj, bool (T::*fPtr)(TxElement*))
    : VersionConverter(), fun_(std::bind(fPtr, obj, std::placeholders::_1)) {}

namespace util {

IVW_CORE_API void renamePort(Deserializer& d,
                             std::vector<std::pair<const Port*, std::string>> rules);

IVW_CORE_API void renameProperty(Deserializer& d,
                                 std::vector<std::pair<const Property*, std::string>> rules,
                                 std::string path = "Properties");

IVW_CORE_API void changePropertyType(Deserializer& d,
                                     std::vector<std::pair<const Property*, std::string>> rules);

}  // namespace util

namespace xml {

IVW_CORE_API bool copyMatchingSubPropsIntoComposite(TxElement* node, const CompositeProperty& prop);

IVW_CORE_API bool hasProp(TxElement* node, const Property& prop);

IVW_CORE_API std::vector<TxElement*> getMatchingElements(TxElement* processornode,
                                                         std::string_view key);

IVW_CORE_API bool findMatchingSubPropertiesForComposites(
    TxElement* node, const std::vector<const CompositeProperty*>& props);

IVW_CORE_API TxElement* getElement(TxElement* node, std::string_view path);

IVW_CORE_API bool copyMatchingCompositeProperty(TxElement* node, const CompositeProperty& prop);

/**
 *	Helper class for specifying a selector path for visitMatchingNodes
 */
struct ElementMatcher {
    struct Attribute {
        std::string name;
        std::string value;
    };
    std::string name;
    std::vector<Attribute> attributes;
};

/**
 * This will traverse the nodes of an xml tree starting at @p root that match the selectors given.
 * and apply a @p visitor. The @p selector is specified by a vector or ElementMatchers,
 * matching on the node name and the list of given attribute name and value pairs.
 */
template <typename Visitor>
void visitMatchingNodes(TxElement* root, const std::vector<ElementMatcher>& selector,
                        Visitor visitor) {

    std::string childName;
    auto visitNodes = [&](auto& self, TxElement* node,
                          std::vector<ElementMatcher>::const_iterator begin,
                          std::vector<ElementMatcher>::const_iterator end) -> void {
        ticpp::Iterator<ticpp::Element> child;

        for (child = child.begin(node); child != child.end(); ++child) {
            child->GetValue(&childName);
            if (childName == begin->name) {
                bool match = true;
                for (const auto& attribute : begin->attributes) {
                    auto val = child->GetAttributeOrDefault(attribute.name, "");
                    match = match && val == attribute.value;
                }
                if (match) {
                    if (begin + 1 == end) {
                        visitor(child.Get());
                    } else {
                        self(self, child.Get(), begin + 1, end);
                    }
                }
            }
        }
    };
    visitNodes(visitNodes, root, selector.begin(), selector.end());
}

/**
 * This will traverse all the nodes of @p root and apply the @p visitor to all nodes that match the
 * @p selector.
 */
template <typename Visitor>
void visitMatchingNodesRecursive(TxElement* root, const ElementMatcher& selector, Visitor visitor) {
    std::string childName;
    auto visitNodes = [&](auto& self, TxElement* node) -> void {
        ticpp::Iterator<ticpp::Element> child;
        for (child = child.begin(node); child != child.end(); ++child) {
            child->GetValue(&childName);
            if (childName == selector.name) {
                bool match = true;
                for (const auto& attribute : selector.attributes) {
                    auto val = child->GetAttributeOrDefault(attribute.name, "");
                    match = match && val == attribute.value;
                }
                if (match) {
                    visitor(child.Get());
                }
            }
            self(self, child.Get());
        }
    };
    visitNodes(visitNodes, root);
}

template <typename Visitor>
void visitMatchingNodesRecursive(TxElement* root, const std::vector<ElementMatcher>& selectors,
                                 Visitor visitor) {

    std::string name;
    const auto match = [&](const ElementMatcher& matcher, const TxElement* node) {
        node->GetValue(&name);
        if (name == matcher.name) {
            bool match = true;
            for (const auto& attribute : matcher.attributes) {
                auto val = node->GetAttributeOrDefault(attribute.name, "");
                match = match && val == attribute.value;
            }
            return match;
        }
        return false;
    };

    std::vector<TxElement*> stack;

    auto visitNodes = [&](auto& self, TxElement* node) -> void {
        stack.push_back(node);

        if (stack.size() >= selectors.size() &&
            std::equal(selectors.rbegin(), selectors.rend(), stack.rbegin(),
                       stack.rbegin() + selectors.size(), match)) {
            std::invoke(visitor, node);
        }

        ticpp::Iterator<ticpp::Element> child;
        for (child = child.begin(node); child != child.end(); ++child) {
            self(self, child.Get());
        }

        stack.pop_back();
    };
    visitNodes(visitNodes, root);
}

/**
 * Helper class to specify a processor network xml path. For example
 * {
 *     xml::Kind::processor("org.inviwo.BackGround"),
 *     xml::Kind::inport("org.inviwo.ImageInport")
 * }
 * Will resolve into:
 * "Processors/Processor&type=org.inviwo.BackGround/InPorts/InPort&type=org.inviwo.ImageInport"
 */
class IVW_CORE_API Kind {
public:
    Kind(const Kind&) = default;
    Kind& operator=(const Kind&) = default;

    Kind(Kind&&) = default;
    Kind& operator=(Kind&&) = default;

    static Kind processor(const std::string& type);
    static Kind inport(const std::string& type);
    static Kind outport(const std::string& type);
    static Kind portgroup(const std::string& type);
    static Kind property(const std::string& type);
    static Kind propertyLinkSource(const std::string& type, const std::string& identifier);
    static Kind propertyLinkDestination(const std::string& type, const std::string& identifier);

    const std::string& name() const;
    const std::string& list() const;
    const std::string& type() const;

    const std::vector<ElementMatcher>& getMatchers() const;

private:
    Kind(const std::string& name, const std::string& list, const std::string& type);

    std::vector<ElementMatcher> matchers_;
    std::string name_;
    std::string list_;
    std::string type_;
};

/**
 * Utility function to change an xml tag matching oldName.
 * @param root The xml node to start from.
 * @param path The elements that you want to change (@see Kind).
 * @param oldName The old tag value. This is also used for identifying the elements.
 * @param newName The new tag value
 */
IVW_CORE_API bool changeTag(TxElement* root, const std::vector<Kind>& path,
                            const std::string& oldName, const std::string& newName);

/**
 * Utility function to change an attribute processor network element, i.e a processor, port, or
 * property.
 * @param root The xml node to start from.
 * @param path The elements that you want to change (@see Kind).
 * @param attribute The name of the attribute to change
 * @param oldValue The old attribute value. This is also used for identifying the elements.
 * @param newValue The new attribute value
 */
IVW_CORE_API bool changeAttribute(TxElement* root, const std::vector<Kind>& path,
                                  const std::string& attribute, const std::string& oldValue,
                                  const std::string& newValue);

/**
 *	Change identifier attribute. @see changeAttribute
 */
IVW_CORE_API bool changeIdentifier(TxElement* root, const std::vector<Kind>& path,
                                   const std::string& oldId, const std::string& newId);

/**
 *	Helper class for changeIdentifiers
 */
struct IVW_CORE_API IdentifierReplacement {
    IdentifierReplacement(const std::vector<Kind>& p, const std::string& oi, const std::string& ni);

    IdentifierReplacement(const IdentifierReplacement&) = default;
    IdentifierReplacement& operator=(const IdentifierReplacement&) = default;

    IdentifierReplacement(IdentifierReplacement&&) noexcept;
    IdentifierReplacement& operator=(IdentifierReplacement&&) noexcept;

    std::vector<Kind> path;
    std::string oldId;
    std::string newId;
};

IVW_CORE_API bool changeIdentifiers(TxElement* root,
                                    const std::vector<IdentifierReplacement>& replacements);

IVW_CORE_API TxElement* createNode(std::string_view desc, TxElement* parent = nullptr);

IVW_CORE_API void logNode(TxElement* root);

IVW_CORE_API bool renamePropertyIdentifier(TxElement* root, std::string_view processorClassId,
                                           std::string_view propertyPath,
                                           std::string_view newIdentifier);

}  // namespace xml

}  // namespace inviwo

/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/propertybasedtesting/testproperty.h>

#include <iostream>

namespace inviwo {
namespace HTML {

class IVW_MODULE_PROPERTYBASEDTESTING_API BaseElement {
    struct Attribute {
        std::string name;
        std::string value;
    };

    void printOpen(std::ostream& out, const size_t indent) const;
    void printContent(std::ostream& out, const size_t indent) const;
    void printClose(std::ostream& out, const size_t indent) const;

protected:
    std::string name;
    std::string content;
    std::vector<Attribute> attributes;
    std::vector<BaseElement> children;

    bool printClosing = true;

    void print(std::ostream& out, const size_t indent) const;

public:
    BaseElement(const std::string& name, const std::string& content = "");
    virtual ~BaseElement() = default;

    friend std::ostream& operator<<(std::ostream& out, const BaseElement& element);
};

template <class Derived>
class IVW_MODULE_PROPERTYBASEDTESTING_API Element : public BaseElement {
public:
    Element(const std::string& name, const std::string& content = "")
        : BaseElement(name, content) {}

    // add child
    virtual Derived& operator<<(const BaseElement& child) {
        children.emplace_back(child);
        return *static_cast<Derived*>(this);
    }
    Derived& addAttribute(const std::string& aName, const std::string& aValue) {
        attributes.push_back({aName, aValue});
        return *static_cast<Derived*>(this);
    }

    virtual ~Element() = default;
};

class IVW_MODULE_PROPERTYBASEDTESTING_API HTML : public Element<HTML> {
public:
    HTML() : Element("html") {}
};

class IVW_MODULE_PROPERTYBASEDTESTING_API Body : public Element<Body> {
public:
    Body() : Element("body") {}
};

class IVW_MODULE_PROPERTYBASEDTESTING_API Head : public Element<Head> {
public:
    Head() : Element("head") {}
    Head& stylesheet(const std::string& s) {
        *this << Element("link").addAttribute("rel", "stylesheet").addAttribute("href", s);
        return *this;
    }
};
class IVW_MODULE_PROPERTYBASEDTESTING_API Style : public Element<Style> {
public:
    Style(const std::string& content) : Element("style", content) {}
};
class IVW_MODULE_PROPERTYBASEDTESTING_API Meta : public Element<Meta> {
public:
    Meta() : Element("meta") { Element::printClosing = false; }
};

class IVW_MODULE_PROPERTYBASEDTESTING_API Text : public Element<Text> {
public:
    Text(const std::string& text) : Element("", text) {}
};

class IVW_MODULE_PROPERTYBASEDTESTING_API Div : public Element<Div> {
public:
    Div(const std::string& mclass) : Element("div") { this->addAttribute("class", mclass); }
};

class IVW_MODULE_PROPERTYBASEDTESTING_API TableCell : public Element<TableCell> {
public:
    TableCell(const BaseElement& el) : Element("td") { *this << el; }
};
class IVW_MODULE_PROPERTYBASEDTESTING_API Row : public Element<Row> {
public:
    Row() : Element("tr") {}
    virtual Row& operator<<(const BaseElement& child) override {
        Element::operator<<(static_cast<const BaseElement&>(TableCell(child)));
        return *this;
    }
    virtual Row& operator<<(const TableCell& child) {
        Element::operator<<(child);
        return *this;
    }
};
class IVW_MODULE_PROPERTYBASEDTESTING_API TableHeadCell : public Element<TableHeadCell> {
public:
    TableHeadCell(const BaseElement& el) : Element("th") { *this << el; }
};
class IVW_MODULE_PROPERTYBASEDTESTING_API HeadRow : public Element<HeadRow> {
public:
    HeadRow() : Element("tr") {}
    virtual HeadRow& operator<<(const BaseElement& child) override {
        Element::operator<<(static_cast<const BaseElement&>(TableHeadCell(child)));
        return *this;
    }
    virtual HeadRow& operator<<(const TableHeadCell& child) {
        Element::operator<<(child);
        return *this;
    }
};

class IVW_MODULE_PROPERTYBASEDTESTING_API Table : public Element<Table> {
public:
    Table() : Element("table") {}
};

class IVW_MODULE_PROPERTYBASEDTESTING_API Image : public Element<Image> {
public:
    Image(const std::filesystem::path& path, const std::string& alt = "") : Element("img") {
        BaseElement::printClosing = false;
        addAttribute("src", path.string());
        addAttribute("alt", alt);
    }
};

class IVW_MODULE_PROPERTYBASEDTESTING_API Details : public Element<Details> {
    class IVW_MODULE_PROPERTYBASEDTESTING_API Summary : public Element<Summary> {
    public:
        Summary() : Element("summary") {}
    };

public:
    Details(const BaseElement& summary, const BaseElement& content) : Element("details") {
        *this << (Summary() << summary) << content;
    }
};

class IVW_MODULE_PROPERTYBASEDTESTING_API Paragraph : public Element<Paragraph> {
public:
    Paragraph(const std::string& content) : Element("p", content) {}
};

class IVW_MODULE_PROPERTYBASEDTESTING_API TreeChildren : public Element<TreeChildren> {
public:
    TreeChildren() : Element("ul") {}
};
class IVW_MODULE_PROPERTYBASEDTESTING_API Tree : public Element<Tree> {
public:
    Tree(const BaseElement& content) : Element("ul") { *this << (Element("span") << content); }
};

}  // namespace HTML
}  // namespace inviwo

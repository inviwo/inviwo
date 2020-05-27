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
class Element {
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
	std::vector<Element> children;

	bool printClosing = true;

	virtual void print(std::ostream& out, const size_t indent) const;
public:
	Element(const std::string& name, const std::string& content = "");
	virtual ~Element();
	Element& addAttribute(const std::string& aName, const std::string& aValue = "");

	friend std::ostream& operator<<(std::ostream& out, const Element& element);

	// add child
	virtual Element& operator<<(const Element& child);
};

class HTML : public Element {
public:
	HTML()
			: Element("html") {
	}
};

class Body : public Element {
public:
	Body()
			: Element("body") {
	}
};

class Head : public Element {
public:
	Head()
		: Element("head") {
	}
	Head& stylesheet(const std::string& s) {
		*this << Element("link").addAttribute("rel", "stylesheet").addAttribute("href",s);
		return *this;
	}
};
class Style : public Element {
public:
	Style(const std::string& content)
		: Element("style", content) {
	}
};

class Text : public Element {
public:
	Text(const std::string& text)
			: Element("", text) {
	}
};

class TableCell : public Element {
public:
	TableCell(const Element& el)
			: Element("td") {
		*this << el;
	}
};
class TableHeadCell : public Element {
public:
	TableHeadCell(const Element& el)
			: Element("th") {
		*this << el;
	}
};
class Row : public Element {
public:
	Row()
			: Element("tr") {
	}
};
class Table : public Element {
public:
	Table()
			: Element("table") {
	}
};

class Image : public Element {
public:
	Image(const std::string& path, const std::string& alt = "")
			: Element("img") {
		Element::printClosing = false;
		addAttribute("src", path);
		addAttribute("alt", alt);
	}
};

class Details : public Element {
public:
	Details(const Element& summary, const Element& content) 
			: Element("details") {
		*this << (Element("summary") << summary) << content;
	}
};

class Paragraph : public Element {
public:
	Paragraph(const std::string& content)
			: Element("p", content) {
	}
};

} // namespace HTML
}  // namespace inviwo

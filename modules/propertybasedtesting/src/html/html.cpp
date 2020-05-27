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
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOr
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/propertybasedtesting/html/html.h>

namespace inviwo {

namespace HTML {
void BaseElement::printOpen(std::ostream& out, const size_t indent) const {
	std::fill_n(std::ostream_iterator<char>(out), indent, ' ');
	out << '<' << name;
	for(const auto&[aname, avalue] : attributes) {
		out << ' ' << aname;
		if(!avalue.empty()) {
			out << "=\"" << avalue << "\"";
		}
	}
	if(content.empty() && children.empty() && printClosing)
		out << '/';
	out << ">\n";
}
void BaseElement::printContent(std::ostream& out, const size_t indent) const {
	if(!content.empty()) {
		std::fill_n(std::ostream_iterator<char>(out), indent, ' ');
		out << content << '\n';
	}
	for(const auto& child : children)
		child.print(out, indent+2);
}
void BaseElement::printClose(std::ostream& out, const size_t indent) const {
	if(content.empty() && children.empty())
		return;

	std::fill_n(std::ostream_iterator<char>(out), indent, ' ');
	out << "</" << name << ">\n";
}
void BaseElement::print(std::ostream& out, const size_t indent) const {
	if(name.empty()) {
		printContent(out, indent);
	} else {
		printOpen(out, indent);
		printContent(out, indent);
		printClose(out, indent);
	}
}
BaseElement::BaseElement(const std::string& name, const std::string& content)
		: name(name)
		, content(content) {
}
std::ostream& operator<<(std::ostream& out, const BaseElement& element) {
	element.print(out, 0);
	return out;
}
} // namespace HTML
}  // namespace inviwo

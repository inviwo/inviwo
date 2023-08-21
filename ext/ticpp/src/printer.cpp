#include <ticpp/printer.h>

#include <ticpp/base.h>
#include <ticpp/element.h>
#include <ticpp/document.h>
#include <ticpp/attribute.h>
#include <ticpp/text.h>
#include <ticpp/comment.h>
#include <ticpp/declaration.h>
#include <ticpp/stylesheet.h>
#include <ticpp/unknown.h>


// TMP
#include <ticpp/ticpp.h>

bool TiXmlPrinter::VisitEnter(const TiXmlDocument&) { return true; }

bool TiXmlPrinter::VisitExit(const TiXmlDocument&) { return true; }

bool TiXmlPrinter::VisitEnter(const TiXmlElement& element, const TiXmlAttribute* firstAttribute) {
    DoIndent();
    buffer += "<";
    buffer += element.Value();

    for (const TiXmlAttribute* attrib = firstAttribute; attrib; attrib = attrib->Next()) {
        buffer += " ";
        attrib->Print(0, 0, &buffer);
    }

    if (!element.FirstChild()) {
        buffer += " />";
        DoLineBreak();
    } else {
        buffer += ">";
        if (element.FirstChild()->ToText() && element.LastChild() == element.FirstChild() &&
            element.FirstChild()->ToText()->CDATA() == false) {
            simpleTextPrint = true;
            // no DoLineBreak()!
        } else {
            DoLineBreak();
        }
    }
    ++depth;
    return true;
}

bool TiXmlPrinter::VisitExit(const TiXmlElement& element) {
    --depth;
    if (!element.FirstChild()) {
        // nothing.
    } else {
        if (simpleTextPrint) {
            simpleTextPrint = false;
        } else {
            DoIndent();
        }
        buffer += "</";
        buffer += element.Value();
        buffer += ">";
        DoLineBreak();
    }
    return true;
}

bool TiXmlPrinter::Visit(const TiXmlText& text) {
    if (text.CDATA()) {
        DoIndent();
        buffer += "<![CDATA[";
        buffer += text.Value();
        buffer += "]]>";
        DoLineBreak();
    } else if (simpleTextPrint) {
        std::string str;
        TiXmlBase::EncodeString(text.ValueTStr(), &str);
        buffer += str;
    } else {
        DoIndent();
        std::string str;
        TiXmlBase::EncodeString(text.ValueTStr(), &str);
        buffer += str;
        DoLineBreak();
    }
    return true;
}

bool TiXmlPrinter::Visit(const TiXmlDeclaration& declaration) {
    DoIndent();
    declaration.Print(0, 0, &buffer);
    DoLineBreak();
    return true;
}

bool TiXmlPrinter::Visit(const TiXmlComment& comment) {
    DoIndent();
    buffer += "<!--";
    buffer += comment.Value();
    buffer += "-->";
    DoLineBreak();
    return true;
}

bool TiXmlPrinter::Visit(const TiXmlUnknown& unknown) {
    DoIndent();
    buffer += "<";
    buffer += unknown.Value();
    buffer += ">";
    DoLineBreak();
    return true;
}

bool TiXmlPrinter::Visit(const TiXmlStylesheetReference& stylesheet) {
    DoIndent();
    stylesheet.Print(0, 0, &buffer);
    DoLineBreak();
    return true;
}

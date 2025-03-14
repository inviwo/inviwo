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

bool TiXmlPrinter::VisitEnter(const TiXmlElement& element, const TiXmlAttribute* firstAttribute) {
    DoIndent();
    buffer.push_back('<');
    buffer.append(element.Value());

    for (const TiXmlAttribute* attribute = firstAttribute; attribute;
         attribute = attribute->Next()) {
        buffer.push_back(' ');
        attribute->Print(buffer);
    }

    if (!element.FirstChild()) {
        buffer.push_back('/');
        buffer.push_back('>');
        DoLineBreak();
    } else {
        buffer.push_back('>');
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
        buffer.push_back('<');
        buffer.push_back('/');
        buffer.append(element.Value());
        buffer.push_back('>');
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
        TiXmlBase::EncodeString(text.Value(), buffer);
    } else {
        DoIndent();
        TiXmlBase::EncodeString(text.Value(), buffer);
        DoLineBreak();
    }
    return true;
}

bool TiXmlPrinter::Visit(const TiXmlDeclaration& declaration) {
    DoIndent();
    declaration.Print(buffer);
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
    stylesheet.Print(buffer);
    DoLineBreak();
    return true;
}

bool TiXmlFilePrinter::VisitEnter(const TiXmlElement& element,
                                  const TiXmlAttribute* firstAttribute) {
    DoIndent();

    fmt::fprintf(file, "<%s", element.Value());

    for (const TiXmlAttribute* attribute = firstAttribute; attribute;
         attribute = attribute->Next()) {
        fmt::fprintf(file, " ");
        attribute->Print(file);
    }

    if (!element.FirstChild()) {
        fmt::fprintf(file, " />");
        DoLineBreak();
    } else {
        fmt::fprintf(file, ">");

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

bool TiXmlFilePrinter::VisitExit(const TiXmlElement& element) {
    --depth;
    if (!element.FirstChild()) {
        // nothing.
    } else {
        if (simpleTextPrint) {
            simpleTextPrint = false;
        } else {
            DoIndent();
        }
        fmt::fprintf(file, "</%s>", element.Value());
        DoLineBreak();
    }
    return true;
}

bool TiXmlFilePrinter::Visit(const TiXmlText& text) {
    if (text.CDATA()) {
        DoIndent();
        fmt::fprintf(file, "<![CDATA[%s]]>\n", text.Value());
        DoLineBreak();
    } else if (simpleTextPrint) {
        std::pmr::string buffer{};
        TiXmlBase::EncodeString(text.Value(), buffer);
        fmt::fprintf(file, "%s", buffer);
    } else {
        DoIndent();
        std::pmr::string buffer{};
        TiXmlBase::EncodeString(text.Value(), buffer);
        fmt::fprintf(file, "%s", buffer);
        DoLineBreak();
    }
    return true;
}

bool TiXmlFilePrinter::Visit(const TiXmlDeclaration& declaration) {
    DoIndent();
    declaration.Print(file);
    DoLineBreak();
    return true;
}

bool TiXmlFilePrinter::Visit(const TiXmlComment& comment) {
    DoIndent();
    fmt::fprintf(file, "<!--%s-->", comment.Value());
    DoLineBreak();
    return true;
}

bool TiXmlFilePrinter::Visit(const TiXmlUnknown& unknown) {
    DoIndent();
    fmt::fprintf(file, "<%s>", unknown.Value());
    DoLineBreak();
    return true;
}

bool TiXmlFilePrinter::Visit(const TiXmlStylesheetReference& stylesheet) {
    DoIndent();
    stylesheet.Print(file);
    DoLineBreak();
    return true;
}

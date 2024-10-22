#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>
#include <ticpp/visitor.h>

#include <string>

/** Print to memory functionality. The TiXmlPrinter is useful when you need to:

    -# Print to memory (especially in non-STL mode)
    -# Control formatting (line endings, etc.)

    When constructed, the TiXmlPrinter is in its default "pretty printing" mode.
    Before calling Accept() you can call methods to control the printing
    of the XML document. After TiXmlNode::Accept() is called, the printed document can
    be accessed via the CStr(), Str(), and Size() methods.

    TiXmlPrinter uses the Visitor API.
    @verbatim
    TiXmlPrinter printer;
    printer.SetIndent( "\t" );

    doc.Accept( &printer );
    fprintf( stdout, "%s", printer.CStr() );
    @endverbatim
*/
class TICPP_API TiXmlPrinter : public TiXmlVisitor {
public:
    TiXmlPrinter() : depth(0), simpleTextPrint(false), buffer(), indent("    "), lineBreak("\n") {}

    virtual bool VisitEnter(const TiXmlDocument& doc);
    virtual bool VisitExit(const TiXmlDocument& doc);

    virtual bool VisitEnter(const TiXmlElement& element, const TiXmlAttribute* firstAttribute);
    virtual bool VisitExit(const TiXmlElement& element);

    virtual bool Visit(const TiXmlDeclaration& declaration);
    virtual bool Visit(const TiXmlText& text);
    virtual bool Visit(const TiXmlComment& comment);
    virtual bool Visit(const TiXmlUnknown& unknown);
    virtual bool Visit(const TiXmlStylesheetReference& stylesheet);

    /** Set the indent characters for printing. By default 4 spaces
        but tab (\t) is also useful, or empty string for no indentation.
    */
    void SetIndent(std::string_view _indent) { indent = _indent; }
    /// Query the indention string.
    const std::string& Indent() { return indent; }

    /** Set the line breaking string. By default set to newline (\n).
        Some operating systems prefer other characters, or can be
        set to the empty string for no breaking.
    */
    void SetLineBreak(std::string_view _lineBreak) { lineBreak = _lineBreak; }
    /// Query the current line breaking string.
    const std::string& LineBreak() { return lineBreak; }

    /** Switch over to "stream printing" which is the most dense formatting without
        line breaks. Common when the XML is needed for network transmission.
    */
    void SetStreamPrinting() {
        indent = "";
        lineBreak = "";
    }
    
    /// Return the result.
    const std::string& Str() { return buffer; }

private:
    void DoIndent() {
        for (int i = 0; i < depth; ++i) buffer += indent;
    }
    void DoLineBreak() { buffer += lineBreak; }

    int depth;
    bool simpleTextPrint;
    std::string buffer;
    std::string indent;
    std::string lineBreak;
};

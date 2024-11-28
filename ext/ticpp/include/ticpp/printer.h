#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>
#include <ticpp/visitor.h>

#include <string>
#include <cstdio>

#include <fmt/printf.h>

enum class TiXmlStreamPrint { No, Yes };

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
class TICPP_API TiXmlPrinter final : public TiXmlVisitor {
public:
    TiXmlPrinter(TiXmlStreamPrint streamPrint = TiXmlStreamPrint::No)
        : buffer{}
        , depth{0}
        , simpleTextPrint{false}
        , indent{streamPrint == TiXmlStreamPrint::No ? "    " : ""}
        , lineBreak{streamPrint == TiXmlStreamPrint::No ? "\n" : ""} {
        buffer.reserve(4096);
    }

    virtual bool VisitEnter(const TiXmlDocument& doc) override { return true; }
    virtual bool VisitExit(const TiXmlDocument& doc) override { return true; }

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
        if (indent.empty()) return;
        for (int i = 0; i < depth; ++i) buffer += indent;
    }
    void DoLineBreak() { buffer += lineBreak; }

    std::string buffer;

    int depth;
    bool simpleTextPrint;
    std::string indent;
    std::string lineBreak;
};

class TICPP_API TiXmlFilePrinter final : public TiXmlVisitor {
public:
    TiXmlFilePrinter(FILE* _file, TiXmlStreamPrint streamPrint = TiXmlStreamPrint::No)
        : file{_file}
        , depth{0}
        , simpleTextPrint{false}
        , indent{streamPrint == TiXmlStreamPrint::No ? "    " : ""}
        , lineBreak{streamPrint == TiXmlStreamPrint::No ? "\n" : ""} {}

    virtual bool VisitEnter(const TiXmlDocument& doc) { return true; }
    virtual bool VisitExit(const TiXmlDocument& doc) { return true; }

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

    FILE* GetFile() { return file; }

private:
    void DoIndent() {
        if (indent.empty()) return;
        for (int i = 0; i < depth; ++i) {
            fmt::fprintf(file, "%s", indent);
        }
    }
    void DoLineBreak() {
        if (lineBreak.empty()) return;
        fmt::fprintf(file, "%s", lineBreak);
    }
    FILE* file;
    int depth;
    bool simpleTextPrint;
    std::string indent;
    std::string lineBreak;
};

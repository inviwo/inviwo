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
    TiXmlPrinter(std::pmr::string& aBuffer, TiXmlStreamPrint streamPrint = TiXmlStreamPrint::No)
        : buffer{aBuffer}
        , depth{0}
        , simpleTextPrint{false}
        , indent{streamPrint == TiXmlStreamPrint::No ? 4 : 0}
        , lineBreak{streamPrint == TiXmlStreamPrint::No} {}

    virtual bool VisitEnter(const TiXmlDocument&) override { return true; }
    virtual bool VisitExit(const TiXmlDocument&) override { return true; }

    virtual bool VisitEnter(const TiXmlElement& element,
                            const TiXmlAttribute* firstAttribute) override;
    virtual bool VisitExit(const TiXmlElement& element) override;

    virtual bool Visit(const TiXmlDeclaration& declaration) override;
    virtual bool Visit(const TiXmlText& text) override;
    virtual bool Visit(const TiXmlComment& comment) override;
    virtual bool Visit(const TiXmlUnknown& unknown) override;
    virtual bool Visit(const TiXmlStylesheetReference& stylesheet) override;

    void SetIndent(int _indent) { indent = _indent; }
    int Indent() const { return indent; }

    void SetLineBreak(bool _lineBreak) { lineBreak = _lineBreak; }
    bool LineBreak() const { return lineBreak; }

    /** Switch over to "stream printing" which is the most dense formatting without
        line breaks. Common when the XML is needed for network transmission.
    */
    void SetStreamPrinting() {
        indent = 0;
        lineBreak = false;
    }

private:
    void DoIndent() {
        if (indent != 0) buffer.append(indent * depth, ' ');
    }
    void DoLineBreak() {
        if (lineBreak) buffer.push_back('\n');
    }

    std::pmr::string& buffer;

    int depth;
    bool simpleTextPrint;
    int indent;
    bool lineBreak;
};

class TICPP_API TiXmlFilePrinter final : public TiXmlVisitor {
public:
    TiXmlFilePrinter(FILE* _file, TiXmlStreamPrint streamPrint = TiXmlStreamPrint::No)
        : file{_file}
        , depth{0}
        , simpleTextPrint{false}
        , indent{streamPrint == TiXmlStreamPrint::No ? 4 : 0}
        , lineBreak{streamPrint == TiXmlStreamPrint::No} {}

    virtual bool VisitEnter(const TiXmlDocument&) override { return true; }
    virtual bool VisitExit(const TiXmlDocument&) override { return true; }

    virtual bool VisitEnter(const TiXmlElement& element,
                            const TiXmlAttribute* firstAttribute) override;
    virtual bool VisitExit(const TiXmlElement& element) override;

    virtual bool Visit(const TiXmlDeclaration& declaration) override;
    virtual bool Visit(const TiXmlText& text) override;
    virtual bool Visit(const TiXmlComment& comment) override;
    virtual bool Visit(const TiXmlUnknown& unknown) override;
    virtual bool Visit(const TiXmlStylesheetReference& stylesheet) override;

    void SetIndent(int _indent) { indent = _indent; }
    int Indent() const { return indent; }

    void SetLineBreak(bool _lineBreak) { lineBreak = _lineBreak; }
    bool LineBreak() const { return lineBreak; }

    /** Switch over to "stream printing" which is the most dense formatting without
        line breaks. Common when the XML is needed for network transmission.
    */
    void SetStreamPrinting() {
        indent = 0;
        lineBreak = false;
    }

    FILE* GetFile() { return file; }

private:
    void DoIndent() {
        if (indent == 0) return;
        fmt::print(file, "{:{}}", "", indent * depth);
    }
    void DoLineBreak() {
        if (lineBreak) fmt::fprintf(file, "\n");
    }
    FILE* file;

    int depth;
    bool simpleTextPrint;
    int indent;
    bool lineBreak;
};

#include <ticpp/comment.h>

#include <ticpp/document.h>
#include <ticpp/parsingdata.h>

#include <cstring>

TiXmlComment::TiXmlComment(const TiXmlComment& copy) : TiXmlNode(TiXmlNode::COMMENT) {
    copy.CopyTo(this);
}

void TiXmlComment::operator=(const TiXmlComment& base) {
    Clear();
    base.CopyTo(this);
}

void TiXmlComment::Print(FILE* cfile, int depth) const {
    assert(cfile);
    for (int i = 0; i < depth; i++) {
        fprintf(cfile, "    ");
    }
    fprintf(cfile, "<!--%s-->", value.c_str());
}

void TiXmlComment::CopyTo(TiXmlComment* target) const { TiXmlNode::CopyTo(target); }

bool TiXmlComment::Accept(TiXmlVisitor* visitor) const { return visitor->Visit(*this); }

TiXmlNode* TiXmlComment::Clone() const {
    TiXmlComment* clone = new TiXmlComment();

    if (!clone) return 0;

    CopyTo(clone);
    return clone;
}

const char* TiXmlComment::Parse(const char* p, TiXmlParsingData* data, const allocator_type& alloc) {
    TiXmlDocument* document = GetDocument();
    value = "";

    p = SkipWhiteSpace(p);

    if (data) {
        data->Stamp(p);
        location = data->Cursor();
    }
    const char* startTag = "<!--";
    const char* endTag = "-->";

    if (!StringEqual(p, startTag, false)) {
        document->SetError(TIXML_ERROR_PARSING_COMMENT, p, data);
        return 0;
    }
    p += strlen(startTag);

    // [ 1475201 ] TinyXML parses entities in comments
    // Oops - ReadText doesn't work, because we don't want to parse the entities.
    // p = ReadText( p, &value, false, endTag, false, encoding );
    //
    // from the XML spec:
    /*
     [Definition: Comments may appear anywhere in a document outside other markup; in addition,
                  they may appear within the document type declaration at places allowed by the
     grammar. They are not part of the document's character data; an XML processor MAY, but need
     not, make it possible for an application to retrieve the text of comments. For compatibility,
                              the string "--" (double-hyphen) MUST NOT occur within comments.]
     Parameter entity references MUST NOT be recognized within comments.

                              An example of a comment:

                              <!-- declarations for <head> & <body> -->
    */

    value = "";
    // Keep all the white space.
    while (p && *p && !StringEqual(p, endTag, false)) {
        value.append(p, 1);
        ++p;
    }
    if (p) p += strlen(endTag);

    return p;
}

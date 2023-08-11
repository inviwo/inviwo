#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>

/**
 * If you call the Accept() method, it requires being passed a TiXmlVisitor class to handle
 * callbacks. For nodes that contain other nodes (Document, Element) you will get called with a
 * VisitEnter/VisitExit pair. Nodes that are always leaves are simple called with Visit().
 *
 * If you return 'true' from a Visit method, recursive parsing will continue. If you return
 * false, <b>no children of this node or its sibilings</b> will be Visited.
 *
 * All flavors of Visit methods have a default implementation that returns 'true' (continue
 * visiting). You need to only override methods that are interesting to you.
 *
 * Generally Accept() is called on the TiXmlDocument, although all nodes suppert Visiting.
 *
 * You should never change the document from a callback.
 *
 * @sa TiXmlNode::Accept()
 */
class TICPP_API TiXmlVisitor {
public:
    virtual ~TiXmlVisitor() {}

    /// Visit a document.
    virtual bool VisitEnter(const TiXmlDocument& /*doc*/) { return true; }
    /// Visit a document.
    virtual bool VisitExit(const TiXmlDocument& /*doc*/) { return true; }

    /// Visit an element.
    virtual bool VisitEnter(const TiXmlElement& /*element*/,
                            const TiXmlAttribute* /*firstAttribute*/) {
        return true;
    }
    /// Visit an element.
    virtual bool VisitExit(const TiXmlElement& /*element*/) { return true; }

    /// Visit a declaration
    virtual bool Visit(const TiXmlDeclaration& /*declaration*/) { return true; }
    /// Visit a stylesheet reference
    virtual bool Visit(const TiXmlStylesheetReference& /*stylesheet*/) { return true; }
    /// Visit a text node
    virtual bool Visit(const TiXmlText& /*text*/) { return true; }
    /// Visit a comment node
    virtual bool Visit(const TiXmlComment& /*comment*/) { return true; }
    /// Visit an unknow node
    virtual bool Visit(const TiXmlUnknown& /*unknown*/) { return true; }
};

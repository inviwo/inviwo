#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>

#include <ticpp/node.h>

#include <iosfwd>

/** Always the top level node. A document binds together all the
    XML pieces. It can be saved, loaded, and printed to the screen.
    The 'value' of a document node is the xml file name.
*/
class TICPP_API TiXmlDocument final : public TiXmlNode {
public:
    /// Create an empty document, that has no name.
    TiXmlDocument(const allocator_type& alloc = {});
    /// Create a document with a name. The name of the document is also the filename of the xml.
    TiXmlDocument(std::string_view documentName, const allocator_type& alloc = {});

    TiXmlDocument(const TiXmlDocument& copy);
    void operator=(const TiXmlDocument& copy);

    virtual ~TiXmlDocument() {}

    /** Load a file using the current document value.
        Returns true if successful. Will delete any existing
        document data before loading.
    */
    void LoadFile();
    /// Save a file using the current document value. Returns true if successful.
    bool SaveFile() const;
    /// Load a file using the given filename. Returns true if successful.

    void LoadFile(std::string_view filename);
    /// Save a file using the given filename. Returns true if successful.
    bool SaveFile(std::string_view filename) const;
    /** Load a file using the given FILE*. Returns true if successful. Note that this method
        doesn't stream - the entire object pointed at by the FILE*
        will be interpreted as an XML file. TinyXML doesn't stream in XML from the current
        file location. Streaming may be added in the future.
    */
    void LoadFile(FILE*);
    /// Save a file using the given FILE*. Returns true if successful.
    bool SaveFile(FILE*) const;

    /// Parse the given null terminated block of xml data.
    const char* Parse(const char* p);

    const char* Parse(const char* p, TiXmlParsingData* data);

    virtual const char* Parse(const char* p, TiXmlParsingData* data,
                              const allocator_type& alloc) override;

    /** Get the root element -- the only top level element -- of the document.
        In well formed XML, there should only be one. TinyXml is tolerant of
        multiple elements at the document level.
    */
    const TiXmlElement* RootElement() const { return FirstChildElement(); }
    TiXmlElement* RootElement() { return FirstChildElement(); }

    /** SetTabSize() allows the error reporting functions (ErrorRow() and ErrorCol())
        to report the correct values for row and column. It does not change the output
        or input in any way.

        By calling this method, with a tab size
        greater than 0, the row and column of each node and attribute is stored
        when the file is loaded. Very useful for tracking the DOM back in to
        the source file.

        The tab size is required for calculating the location of nodes. If not
        set, the default of 4 is used. The tabsize is set per document. Setting
        the tabsize to 0 disables row/column tracking.

        Note that row and column tracking is not supported when using operator>>.

        The tab size needs to be enabled before the parse or load. Correct usage:
        @verbatim
        TiXmlDocument doc;
        doc.SetTabSize( 8 );
        doc.Load( "myfile.xml" );
        @endverbatim

        @sa Row, Column
    */
    void SetTabSize(int _tabsize) { tabsize = _tabsize; }

    int TabSize() const { return tabsize; }

    virtual const TiXmlDocument* ToDocument() const override { return this; }
    virtual TiXmlDocument* ToDocument() override { return this; }

    /// Walk the XML tree visiting this node and all of its children.
    virtual bool Accept(TiXmlVisitor* content) const override;

    const allocator_type& getAllocator() const { return allocator; }

protected:
    virtual TiXmlNode* Clone() const override;

private:
    void CopyTo(TiXmlDocument* target) const;

    allocator_type allocator;
    int tabsize;
};

#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>

#include <ticpp/node.h>

#include <iosfwd>

/** Always the top level node. A document binds together all the
    XML pieces. It can be saved, loaded, and printed to the screen.
    The 'value' of a document node is the xml file name.
*/
class TICPP_API TiXmlDocument : public TiXmlNode {
public:
    /// Create an empty document, that has no name.
    TiXmlDocument();
    /// Create a document with a name. The name of the document is also the filename of the xml.
    TiXmlDocument(std::string_view documentName);

    TiXmlDocument(const TiXmlDocument& copy);
    void operator=(const TiXmlDocument& copy);

    virtual ~TiXmlDocument() {}

    /** Load a file using the current document value.
        Returns true if successful. Will delete any existing
        document data before loading.
    */
    bool LoadFile();
    /// Save a file using the current document value. Returns true if successful.
    bool SaveFile() const;
    /// Load a file using the given filename. Returns true if successful.

    bool LoadFile(std::string_view filename);
    /// Save a file using the given filename. Returns true if successful.
    bool SaveFile(std::string_view filename) const;
    /** Load a file using the given FILE*. Returns true if successful. Note that this method
        doesn't stream - the entire object pointed at by the FILE*
        will be interpreted as an XML file. TinyXML doesn't stream in XML from the current
        file location. Streaming may be added in the future.
    */
    bool LoadFile(FILE*);
    /// Save a file using the given FILE*. Returns true if successful.
    bool SaveFile(FILE*) const;


    /** Parse the given null terminated block of xml data.
    */
    virtual const char* Parse(const char* p, TiXmlParsingData* data = nullptr);

    /** Get the root element -- the only top level element -- of the document.
        In well formed XML, there should only be one. TinyXml is tolerant of
        multiple elements at the document level.
    */
    const TiXmlElement* RootElement() const { return FirstChildElement(); }
    TiXmlElement* RootElement() { return FirstChildElement(); }

    /** If an error occurs, Error will be set to true. Also,
        The ErrorId() will contain the integer identifier of the error (not generally useful)
        The ErrorDesc() method will return the name of the error. (very useful)
        The ErrorRow() and ErrorCol() will return the location of the error (if known)
    */
    bool Error() const { return error; }

    /// Contains a textual (english) description of the error if one occurs.
    const char* ErrorDesc() const { return errorDesc.c_str(); }

    /** Generally, you probably want the error string ( ErrorDesc() ). But if you
        prefer the ErrorId, this function will fetch it.
    */
    int ErrorId() const { return errorId; }

    /** Returns the location (if known) of the error. The first column is column 1,
        and the first row is row 1. A value of 0 means the row and column wasn't applicable
        (memory errors, for example, have no row/column) or the parser lost the error. (An
        error in the error reporting, in that case.)

        @sa SetTabSize, Row, Column
    */
    int ErrorRow() const { return errorLocation.row + 1; }

    /// The column where the error occurred. See ErrorRow()
    int ErrorCol() const { return errorLocation.col + 1; }

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

    /** If you have handled the error, it can be reset with this call. The error
        state is automatically cleared if you Parse a new XML block.
    */
    void ClearError() {
        error = false;
        errorId = 0;
        errorDesc = "";
        errorLocation.row = errorLocation.col = 0;
    }

    /** Write the document to standard out using formatted printing ("pretty print"). */
    void Print() const { Print(stdout, 0); }

    /// Print this Document to a FILE stream.
    virtual void Print(FILE* cfile, int depth = 0) const;
    // [internal use]
    void SetError(int err, const char* errorLocation, TiXmlParsingData* prevData);

    /// The column where the error occured. See ErrorRow()
    virtual const TiXmlDocument* ToDocument() const { return this; }
    /// The column where the error occured. See ErrorRow()
    virtual TiXmlDocument* ToDocument() { return this; }

    /// Walk the XML tree visiting this node and all of its children.
    virtual bool Accept(TiXmlVisitor* content) const;

protected:
    virtual TiXmlNode* Clone() const;

private:
    void CopyTo(TiXmlDocument* target) const;

    bool error;
    int errorId;
    std::string errorDesc;
    int tabsize;
    TiXmlCursor errorLocation;
};

#pragma once

#include <ticpp/ticppapi.h>
#include <ticpp/fwd.h>

#include <ticpp/base.h>

/** An attribute is a name-value pair. Elements have an arbitrary
 * number of attributes, each with a unique name.
 *
 * @note The attributes are not TiXmlNodes, since they are not
 * part of the tinyXML document object model. There are other
 * suggested ways to look at this problem.
 */
class TICPP_API TiXmlAttribute : public TiXmlBase {
    friend class TiXmlAttributeSet;

public:
    /// Construct an empty attribute.
    TiXmlAttribute() : TiXmlBase() {
        document = 0;
        prev = next = 0;
    }

    /// std::string constructor.
    TiXmlAttribute(const std::string& _name, const std::string& _value) {
        name = _name;
        value = _value;
        document = 0;
        prev = next = 0;
    }

    /// Construct an attribute with a name and value.
    TiXmlAttribute(const char* _name, const char* _value) {
        name = _name;
        value = _value;
        document = 0;
        prev = next = 0;
    }

    const char* Name() const { return name.c_str(); }    ///< Return the name of this attribute.
    const char* Value() const { return value.c_str(); }  ///< Return the value of this attribute.

    const std::string& ValueStr() const { return value; }  ///< Return the value of this attribute.

    int IntValue() const;        ///< Return the value of this attribute, converted to an integer.
    double DoubleValue() const;  ///< Return the value of this attribute, converted to a double.

    // Get the tinyxml string representation
    const std::string& NameTStr() const { return name; }

    /** QueryIntValue examines the value string. It is an alternative to the
     * IntValue() method with richer error checking.
     * If the value is an integer, it is stored in 'value' and
     * the call returns TIXML_SUCCESS. If it is not
     * an integer, it returns TIXML_WRONG_TYPE.
     * 
     * A specialized but useful call. Note that for success it returns 0,
     * which is the opposite of almost all other TinyXml calls.
     */
    int QueryIntValue(int* _value) const;
    /// QueryDoubleValue examines the value string. See QueryIntValue().
    int QueryDoubleValue(double* _value) const;

    void SetName(const char* _name) { name = _name; }      ///< Set the name of this attribute.
    void SetValue(const char* _value) { value = _value; }  ///< Set the value.

    void SetIntValue(int _value);        ///< Set the value from an integer.
    void SetDoubleValue(double _value);  ///< Set the value from a double.

    /// STL std::string form.
    void SetName(const std::string& _name) { name = _name; }
    /// STL std::string form.
    void SetValue(const std::string& _value) { value = _value; }

    /// Get the next sibling attribute in the DOM. Returns null at end.
    const TiXmlAttribute* Next() const;
    TiXmlAttribute* Next() {
        return const_cast<TiXmlAttribute*>((const_cast<const TiXmlAttribute*>(this))->Next());
    }

    /// Get the previous sibling attribute in the DOM. Returns null at beginning.
    const TiXmlAttribute* Previous() const;
    TiXmlAttribute* Previous() {
        return const_cast<TiXmlAttribute*>((const_cast<const TiXmlAttribute*>(this))->Previous());
    }

    bool operator==(const TiXmlAttribute& rhs) const { return rhs.name == name; }
    bool operator<(const TiXmlAttribute& rhs) const { return name < rhs.name; }
    bool operator>(const TiXmlAttribute& rhs) const { return name > rhs.name; }

    /*	Attribute parsing starts: first letter of the name
                                             returns: the next char after the value end quote
    */
    virtual const char* Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding);

    // Prints this Attribute to a FILE stream.
    virtual void Print(FILE* cfile, int depth) const { Print(cfile, depth, 0); }
    void Print(FILE* cfile, int depth, std::string* str) const;

    // [internal use]
    // Set the document pointer so the attribute can report errors.
    void SetDocument(TiXmlDocument* doc) { document = doc; }

private:
    TiXmlAttribute(const TiXmlAttribute&);       // not implemented.
    void operator=(const TiXmlAttribute& base);  // not allowed.

    TiXmlDocument* document;  // A pointer back to a document, for error reporting.
    std::string name;
    std::string value;
    TiXmlAttribute* prev;
    TiXmlAttribute* next;
};

/**	A class used to manage a group of attributes.
 * It is only used internally, both by the ELEMENT and the DECLARATION.
 *
 * The set can be changed transparent to the Element and Declaration
 * classes that use it, but NOT transparent to the Attribute
 * which has to implement a next() and previous() method. Which makes
 * it a bit problematic and prevents the use of STL.
 *
 * This version is implemented with circular lists because:
 *         - I like circular lists
 *         - it demonstrates some independence from the (typical) doubly linked list.
 */
class TICPP_API TiXmlAttributeSet {
public:
    TiXmlAttributeSet();
    ~TiXmlAttributeSet();

    void Add(TiXmlAttribute* attribute);
    void Remove(TiXmlAttribute* attribute);

    const TiXmlAttribute* First() const { return (sentinel.next == &sentinel) ? 0 : sentinel.next; }
    TiXmlAttribute* First() { return (sentinel.next == &sentinel) ? 0 : sentinel.next; }
    const TiXmlAttribute* Last() const { return (sentinel.prev == &sentinel) ? 0 : sentinel.prev; }
    TiXmlAttribute* Last() { return (sentinel.prev == &sentinel) ? 0 : sentinel.prev; }

    const TiXmlAttribute* Find(const char* _name) const;
    TiXmlAttribute* Find(const char* _name) {
        return const_cast<TiXmlAttribute*>(
            (const_cast<const TiXmlAttributeSet*>(this))->Find(_name));
    }

    const TiXmlAttribute* Find(const std::string& _name) const;
    TiXmlAttribute* Find(const std::string& _name) {
        return const_cast<TiXmlAttribute*>(
            (const_cast<const TiXmlAttributeSet*>(this))->Find(_name));
    }

private:
    //*ME:	Because of hidden/disabled copy-construktor in TiXmlAttribute (sentinel-element),
    //*ME:	this class must be also use a hidden/disabled copy-constructor !!!
    TiXmlAttributeSet(const TiXmlAttributeSet&);  // not allowed
    void operator=(const TiXmlAttributeSet&);     // not allowed (as TiXmlAttribute)

    TiXmlAttribute sentinel;
};

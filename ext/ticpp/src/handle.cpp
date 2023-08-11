#include <ticpp/handle.h>

#include <ticpp/element.h>

TiXmlHandle TiXmlHandle::FirstChild() const {
    if (node) {
        TiXmlNode* child = node->FirstChild();
        if (child) return TiXmlHandle(child);
    }
    return TiXmlHandle(0);
}

TiXmlHandle TiXmlHandle::FirstChild(const char* value) const {
    if (node) {
        TiXmlNode* child = node->FirstChild(value);
        if (child) return TiXmlHandle(child);
    }
    return TiXmlHandle(0);
}

TiXmlHandle TiXmlHandle::FirstChildElement() const {
    if (node) {
        TiXmlElement* child = node->FirstChildElement();
        if (child) return TiXmlHandle(child);
    }
    return TiXmlHandle(0);
}

TiXmlHandle TiXmlHandle::FirstChildElement(const char* value) const {
    if (node) {
        TiXmlElement* child = node->FirstChildElement(value);
        if (child) return TiXmlHandle(child);
    }
    return TiXmlHandle(0);
}

TiXmlHandle TiXmlHandle::Child(int count) const {
    if (node) {
        int i;
        TiXmlNode* child = node->FirstChild();
        for (i = 0; child && i < count; child = child->NextSibling(), ++i) {
            // nothing
        }
        if (child) return TiXmlHandle(child);
    }
    return TiXmlHandle(0);
}

TiXmlHandle TiXmlHandle::Child(const char* value, int count) const {
    if (node) {
        int i;
        TiXmlNode* child = node->FirstChild(value);
        for (i = 0; child && i < count; child = child->NextSibling(value), ++i) {
            // nothing
        }
        if (child) return TiXmlHandle(child);
    }
    return TiXmlHandle(0);
}

TiXmlHandle TiXmlHandle::ChildElement(int count) const {
    if (node) {
        int i;
        TiXmlElement* child = node->FirstChildElement();
        for (i = 0; child && i < count; child = child->NextSiblingElement(), ++i) {
            // nothing
        }
        if (child) return TiXmlHandle(child);
    }
    return TiXmlHandle(0);
}

TiXmlHandle TiXmlHandle::ChildElement(const char* value, int count) const {
    if (node) {
        int i;
        TiXmlElement* child = node->FirstChildElement(value);
        for (i = 0; child && i < count; child = child->NextSiblingElement(value), ++i) {
            // nothing
        }
        if (child) return TiXmlHandle(child);
    }
    return TiXmlHandle(0);
}

#include <ticpp/handle.h>

#include <ticpp/element.h>

TiXmlHandle TiXmlHandle::FirstChild() const {
    if (node) {
        return TiXmlHandle(node->FirstChild());
    }
    return TiXmlHandle(nullptr);
}

TiXmlHandle TiXmlHandle::FirstChild(std::string_view value) const {
    if (node) {
        return TiXmlHandle(node->FirstChild(value));
    }
    return TiXmlHandle(nullptr);
}

TiXmlHandle TiXmlHandle::FirstChildElement() const {
    if (node) {
        return TiXmlHandle(node->FirstChildElement());
    }
    return TiXmlHandle(nullptr);
}

TiXmlHandle TiXmlHandle::FirstChildElement(std::string_view value) const {
    if (node) {
        return TiXmlHandle(node->FirstChildElement(value));
    }
    return TiXmlHandle(nullptr);
}

TiXmlHandle TiXmlHandle::Child(int count) const {
    if (node) {
        TiXmlNode* child = node->FirstChild();
        for (int i = 0; child && i < count; child = child->NextSibling(), ++i) {
            // nothing
        }
        return TiXmlHandle(child);
    }
    return TiXmlHandle(nullptr);
}

TiXmlHandle TiXmlHandle::Child(std::string_view value, int count) const {
    if (node) {
        TiXmlNode* child = node->FirstChild(value);
        for (int i = 0; child && i < count; child = child->NextSibling(value), ++i) {
            // nothing
        }
        return TiXmlHandle(child);
    }
    return TiXmlHandle(nullptr);
}

TiXmlHandle TiXmlHandle::ChildElement(int count) const {
    if (node) {
        TiXmlElement* child = node->FirstChildElement();
        for (int i = 0; child && i < count; child = child->NextSiblingElement(), ++i) {
            // nothing
        }
        return TiXmlHandle(child);
    }
    return TiXmlHandle(nullptr);
}

TiXmlHandle TiXmlHandle::ChildElement(std::string_view value, int count) const {
    if (node) {
        TiXmlElement* child = node->FirstChildElement(value);
        for (int i = 0; child && i < count; child = child->NextSiblingElement(value), ++i) {
            // nothing
        }
        return TiXmlHandle(child);
    }
    return TiXmlHandle(nullptr);
}

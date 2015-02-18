/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *********************************************************************************/

#include <inviwo/core/datastructures/geometry/mesh.h>

namespace inviwo {

Mesh::Mesh() : Geometry(), defaultAttributeInfo_(AttributesInfo()) {}

Mesh::Mesh(GeometryEnums::RenderType rt, GeometryEnums::ConnectivityType ct) : Geometry(), defaultAttributeInfo_(AttributesInfo(rt, ct)) {}

Mesh::Mesh(const Mesh& rhs) : Geometry(rhs) {
    std::vector<bool>::const_iterator itOwnership = rhs.attributesOwnership_.begin();
    std::vector<Buffer*>::const_iterator it;
    for (it = rhs.attributes_.begin(); it != rhs.attributes_.end(); ++it, ++itOwnership) {
        if (*itOwnership) {
            addAttribute(static_cast<Buffer*>((*it)->clone()));
        } else {
            addAttribute(*it, false);
        }
    }

    for (IndexVector::const_iterator it = rhs.indexAttributes_.begin();
         it != rhs.indexAttributes_.end(); ++it) {
        addIndicies(it->first, static_cast<IndexBuffer*>(it->second->clone()));
    }
}

Mesh& Mesh::operator=(const Mesh& that) {
    if (this != &that) {
        Geometry::operator=(that);
        deinitialize();

        std::vector<bool>::const_iterator itOwnership = that.attributesOwnership_.begin();
        std::vector<Buffer*>::const_iterator it;
        for (it = that.attributes_.begin(); it != that.attributes_.end(); ++it, ++itOwnership) {
            if (*itOwnership) {
                addAttribute(static_cast<Buffer*>((*it)->clone()));
            } else {
                addAttribute(*it, false);
            }
        }

        for (IndexVector::const_iterator it = that.indexAttributes_.begin();
             it != that.indexAttributes_.end(); ++it) {
            addIndicies(it->first, static_cast<IndexBuffer*>(it->second->clone()));
        }
    }
    return *this;
}

Mesh::~Mesh() {
    deinitialize();
}


Mesh* Mesh::clone() const {
    return new Mesh(*this);
}

void Mesh::deinitialize() {
    std::vector<bool>::const_iterator itOwnership = attributesOwnership_.begin();
    for (std::vector<Buffer*>::iterator it = attributes_.begin(), itEnd = attributes_.end();
         it != itEnd; ++it, ++itOwnership) {
        if (*itOwnership) delete (*it);
    }

    for (IndexVector::iterator it = indexAttributes_.begin(), itEnd = indexAttributes_.end();
         it != itEnd; ++it) {
        delete it->second;
    }

    attributes_.clear();
    attributesOwnership_.clear();
    indexAttributes_.clear();
}

const std::vector<Buffer*>& Mesh::getBuffers() const {
    return attributes_;
}

const Mesh::IndexVector& Mesh::getIndexBuffers() const {
    return indexAttributes_;
}

std::string Mesh::getDataInfo() const{
    std::ostringstream ss;

    ss << "<table border='0' cellspacing='0' cellpadding='0' style='border-color:white;white-space:pre;'>\n"
        << "<tr><td style='color:#bbb;padding-right:8px;'>Type</td><td><nobr>Mesh</nobr></td></tr>\n"
        << "<tr><td style='color:#bbb;padding-right:8px;'>Data</td><td><nobr>";

    switch (getDefaultAttributesInfo().rt) {
        case GeometryEnums::POINTS:
            ss << "Points";
            break;
        case GeometryEnums::LINES:
            ss << "Lines";
            break;
        case GeometryEnums::TRIANGLES:
            ss << "Triangles";
            break;
        default:
            ss << "Not specified";
    }
    ss << "</nobr></td></tr>\n"
        << "</tr></table>\n";

    return ss.str();
}


void Mesh::addAttribute(Buffer* att, bool takeOwnership /*= true*/) {
    attributes_.push_back(att);
    attributesOwnership_.push_back(takeOwnership);
}


void Mesh::setAttribute(size_t idx, Buffer* att, bool takeOwnership /*= true*/) {
    if (idx < attributes_.size()) {
        if (attributesOwnership_[idx]) {
            delete attributes_[idx];
        }
        attributes_[idx] = att;
        attributesOwnership_[idx] = takeOwnership;
    }
}

void Mesh::addIndicies(AttributesInfo info, IndexBuffer* ind) {
    indexAttributes_.push_back(std::make_pair(info, ind));
}

const Buffer* Mesh::getAttributes(size_t idx) const {
    return attributes_[idx];
}

const Buffer* Mesh::getIndicies(size_t idx) const {
    return indexAttributes_[idx].second;
}

Buffer* Mesh::getAttributes(size_t idx) {
    return attributes_[idx];
}

Buffer* Mesh::getIndicies(size_t idx) {
    return indexAttributes_[idx].second;
}

Mesh::AttributesInfo Mesh::getDefaultAttributesInfo() const {
    return defaultAttributeInfo_;
}

Mesh::AttributesInfo Mesh::getIndexAttributesInfo(size_t idx) const {
    return indexAttributes_[idx].first;
}

size_t Mesh::getNumberOfAttributes() const {
    return attributes_.size();
}

size_t Mesh::getNumberOfIndicies() const {
    return indexAttributes_.size();
}

} // namespace


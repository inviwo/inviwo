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
#include <inviwo/core/util/tooltiphelper.h>

namespace inviwo {

Mesh::Mesh(DrawType dt, ConnectivityType ct)
    : DataGroup(), SpatialEntity<3>(), defaultAttributeInfo_(AttributesInfo(dt, ct)) {}

Mesh::Mesh(const Mesh& rhs)
    : DataGroup(rhs), SpatialEntity<3>(rhs), defaultAttributeInfo_(rhs.defaultAttributeInfo_) {
    for (const auto& attribute : rhs.attributes_) {
        attributes_.push_back(std::shared_ptr<Buffer>(attribute->clone()));
    }
    for (const auto& elem : rhs.indexAttributes_) {
        indexAttributes_.push_back(
            std::make_pair(elem.first, std::shared_ptr<IndexBuffer>(elem.second->clone())));
    }
}

Mesh& Mesh::operator=(const Mesh& that) {
    if (this != &that) {
        DataGroup::operator=(that);
        SpatialEntity<3>::operator=(that);
       
        std::vector<std::shared_ptr<Buffer>> attributes;
        IndexVector indexAttributes;
               
        for (const auto& attribute : that.attributes_) {
            attributes.push_back(std::shared_ptr<Buffer>(attribute->clone()));
        }
        for (const auto& elem : that.indexAttributes_) {
            indexAttributes.push_back(
                std::make_pair(elem.first, std::shared_ptr<IndexBuffer>(elem.second->clone())));
        }

        std::swap(attributes, attributes_);
        std::swap(indexAttributes, indexAttributes_);
        defaultAttributeInfo_ = that.defaultAttributeInfo_;    
    }
    return *this;
}

Mesh* Mesh::clone() const { return new Mesh(*this); }

const std::vector<std::shared_ptr<Buffer>>& Mesh::getBuffers() const { return attributes_; }

const Mesh::IndexVector& Mesh::getIndexBuffers() const { return indexAttributes_; }

std::string Mesh::getDataInfo() const {
    ToolTipHelper t("Mesh");
    t.tableTop();

    for(const auto& elem : indexAttributes_) {
        std::stringstream ss;
        switch (elem.first.dt) {
            case DrawType::POINTS:
                ss << "Points";
                break;
            case DrawType::LINES:
                ss << "Lines";
                break;
            case DrawType::TRIANGLES:
                ss << "Triangles";
                break;
            case DrawType::NOT_SPECIFIED:
            case DrawType::NUMBER_OF_DRAW_TYPES:
            default:
                ss << "Not specified";
        }
        ss << " ";
        switch (elem.first.ct) {
            case ConnectivityType::NONE:
                ss << "None";
                break;
            case ConnectivityType::STRIP:
                ss << "Strip";
                break;
            case ConnectivityType::LOOP:
                ss << "Loop";
                break;
            case ConnectivityType::FAN:
                ss << "Fan";
                break;
            case ConnectivityType::ADJACENCY:
                ss << "Adjacency";
                break;
            case ConnectivityType::STRIP_ADJACENCY:
                ss << "Strip adjacency";
                break;
            case ConnectivityType::NUMBER_OF_CONNECTIVITY_TYPES:
            default:
                ss << "Not specified";
        }

        ss << " (" << elem.second->getSize() << ")";
        t.row("IndexBuffer", ss.str());
    }

    for (const auto& elem : attributes_) {
        std::stringstream ss;
        switch (elem->getBufferType()) {
            case BufferType::POSITION_ATTRIB:
                ss << "Positions";
                break;
            case BufferType::NORMAL_ATTRIB:
                ss << "Normals";
                break;
            case BufferType::COLOR_ATTRIB:
                ss << "Colors";
                break;
            case BufferType::TEXCOORD_ATTRIB:
                ss << "Texture";
                break;
            case BufferType::CURVATURE_ATTRIB:
                ss << "Curvature";
                break;
            case BufferType::INDEX_ATTRIB:
                ss << "Index";
                break;
            case BufferType::NUMBER_OF_BUFFER_TYPES:
            default:
                ss << "Type not specified";
        }
        switch (elem->getBufferUsage()) {
            case BufferUsage::STATIC:
                ss << " Static";
                break;
            case BufferUsage::DYNAMIC:
                ss << " Dynamic";
                break;
            default:
                ss << " Usage not specified";
        }
        ss << " (" << elem->getSize() << ")";
        t.row("Buffer", ss.str());
    }

    t.tableBottom();
    return t;
}

void Mesh::addAttribute(std::shared_ptr<Buffer> att) {
    attributes_.push_back(att);
}

void Mesh::setAttribute(size_t idx, std::shared_ptr<Buffer> att) {
    if (idx < attributes_.size()) {
        attributes_[idx] = att;
    }
}

void Mesh::addIndicies(AttributesInfo info, std::shared_ptr<IndexBuffer> ind) {
    indexAttributes_.push_back(std::make_pair(info, ind));
}

const Buffer* Mesh::getAttributes(size_t idx) const { return attributes_[idx].get(); }

const Buffer* Mesh::getIndicies(size_t idx) const { return indexAttributes_[idx].second.get(); }

Buffer* Mesh::getAttributes(size_t idx) { return attributes_[idx].get(); }

Buffer* Mesh::getIndicies(size_t idx) { return indexAttributes_[idx].second.get(); }

Mesh::AttributesInfo Mesh::getDefaultAttributesInfo() const { return defaultAttributeInfo_; }

Mesh::AttributesInfo Mesh::getIndexAttributesInfo(size_t idx) const {
    return indexAttributes_[idx].first;
}

size_t Mesh::getNumberOfAttributes() const { return attributes_.size(); }

size_t Mesh::getNumberOfIndicies() const { return indexAttributes_.size(); }

const SpatialCameraCoordinateTransformer<3>& Mesh::getCoordinateTransformer(
    const Camera& camera) const {
    return SpatialEntity<3>::getCoordinateTransformer(camera);
}

inviwo::uvec3 Mesh::COLOR_CODE = uvec3(188, 188, 101);

const std::string Mesh::CLASS_IDENTIFIER = "org.inviwo.Mesh";

}  // namespace

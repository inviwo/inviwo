/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/core/network/workspaceannotations.h>

namespace inviwo {

WorkspaceAnnotations::Base64Image::Base64Image(std::string name, std::string base64jpeg, int w,
                                               int h)
    : Base64Image{name, base64jpeg, ivec2{w, h}} {}

WorkspaceAnnotations::Base64Image::Base64Image(std::string name, std::string base64jpeg, ivec2 size)
    : name(name), base64jpeg(base64jpeg), size(size) {}

bool WorkspaceAnnotations::Base64Image::isValid() const {
    return !base64jpeg.empty() && glm::compMul(size) > 0;
}

void WorkspaceAnnotations::Base64Image::serialize(Serializer &s) const {
    s.serialize("name", name);
    s.serialize("size", size);
    s.serialize("base64", base64jpeg);
}

void WorkspaceAnnotations::Base64Image::deserialize(Deserializer &d) {
    d.deserialize("name", name);
    d.deserialize("size", size);
    d.deserialize("base64", base64jpeg);
}

WorkspaceAnnotations::WorkspaceAnnotations() : WorkspaceAnnotations(ImageVector{}) {}

WorkspaceAnnotations::WorkspaceAnnotations(const ImageVector &canvasImages)
    : title_{"title", "Title", ""}
    , author_{"author", "Author", ""}
    , tags_{"tags", "Tags", ""}
    , categories_{"categories", "Categories", ""}
    , description_("description", "Description", "", InvalidationLevel::InvalidOutput,
                   PropertySemantics::Multiline)
    , canvases_{canvasImages} {

    addProperty(title_);
    addProperty(author_);
    addProperty(tags_);
    addProperty(categories_);
    addProperty(description_);
}

void WorkspaceAnnotations::serialize(Serializer &s) const {
    try {
        PropertyOwner::serialize(s);
        if (!canvases_.empty()) {
            s.serialize("Canvases", canvases_, "CanvasImage");
        }
    } catch (const Exception &e) {
        util::log(e.getContext(), e.getMessage(), LogLevel::Error);
    } catch (const std::exception &e) {
        LogWarn(e.what());
    }
}

void WorkspaceAnnotations::deserialize(Deserializer &d) {
    canvases_.clear();
    // an error is not critical as default values will be used.
    try {
        PropertyOwner::deserialize(d);
        d.deserialize("Canvases", canvases_, "CanvasImage");
    } catch (const Exception &e) {
        util::log(e.getContext(), e.getMessage(), LogLevel::Error);
    } catch (const std::exception &e) {
        LogError(e.what());
    }
}

void WorkspaceAnnotations::setTitle(const std::string &title) { title_ = title; }

std::string WorkspaceAnnotations::getTitle() const { return title_; }

void WorkspaceAnnotations::setAuthor(const std::string &author) { author_ = author; }

std::string WorkspaceAnnotations::getAuthor() const { return author_; }

void WorkspaceAnnotations::setTags(const std::string &tags) { tags_ = tags; }

std::string WorkspaceAnnotations::getTags() const { return tags_; }

void WorkspaceAnnotations::setCategories(const std::string &cat) { categories_ = cat; }

std::string WorkspaceAnnotations::getCategories() const { return categories_; }

void WorkspaceAnnotations::setDescription(const std::string &desc) { description_ = desc; }

std::string WorkspaceAnnotations::getDescription() const { return description_; }

void WorkspaceAnnotations::setCanvasImages(const ImageVector &canvases) { canvases_ = canvases; }

const WorkspaceAnnotations::ImageVector WorkspaceAnnotations::getCanvasImages() const {
    return canvases_;
}

}  // namespace inviwo

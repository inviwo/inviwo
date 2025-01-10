/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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
#include <inviwo/core/common/inviwoapplication.h>

#include <inviwo/core/util/glm.h>

#include <fstream>
#include <fmt/std.h>

namespace inviwo {

WorkspaceAnnotations::Base64Image::Base64Image(std::string name, std::string base64jpeg, int w,
                                               int h)
    : Base64Image{name, base64jpeg, ivec2{w, h}} {}

WorkspaceAnnotations::Base64Image::Base64Image(std::string name, std::string base64jpeg, ivec2 size)
    : name(name), base64jpeg(base64jpeg), size(size) {}

bool WorkspaceAnnotations::Base64Image::isValid() const {
    return !base64jpeg.empty() && glm::compMul(size) > 0;
}

void WorkspaceAnnotations::Base64Image::serialize(Serializer& s) const {
    s.serialize("name", name);
    s.serialize("size", size);
    s.serialize("base64", base64jpeg);
}

void WorkspaceAnnotations::Base64Image::deserialize(Deserializer& d) {
    d.deserialize("name", name);
    d.deserialize("size", size);
    d.deserialize("base64", base64jpeg);
}

WorkspaceAnnotations::WorkspaceAnnotations(InviwoApplication* app)
    : WorkspaceAnnotations(std::vector<Base64Image>{}, app) {}

WorkspaceAnnotations::WorkspaceAnnotations(const std::vector<Base64Image>& canvasImages,
                                           InviwoApplication* app)
    : title_{"title", "Title", ""}
    , author_{"author", "Author", ""}
    , tags_{"tags", "Tags", ""}
    , categories_{"categories", "Categories", ""}
    , description_("description", "Description", "", InvalidationLevel::InvalidOutput,
                   PropertySemantics::Multiline)
    , primaryCanvasId_{"primaryCanvasId", "Primany canvas identifier", ""}
    , canvases_{canvasImages}
    , app_{app} {

    addProperties(title_, author_, tags_, categories_, description_, primaryCanvasId_);
}

WorkspaceAnnotations::WorkspaceAnnotations(const std::filesystem::path& path,
                                           InviwoApplication* app)
    : WorkspaceAnnotations{std::vector<Base64Image>{}, app} {
    if (auto f = std::ifstream(path)) {
        LogFilter logger{LogCentral::getPtr(), LogVerbosity::None};
        auto d = app->getWorkspaceManager()->createWorkspaceDeserializer(f, path, &logger);
        d.deserialize("WorkspaceAnnotations", *this);
    } else {
        throw Exception(IVW_CONTEXT, "Unable to open file {}", path);
    }
}

void WorkspaceAnnotations::serialize(Serializer& s) const {
    try {
        PropertyOwner::serialize(s);
        if (!canvases_.empty()) {
            s.serialize("Canvases", canvases_, "CanvasImage");
        }
    } catch (const Exception& e) {
        log::exception(e);
    } catch (const std::exception& e) {
        log::exception(e);
    }
}

void WorkspaceAnnotations::deserialize(Deserializer& d) {
    canvases_.clear();
    // an error is not critical as default values will be used.
    try {
        PropertyOwner::deserialize(d);
        d.deserialize("Canvases", canvases_, "CanvasImage");
    } catch (const Exception& e) {
        log::exception(e);
    } catch (const std::exception& e) {
        log::exception(e);
    }
}

InviwoApplication* WorkspaceAnnotations::getInviwoApplication() { return app_; }

void WorkspaceAnnotations::setTitle(const std::string& title) { title_ = title; }

const std::string& WorkspaceAnnotations::getTitle() const { return title_; }

void WorkspaceAnnotations::setAuthor(const std::string& author) { author_ = author; }

const std::string& WorkspaceAnnotations::getAuthor() const { return author_; }

void WorkspaceAnnotations::setTags(const std::string& tags) { tags_ = tags; }

const std::string& WorkspaceAnnotations::getTags() const { return tags_; }

void WorkspaceAnnotations::setCategories(const std::string& cat) { categories_ = cat; }

const std::string& WorkspaceAnnotations::getCategories() const { return categories_; }

void WorkspaceAnnotations::setDescription(const std::string& desc) { description_ = desc; }

const std::string& WorkspaceAnnotations::getDescription() const { return description_; }

void WorkspaceAnnotations::setCanvasImages(const std::vector<Base64Image>& canvases) {
    canvases_ = canvases;
}

size_t WorkspaceAnnotations::numberOfCanvases() const { return canvases_.size(); }
const WorkspaceAnnotations::Base64Image& WorkspaceAnnotations::getCanvasImage(size_t i) const {
    return canvases_[i];
}

std::optional<size_t> WorkspaceAnnotations::getPrimaryCanvasIndex() const {
    auto it = std::find_if(canvases_.begin(), canvases_.end(), [&](const Base64Image& img) {
        return img.name == primaryCanvasId_.get();
    });
    if (it != canvases_.end()) {
        return std::distance(canvases_.begin(), it);
    } else if (!canvases_.empty()) {
        return 0;
    } else {
        return std::nullopt;
    }
}

const WorkspaceAnnotations::Base64Image* WorkspaceAnnotations::getPrimaryCanvasImage() const {
    if (auto index = getPrimaryCanvasIndex()) {
        return &canvases_[*index];
    } else {
        return nullptr;
    }
}

const std::vector<WorkspaceAnnotations::Base64Image>& WorkspaceAnnotations::getCanvasImages()
    const {
    return canvases_;
}

WorkspaceAnnotations::ModifiedHandle WorkspaceAnnotations::onModified(
    const ModifiedCallback& callback) {
    return modified_.add(callback);
}

void WorkspaceAnnotations::invalidate(InvalidationLevel, Property*) { modified_.invoke(); }

}  // namespace inviwo

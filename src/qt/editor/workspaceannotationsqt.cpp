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

#include <inviwo/qt/editor/workspaceannotationsqt.h>

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <fmt/std.h>

namespace inviwo {

void WorkspaceAnnotationsQt::ProcessorShim::deserialize(Deserializer& d) {
    d.deserialize("type", type, SerializationTarget::Attribute);
    d.deserialize("identifier", identifier, SerializationTarget::Attribute);
    d.deserialize("displayName", displayName, SerializationTarget::Attribute);
}

namespace {

struct NetworkShim {
    void serialize([[maybe_unused]] Serializer& s) const {}
    void deserialize(Deserializer& d) { d.deserialize("Processors", processors, "Processor"); }
    std::vector<WorkspaceAnnotationsQt::ProcessorShim>& processors;
};

}  // namespace

WorkspaceAnnotationsQt::WorkspaceAnnotationsQt(InviwoApplication* app)
    : WorkspaceAnnotations(app) {};

WorkspaceAnnotationsQt::WorkspaceAnnotationsQt(
    const QImage& network, const std::vector<std::pair<std::string, QImage>>& canvasImages,
    InviwoApplication* app)
    : WorkspaceAnnotations(app) {
    setNetworkImage(network);
    setCanvasImages(canvasImages);
}

WorkspaceAnnotationsQt::WorkspaceAnnotationsQt(const std::filesystem::path& path,
                                               InviwoApplication* app)
    : WorkspaceAnnotations{app} {

    // Can't delegate to the WorkspaceAnnotations since the virtual call to deserialize will not
    // work in the base constructor.
    if (auto f = std::ifstream(path)) {
        LogFilter logger{LogCentral::getPtr(), LogVerbosity::None};
        auto d = app->getWorkspaceManager()->createWorkspaceDeserializer(f, path, &logger);
        d.deserialize("WorkspaceAnnotations", *this);

        processorList_.clear();
        processorCounts_.clear();

        NetworkShim dummy{processorList_};
        d.deserialize("ProcessorNetwork", dummy);

        for (const auto& p : dummy.processors) {
            ++processorCounts_[p.displayName];
        }
    } else {
        throw Exception(SourceContext{}, "Unable to open file {}", path);
    }
}

void WorkspaceAnnotationsQt::serialize(Serializer& s) const {
    WorkspaceAnnotations::serialize(s);

    s.serialize("Network", network_);
}

void WorkspaceAnnotationsQt::deserialize(Deserializer& d) {
    WorkspaceAnnotations::deserialize(d);

    network_ = Base64Image{"Network"};
    d.deserialize("Network", network_);
}

auto WorkspaceAnnotationsQt::getProcessorList() const -> const std::vector<ProcessorShim>& {
    return processorList_;
}
const std::map<std::string, int> WorkspaceAnnotationsQt::getProcessorCounts() const {
    return processorCounts_;
}

void WorkspaceAnnotationsQt::setNetworkImage(const QImage& network) {
    network_ = Base64Image{"Network"};
    if (!network.isNull()) {
        network_ = Base64Image{"Network", utilqt::toBase64(network, "JPEG", 95), network.width(),
                               network.height()};
    }
}

void WorkspaceAnnotationsQt::setCanvasImages(
    const std::vector<std::pair<std::string, QImage>>& canvasImages) {
    std::vector<Base64Image> images;
    images.reserve(canvasImages.size());

    for (auto& elem : canvasImages) {
        images.push_back({elem.first, utilqt::toBase64(elem.second, "JPEG", 90),
                          elem.second.width(), elem.second.height()});
    }
    setCanvasImages(images);
}

const WorkspaceAnnotationsQt::Base64Image& WorkspaceAnnotationsQt::getNetworkImage() const {
    return network_;
}

const QImage& WorkspaceAnnotationsQt::getNetworkQImage() const {
    if (networkCache_.isNull()) {
        networkCache_ = utilqt::fromBase64(network_.base64jpeg, "JPEG");
    }
    return networkCache_;
}
const QImage& WorkspaceAnnotationsQt::getCanvasQImage(size_t i) const {
    if (i < canvases_.size()) {
        if (imageCache_.size() != canvases_.size()) {
            imageCache_.resize(canvases_.size());
        }
        if (imageCache_[i].isNull()) {
            imageCache_[i] = utilqt::fromBase64(canvases_[i].base64jpeg, "JPEG");
        }
        return imageCache_[i];
    } else {
        return getMissingImage();
    }
}

const QImage& WorkspaceAnnotationsQt::getPrimaryCanvasQImage() const {
    if (auto index = getPrimaryCanvasIndex()) {
        return getCanvasQImage(*index);
    } else {
        return getMissingImage();
    }
}

const QImage& WorkspaceAnnotationsQt::getMissingImage() {
    static QImage img{":/inviwo/inviwo-logo-light.svg"};
    return img;
}

}  // namespace inviwo

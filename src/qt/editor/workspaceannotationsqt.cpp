/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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

WorkspaceAnnotationsQt::WorkspaceAnnotationsQt(InviwoApplication* app)
    : WorkspaceAnnotations(app){};

WorkspaceAnnotationsQt::WorkspaceAnnotationsQt(
    const QImage& network, const std::vector<std::pair<std::string, QImage>>& canvasImages,
    InviwoApplication* app)
    : WorkspaceAnnotations(app) {
    setNetworkImage(network);
    setCanvasImages(canvasImages);
}

WorkspaceAnnotationsQt::WorkspaceAnnotationsQt(const std::filesystem::path& path,
                                               InviwoApplication* app)
    : WorkspaceAnnotations{std::vector<Base64Image>{}, app} {

    // Can't delegate to the WorkspaceAnnotations since the virtual call to deserialize will not
    // work in the base constructor.
    if (auto f = filesystem::ifstream(path)) {
        LogFilter logger{LogCentral::getPtr(), LogVerbosity::None};
        auto d = app->getWorkspaceManager()->createWorkspaceDeserializer(f, path, &logger);
        d.deserialize("WorkspaceAnnotations", *this);
    } else {
        throw Exception(IVW_CONTEXT, "Unable to open file {}", path);
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

QImage WorkspaceAnnotationsQt::getNetworkQImage() const {
    return utilqt::fromBase64(network_.base64jpeg, "JPEG");
}
QImage WorkspaceAnnotationsQt::getCanvasQImage(size_t i) const {
    return utilqt::fromBase64(canvases_[i].base64jpeg, "JPEG");
}

QImage WorkspaceAnnotationsQt::getPrimaryCanvasQImage() const {
    if (auto img = getPrimaryCanvasImage()) {
        return utilqt::fromBase64(img->base64jpeg, "JPEG");
    } else {
        return QImage{};
    }
}

namespace {

struct DummyProcessor : Serializable {
    virtual void serialize([[maybe_unused]] Serializer& s) const override {}
    virtual void deserialize(Deserializer& d) override {
        d.deserialize("type", type, SerializationTarget::Attribute);
        d.deserialize("identifier", identifier, SerializationTarget::Attribute);
        d.deserialize("displayName", displayName, SerializationTarget::Attribute);
    }
    std::string type;
    std::string identifier;
    std::string displayName;
};

struct DummyNetwork : Serializable {

    virtual void serialize([[maybe_unused]] Serializer& s) const override {}
    virtual void deserialize(Deserializer& d) override {
        d.deserialize("Processors", processors, "Processor");
    }
    std::vector<DummyProcessor> processors;
};

}  // namespace

QStringList WorkspaceAnnotationsQt::workspaceProcessors(const std::filesystem::path& path,
                                                        InviwoApplication* app) {

    if (auto f = filesystem::ifstream(path)) {
        LogFilter logger{LogCentral::getPtr(), LogVerbosity::None};
        auto d = app->getWorkspaceManager()->createWorkspaceDeserializer(f, path, &logger);

        DummyNetwork dummy;
        d.deserialize("ProcessorNetwork", dummy);

        QStringList list{};

        for (const auto& p : dummy.processors) {
            list << utilqt::toQString(p.type) << utilqt::toQString(p.identifier)
                 << utilqt::toQString(p.displayName);
        }
        list.removeDuplicates();
        return list;

    } else {
        throw Exception(IVW_CONTEXT_CUSTOM("WorkspaceAnnotationsQt"), "Unable to open file {}",
                        path);
    }
}

}  // namespace inviwo

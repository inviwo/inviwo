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

#include <inviwo/qt/editor/workspacepreview.h>

#include <modules/qtwidgets/inviwoqtutils.h>

namespace inviwo {

WorkspacePreview::Item::Item(std::string name, std::string base64img, int w, int h)
    : name(name), base64img(base64img), size{w, h} {}

bool WorkspacePreview::Item::isValid() const {
    return !base64img.empty() && glm::compMul(size) > 0;
}

void WorkspacePreview::Item::serialize(Serializer &s) const {
    s.serialize("name", name);
    s.serialize("size", size);
    s.serialize("base64", base64img);
}

void WorkspacePreview::Item::deserialize(Deserializer &d) {
    d.deserialize("name", name);
    d.deserialize("size", size);
    d.deserialize("base64", base64img);
}

WorkspacePreview::WorkspacePreview(const QImage &network, const ImageVector &canvases) {
    if (!network.isNull()) {
        network_ = Item{"Network", utilqt::toBase64(network), network.width(), network.height()};
    }
    for (auto &elem : canvases) {
        if (!elem.second.isNull()) {
            canvases_.push_back({elem.first, utilqt::toBase64(elem.second), elem.second.width(),
                                 elem.second.height()});
        }
    }
}

void WorkspacePreview::serialize(Serializer &s) const {
    s.serialize("Network", network_);
    s.serialize("Canvases", canvases_, "CanvasImage");
}

void WorkspacePreview::deserialize(Deserializer &d) {
    network_ = Item{};
    canvases_.clear();

    d.deserialize("Network", network_);
    d.deserialize("Canvases", canvases_, "CanvasImage");
}

const WorkspacePreview::Item &WorkspacePreview::getNetworkImage() const { return network_; }

const std::vector<WorkspacePreview::Item> WorkspacePreview::getCanvases() const {
    return canvases_;
}

}  // namespace inviwo

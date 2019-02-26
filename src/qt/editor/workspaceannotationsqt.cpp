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

#include <inviwo/qt/editor/workspaceannotationsqt.h>

#include <modules/qtwidgets/inviwoqtutils.h>

namespace inviwo {

WorkspaceAnnotationsQt::WorkspaceAnnotationsQt(const QImage &network,
                                               const QImageVector &canvasImages) {
    setNetworkImage(network);
    setCanvasImages(canvasImages);
}

void WorkspaceAnnotationsQt::serialize(Serializer &s) const {
    WorkspaceAnnotations::serialize(s);

    s.serialize("Network", network_);
}

void WorkspaceAnnotationsQt::deserialize(Deserializer &d) {
    WorkspaceAnnotations::deserialize(d);

    network_ = Base64Image{"Network"};
    d.deserialize("Network", network_);
}

void WorkspaceAnnotationsQt::setNetworkImage(const QImage &network) {
    network_ = Base64Image{"Network"};

    if (!network.isNull()) {
        network_ = Base64Image{"Network", utilqt::toBase64(network, "JPEG", 95), network.width(),
                               network.height()};
    }
}

void WorkspaceAnnotationsQt::setCanvasImages(const QImageVector &canvasImages) {
    ImageVector images;
    images.reserve(canvasImages.size());

    for (auto &elem : canvasImages) {
        images.push_back({elem.first, utilqt::toBase64(elem.second, "JPEG", 90),
                          elem.second.width(), elem.second.height()});
    }
    setCanvasImages(images);
}

const WorkspaceAnnotationsQt::Base64Image &WorkspaceAnnotationsQt::getNetworkImage() const {
    return network_;
}

}  // namespace inviwo

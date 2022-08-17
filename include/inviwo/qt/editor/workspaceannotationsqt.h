/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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
#pragma once

#include <inviwo/qt/editor/inviwoqteditordefine.h>

#include <inviwo/core/network/workspaceannotations.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QImage>
#include <QStringList>
#include <warn/pop>

#include <vector>

namespace inviwo {

class IVW_QTEDITOR_API WorkspaceAnnotationsQt : public WorkspaceAnnotations {
public:
    WorkspaceAnnotationsQt(InviwoApplication* app = InviwoApplication::getPtr());
    WorkspaceAnnotationsQt(const QImage& network,
                           const std::vector<std::pair<std::string, QImage>>& canvasImages,
                           InviwoApplication* app = InviwoApplication::getPtr());
    WorkspaceAnnotationsQt(std::string_view path,
                           InviwoApplication* app = InviwoApplication::getPtr());

    virtual ~WorkspaceAnnotationsQt() = default;

    void setNetworkImage(const QImage& network);
    const Base64Image& getNetworkImage() const;
    QImage getNetworkQImage() const;
    QImage getCanvasQImage(size_t i) const;
    QImage getPrimaryCanvasQImage() const;

    using WorkspaceAnnotations::setCanvasImages;
    void setCanvasImages(const std::vector<std::pair<std::string, QImage>>& canvasImages);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    static QStringList workspaceProcessors(std::string_view path,
                                           InviwoApplication* app = InviwoApplication::getPtr());

private:
    Base64Image network_;
};

}  // namespace inviwo

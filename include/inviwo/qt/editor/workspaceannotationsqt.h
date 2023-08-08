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
#pragma once

#include <inviwo/qt/editor/inviwoqteditordefine.h>

#include <inviwo/core/network/workspaceannotations.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QImage>
#include <QStringList>
#include <QMetaType>
#include <warn/pop>

#include <vector>
#include <filesystem>
#include <map>

namespace inviwo {

class IVW_QTEDITOR_API WorkspaceAnnotationsQt : public WorkspaceAnnotations {
    struct ProcessorId {
        std::string type;
        std::string identifier;
        std::string displayName;
    };

public:
    WorkspaceAnnotationsQt(InviwoApplication* app = util::getInviwoApplication());
    WorkspaceAnnotationsQt(const QImage& network,
                           const std::vector<std::pair<std::string, QImage>>& canvasImages,
                           InviwoApplication* app = util::getInviwoApplication());
    WorkspaceAnnotationsQt(const std::filesystem::path& path,
                           InviwoApplication* app = util::getInviwoApplication());

    virtual ~WorkspaceAnnotationsQt() = default;

    void setNetworkImage(const QImage& network);
    const Base64Image& getNetworkImage() const;
    QImage getNetworkQImage() const;
    QImage getCanvasQImage(size_t i) const;
    QImage getPrimaryCanvasQImage() const;
    QStringList getProcessorsQString() const;

    using WorkspaceAnnotations::setCanvasImages;
    void setCanvasImages(const std::vector<std::pair<std::string, QImage>>& canvasImages);

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    const std::vector<ProcessorId>& getProcessorList() const;
    const std::map<std::string, int> getProcessorCounts() const;

private:
    Base64Image network_;
    std::vector<ProcessorId> processorList_;
    std::map<std::string, int> processorCounts_;
};

}  // namespace inviwo

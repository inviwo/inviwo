/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/network/networkannotations.h>

#include <vector>
#include <optional>

class QGraphicsItem;

namespace inviwo {

class NetworkEditor;
class ProcessorNetwork;
class NetworkAnnotationGraphicsItem;

class IVW_QTEDITOR_API NetworkAnnotationsQt : public NetworkAnnotationsObserver {
public:
    explicit NetworkAnnotationsQt(NetworkEditor* editor);
    NetworkAnnotationsQt(const NetworkAnnotationsQt&) = delete;
    NetworkAnnotationsQt(NetworkAnnotationsQt&&) = delete;
    NetworkAnnotationsQt& operator=(const NetworkAnnotationsQt&) = delete;
    NetworkAnnotationsQt& operator=(NetworkAnnotationsQt&&) = delete;

    virtual ~NetworkAnnotationsQt();

    NetworkAnnotationGraphicsItem* getGraphicsItem(size_t index);
    const NetworkAnnotationGraphicsItem* getGraphicsItem(size_t index) const;

    std::optional<size_t> getIndex(const QGraphicsItem* item) const;

    void showAnnotationDetails(NetworkAnnotationGraphicsItem* item);
    void hideAnnotationDetails(NetworkAnnotationGraphicsItem* item);

private:
    virtual void onNetworkAnnotationAdded(NetworkAnnotation&, size_t) override;
    virtual void onNetworkAnnotationWasRemoved(NetworkAnnotation&, size_t) override;
    virtual void onNetworkAnnotationChanged(NetworkAnnotation&, size_t) override;

    NetworkEditor* editor_;
    std::vector<NetworkAnnotationGraphicsItem*> annotationGraphicItems_;
};

}  // namespace inviwo

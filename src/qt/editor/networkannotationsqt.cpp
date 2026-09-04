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

#include <inviwo/qt/editor/networkannotationsqt.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/networkannotationgraphicsitem.h>

#include <memory>
#include <algorithm>

#include <QScrollArea>
#include <QVBoxLayout>

namespace inviwo {

NetworkAnnotationsQt::NetworkAnnotationsQt(NetworkEditor* editor) : editor_{editor} {}

NetworkAnnotationsQt::~NetworkAnnotationsQt() = default;

NetworkAnnotationGraphicsItem* NetworkAnnotationsQt::getGraphicsItem(size_t index) {
    if (index >= annotationGraphicItems_.size()) {
        throw RangeException{SourceContext{}, "NetworkAnnotation index {} out of range", index};
    }
    return annotationGraphicItems_[index];
}

const NetworkAnnotationGraphicsItem* NetworkAnnotationsQt::getGraphicsItem(size_t index) const {
    if (index >= annotationGraphicItems_.size()) {
        throw RangeException{SourceContext{}, "NetworkAnnotation index {} out of range", index};
    }
    return annotationGraphicItems_[index];
}

void NetworkAnnotationsQt::onNetworkAnnotationAdded(NetworkAnnotation& annotation, size_t index) {
    if (index < annotationGraphicItems_.size()) {
        throw Exception{SourceContext{}, "GraphicsItem exists already for NetworkAnnotation {}",
                        index};
    }
    if (index > annotationGraphicItems_.size()) {
        throw RangeException{SourceContext{}, "Unexpected NetworkAnnotation index {}, expected {}",
                             index, annotationGraphicItems_.size()};
    }

    annotationGraphicItems_.emplace_back(
        new NetworkAnnotationGraphicsItem{editor_->getNetwork(), annotation});
    editor_->addItem(annotationGraphicItems_.back());
    editor_->updateSceneSize();
    editor_->ensureVisible(annotationGraphicItems_.back());
}

void NetworkAnnotationsQt::onNetworkAnnotationWasRemoved(NetworkAnnotation&, size_t index) {
    if (index >= annotationGraphicItems_.size()) {
        throw RangeException{SourceContext{}, "NetworkAnnotation index {} out of range", index};
    }

    auto* item = annotationGraphicItems_[index];
    editor_->removeItem(item);
    annotationGraphicItems_.erase(annotationGraphicItems_.begin() +
                                  static_cast<std::ptrdiff_t>(index));
    delete item;

    editor_->updateSceneSize();
}

void NetworkAnnotationsQt::onNetworkAnnotationChanged(NetworkAnnotation& annotation, size_t index) {
    if (index >= annotationGraphicItems_.size()) {
        throw RangeException{SourceContext{}, "NetworkAnnotation index {} out of range", index};
    }
    annotationGraphicItems_[index]->updateAnnotation(annotation);
}

std::optional<size_t> NetworkAnnotationsQt::getIndex(const QGraphicsItem* item) const {
    if (const auto* annotation = qgraphicsitem_cast<const NetworkAnnotationGraphicsItem*>(item)) {
        if (auto it = std::ranges::find(annotationGraphicItems_, annotation);
            it != annotationGraphicItems_.end()) {
            return static_cast<size_t>(std::distance(annotationGraphicItems_.begin(), it));
        }
    }
    return std::nullopt;
}

void NetworkAnnotationsQt::showAnnotationDetails(NetworkAnnotationGraphicsItem* item) {
    if (auto index = getIndex(item); index.has_value()) {
        editor_->showNetworkAnnotationDetails(*index);
    }
}

void NetworkAnnotationsQt::hideAnnotationDetails(NetworkAnnotationGraphicsItem* item) {
    if (auto index = getIndex(item); index.has_value()) {
        editor_->hideNetworkAnnotationDetails(*index);
    }
}

}  // namespace inviwo

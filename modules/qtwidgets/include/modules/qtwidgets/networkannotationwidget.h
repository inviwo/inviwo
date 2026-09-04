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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>

#include <inviwo/core/network/networkannotations.h>
#include <inviwo/core/network/workspacemanager.h>

#include <modules/qtwidgets/inviwodockwidget.h>
#include <modules/qtwidgets/numberwidget.h>

#include <optional>

class QVBoxLayout;
class QScrollArea;
class QToolButton;
class QColorDialog;
class QComboBox;

namespace inviwo {

class ProcessorNetwork;
class LineEditQt;
class EditorDockWidget;

class IVW_MODULE_QTWIDGETS_API NetworkAnnotationWidget : public InviwoDockWidget {
    Q_OBJECT
public:
    explicit NetworkAnnotationWidget(ProcessorNetwork* network, QWidget* parent = nullptr);
    NetworkAnnotationWidget(const NetworkAnnotationWidget&) = delete;
    NetworkAnnotationWidget(NetworkAnnotationWidget&&) = delete;
    NetworkAnnotationWidget& operator=(const NetworkAnnotationWidget&) = delete;
    NetworkAnnotationWidget& operator=(NetworkAnnotationWidget&&) = delete;

    virtual ~NetworkAnnotationWidget();

    void setNetwork(ProcessorNetwork* network) { network_ = network; }

    void showAnnotation(size_t index, const NetworkAnnotation& annotation);
    void hideAnnotation(size_t index);
    void clear();

signals:
    void modifiedAnnotation(size_t index, const NetworkAnnotation& annotation);

private:
    void updateWidgets();
    void updateColorButton(QColor color);
    void openColorDialog();
    bool setDescription(const QString& text);

    QWidget* mainWidget_ = nullptr;
    QScrollArea* scrollArea_ = nullptr;

    ProcessorNetwork* network_;

    std::optional<size_t> annotationIndex_;
    NetworkAnnotation annotation_;

    LineEditQt* lineEditTitle_;
    LineEditQt* lineEditDesc_;
    QComboBox* comboAlignment_;
    NumberWidget<int>* numberTextWidth_;
    QToolButton* btnColor_;
    QColorDialog* colorDialog_;
    EditorDockWidget* editorDockWidget_;

    WorkspaceManager::ClearHandle annotationClearHandle_;
};

}  // namespace inviwo

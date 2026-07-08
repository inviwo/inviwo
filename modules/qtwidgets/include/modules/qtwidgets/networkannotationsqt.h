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
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <modules/qtwidgets/inviwodockwidget.h>

#include <optional>
#include <memory>

class QVBoxLayout;
class QScrollArea;

namespace inviwo {

class NetworkEditor;

class IVW_MODULE_QTWIDGETS_API NetworkAnnotationsQt : public InviwoDockWidget {
public:
    explicit NetworkAnnotationsQt(std::shared_ptr<NetworkAnnotations> annotations, QWidget* parent = nullptr);
    virtual ~NetworkAnnotationsQt();

    void setNetworkAnnotations(const std::shared_ptr<NetworkAnnotations>& annotations);

    void showAnnotation(size_t index);
    void hideAnnotation(size_t index);
    void clear();

private:
    void updateWidgets();

    QWidget* mainWidget_ = nullptr;
    QVBoxLayout* layout_ = nullptr;
    QScrollArea* scrollArea_ = nullptr;

    std::shared_ptr<NetworkAnnotations> annotations_;
    std::optional<size_t> annotationIndex_;

    StringProperty title_;
    FloatVec3Property color_;
    StringProperty markdown_;
    OptionProperty<Description::TextAlignment> alignment_;
    IntProperty textWidth_;
};

}  // namespace inviwo

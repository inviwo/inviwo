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

#include <modules/qtwidgets/networkannotationsqt.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networkannotations.h>
#include <modules/qtwidgets/inviwoqtutils.h>

// #include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>
//  #include <modules/qtwidgets/properties/boolpropertywidgetqt.h>
#include <modules/qtwidgets/properties/stringpropertywidgetqt.h>
#include <modules/qtwidgets/properties/optionpropertywidgetqt.h>
#include <modules/qtwidgets/properties/ordinalpropertywidgetqt.h>

#include <memory>

#include <QScrollArea>
#include <QVBoxLayout>

namespace inviwo {

NetworkAnnotationsQt::NetworkAnnotationsQt(std::shared_ptr<NetworkAnnotations> annotations,
                                           QWidget* parent)
    : InviwoDockWidget{tr("Network Annotations"), parent, "NetworkAnnotationsWidget"}
    , annotations_{annotations}
    , title_{"title", "Title"}
    , color_{"color", "Color",
             util::ordinalColor(vec3{0.6f, 0.6f, 0.65f})
                 .set("Color of the annotation"_help)
                 .set(PropertySemantics::TextEditor)}
    , markdown_{"description",
                "Description",
                "Description shown inside the annotation. Supports Markdown syntax."_help,
                "testing markdown\n*italics* **bold**",
                InvalidationLevel::InvalidOutput,
                PropertySemantics::Multiline}
    , alignment_{"alignment",
                 "Alignment",
                 "Text alignment and positioning of the description."_help,
                 {{"left", "Left", Description::TextAlignment::Left},
                  {"top", "Top", Description::TextAlignment::Top},
                  {"right", "Right", Description::TextAlignment::Right},
                  {"bottom", "Bottom", Description::TextAlignment::Bottom}}}
    , textWidth_{"textWidth", "Text Width (px)",
                 util::ordinalLength(100, 250).set(
                     "Width of the description in pixel. Used as height of the text block when "
                     "aligned at the bottom."_help)} {

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    resize(utilqt::emToPx(this, QSizeF(45, 80)));  // default size

    QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    sp.setVerticalStretch(1);
    sp.setHorizontalStretch(1);
    setSizePolicy(sp);

    const auto space = utilqt::refSpacePx(this);

    scrollArea_ = new QScrollArea(this);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setMinimumWidth(utilqt::emToPx(this, 30));
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#ifdef __APPLE__
    // Scrollbars are overlayed in different way on mac...
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#else
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
#endif
    scrollArea_->setFrameShape(QFrame::NoFrame);
    scrollArea_->setContentsMargins(0, space, 0, space);

    mainWidget_ = new QWidget{};
    layout_ = new QVBoxLayout{mainWidget_};
    layout_->setAlignment(Qt::AlignTop);

    const auto propertySpacing = utilqt::emToPx(mainWidget_, PropertyWidgetQt::spacingEm);
    layout_->setContentsMargins(propertySpacing * 2, propertySpacing, propertySpacing,
                                propertySpacing);
    layout_->setSpacing(propertySpacing);

    layout_->addWidget(new StringPropertyWidgetQt{&title_});
    layout_->addWidget(
        new OrdinalPropertyWidgetQt<vec3, OrdinalPropertyWidgetQtSemantics::Default>{&color_});
    layout_->addWidget(new StringPropertyWidgetQt{&markdown_});
    layout_->addWidget(new OptionPropertyWidgetQt{&alignment_});
    layout_->addWidget(
        new OrdinalPropertyWidgetQt<int, OrdinalPropertyWidgetQtSemantics::Default>{&textWidth_});

    scrollArea_->setWidget(mainWidget_);
    setWidget(scrollArea_);

    title_.onChange([this]() {
        if (annotationIndex_.has_value() && annotations_) {
            if (!title_.get().empty()) {
                annotations_->getAnnotation(*annotationIndex_).title = title_;
            } else {
                annotations_->getAnnotation(*annotationIndex_).title.reset();
            }
            annotations_->setModified(*annotationIndex_);
        }
    });
    color_.onChange([this]() {
        if (annotationIndex_.has_value() && annotations_) {
            annotations_->getAnnotation(*annotationIndex_).color = color_;
            annotations_->setModified(*annotationIndex_);
        }
    });
    markdown_.onChange([this]() {
        if (annotationIndex_.has_value() && annotations_) {
            if (!markdown_.get().empty()) {
                annotations_->getAnnotation(*annotationIndex_).description.markdown = markdown_;
            } else {
                annotations_->getAnnotation(*annotationIndex_).description.markdown.reset();
            }
            annotations_->setModified(*annotationIndex_);
        }
    });
    alignment_.onChange([this]() {
        if (annotationIndex_.has_value() && annotations_) {
            annotations_->getAnnotation(*annotationIndex_).description.alignment = alignment_;
            annotations_->setModified(*annotationIndex_);
        }
    });
    textWidth_.onChange([this]() {
        if (annotationIndex_.has_value() && annotations_) {
            annotations_->getAnnotation(*annotationIndex_).description.width = textWidth_;
            annotations_->setModified(*annotationIndex_);
        }
    });
}

NetworkAnnotationsQt::~NetworkAnnotationsQt() {
    // manual clean-up since the property widgets refer to properties owned by this class
    while (auto* item = layout_->takeAt(0)) {
        delete item->widget();
        delete item;
    }
}

void NetworkAnnotationsQt::setNetworkAnnotations(
    const std::shared_ptr<NetworkAnnotations>& annotations) {
    annotations_ = annotations;
}

void NetworkAnnotationsQt::showAnnotation(size_t index) {
    if (annotations_ && index < annotations_->size()) {
        annotationIndex_ = index;
    } else {
        annotationIndex_.reset();
    }
    updateWidgets();
    if (annotationIndex_.has_value()) {
        QWidget::raise();
    }
}

void NetworkAnnotationsQt::hideAnnotation(size_t index) {
    if (annotations_ && index == annotationIndex_) {
        annotationIndex_.reset();
        updateWidgets();
    }
}

void NetworkAnnotationsQt::clear() {
    annotationIndex_.reset();
    mainWidget_->setVisible(false);
}

void NetworkAnnotationsQt::updateWidgets() {
    mainWidget_->setVisible(annotationIndex_.has_value());
    if (!annotationIndex_.has_value() || !annotations_) return;

    const auto& annotation = annotations_->getAnnotation(*annotationIndex_);

    auto setAndDefault = [](auto& property, const auto& value) {
        property.set(value);
        property.setCurrentStateAsDefault();
    };
    setAndDefault(title_, annotation.title.value_or(""));
    setAndDefault(color_, annotation.color);
    setAndDefault(markdown_, annotation.description.markdown.value_or(""));
    setAndDefault(alignment_, annotation.description.alignment);
    setAndDefault(textWidth_, annotation.description.width);
}

}  // namespace inviwo

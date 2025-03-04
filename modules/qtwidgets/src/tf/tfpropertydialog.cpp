/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/qtwidgets/tf/tfpropertydialog.h>

#include <inviwo/core/common/inviwoapplication.h>      // for InviwoApplication
#include <inviwo/core/common/inviwoapplicationutil.h>  // for getInviwoApplication
#include <inviwo/core/util/moduleutils.h>
#include <inviwo/core/datastructures/datamapper.h>                // for DataMapper
#include <inviwo/core/datastructures/histogram.h>                 // for HistogramMode
#include <inviwo/core/datastructures/isovaluecollection.h>        // for IsoValueCollection
#include <inviwo/core/datastructures/tfprimitive.h>               // for TFPrimitive
#include <inviwo/core/datastructures/tfprimitiveset.h>            // for TFPrimitiveSet, TFPrimi...
#include <inviwo/core/datastructures/transferfunction.h>          // for TransferFunction
#include <inviwo/core/ports/volumeport.h>                         // for VolumeInport
#include <inviwo/core/processors/processor.h>                     // for Processor, Processor::N...
#include <inviwo/core/properties/isotfproperty.h>                 // for IsoTFProperty
#include <inviwo/core/properties/isovalueproperty.h>              // for IsoValueProperty
#include <inviwo/core/properties/property.h>                      // for Property
#include <inviwo/core/properties/propertyowner.h>                 // for PropertyOwner
#include <inviwo/core/properties/transferfunctionproperty.h>      // for TransferFunctionProperty
#include <inviwo/core/util/glmvec.h>                              // for dvec2, ivec2, vec2
#include <inviwo/core/util/stringconversion.h>                    // for toString
#include <inviwo/core/network/networklock.h>                      // for NetworkLock
#include <modules/qtwidgets/colorwheel.h>                         // for ColorWheel
#include <modules/qtwidgets/inviwodockwidgettitlebar.h>           // for InviwoDockWidgetTitleBar
#include <modules/qtwidgets/inviwoqtutils.h>                      // for emToPx, refSpacePx, toQ...
#include <modules/qtwidgets/properties/propertyeditorwidgetqt.h>  // for PropertyEditorWidgetQt
#include <modules/qtwidgets/qtwidgetsmodule.h>                    // for QtWidgetsModule
#include <modules/qtwidgets/rangesliderqt.h>                      // for RangeSliderQt
#include <modules/qtwidgets/tf/tfcoloredit.h>                     // for TFColorEdit
#include <modules/qtwidgets/tf/tfeditor.h>                        // for TFEditor
#include <modules/qtwidgets/tf/tfeditorview.h>                    // for TFEditorView
#include <modules/qtwidgets/tf/tflineedit.h>                      // for TFLineEdit
#include <modules/qtwidgets/tf/tfpropertyconcept.h>               // for TFPropertyModel, TFProp...
#include <modules/qtwidgets/tf/tfselectionwatcher.h>              // for TFSelectionWatcher
#include <modules/qtwidgets/tf/tfmovemode.h>

#include <algorithm>    // for all_of
#include <string_view>  // for string_view
#include <type_traits>  // for remove_extent_t
#include <utility>      // for move

#include <QColor>          // for QColor
#include <QColorDialog>    // for QColorDialog, QColorDia...
#include <QComboBox>       // for QComboBox
#include <QFrame>          // for QFrame
#include <QGridLayout>     // for QGridLayout
#include <QHBoxLayout>     // for QHBoxLayout
#include <QIcon>           // for QIcon
#include <QLabel>          // for QLabel
#include <QLayout>         // for QLayout
#include <QObject>         // for QObject
#include <QPoint>          // for QPoint
#include <QPolygonF>       // for QPolygonF
#include <QRect>           // for QRect
#include <QRectF>          // for QRectF
#include <QSettings>       // for QSettings
#include <QSignalBlocker>  // for QSignalBlocker
#include <QSizeF>          // for QSizeF
#include <QSizePolicy>     // for QSizePolicy
#include <QString>         // for QString
#include <QToolButton>     // for QToolButton
#include <QVBoxLayout>     // for QVBoxLayout
#include <QVariant>        // for QVariant
#include <QWidget>         // for QWidget
#include <Qt>              // for operator|, ScrollBarAlw...
#include <fmt/core.h>      // for format
#include <glm/common.hpp>  // for mix
#include <glm/vec2.hpp>    // for vec<>::(anonymous)

class QHBoxLayout;
class QVBoxLayout;

namespace inviwo {

TFPropertyDialog::TFPropertyDialog(TransferFunctionProperty* property)
    : TFPropertyDialog(std::make_unique<TFPropertyModel<TransferFunctionProperty>>(property)) {}

TFPropertyDialog::TFPropertyDialog(IsoValueProperty* property)
    : TFPropertyDialog(std::make_unique<TFPropertyModel<IsoValueProperty>>(property)) {}

TFPropertyDialog::TFPropertyDialog(IsoTFProperty* property)
    : TFPropertyDialog(std::make_unique<TFPropertyModel<IsoTFProperty>>(property)) {}

TFPropertyDialog::TFPropertyDialog(std::unique_ptr<TFPropertyConcept> model)
    : PropertyEditorWidgetQt(model->getProperty(), "Transfer Function Editor", "TFEditorWidget")
    , preview_{new QLabel()}
    , concept_(std::move(model)) {

    if (auto titlebar = dynamic_cast<InviwoDockWidgetTitleBar*>(titleBarWidget())) {
        if (auto layout = dynamic_cast<QHBoxLayout*>(titlebar->layout())) {
            QToolButton* helpBtn = new QToolButton();
            helpBtn->setIcon(QIcon(":/svgicons/dock-help.svg"));
            const auto iconsize =
                utilqt::emToPx(this, QSizeF(titlebar->getIconSize(), titlebar->getIconSize()));
            helpBtn->setIconSize(iconsize);
            helpBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
            layout->insertWidget(1, helpBtn);

            if (auto* qtModule = util::getModuleByType<QtWidgetsModule>()) {
                QObject::connect(helpBtn, &QToolButton::clicked, this,
                                 [qtModule]() { qtModule->showTFHelpWindow(); });
            }
        }
    }

    if (auto owner = concept_->getProperty()->getOwner()) {
        if (auto p = owner->getProcessor()) {
            onNameChange_ = p->onDisplayNameChange(
                [this](std::string_view, std::string_view) { updateTitleFromProperty(); });
        }
    }

    preview_ = new QLabel();
    preview_->setMinimumSize(1, 20);
    QSizePolicy sliderPol = preview_->sizePolicy();
    sliderPol.setHorizontalStretch(3);
    preview_->setSizePolicy(sliderPol);

    concept_->addObserver(this);
    for (auto* set : concept_->sets()) {
        set->addObserver(this);
    }

    editor_ = std::make_unique<TFEditor>(concept_.get(), this);
    tfSelectionWatcher_ =
        std::make_unique<TFSelectionWatcher>(concept_->getProperty(), concept_->sets());

    connect(editor_.get(), &TFEditor::selectionChanged, this,
            [this]() { tfSelectionWatcher_->updateSelection(editor_->getSelectedPrimitives()); });

    connect(editor_.get(), &TFEditor::updateBegin, this, [&]() { ongoingUpdate_ = true; });
    connect(editor_.get(), &TFEditor::updateEnd, this, [&]() {
        ongoingUpdate_ = false;
        updateTFPreview();
    });

    view_ = new TFEditorView(concept_.get(), editor_.get());
    view_->setFocusPolicy(Qt::StrongFocus);
    view_->scale(1.0, -1.0);  // put origin to bottom left corner
    view_->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    view_->setMinimumSize(255, 100);
    view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    zoomVSlider_ = new RangeSliderQt(Qt::Vertical, this, true);
    zoomVSlider_->setRange(0, verticalSliderRange_);
    zoomVSlider_->setMinSeparation(5);
    // flip slider values to compensate for vertical slider layout
    onZoomVChange(concept_->getZoomV());
    connect(zoomVSlider_, &RangeSliderQt::valuesChanged, this,
            &TFPropertyDialog::changeVerticalZoom);

    zoomVSlider_->setTooltipFormat([range = verticalSliderRange_](int /*handle*/, int val) {
        return toString(1.0f - static_cast<float>(val) / range);
    });

    zoomHSlider_ = new RangeSliderQt(Qt::Horizontal, this, true);
    zoomHSlider_->setRange(0, sliderRange_);
    zoomHSlider_->setMinSeparation(5);
    onZoomHChange(concept_->getZoomH());
    connect(zoomHSlider_, &RangeSliderQt::valuesChanged, this,
            &TFPropertyDialog::changeHorizontalZoom);

    zoomHSlider_->setTooltipFormat([range = sliderRange_](int /*handle*/, int val) {
        return toString(static_cast<float>(val) / range);
    });

    // set up color wheel
    {
        colorWheel_ = std::make_unique<ColorWheel>(QSize(150, 150));
        connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetColor,
                colorWheel_.get(), [cw = colorWheel_.get()](const QColor& c, bool /*ambiguous*/) {
                    QSignalBlocker block(cw);
                    cw->setColor(c);
                });
        connect(colorWheel_.get(), &ColorWheel::colorChange, tfSelectionWatcher_.get(),
                &TFSelectionWatcher::setColor);
    }

    chkShowHistogram_ = new QComboBox();
    chkShowHistogram_->addItem("Histogram: Off");
    chkShowHistogram_->addItem("Histogram: 100%");
    chkShowHistogram_->addItem("Histogram: 99%");
    chkShowHistogram_->addItem("Histogram: 95%");
    chkShowHistogram_->addItem("Histogram: 90%");
    chkShowHistogram_->addItem("Histogram: Log");
    chkShowHistogram_->setCurrentIndex(static_cast<int>(concept_->getHistogramMode()));
    connect(chkShowHistogram_,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this](int i) { concept_->setHistogramMode(static_cast<HistogramMode>(i)); });

    pointMoveMode_ = new QComboBox();
    pointMoveMode_->addItem("Point Movement: Free");
    pointMoveMode_->addItem("Point Movement: Restrict");
    pointMoveMode_->addItem("Point Movement: Push");
    pointMoveMode_->setCurrentIndex(0);
    connect(pointMoveMode_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int i) { editor_->setMoveMode(static_cast<TFMoveMode>(i)); });
    connect(editor_.get(), &TFEditor::moveModeChange, this,
            [this](TFMoveMode m) { pointMoveMode_->setCurrentIndex(static_cast<int>(m)); });

    domainMin_ = new QLabel("0.0");
    domainMax_ = new QLabel("1.0");

    const auto dataChange = [this]() {
        if (const auto* dataMap = concept_->getDataMap()) {
            domainMin_->setText(QString("%1").arg(dataMap->mapFromNormalizedToValue(0.0)));
            domainMax_->setText(QString("%1").arg(dataMap->mapFromNormalizedToValue(1.0)));
        } else {
            domainMin_->setText("0.0");
            domainMax_->setText("1.0");
        }
        // ensure that the range of primitive scalar is matching value range of volume data
        onTFTypeChangedInternal();
    };
    dataChangeHandle_ = concept_->onDataChange(dataChange);

    // set up TF primitive widgets
    primitivePos_ = new TFLineEdit();
    connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetPosition, primitivePos_,
            &TFLineEdit::setValue);
    connect(primitivePos_, &TFLineEdit::valueChanged, tfSelectionWatcher_.get(),
            &TFSelectionWatcher::setPosition);

    primitiveAlpha_ = new TFLineEdit();
    // only accept values in [0, 1]
    primitiveAlpha_->setValidRange(dvec2(0.0, 1.0), 0.0001);
    connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetAlpha, primitiveAlpha_,
            &TFLineEdit::setValue);
    connect(primitiveAlpha_, &TFLineEdit::valueChanged, tfSelectionWatcher_.get(),
            &TFSelectionWatcher::setAlpha);

    primitiveColor_ = new TFColorEdit();
    primitiveColor_->setColor(QColor(Qt::black), true);
    connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetColor, primitiveColor_,
            &TFColorEdit::setColor);
    connect(primitiveColor_, &TFColorEdit::colorChanged, tfSelectionWatcher_.get(),
            &TFSelectionWatcher::setColor);

    dataChange();

    QFrame* leftPanel = new QFrame(this);
    QGridLayout* leftLayout = new QGridLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(utilqt::refSpacePx(this));
    leftLayout->addWidget(zoomVSlider_, 0, 0);
    leftLayout->addWidget(view_, 0, 1);
    leftLayout->addWidget(zoomHSlider_, 1, 1);
    leftLayout->addWidget(preview_, 2, 1);
    leftPanel->setLayout(leftLayout);

    QFrame* rightPanel = new QFrame(this);
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(utilqt::refSpacePx(this));
    rightLayout->setAlignment(Qt::AlignTop);
    rightLayout->addWidget(chkShowHistogram_);
    rightLayout->addWidget(pointMoveMode_);
    rightLayout->addWidget(colorWheel_.get());

    auto primitivePropLayout = new QGridLayout();
    primitivePropLayout->addWidget(new QLabel("Scalar"), 0, 0);
    primitivePropLayout->addWidget(primitivePos_, 0, 1);
    primitivePropLayout->addWidget(new QLabel("Alpha"), 1, 0);
    primitivePropLayout->addWidget(primitiveAlpha_, 1, 1);
    primitivePropLayout->addWidget(new QLabel("Color"), 2, 0);
    primitivePropLayout->addWidget(primitiveColor_, 2, 1);

    primitivePropLayout->addWidget(new QLabel("Min"), 3, 0);
    primitivePropLayout->addWidget(domainMin_, 3, 1);

    primitivePropLayout->addWidget(new QLabel("Max"), 4, 0);
    primitivePropLayout->addWidget(domainMax_, 4, 1);

    rightLayout->addLayout(primitivePropLayout);

    rightLayout->addStretch(3);

    rightPanel->setLayout(rightLayout);

    QWidget* mainPanel = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout();
    const auto space = utilqt::refSpacePx(this);
    mainLayout->setContentsMargins(space, space, space, space);
    mainLayout->setSpacing(space);
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);
    mainPanel->setLayout(mainLayout);

    setWidget(mainPanel);

    // set up color dialog
    {
        colorDialog_ = std::make_unique<QColorDialog>(this);
        colorDialog_->hide();
        colorDialog_->setAttribute(Qt::WA_DeleteOnClose, false);
        // we don't want to see alpha in the color dialog
        colorDialog_->setOption(QColorDialog::ShowAlphaChannel, false);
        colorDialog_->setOption(QColorDialog::NoButtons, true);
        colorDialog_->setWindowModality(Qt::NonModal);
        colorDialog_->setWindowTitle(
            QString("TF Primitive Color - %1")
                .arg(utilqt::toQString(concept_->getProperty()->getDisplayName())));

        connect(editor_.get(), &TFEditor::showColorDialog, colorDialog_.get(),
                [dialog = colorDialog_.get()]() {
#ifdef __APPLE__
                    // OSX Bug workaround: hide the dialog, due to some Mac issues
                    dialog->hide();
#endif  // __APPLE__
                    dialog->show();
                });

        connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetColor,
                colorDialog_.get(),
                [dialog = colorDialog_.get()](const QColor& c, bool /*ambiguous*/) {
                    QSignalBlocker block(dialog);
                    if (c.isValid()) {
                        dialog->setCurrentColor(c);
                    } else {
                        // nothing selected
                        dialog->setCurrentColor(QColor("#95baff"));
                    }
                });
        connect(colorDialog_.get(), &QColorDialog::currentColorChanged, tfSelectionWatcher_.get(),
                &TFSelectionWatcher::setColor);

        colorDialog_->installEventFilter(new utilqt::WidgetCloseEventFilter(this));
    }

    // ensure that the TF dialog has its minimal size when showing up for the first time
    resize(utilqt::emToPx(this, 14.0), utilqt::emToPx(this, 12.0));

    {
        // make sure the help dialog for the TF editor is shown once
        QSettings settings;
        settings.beginGroup(objectName());
        if (!settings.contains("shownonce") || !settings.value("shownonce").toBool()) {
            settings.setValue("shownonce", true);

            auto* app = util::getInviwoApplication(concept_->getProperty());
            util::getModuleByTypeOrThrow<QtWidgetsModule>(app).showTFHelpWindow();
        }
        settings.endGroup();
    }

    updateFromProperty();
    updateTitleFromProperty();

    loadState();
}

TFPropertyDialog::~TFPropertyDialog() {
    editor_->disconnect();
    hide();
}

Property* TFPropertyDialog::getProperty() const { return concept_->getProperty(); }

QSize TFPropertyDialog::minimumSizeHint() const { return TFPropertyDialog::sizeHint(); }

QSize TFPropertyDialog::sizeHint() const { return layout()->sizeHint(); }

void TFPropertyDialog::updateFromProperty() { updateTFPreview(); }

TFEditorView* TFPropertyDialog::getEditorView() const { return view_; }

void TFPropertyDialog::changeVerticalZoom(int zoomMin, int zoomMax) {
    // normalize zoom values, as sliders in TFPropertyDialog
    // have the range [0...100]
    // and flip/rescale values to compensate slider layout
    const auto zoomMaxF =
        static_cast<double>(verticalSliderRange_ - zoomMin) / verticalSliderRange_;
    const auto zoomMinF =
        static_cast<double>(verticalSliderRange_ - zoomMax) / verticalSliderRange_;
    concept_->setZoomV(zoomMinF, zoomMaxF);
}

void TFPropertyDialog::changeHorizontalZoom(int zoomMin, int zoomMax) {
    const auto zoomMinF = static_cast<double>(zoomMin) / sliderRange_;
    const auto zoomMaxF = static_cast<double>(zoomMax) / sliderRange_;
    concept_->setZoomH(zoomMinF, zoomMaxF);
}

void TFPropertyDialog::resizeEvent(QResizeEvent* event) {
    PropertyEditorWidgetQt::resizeEvent(event);
    updateTFPreview();
}

void TFPropertyDialog::showEvent(QShowEvent* event) {
    updateTFPreview();
    view_->update();
    PropertyEditorWidgetQt::showEvent(event);
}

void TFPropertyDialog::updateTitleFromProperty() {
    if (!getProperty()->getOwner()) return;

    const auto processorName = getProperty()->getOwner()->getProcessor()->getDisplayName();
    const auto windowTitle =
        fmt::format("Transfer Function Editor - {} ({}){}", getProperty()->getDisplayName(),
                    processorName, (getProperty()->getReadOnly() ? " - Read Only" : ""));
    setWindowTitle(utilqt::toQString(windowTitle));
}

void TFPropertyDialog::onSetDisplayName(Property*, const std::string&) {
    updateTitleFromProperty();
}

void TFPropertyDialog::onTFPrimitiveAdded(const TFPrimitiveSet&, TFPrimitive&) {
    updateFromProperty();
}

void TFPropertyDialog::onTFPrimitiveRemoved(const TFPrimitiveSet&, TFPrimitive&) {
    updateFromProperty();
}

void TFPropertyDialog::onTFPrimitiveChanged(const TFPrimitiveSet&, const TFPrimitive&) {
    updateFromProperty();
}

void TFPropertyDialog::onTFTypeChanged(const TFPrimitiveSet&, TFPrimitiveSetType) {
    onTFTypeChangedInternal();
}

void TFPropertyDialog::onTFMaskChanged(const TFPrimitiveSet&, dvec2) { updateFromProperty(); }

void TFPropertyDialog::onTFTypeChangedInternal() {
    // adjust value mapping in primitive widget for position
    dvec2 valueRange{0.0, 1.0};
    if (const auto* dataMap = concept_->getDataMap()) {
        valueRange = dataMap->valueRange;
    }

    // TODO: how to handle different TF types?
    // perform mapping only, if all TF are of relative type
    const bool allRelative = true;

    // make increment depending on the size of the underlying TF texture
    const double incr =
        concept_->hasTF()
            ? 1.0 / static_cast<double>(concept_->getTFProperty()->getLookUpTableSize())
            : 0.01;

    primitivePos_->setValueMapping(allRelative, valueRange, incr * (valueRange.y - valueRange.x));

    zoomHSlider_->setTooltipFormat([sliderRange = sliderRange_, valueRange](int, int val) {
        return toString(
            glm::mix(valueRange.x, valueRange.y, static_cast<double>(val) / sliderRange));
    });
}

void TFPropertyDialog::onZoomHChange(const dvec2& zoomH) {
    zoomHSlider_->setValue(static_cast<int>(zoomH.x * sliderRange_),
                           static_cast<int>(zoomH.y * sliderRange_));
}

void TFPropertyDialog::onZoomVChange(const dvec2& zoomV) {
    zoomVSlider_->setValue(verticalSliderRange_ - static_cast<int>(zoomV.y * verticalSliderRange_),
                           verticalSliderRange_ - static_cast<int>(zoomV.x * verticalSliderRange_));
}

void TFPropertyDialog::onHistogramModeChange(HistogramMode mode) {
    chkShowHistogram_->setCurrentIndex(static_cast<int>(mode));
}

void TFPropertyDialog::setReadOnly(bool readonly) {
    colorWheel_->setDisabled(readonly);
    view_->setDisabled(readonly);
    primitivePos_->setDisabled(readonly);
    primitiveAlpha_->setDisabled(readonly);
    primitiveColor_->setDisabled(readonly);
    pointMoveMode_->setDisabled(readonly);
}

void TFPropertyDialog::updateTFPreview() {
    if (ongoingUpdate_) return;

    auto pixmap = utilqt::toQPixmap(*concept_, QSize(preview_->width(), 20));
    preview_->setPixmap(pixmap);
}

}  // namespace inviwo

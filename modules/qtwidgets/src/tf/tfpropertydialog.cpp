/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/isotfproperty.h>

#include <modules/qtwidgets/properties/tfpropertywidgetqt.h>
#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>
#include <modules/qtwidgets/tf/tfeditorcontrolpoint.h>
#include <modules/qtwidgets/tf/tfselectionwatcher.h>
#include <modules/qtwidgets/tf/tflineedit.h>
#include <modules/qtwidgets/tf/tfcoloredit.h>
#include <modules/qtwidgets/rangesliderqt.h>
#include <modules/qtwidgets/colorwheel.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/inviwodockwidgettitlebar.h>
#include <modules/qtwidgets/qtwidgetsmodule.h>
#include <modules/qtwidgets/tfhelpwindow.h>

#include <modules/qtwidgets/tf/tfutils.h>

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QImage>
#include <QGraphicsItem>
#include <QPushButton>
#include <QSizePolicy>
#include <QComboBox>
#include <QGradientStops>
#include <QPixmap>
#include <QColorDialog>
#include <QGridLayout>
#include <QSignalBlocker>
#include <QHBoxLayout>
#include <QToolButton>
#include <QSettings>
#include <warn/pop>

namespace inviwo {

TFPropertyDialog::TFPropertyDialog(TransferFunctionProperty* property,
                                   const std::vector<TFPrimitiveSet*>& primitiveSets)
    : PropertyEditorWidgetQt(property, "Transfer Function Editor", "TFEditorWidget")
    , sliderRange_(static_cast<int>(property->get().getTextureSize()))
    , propertyPtr_(std::make_unique<util::TFPropertyModel<TransferFunctionProperty*>>(property))
    , tfSets_(primitiveSets) {
    if (tfSets_.empty()) {
        // no sets given, make sure that the primitive set of the property is used
        tfSets_.push_back(&property->get());
    }
    initializeDialog();
}

TFPropertyDialog::TFPropertyDialog(IsoValueProperty* property,
                                   const std::vector<TFPrimitiveSet*>& primitiveSets)
    : PropertyEditorWidgetQt(property, "Transfer Function Editor", "TFEditorWidget")
    , sliderRange_(1024)
    , propertyPtr_(std::make_unique<util::TFPropertyModel<IsoValueProperty*>>(property))
    , tfSets_(primitiveSets) {
    if (tfSets_.empty()) {
        // no sets given, make sure that the primitive set of the property is used
        tfSets_.push_back(&property->get());
    }
    initializeDialog();
}

TFPropertyDialog::TFPropertyDialog(IsoTFProperty* property,
                                   const std::vector<TFPrimitiveSet*>& primitiveSets)
    : PropertyEditorWidgetQt(property, "Transfer Function Editor", "TFEditorWidget")
    , sliderRange_(static_cast<int>(property->tf_.get().getTextureSize()))
    , propertyPtr_(std::make_unique<util::TFPropertyModel<IsoTFProperty*>>(property))
    , tfSets_(primitiveSets) {
    if (tfSets_.empty()) {
        // no sets given, make sure that the primitive sets of the property are used
        tfSets_.push_back(&property->tf_.get());
        tfSets_.push_back(&property->isovalues_.get());
    }
    initializeDialog();
}

TFPropertyDialog::~TFPropertyDialog() {
    tfEditor_->disconnect();
    hide();
}

void TFPropertyDialog::initializeDialog() {
    if (auto titlebar = dynamic_cast<InviwoDockWidgetTitleBar*>(titleBarWidget())) {
        if (auto layout = dynamic_cast<QHBoxLayout*>(titlebar->layout())) {
            QToolButton* helpBtn = new QToolButton();
            helpBtn->setIcon(QIcon(":/svgicons/dock-help.svg"));
            const auto iconsize =
                utilqt::emToPx(this, QSizeF(titlebar->getIconSize(), titlebar->getIconSize()));
            helpBtn->setIconSize(iconsize);
            layout->insertWidget(1, helpBtn);

            auto module = util::getInviwoApplication(property_)->getModuleByType<QtWidgetsModule>();
            QObject::connect(helpBtn, &QToolButton::clicked, this,
                             [module]() { module->showTFHelpWindow(); });
        }
    }

    propertyPtr_->addObserver(this);
    for (auto& tf : tfSets_) {
        tf->addObserver(this);
    }

    tfEditor_ = std::make_unique<TFEditor>(propertyPtr_.get(), tfSets_, this);
    tfSelectionWatcher_ = std::make_unique<TFSelectionWatcher>(tfEditor_.get(), property_, tfSets_);

    connect(tfEditor_.get(), &TFEditor::selectionChanged, this,
            [this]() { tfSelectionWatcher_->updateSelection(tfEditor_->getSelectedPrimitives()); });
    connect(tfEditor_.get(), &TFEditor::importTF, this,
            [&](auto& primitiveSet) { util::importFromFile(primitiveSet, this); });
    connect(tfEditor_.get(), &TFEditor::exportTF, this,
            [&](auto& primitiveSet) { util::exportToFile(primitiveSet, this); });

    tfEditorView_ = new TFEditorView(propertyPtr_.get(), tfEditor_.get());

    // put origin to bottom left corner
    ivec2 minEditorDims = vec2(255, 100);
    tfEditorView_->setFocusPolicy(Qt::StrongFocus);
    tfEditorView_->scale(1.0, -1.0);
    tfEditorView_->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    tfEditorView_->setMinimumSize(minEditorDims.x, minEditorDims.y);
    tfEditorView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tfEditorView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    zoomVSlider_ = new RangeSliderQt(Qt::Vertical, this, true);
    zoomVSlider_->setRange(0, sliderRange_);
    zoomVSlider_->setMinSeparation(5);
    // flip slider values to compensate for vertical slider layout
    onZoomVChange(propertyPtr_->getZoomV());
    connect(zoomVSlider_, &RangeSliderQt::valuesChanged, this,
            &TFPropertyDialog::changeVerticalZoom);

    zoomVSlider_->setTooltipFormat([range = sliderRange_](int /*handle*/, int val) {
        return toString(1.0f - static_cast<float>(val) / range);
    });

    zoomHSlider_ = new RangeSliderQt(Qt::Horizontal, this, true);
    zoomHSlider_->setRange(0, sliderRange_);
    zoomHSlider_->setMinSeparation(5);
    onZoomHChange(propertyPtr_->getZoomH());
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

    tfPreview_ = new QLabel();
    tfPreview_->setMinimumSize(1, 20);
    QSizePolicy sliderPol = tfPreview_->sizePolicy();
    sliderPol.setHorizontalStretch(3);
    tfPreview_->setSizePolicy(sliderPol);

    chkShowHistogram_ = new QComboBox();
    chkShowHistogram_->addItem("Histogram: Off");
    chkShowHistogram_->addItem("Histogram: 100%");
    chkShowHistogram_->addItem("Histogram: 99%");
    chkShowHistogram_->addItem("Histogram: 95%");
    chkShowHistogram_->addItem("Histogram: 90%");
    chkShowHistogram_->addItem("Histogram: Log");
    chkShowHistogram_->setCurrentIndex(static_cast<int>(propertyPtr_->getHistogramMode()));
    connect(chkShowHistogram_,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &TFPropertyDialog::showHistogram);

    pointMoveMode_ = new QComboBox();
    pointMoveMode_->addItem("Point Movement: Free");
    pointMoveMode_->addItem("Point Movement: Restrict");
    pointMoveMode_->addItem("Point Movement: Push");
    pointMoveMode_->setCurrentIndex(0);
    connect(pointMoveMode_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &TFPropertyDialog::changeMoveMode);

    // set up TF primitive widgets
    {
        primitivePos_ = new TFLineEdit();
        connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetPosition, primitivePos_,
                &TFLineEdit::setValue);
        connect(primitivePos_, &TFLineEdit::valueChanged, tfSelectionWatcher_.get(),
                &TFSelectionWatcher::setPosition);

        // ensure that the range of primitive scalar is matching value range of volume data
        if (auto port = propertyPtr_->getVolumeInport()) {
            const auto portChange = [this]() { onTFTypeChangedInternal(); };

            port->onChange(portChange);
            port->onConnect(portChange);
            port->onDisconnect(portChange);
        }
        // update value mapping for position widget with respect to TF type and port
        onTFTypeChangedInternal();

        primitiveAlpha_ = new TFLineEdit();
        // only accept values in [0, 1]
        primitiveAlpha_->setValidRange(dvec2(0.0, 1.0));
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
    }

    QFrame* leftPanel = new QFrame(this);
    QGridLayout* leftLayout = new QGridLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(utilqt::refSpacePx(this));
    leftLayout->addWidget(zoomVSlider_, 0, 0);
    leftLayout->addWidget(tfEditorView_, 0, 1);
    leftLayout->addWidget(zoomHSlider_, 1, 1);
    leftLayout->addWidget(tfPreview_, 2, 1);
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
    primitivePropLayout->addWidget(new QLabel("Scalar"), 1, 1);
    primitivePropLayout->addWidget(primitivePos_, 1, 2);
    primitivePropLayout->addWidget(new QLabel("Alpha"), 2, 1);
    primitivePropLayout->addWidget(primitiveAlpha_, 2, 2);
    primitivePropLayout->addWidget(new QLabel("Color"), 3, 1);
    primitivePropLayout->addWidget(primitiveColor_, 3, 2);
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
            QString("TF Primitive Color - %1").arg(utilqt::toQString(property_->getDisplayName())));

        connect(tfEditor_.get(), &TFEditor::showColorDialog, colorDialog_.get(),
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
    }

    // ensure that the TF dialog has its minimal size when showing up for the first time
    resize(utilqt::emToPx(this, 14.0), utilqt::emToPx(this, 12.0));

    {
        // make sure the help dialog for the TF editor is shown once
        QSettings settings;
        settings.beginGroup(objectName());
        if (!settings.contains("shownonce") || !settings.value("shownonce").toBool()) {
            settings.setValue("shownonce", true);

            util::getInviwoApplication(property_)
                ->getModuleByType<QtWidgetsModule>()
                ->showTFHelpWindow();
        }
        settings.endGroup();
    }

    updateFromProperty();
    if (!propertyPtr_->getVolumeInport()) {
        chkShowHistogram_->setVisible(false);
    }
    loadState();
}  // namespace inviwo

QSize TFPropertyDialog::minimumSizeHint() const { return TFPropertyDialog::sizeHint(); }

QSize TFPropertyDialog::sizeHint() const { return layout()->sizeHint(); }

void TFPropertyDialog::updateFromProperty() {
    if (!property_->getOwner()) return;

    auto processorName = property_->getOwner()->getProcessor()->getDisplayName();
    auto windowTitle =
        "Transfer Function Editor - " + property_->getDisplayName() + " (" + processorName + ")";
    if (property_->getReadOnly()) {
        windowTitle += " - Read Only";
    }
    setWindowTitle(utilqt::toQString(windowTitle));

    updateTFPreview();
}

TFEditorView* TFPropertyDialog::getEditorView() const { return tfEditorView_; }

void TFPropertyDialog::changeVerticalZoom(int zoomMin, int zoomMax) {
    // normalize zoom values, as sliders in TFPropertyDialog
    // have the range [0...100]
    // and flip/rescale values to compensate slider layout
    const auto zoomMaxF = static_cast<float>(sliderRange_ - zoomMin) / sliderRange_;
    const auto zoomMinF = static_cast<float>(sliderRange_ - zoomMax) / sliderRange_;

    propertyPtr_->setZoomV(zoomMinF, zoomMaxF);
    tfEditor_->setRelativeSceneOffset(getRelativeSceneOffset());
}

void TFPropertyDialog::changeHorizontalZoom(int zoomMin, int zoomMax) {
    const auto zoomMinF = static_cast<float>(zoomMin) / sliderRange_;
    const auto zoomMaxF = static_cast<float>(zoomMax) / sliderRange_;

    propertyPtr_->setZoomH(zoomMinF, zoomMaxF);
    tfEditor_->setRelativeSceneOffset(getRelativeSceneOffset());
}

void TFPropertyDialog::showHistogram(int type) {
    propertyPtr_->setHistogramMode(static_cast<HistogramMode>(type));
}

void TFPropertyDialog::resizeEvent(QResizeEvent* event) {
    PropertyEditorWidgetQt::resizeEvent(event);

    tfEditor_->setRelativeSceneOffset(getRelativeSceneOffset());

    updateTFPreview();
}

void TFPropertyDialog::showEvent(QShowEvent* event) {
    updateTFPreview();
    tfEditorView_->update();
    PropertyEditorWidgetQt::showEvent(event);
}

void TFPropertyDialog::onTFPrimitiveAdded(TFPrimitive& p) {
    tfEditor_->onControlPointAdded(p);
    updateFromProperty();
}

void TFPropertyDialog::onTFPrimitiveRemoved(TFPrimitive& p) {
    tfEditor_->onControlPointRemoved(p);
    updateFromProperty();
}

void TFPropertyDialog::onTFPrimitiveChanged(const TFPrimitive& p) {
    tfEditor_->onControlPointChanged(p);
    updateFromProperty();
}

void TFPropertyDialog::onTFTypeChanged(const TFPrimitiveSet&) { onTFTypeChangedInternal(); }

void TFPropertyDialog::onTFTypeChangedInternal() {
    // adjust value mapping in primitive widget for position
    dvec2 valueRange(0.0, 1.0);
    if (auto port = propertyPtr_->getVolumeInport()) {
        if (port->hasData()) {
            valueRange = port->getData()->dataMap_.valueRange;
        }
    }
    // TODO: how to handle different TF types?
    // perform mapping only, if all TF are of relative type
    const bool allRelative = std::all_of(tfSets_.begin(), tfSets_.end(), [](TFPrimitiveSet* elem) {
        return elem->getType() == TFPrimitiveSetType::Relative;
    });

    // make increment depending on the size of the underlying TF texture
    const double incr =
        propertyPtr_->hasTF()
            ? 1.0 / static_cast<double>(propertyPtr_->getTFProperty()->get().getTextureSize())
            : 0.01;

    primitivePos_->setValueMapping(allRelative, valueRange, incr * (valueRange.y - valueRange.x));

    zoomHSlider_->setTooltipFormat([sliderRange = sliderRange_, valueRange](int, int val) {
        return toString(
            glm::mix(valueRange.x, valueRange.y, static_cast<double>(val) / sliderRange));
    });
}

void TFPropertyDialog::onMaskChange(const dvec2&) { updateTFPreview(); }

void TFPropertyDialog::onZoomHChange(const dvec2& zoomH) {
    zoomHSlider_->setValue(static_cast<int>(zoomH.x * sliderRange_),
                           static_cast<int>(zoomH.y * sliderRange_));
}

void TFPropertyDialog::onZoomVChange(const dvec2& zoomV) {
    zoomVSlider_->setValue(sliderRange_ - static_cast<int>(zoomV.y * sliderRange_),
                           sliderRange_ - static_cast<int>(zoomV.x * sliderRange_));
}

void TFPropertyDialog::setReadOnly(bool readonly) {
    colorWheel_->setDisabled(readonly);
    tfEditorView_->setDisabled(readonly);
    primitivePos_->setDisabled(readonly);
    primitiveAlpha_->setDisabled(readonly);
    primitiveColor_->setDisabled(readonly);
    pointMoveMode_->setDisabled(readonly);
}

void TFPropertyDialog::changeMoveMode(int i) { tfEditor_->setMoveMode(i); }

void TFPropertyDialog::updateTFPreview() {
    auto pixmap = utilqt::toQPixmap(*propertyPtr_, QSize(tfPreview_->width(), 20));
    tfPreview_->setPixmap(pixmap);
}

dvec2 TFPropertyDialog::getRelativeSceneOffset() const {
    // to determine the offset in scene coords, map a square where each side has length
    // defaultOffset_ to the scene. We assume that there is no rotation or non-linear
    // view transformation.
    auto rect =
        tfEditorView_->mapToScene(QRect(QPoint(0, 0), QSize(defaultOffset_, defaultOffset_)))
            .boundingRect();

    return dvec2(rect.width(), rect.height());
}

}  // namespace inviwo

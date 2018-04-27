/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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
#include <modules/qtwidgets/properties/tfpropertywidgetqt.h>
#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>
#include <modules/qtwidgets/tf/tfeditorcontrolpoint.h>
#include <modules/qtwidgets/tf/tfselectionwatcher.h>
#include <modules/qtwidgets/tf/tflineedit.h>
#include <modules/qtwidgets/tf/tfcoloredit.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/rangesliderqt.h>
#include <modules/qtwidgets/colorwheel.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/inviwodockwidgettitlebar.h>
#include <modules/qtwidgets/qtwidgetsmodule.h>
#include <modules/qtwidgets/tfhelpwindow.h>

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datawriter.h>
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

TFPropertyDialog::TFPropertyDialog(TransferFunctionProperty* tfProperty)
    : PropertyEditorWidgetQt(tfProperty, "Transfer Function Editor", "TFEditorWidget")
    , sliderRange_(static_cast<int>(tfProperty->get().getTextureSize()))
    , tfProperty_(tfProperty)
    , tfEditor_(nullptr)
    , tfEditorView_(nullptr) {

    if (auto titlebar = dynamic_cast<InviwoDockWidgetTitleBar*>(titleBarWidget())) {
        if (auto layout = dynamic_cast<QHBoxLayout*>(titlebar->layout())) {
            QToolButton* helpBtn = new QToolButton();
            helpBtn->setIcon(QIcon(":/stylesheets/images/dock-help.png"));
            helpBtn->setObjectName("helpBtn");

            layout->insertWidget(1, helpBtn);

            auto module = util::getInviwoApplication(tfProperty)
                ->getModuleByType<QtWidgetsModule>();
            QObject::connect(helpBtn, &QToolButton::clicked, this, [this, module]() {
                module->showTFHelpWindow();
            });
        }
    }

    tfProperty->TransferFunctionPropertyObservable::addObserver(this);
    tfProperty_->get().addObserver(this);

    tfEditor_ = util::make_unique<TFEditor>(tfProperty_, this);
    tfSelectionWatcher_ = util::make_unique<TFSelectionWatcher>(tfEditor_.get(), tfProperty_);

    connect(tfEditor_.get(), &TFEditor::selectionChanged, this,
            [this]() { tfSelectionWatcher_->updateSelection(tfEditor_->getSelectedPrimitives()); });
    connect(tfEditor_.get(), &TFEditor::importTF, this, &TFPropertyDialog::importTransferFunction);
    connect(tfEditor_.get(), &TFEditor::exportTF, this, &TFPropertyDialog::exportTransferFunction);

    tfEditorView_ = new TFEditorView(tfProperty_);

    // put origin to bottom left corner
    ivec2 minEditorDims = vec2(255, 100);
    tfEditorView_->setFocusPolicy(Qt::StrongFocus);
    tfEditorView_->scale(1.0, -1.0);
    tfEditorView_->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    tfEditorView_->setMinimumSize(minEditorDims.x, minEditorDims.y);
    tfEditorView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tfEditorView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tfEditorView_->setScene(tfEditor_.get());

    zoomVSlider_ = new RangeSliderQt(Qt::Vertical, this, true);
    zoomVSlider_->setRange(0, sliderRange_);
    zoomVSlider_->setMinSeparation(5);
    // flip slider values to compensate for vertical slider layout
    onZoomVChange(tfProperty_->getZoomV());
    connect(zoomVSlider_, &RangeSliderQt::valuesChanged, this,
            &TFPropertyDialog::changeVerticalZoom);

    zoomVSlider_->setTooltipFormat([range = sliderRange_](int /*handle*/, int val) {
        return toString(1.0f - static_cast<float>(val) / range);
    });

    zoomHSlider_ = new RangeSliderQt(Qt::Horizontal, this, true);
    zoomHSlider_->setRange(0, sliderRange_);
    zoomHSlider_->setMinSeparation(5);
    onZoomHChange(tfProperty_->getZoomH());
    connect(zoomHSlider_, &RangeSliderQt::valuesChanged, this,
            &TFPropertyDialog::changeHorizontalZoom);

    zoomHSlider_->setTooltipFormat([range = sliderRange_](int /*handle*/, int val) {
        return toString(static_cast<float>(val) / range);
    });

    // set up color wheel
    {
        colorWheel_ = util::make_unique<ColorWheel>(QSize(150, 150));
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
    chkShowHistogram_->setCurrentIndex(static_cast<int>(tfProperty_->getHistogramMode()));
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
        if (auto port = tfProperty_->getVolumeInport()) {
            const auto portChange = [this, port]() {
                auto range =
                    port->hasData() ? port->getData()->dataMap_.valueRange : dvec2(0.0, 1.0);
                primitivePos_->setValueMapping(
                    tfProperty_->get().getType() == TFPrimitiveSetType::Relative, range);
            };

            port->onChange(portChange);
            port->onConnect(portChange);
            port->onDisconnect(portChange);
        }
        // update value mapping for position widget with respect to TF type and port
        onTFTypeChanged(nullptr);

        primitiveAlpha_ = new TFLineEdit();
        // only accept values in [0, 1]
        primitiveAlpha_->setValidRange(dvec2(0.0, 1.0));
        connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetAlpha, primitiveAlpha_,
                &TFLineEdit::setValue);
        connect(primitiveAlpha_, &TFLineEdit::valueChanged, tfSelectionWatcher_.get(),
                &TFSelectionWatcher::setAlpha);

        primitiveColor_ = new TFColorEdit();
        connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetColor, primitiveColor_,
                &TFColorEdit::setColor);
        connect(primitiveColor_, &TFColorEdit::colorChanged, tfSelectionWatcher_.get(),
                &TFSelectionWatcher::setColor);
    }

    QFrame* leftPanel = new QFrame(this);
    QGridLayout* leftLayout = new QGridLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(7);
    leftLayout->addWidget(zoomVSlider_, 0, 0);
    leftLayout->addWidget(tfEditorView_, 0, 1);
    leftLayout->addWidget(zoomHSlider_, 1, 1);
    leftLayout->addWidget(tfPreview_, 2, 1);
    leftPanel->setLayout(leftLayout);

    QFrame* rightPanel = new QFrame(this);
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(7);
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
    mainLayout->setContentsMargins(7, 7, 7, 7);
    mainLayout->setSpacing(7);
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);
    mainPanel->setLayout(mainLayout);

    setWidget(mainPanel);

    // set up color dialog
    {
        colorDialog_ = util::make_unique<QColorDialog>(this);
        colorDialog_->hide();
        // we don't want to see alpha in the color dialog
        colorDialog_->setOption(QColorDialog::ShowAlphaChannel, false);
        colorDialog_->setOption(QColorDialog::NoButtons, true);
        colorDialog_->setWindowModality(Qt::NonModal);
        colorDialog_->setWindowTitle(QString("TF Primitive Color - %1")
                                         .arg(utilqt::toQString(tfProperty_->getDisplayName())));

        connect(tfEditor_.get(), &TFEditor::showColorDialog,
                colorDialog_.get(), [dialog = colorDialog_.get()]() {
#ifdef __APPLE__
                    // OSX Bug workaround: hide the dialog, due to some Mac issues
                    dialog->hide();
#endif  // __APPLE__
                    dialog->show();
                });

        connect(
            tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetColor,
            colorDialog_.get(), [dialog = colorDialog_.get()](const QColor& c, bool /*ambiguous*/) {
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
    resize(100, 100);

    {
        // make sure the help dialog for the TF editor is shown once
        QSettings settings;
        settings.beginGroup(objectName());
        if (!settings.contains("shownonce") || !settings.value("shownonce").toBool()) {
            settings.setValue("shownonce", true);

            util::getInviwoApplication(tfProperty)
                ->getModuleByType<QtWidgetsModule>()->showTFHelpWindow();
        }
        settings.endGroup();
    }

    updateFromProperty();
    if (!tfProperty_->getVolumeInport()) {
        chkShowHistogram_->setVisible(false);
    }
    loadState();
}

TFPropertyDialog::~TFPropertyDialog() {
    tfEditor_->disconnect();
    hide();
}

QSize TFPropertyDialog::minimumSizeHint() const {
    return TFPropertyDialog::sizeHint();
}

QSize TFPropertyDialog::sizeHint() const { return layout()->sizeHint(); }

void TFPropertyDialog::updateFromProperty() {
    if (!tfProperty_->getOwner()) return;

    auto processorName = tfProperty_->getOwner()->getProcessor()->getDisplayName();
    auto windowTitle =
        "Transfer Function Editor - " + tfProperty_->getDisplayName() + " (" + processorName + ")";
    setWindowTitle(utilqt::toQString(windowTitle));

    updateTFPreview();
}

void TFPropertyDialog::changeVerticalZoom(int zoomMin, int zoomMax) {
    // normalize zoom values, as sliders in TFPropertyDialog
    // have the range [0...100]
    // and flip/rescale values to compensate slider layout
    const auto zoomMaxF = static_cast<float>(sliderRange_ - zoomMin) / sliderRange_;
    const auto zoomMinF = static_cast<float>(sliderRange_ - zoomMax) / sliderRange_;

    tfProperty_->setZoomV(zoomMinF, zoomMaxF);
    tfEditor_->setRelativeSceneOffset(getRelativeSceneOffset());
}

void TFPropertyDialog::changeHorizontalZoom(int zoomMin, int zoomMax) {
    const auto zoomMinF = static_cast<float>(zoomMin) / sliderRange_;
    const auto zoomMaxF = static_cast<float>(zoomMax) / sliderRange_;

    tfProperty_->setZoomH(zoomMinF, zoomMaxF);
    tfEditor_->setRelativeSceneOffset(getRelativeSceneOffset());
}

void TFPropertyDialog::importTransferFunction() {
    InviwoFileDialog importFileDialog(this, "Import transfer function", "transferfunction");
    importFileDialog.setAcceptMode(AcceptMode::Open);
    importFileDialog.setFileMode(FileMode::ExistingFile);
    importFileDialog.addExtension("itf", "Inviwo Transfer Function");
    importFileDialog.addExtension("png", "Transfer Function Image");
    importFileDialog.addExtension("", "All files");  // this will add "All files (*)"

    if (importFileDialog.exec()) {
        QString file = importFileDialog.selectedFiles().at(0);
        try {
            NetworkLock lock(tfProperty_);
            tfProperty_->get().load(file.toLocal8Bit().constData(),
                                    importFileDialog.getSelectedFileExtension());
        } catch (DataReaderException& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
        }
    }
}

void TFPropertyDialog::exportTransferFunction() {
    InviwoFileDialog exportFileDialog(this, "Export transfer function", "transferfunction");
    exportFileDialog.setAcceptMode(AcceptMode::Save);
    exportFileDialog.setFileMode(FileMode::AnyFile);
    exportFileDialog.addExtension("itf", "Inviwo Transfer Function");
    exportFileDialog.addExtension("png", "Transfer Function Image");
    exportFileDialog.addExtension("", "All files");  // this will add "All files (*)"

    if (exportFileDialog.exec()) {
        const std::string file = exportFileDialog.selectedFiles().at(0).toLocal8Bit().constData();
        const auto fileExt = exportFileDialog.getSelectedFileExtension();
        try {
            tfProperty_->get().save(file, fileExt);
            util::log(IvwContext, "Data exported to disk: " + file, LogLevel::Info,
                      LogAudience::User);
        } catch (DataWriterException& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
        }
    }
}

void TFPropertyDialog::showHistogram(int type) {
    tfProperty_->setHistogramMode(static_cast<HistogramMode>(type));
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

void TFPropertyDialog::onTFPrimitiveAdded(TFPrimitive* p) {
    tfEditor_->onControlPointAdded(p);
    updateFromProperty();
}

void TFPropertyDialog::onTFPrimitiveRemoved(TFPrimitive* p) {
    tfEditor_->onControlPointRemoved(p);
    updateFromProperty();
}

void TFPropertyDialog::onTFPrimitiveChanged(const TFPrimitive* p) {
    tfEditor_->onControlPointChanged(p);
    updateFromProperty();
}

void TFPropertyDialog::onTFTypeChanged(const TFPrimitiveSet*) {
    // adjust value mapping in primitive widget for position
    dvec2 valueRange(0.0, 1.0);
    if (auto port = tfProperty_->getVolumeInport()) {
        if (port->hasData()) {
            valueRange = port->getData()->dataMap_.valueRange;
        }
    }
    primitivePos_->setValueMapping(tfProperty_->get().getType() == TFPrimitiveSetType::Relative,
                                   valueRange);
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

TFEditorView* TFPropertyDialog::getEditorView() const { return tfEditorView_; }

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
    auto pixmap = utilqt::toQPixmap(*tfProperty_, QSize(tfPreview_->width(), 20));
    tfPreview_->setPixmap(pixmap);
}

dvec2 TFPropertyDialog::getRelativeSceneOffset() const {
    // to determine the offset in scene coords, map a square where each side has length
    // defaultOffset_ to the scene. We assume that there is no rotation or non-linear view
    // transformation.
    auto rect =
        tfEditorView_->mapToScene(QRect(QPoint(0, 0), QSize(defaultOffset_, defaultOffset_)))
            .boundingRect();

    return dvec2(rect.width(), rect.height());
}

}  // namespace inviwo

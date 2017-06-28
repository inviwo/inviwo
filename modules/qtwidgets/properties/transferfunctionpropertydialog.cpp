/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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

#include <modules/qtwidgets/properties/transferfunctionpropertydialog.h>
#include <modules/qtwidgets/properties/transferfunctionpropertywidgetqt.h>
#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>
#include <modules/qtwidgets/properties/transferfunctioneditorcontrolpoint.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/colorwheel.h>
#include <modules/qtwidgets/rangesliderqt.h>

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datawriter.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QImage>
#include <QGraphicsItem>
#include <QPushButton>
#include <QSizePolicy>
#include <QComboBox>
#include <QGradientStops>
#include <QPixmap>
#include <warn/pop>

namespace inviwo {

TransferFunctionPropertyDialog::TransferFunctionPropertyDialog(TransferFunctionProperty* tfProperty,
                                                               QWidget* parent)
    : PropertyEditorWidgetQt(tfProperty, "Transfer Function Editor", parent)
    , TransferFunctionObserver()
    , sliderRange_(static_cast<int>(tfProperty->get().getTextureSize()))
    , tfProperty_(tfProperty)
    , tfEditor_(nullptr)
    , tfEditorView_(nullptr)
    , tfPixmap_(nullptr) 
    , gradient_(0, 0, 100, 20)
{
    tfProperty_->get().addObserver(this);

    std::stringstream ss;
    ss << "Transfer Function Editor - " << tfProperty_->getDisplayName() << "("
       << tfProperty_->getOwner()->getProcessor() << ")";
    setWindowTitle(ss.str().c_str());

    generateWidget();
    if (!tfProperty_->getVolumeInport()) chkShowHistogram_->setVisible(false);
    
    updateTFPreview();
    updateFromProperty();

    if (!tfProperty_->getVolumeInport()) chkShowHistogram_->setVisible(false);
}

TransferFunctionPropertyDialog::~TransferFunctionPropertyDialog() {
    tfEditor_->disconnect();
    hide();
}

void TransferFunctionPropertyDialog::generateWidget() {
    ivec2 minEditorDims = vec2(255, 100);

    tfEditor_ = util::make_unique<TransferFunctionEditor>(tfProperty_, this);

    auto updateColorWheel = [this](const QColor& color) {
        colorWheel_->blockSignals(true);
        colorWheel_->setColor(color);
        colorWheel_->blockSignals(false);
    };

    connect(tfEditor_.get(), &TransferFunctionEditor::colorChanged, updateColorWheel);

    tfEditorView_ = new TransferFunctionEditorView(tfProperty_);
    
    // put origin to bottom left corner
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
    zoomVSlider_->setValue(
        sliderRange_ - static_cast<int>(tfProperty_->getZoomV().y * sliderRange_),
        sliderRange_ - static_cast<int>(tfProperty_->getZoomV().x * sliderRange_));
    connect(zoomVSlider_, SIGNAL(valuesChanged(int, int)), this,
            SLOT(changeVerticalZoom(int, int)));

    zoomVSlider_->setTooltipFormat([range = sliderRange_](int handle, int val) {
        return toString(1.0f - static_cast<float>(val)/range);
    });

    zoomHSlider_ = new RangeSliderQt(Qt::Horizontal, this, true);
    zoomHSlider_->setRange(0, sliderRange_);
    zoomHSlider_->setMinSeparation(5);
    zoomHSlider_->setValue(static_cast<int>(tfProperty_->getZoomH().x * sliderRange_),
                           static_cast<int>(tfProperty_->getZoomH().y * sliderRange_));
    connect(zoomHSlider_, SIGNAL(valuesChanged(int, int)), this,
            SLOT(changeHorizontalZoom(int, int)));

    zoomHSlider_->setTooltipFormat([range = sliderRange_](int handle, int val) {
        return toString(static_cast<float>(val) / range);
    });

    maskSlider_ = new RangeSliderQt(Qt::Horizontal, this, true);
    maskSlider_->setRange(0, sliderRange_);
    maskSlider_->setValue(static_cast<int>(tfProperty_->getMask().x * sliderRange_),
                          static_cast<int>(tfProperty_->getMask().y * sliderRange_));
    connect(maskSlider_, SIGNAL(valuesChanged(int, int)), this, SLOT(changeMask(int, int)));
    maskSlider_->setTooltipFormat([range = sliderRange_](int handle, int val) {
        return toString(static_cast<float>(val) / range);
    });


    colorWheel_ = util::make_unique<ColorWheel>();
    connect(colorWheel_.get(), &ColorWheel::colorChange, 
            tfEditor_.get(), &TransferFunctionEditor::setPointColor);

    btnClearTF_ = new QPushButton("Reset");
    connect(btnClearTF_, &QPushButton::clicked, [this](){tfEditor_->resetTransferFunction();});
    btnClearTF_->setStyleSheet(QString("min-width: 30px; padding-left: 7px; padding-right: 7px;"));

    btnImportTF_ = new QPushButton("Import");
    connect(btnImportTF_, &QPushButton::clicked, [this](){importTransferFunction();});
    btnImportTF_->setStyleSheet(QString("min-width: 30px; padding-left: 7px; padding-right: 7px;"));

    btnExportTF_ = new QPushButton("Export");
    connect(btnExportTF_, &QPushButton::clicked, [this](){exportTransferFunction();});
    btnExportTF_->setStyleSheet(QString("min-width: 30px; padding-left: 7px; padding-right: 7px;"));

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
    connect(chkShowHistogram_, SIGNAL(currentIndexChanged(int)), this, SLOT(showHistogram(int)));

    pointMoveMode_ = new QComboBox();
    pointMoveMode_->addItem("Point Movement: Free");
    pointMoveMode_->addItem("Point Movement: Restrict");
    pointMoveMode_->addItem("Point Movement: Push");
    pointMoveMode_->setCurrentIndex(0);
    connect(pointMoveMode_, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMoveMode(int)));

    QFrame* leftPanel = new QFrame(this);
    QGridLayout* leftLayout = new QGridLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(7);
    leftLayout->addWidget(zoomVSlider_, 0, 0);
    leftLayout->addWidget(tfEditorView_, 0, 1);
    leftLayout->addWidget(zoomHSlider_, 1, 1);
    leftLayout->addWidget(tfPreview_, 2, 1);
    leftLayout->addWidget(maskSlider_, 3, 1);
    leftPanel->setLayout(leftLayout);

    QFrame* rightPanel = new QFrame(this);
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(7);
    rightLayout->setAlignment(Qt::AlignTop);
    rightLayout->addWidget(colorWheel_.get());
    rightLayout->addWidget(chkShowHistogram_);
    rightLayout->addWidget(pointMoveMode_);
    rightLayout->addStretch(3);
    QHBoxLayout* rowLayout = new QHBoxLayout();
    rowLayout->addWidget(btnClearTF_);
    rowLayout->addWidget(btnImportTF_);
    rowLayout->addWidget(btnExportTF_);
    rightLayout->addLayout(rowLayout);

    rightPanel->setLayout(rightLayout);

    QWidget* mainPanel = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(7, 7, 7, 7);
    mainLayout->setSpacing(7);
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);
    mainPanel->setLayout(mainLayout);

    setWidget(mainPanel);
}

void TransferFunctionPropertyDialog::updateTFPreview() {
    int gradientWidth = tfPreview_->width();
    gradient_.setFinalStop(gradientWidth, 0);

    if (!tfPixmap_ || gradientWidth != tfPixmap_->width()) {
        tfPixmap_ = util::make_unique<QPixmap>(gradientWidth, 20);
    }

    QPainter tfPainter(tfPixmap_.get());
    QPixmap checkerBoard(10, 10);
    QPainter checkerBoardPainter(&checkerBoard);
    checkerBoardPainter.fillRect(0, 0, 5, 5, Qt::lightGray);
    checkerBoardPainter.fillRect(5, 0, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(0, 5, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(5, 5, 5, 5, Qt::lightGray);
    checkerBoardPainter.end();
    tfPainter.fillRect(0, 0, gradientWidth, 20, QBrush(checkerBoard));
    tfPainter.fillRect(0, 0, gradientWidth, 20, gradient_);

    // draw masking indicators
    if (tfProperty_->getMask().x > 0.0f) {
        tfPainter.fillRect(0, 0, static_cast<int>(tfProperty_->getMask().x * gradientWidth), 20,
                           QColor(25, 25, 25, 100));

        tfPainter.drawLine(static_cast<int>(tfProperty_->getMask().x * gradientWidth), 0,
                           static_cast<int>(tfProperty_->getMask().x * gradientWidth), 20);
    }

    if (tfProperty_->getMask().y < 1.0f) {
        tfPainter.fillRect(static_cast<int>(tfProperty_->getMask().y * gradientWidth), 0,
                           static_cast<int>((1.0f - tfProperty_->getMask().y) * gradientWidth) + 1,
                           20, QColor(25, 25, 25, 150));

        tfPainter.drawLine(static_cast<int>(tfProperty_->getMask().y * gradientWidth), 0,
                           static_cast<int>(tfProperty_->getMask().y * gradientWidth), 20);
    }

    tfPreview_->setPixmap(*tfPixmap_);
}

void TransferFunctionPropertyDialog::updateFromProperty() {
    if (!tfProperty_->getOwner()) return;

    std::string processorName = tfProperty_->getOwner()->getProcessor()->getIdentifier();
    QString windowTitle = QString::fromStdString(tfProperty_->getDisplayName() + " (") +
                          QString::fromStdString(processorName) + QString::fromStdString(")");
    setWindowTitle(windowTitle);

    TransferFunction& transFunc = tfProperty_->get();
    QVector<QGradientStop> gradientStops;

    for (size_t i = 0; i < transFunc.getNumPoints(); i++) {
        const auto curPoint = transFunc.getPoint(i);
        vec4 curColor = curPoint->getRGBA();

        // increase alpha to allow better visibility by 1 - (a - 1)^4
        const float factor = (1.0f - curColor.a) * (1.0f - curColor.a);
        curColor.a = 1.0f - factor * factor;

        gradientStops.append(
            QGradientStop(curPoint->getPos(),
                          QColor::fromRgbF(curColor.r, curColor.g, curColor.b, curColor.a)));
    }

    gradient_.setStops(gradientStops);
    updateTFPreview();
}

void TransferFunctionPropertyDialog::changeVerticalZoom(int zoomMin, int zoomMax) {
    // normalize zoom values, as sliders in TransferFunctionPropertyDialog
    // have the range [0...100]
    // and flip/rescale values to compensate slider layout
    const auto zoomMaxF = static_cast<float>(sliderRange_ - zoomMin) / sliderRange_;
    const auto zoomMinF = static_cast<float>(sliderRange_ - zoomMax) / sliderRange_;

    tfProperty_->setZoomV(zoomMinF, zoomMaxF);
}

void TransferFunctionPropertyDialog::changeHorizontalZoom(int zoomMin, int zoomMax) {
    const auto zoomMinF = static_cast<float>(zoomMin) / sliderRange_;
    const auto zoomMaxF = static_cast<float>(zoomMax) / sliderRange_;

    tfProperty_->setZoomH(zoomMinF, zoomMaxF);
}

// Connected to valuesChanged on the maskSlider
void TransferFunctionPropertyDialog::changeMask(int maskMin, int maskMax) {
    const auto maskMinF = static_cast<float>(maskMin) / sliderRange_;
    const auto maskMaxF = static_cast<float>(maskMax) / sliderRange_;
    tfProperty_->setMask(maskMinF, maskMaxF);

    updateTFPreview();
}

void TransferFunctionPropertyDialog::importTransferFunction() {
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

void TransferFunctionPropertyDialog::exportTransferFunction() {
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

void TransferFunctionPropertyDialog::showHistogram(int type) {
    tfProperty_->setHistogramMode(static_cast<HistogramMode>(type));
}

void TransferFunctionPropertyDialog::resizeEvent(QResizeEvent* event) {
    PropertyEditorWidgetQt::resizeEvent(event);
    updateTFPreview();
}

void TransferFunctionPropertyDialog::showEvent(QShowEvent* event) {
    updateTFPreview();
    tfEditorView_->update();
    PropertyEditorWidgetQt::showEvent(event);
}

void TransferFunctionPropertyDialog::onControlPointAdded(TransferFunctionDataPoint* p) {
    tfEditor_->onControlPointAdded(p);
    updateFromProperty();
}

void TransferFunctionPropertyDialog::onControlPointRemoved(TransferFunctionDataPoint* p) {
    tfEditor_->onControlPointRemoved(p);
    updateFromProperty();
}

void TransferFunctionPropertyDialog::onControlPointChanged(const TransferFunctionDataPoint* p) {
    tfEditor_->onControlPointChanged(p);
    updateFromProperty();
}

TransferFunctionEditorView* TransferFunctionPropertyDialog::getEditorView() const {
    return tfEditorView_;
}

void TransferFunctionPropertyDialog::changeMoveMode(int i) { tfEditor_->setMoveMode(i); }

}  // namespace

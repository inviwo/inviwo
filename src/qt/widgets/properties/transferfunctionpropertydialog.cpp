/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/properties/transferfunctionpropertydialog.h>
#include <inviwo/qt/widgets/properties/transferfunctionpropertywidgetqt.h>
#include <inviwo/qt/widgets/properties/collapsiblegroupboxwidgetqt.h>
#include <inviwo/qt/widgets/properties/transferfunctioneditorcontrolpoint.h>
#include <inviwo/qt/widgets/inviwofiledialog.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/processors/processor.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QImage>
#include <QDockWidget>
#include <QGraphicsItem>
#include <QPushButton>
#include <QSizePolicy>
#include <warn/pop>

namespace inviwo {

TransferFunctionPropertyDialog::TransferFunctionPropertyDialog(TransferFunctionProperty* tfProperty,
                                                               QWidget* parent)
    : PropertyEditorWidgetQt("Transfer Function Editor", parent)
    , TransferFunctionObserver()
    , sliderRange_(1000)
    , tfProperty_(tfProperty)
    , tfEditor_(nullptr)
    , tfEditorView_(nullptr)
    , tfPixmap_(nullptr) {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    generateWidget();
    tfProperty_->get().addObserver(this);

    std::stringstream ss;
    ss << "Transfer Function Editor - " << tfProperty_->getDisplayName() << "("
       << tfProperty_->getOwner()->getProcessor() << ")";
    setWindowTitle(ss.str().c_str());

    if (!tfProperty_->getVolumeInport()) chkShowHistogram_->setVisible(false);

    gradient_ = new QLinearGradient(0, 0, 100, 20);
    updateTFPreview();
    updateFromProperty();
}

TransferFunctionPropertyDialog::~TransferFunctionPropertyDialog() {
    hide();
    delete tfPixmap_;
    delete tfEditor_;
    delete colorWheel_;
    delete gradient_;
    delete colorDialog_;
}

void TransferFunctionPropertyDialog::generateWidget() {
    vec2 minEditorDims = vec2(255.0f, 100.0f);

    tfEditorView_ = new TransferFunctionEditorView(tfProperty_);
    tfProperty_->get().addObserver(tfEditorView_);
    // put origin to bottom left corner
    tfEditorView_->scale(1.0, -1.0);
    tfEditorView_->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    tfEditorView_->setMinimumSize(minEditorDims.x, minEditorDims.y);
    tfEditorView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tfEditorView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    tfEditor_ = new TransferFunctionEditor(&tfProperty_->get(), tfEditorView_);
    connect(tfEditor_, SIGNAL(doubleClick()), this, SLOT(showColorDialog()));
    connect(tfEditor_, SIGNAL(selectionChanged()), this, SLOT(updateColorWheel()));
    tfEditorView_->setScene(tfEditor_);

    zoomVSlider_ = new RangeSliderQt(Qt::Vertical, this);
    zoomVSlider_->setRange(0, sliderRange_);
    zoomVSlider_->setMinSeparation(5);
    // flip slider values to compensate for vertical slider layout
    zoomVSlider_->setValue(
        sliderRange_ - static_cast<int>(tfProperty_->getZoomV().y * sliderRange_),
        sliderRange_ - static_cast<int>(tfProperty_->getZoomV().x * sliderRange_));
    connect(zoomVSlider_, SIGNAL(valuesChanged(int, int)), this,
            SLOT(changeVerticalZoom(int, int)));

    zoomHSlider_ = new RangeSliderQt(Qt::Horizontal, this);
    zoomHSlider_->setRange(0, sliderRange_);
    zoomHSlider_->setMinSeparation(5);
    zoomHSlider_->setValue(static_cast<int>(tfProperty_->getZoomH().x * sliderRange_),
                           static_cast<int>(tfProperty_->getZoomH().y * sliderRange_));
    connect(zoomHSlider_, SIGNAL(valuesChanged(int, int)), this,
            SLOT(changeHorizontalZoom(int, int)));

    maskSlider_ = new RangeSliderQt(Qt::Horizontal, this);
    maskSlider_->setRange(0, sliderRange_);
    maskSlider_->setValue(static_cast<int>(tfProperty_->getMask().x * sliderRange_),
                          static_cast<int>(tfProperty_->getMask().y * sliderRange_));
    connect(maskSlider_, SIGNAL(valuesChanged(int, int)), this, SLOT(changeMask(int, int)));

    colorWheel_ = new ColorWheel();
    connect(colorWheel_, SIGNAL(colorChange(QColor)), this, SLOT(setPointColor(QColor)));

    btnClearTF_ = new QPushButton("Reset");
    connect(btnClearTF_, SIGNAL(clicked()), tfEditor_, SLOT(resetTransferFunction()));
    btnClearTF_->setStyleSheet(QString("min-width: 30px; padding-left: 7px; padding-right: 7px;"));

    btnImportTF_ = new QPushButton("Import");
    connect(btnImportTF_, SIGNAL(clicked()), this, SLOT(importTransferFunction()));
    btnImportTF_->setStyleSheet(QString("min-width: 30px; padding-left: 7px; padding-right: 7px;"));

    btnExportTF_ = new QPushButton("Export");
    connect(btnExportTF_, SIGNAL(clicked()), this, SLOT(exportTransferFunction()));
    btnExportTF_->setStyleSheet(QString("min-width: 30px; padding-left: 7px; padding-right: 7px;"));

    tfPreview_ = new QLabel();
    tfPreview_->setMinimumSize(1, 20);
    QSizePolicy sliderPol = tfPreview_->sizePolicy();
    sliderPol.setHorizontalStretch(3);
    tfPreview_->setSizePolicy(sliderPol);

    cmbInterpolation_ = new QComboBox();
    cmbInterpolation_->addItem("Interpolation: Linear");
    // cmbInterpolation_->addItem("Interpolation: Cubic"); // Not implemented... (yet)
    cmbInterpolation_->setCurrentIndex(tfProperty_->get().getInterpolationType());
    connect(cmbInterpolation_, SIGNAL(currentIndexChanged(int)), this,
            SLOT(switchInterpolationType(int)));

    chkShowHistogram_ = new QComboBox();
    chkShowHistogram_->addItem("Histogram: Off");
    chkShowHistogram_->addItem("Histogram: 100%");
    chkShowHistogram_->addItem("Histogram: 99%");
    chkShowHistogram_->addItem("Histogram: 95%");
    chkShowHistogram_->addItem("Histogram: 90%");
    chkShowHistogram_->addItem("Histogram: Log");
    chkShowHistogram_->setCurrentIndex(tfProperty_->getShowHistogram());
    connect(chkShowHistogram_, SIGNAL(currentIndexChanged(int)), this, SLOT(showHistogram(int)));

    pointMoveMode_ = new QComboBox();
    pointMoveMode_->addItem("Point Movement: Free");
    pointMoveMode_->addItem("Point Movement: Restrict");
    pointMoveMode_->addItem("Point Movement: Push");
    pointMoveMode_->setCurrentIndex(0);
    connect(pointMoveMode_, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMoveMode(int)));

    colorDialog_ = new QColorDialog(this);
    colorDialog_->hide();
    colorDialog_->setOption(QColorDialog::ShowAlphaChannel, true);
    colorDialog_->setOption(QColorDialog::NoButtons, true);
    colorDialog_->setWindowModality(Qt::NonModal);
    colorDialog_->setWindowTitle(QString::fromStdString(tfProperty_->getDisplayName()));
    connect(colorDialog_, SIGNAL(currentColorChanged(QColor)), this,
            SLOT(setPointColorDialog(QColor)));

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
    rightLayout->addWidget(colorWheel_);
    rightLayout->addWidget(cmbInterpolation_);
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
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this,
            SLOT(dockLocationChanged(Qt::DockWidgetArea)));

    initialize(tfProperty_);
    setFloating(true);
    setVisible(false);
}

// Connected to the cmbInterpolation_ button
void TransferFunctionPropertyDialog::switchInterpolationType(int interpolationType) {
    if (interpolationType == 0) {
        tfProperty_->get().setInterpolationType(TransferFunction::InterpolationLinear);
    } else {
        tfProperty_->get().setInterpolationType(TransferFunction::InterpolationCubic);
    }
}

void TransferFunctionPropertyDialog::updateTFPreview() {
    int gradientWidth = tfPreview_->width();
    gradient_->setFinalStop(gradientWidth, 0);

    if (!tfPixmap_ || gradientWidth != tfPixmap_->width()) {
        if (tfPixmap_) {
            delete tfPixmap_;
        }
        tfPixmap_ = new QPixmap(gradientWidth, 20);
    }

    QPainter tfPainter(tfPixmap_);
    QPixmap checkerBoard(10, 10);
    QPainter checkerBoardPainter(&checkerBoard);
    checkerBoardPainter.fillRect(0, 0, 5, 5, Qt::lightGray);
    checkerBoardPainter.fillRect(5, 0, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(0, 5, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(5, 5, 5, 5, Qt::lightGray);
    checkerBoardPainter.end();
    tfPainter.fillRect(0, 0, gradientWidth, 20, QBrush(checkerBoard));
    tfPainter.fillRect(0, 0, gradientWidth, 20, *gradient_);

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

    for (int i = 0; i < transFunc.getNumPoints(); i++) {
        TransferFunctionDataPoint* curPoint = transFunc.getPoint(i);
        vec4 curColor = curPoint->getRGBA();

        // increase alpha to allow better visibility by 1 - (a - 1)^4
        float factor = (1.0f - curColor.a) * (1.0f - curColor.a);
        curColor.a = 1.0f - factor * factor;

        gradientStops.append(
            QGradientStop(curPoint->getPos().x,
                          QColor::fromRgbF(curColor.r, curColor.g, curColor.b, curColor.a)));
    }

    gradient_->setStops(gradientStops);
    updateTFPreview();
}

// Connected to selectionChanged() on the tfEditor
void TransferFunctionPropertyDialog::updateColorWheel() {
    QList<QGraphicsItem*> selection = tfEditor_->selectedItems();

    if (selection.size() > 0) {
        TransferFunctionEditorControlPoint* tfPoint =
            qgraphicsitem_cast<TransferFunctionEditorControlPoint*>(selection.at(0));

        if (selection.size() == 1 && tfPoint) {
            vec4 color = tfPoint->getPoint()->getRGBA() * 255.0f;
            colorWheel_->blockSignals(true);
            colorWheel_->setColor(QColor(color.r, color.g, color.b, color.a));
            colorWheel_->blockSignals(false);

            setColorDialogColor(QColor(color.r, color.g, color.b, color.a));
        }
    }
}

// Connected to doubleClick on the tfEditor
void TransferFunctionPropertyDialog::showColorDialog() {
    QList<QGraphicsItem*> selection = tfEditor_->selectedItems();
    if (selection.size() > 0) {
        colorDialog_->hide();  // Bug workaround
        colorDialog_->show();
    }
}

// Connected to colorChange on the colorWheel_
void TransferFunctionPropertyDialog::setPointColor(QColor color) {
    QList<QGraphicsItem*> selection = tfEditor_->selectedItems();
    vec3 newRgb = vec3(color.redF(), color.greenF(), color.blueF());

    // update Color dialog to reflect the color changes
    setColorDialogColor(color);

    for (auto& elem : selection) {
        TransferFunctionEditorControlPoint* tfcp =
            qgraphicsitem_cast<TransferFunctionEditorControlPoint*>(elem);

        if (tfcp) {
            tfcp->getPoint()->setRGB(newRgb);
        }
    }
}

// Connected to currentColorChanged on the colorDialog_
void TransferFunctionPropertyDialog::setPointColorDialog(QColor color) {
    QList<QGraphicsItem*> selection = tfEditor_->selectedItems();
    vec3 newRgb = vec3(color.redF(), color.greenF(), color.blueF());

    colorWheel_->blockSignals(true);
    colorWheel_->setColor(color);
    colorWheel_->blockSignals(false);

    for (auto& elem : selection) {
        TransferFunctionEditorControlPoint* tfcp =
            qgraphicsitem_cast<TransferFunctionEditorControlPoint*>(elem);

        if (tfcp) {
            tfcp->getPoint()->setRGB(newRgb);
        }
    }
}

void TransferFunctionPropertyDialog::changeVerticalZoom(int zoomMin, int zoomMax) {
    // normalize zoom values, as sliders in TransferFunctionPropertyDialog
    // have the range [0...100]
    // and flip/rescale values to compensate slider layout
    float zoomMaxF = static_cast<float>(sliderRange_ - zoomMin) / sliderRange_;
    float zoomMinF = static_cast<float>(sliderRange_ - zoomMax) / sliderRange_;

    tfProperty_->setZoomV(zoomMinF, zoomMaxF);
    tfEditorView_->updateZoom();
}

void TransferFunctionPropertyDialog::changeHorizontalZoom(int zoomMin, int zoomMax) {
    float zoomMinF = static_cast<float>(zoomMin) / sliderRange_;
    float zoomMaxF = static_cast<float>(zoomMax) / sliderRange_;

    tfProperty_->setZoomH(zoomMinF, zoomMaxF);
    tfEditorView_->updateZoom();
}

// Connected to valuesChanged on the maskSlider
void TransferFunctionPropertyDialog::changeMask(int maskMin, int maskMax) {
    float maskMinF = static_cast<float>(maskMin) / sliderRange_;
    float maskMaxF = static_cast<float>(maskMax) / sliderRange_;
    tfProperty_->setMask(maskMinF, maskMaxF);
    tfEditorView_->setMask(maskMinF, maskMaxF);

    updateTFPreview();
}

void TransferFunctionPropertyDialog::importTransferFunction() {
    InviwoFileDialog importFileDialog(
        this, "Import transfer function", "transferfunction",
        InviwoApplication::getPtr()->getPath(PathType::TransferFunctions));
    importFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    importFileDialog.setFileMode(QFileDialog::ExistingFile);
    importFileDialog.addExtension("itf", "Inviwo Transfer Function");

    if (importFileDialog.exec()) {
        QString file = importFileDialog.selectedFiles().at(0);
        // TODO: we need to check whether it is a valid itf file!
        Deserializer deserializer(file.toLocal8Bit().constData());
        TransferFunction tf;
        tf.deserialize(deserializer);
        tfProperty_->set(tf);
        updateFromProperty();
    }
}

void TransferFunctionPropertyDialog::exportTransferFunction() {
    InviwoFileDialog exportFileDialog(
        this, "Export transfer function", "transferfunction",
        InviwoApplication::getPtr()->getPath(PathType::TransferFunctions));
    exportFileDialog.setAcceptMode(QFileDialog::AcceptSave);
    exportFileDialog.setFileMode(QFileDialog::AnyFile);
    exportFileDialog.addExtension("itf", "Inviwo Transfer Function");
    exportFileDialog.addExtension("png", "Transfer Function Image");
    exportFileDialog.addExtension("", "All files");  // this will add "All files (*)"

    if (exportFileDialog.exec()) {
        std::string file = exportFileDialog.selectedFiles().at(0).toLocal8Bit().constData();
        std::string extension = filesystem::getFileExtension(file);

        FileExtension fileExt = exportFileDialog.getSelectedFileExtension();

        if (fileExt.extension_.empty()) {
            // fall-back to standard inviwo TF format
            fileExt.extension_ = "itf";
        }

        // check whether file extension matches the selected one
        if (fileExt.extension_ != extension) {
            file.append(fileExt.extension_);
        }

        if (fileExt.extension_ == "png") {
            TransferFunction& tf = tfProperty_->get();
            const Layer* layer = tf.getData();
            vec2 texSize(tf.getTextureSize(), 1);
            const vec4* readData =
                static_cast<const vec4*>(layer->getRepresentation<LayerRAM>()->getData());
            Layer writeLayer(layer->getDimensions(), DataVec4UInt8::get());
            glm::u8vec4* writeData = static_cast<glm::u8vec4*>(
                writeLayer.getEditableRepresentation<LayerRAM>()->getData());

            for (std::size_t i = 0; i < texSize.x * texSize.y; ++i) {
                for (int c = 0; c < 4; ++c) {
                    writeData[i][c] = static_cast<glm::u8>(
                        std::min(std::max(readData[i][c] * 255.0f, 0.0f), 255.0f));
                }
            }

            auto factory = InviwoApplication::getPtr()->getDataWriterFactory();

            if (auto writer = factory->getWriterForTypeAndExtension<Layer>(extension)) {
                try {
                    writer->setOverwrite(true);
                    writer->writeData(&writeLayer, file);
                } catch (DataWriterException const& e) {
                    util::log(e.getContext(), e.getMessage(), LogLevel::Error);
                }
            } else {
                LogError(
                    "Error: Cound not find a writer for the specified extension and data type");
            }
        } else if (fileExt.extension_ == "itf") {
            Serializer serializer(file);
            tfProperty_->get().serialize(serializer);
            serializer.writeFile();
        }
    }
}

void TransferFunctionPropertyDialog::showHistogram(int type) {
    tfProperty_->setShowHistogram(type);
    tfEditorView_->setShowHistogram(type);
}

void TransferFunctionPropertyDialog::resizeEvent(QResizeEvent* event) {
    setEditorDimensions(ivec2(event->size().width(), event->size().height()));
    QWidget::resizeEvent(event);
    updateTFPreview();
}

void TransferFunctionPropertyDialog::showEvent(QShowEvent* event) {
    updateTFPreview();
    tfEditorView_->update();
    showEditor();
}

void TransferFunctionPropertyDialog::closeEvent(QCloseEvent* event) { hideEditor(); }

void TransferFunctionPropertyDialog::moveEvent(QMoveEvent* event) {
    ivec2 pos = ivec2(event->pos().x(), event->pos().y());
    moveEditor(pos);

    if (isFloating() && !(getEditorDockStatus() == PropertyEditorWidgetDockStatus::Floating))
        setDockStatus(PropertyEditorWidgetDockStatus::Floating);

    QWidget::moveEvent(event);
}

void TransferFunctionPropertyDialog::dockLocationChanged(Qt::DockWidgetArea dockArea) {
    if (dockArea == Qt::LeftDockWidgetArea)
        setDockStatus(PropertyEditorWidgetDockStatus::DockedLeft);
    else if (dockArea == Qt::RightDockWidgetArea)
        setDockStatus(PropertyEditorWidgetDockStatus::DockedRight);
    else
        setDockStatus(PropertyEditorWidgetDockStatus::Floating);
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

QLinearGradient* TransferFunctionPropertyDialog::getTFGradient() { return gradient_; }

TransferFunctionEditorView* TransferFunctionPropertyDialog::getEditorView() {
    return tfEditorView_;
}

void TransferFunctionPropertyDialog::changeMoveMode(int i) { tfEditor_->setMoveMode(i); }

void TransferFunctionPropertyDialog::setColorDialogColor(QColor c) {
    colorDialog_->blockSignals(true);
    colorDialog_->setCurrentColor(c);
    colorDialog_->blockSignals(false);
}

}  // namespace

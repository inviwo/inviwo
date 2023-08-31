/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <modules/qtwidgets/rawdatareaderdialogqt.h>

#include <inviwo/core/datastructures/datamapper.h>  // for DataMapper
#include <inviwo/core/datastructures/unitsystem.h>  // for Axis, Unit
#include <inviwo/core/util/formats.h>               // for DataFormatBase, DataFormatId, DataFor...
#include <inviwo/core/util/glmvec.h>                // for uvec3, dvec2, dvec3
#include <inviwo/core/util/rendercontext.h>         // for RenderContext
#include <modules/qtwidgets/inviwoqtutils.h>        // for emToPx, fromLocalQString, toLocalQString

#include <limits>  // for numeric_limits

#include <QChar>             // for QChar
#include <QComboBox>         // for QComboBox
#include <QDialogButtonBox>  // for QDialogButtonBox, operator|, QDialogB...
#include <QDoubleSpinBox>    // for QDoubleSpinBox
#include <QDoubleValidator>  // for QDoubleValidator
#include <QGridLayout>       // for QGridLayout
#include <QGroupBox>         // for QGroupBox
#include <QHBoxLayout>       // for QHBoxLayout
#include <QLabel>            // for QLabel
#include <QLineEdit>         // for QLineEdit
#include <QLocale>           // for QLocale
#include <QObject>           // for SIGNAL, SLOT
#include <QSpinBox>          // for QSpinBox
#include <QString>           // for QString
#include <QVariant>          // for QVariant
#include <QWidget>           // for QWidget
#include <glm/fwd.hpp>       // for dvec3
#include <glm/vec2.hpp>      // for vec<>::(anonymous)
#include <glm/vec3.hpp>      // for vec<>::(anonymous)
#include <llnl-units/units.hpp>   // for to_string, unit_from_string

class QHBoxLayout;

namespace inviwo {

RawDataReaderDialogQt::RawDataReaderDialogQt() {
    setWindowTitle("Importing Raw Data");
    QGridLayout* mainLayout = new QGridLayout(this);
    {
        const auto space = utilqt::emToPx(this, 15.0 / 9.0);
        mainLayout->setContentsMargins(space, space, space, space);
    }
    QLabel* fileNameLabel = new QLabel("Importing file:");
    fileName_ = new QLabel();
    QGridLayout* dataTypeLayout = new QGridLayout();
    QLabel* bitDepthLabel = new QLabel("Data format");
    bitDepth_ = new QComboBox();
    bitDepth_->addItem("char (8-bit signed integer)", static_cast<int>(DataFormatId::Int8));
    bitDepth_->addItem("unsigned char (8-bit unsigned integer)",
                       static_cast<int>(DataFormatId::UInt8));
    bitDepth_->addItem("short (16-bit signed integer)", static_cast<int>(DataFormatId::Int16));
    bitDepth_->addItem("unsigned 12-bit (12-bit unsigned integer)",
                       static_cast<int>(DataFormatId::NotSpecialized));
    bitDepth_->addItem("unsigned short (16-bit unsigned integer)",
                       static_cast<int>(DataFormatId::UInt16));
    bitDepth_->addItem("signed int (32-bit signed integer)", static_cast<int>(DataFormatId::Int32));
    bitDepth_->addItem("unsigned int (32-bit unsigned integer)",
                       static_cast<int>(DataFormatId::UInt32));
    bitDepth_->addItem("signed long int (64-bit integer)", static_cast<int>(DataFormatId::Int64));
    bitDepth_->addItem("unsigned long int (64-bit unsigned integer)",
                       static_cast<int>(DataFormatId::UInt64));
    bitDepth_->addItem("half (16-bit floating point)", static_cast<int>(DataFormatId::Float16));
    bitDepth_->addItem("float (32-bit floating point)", static_cast<int>(DataFormatId::Float32));
    bitDepth_->addItem("double (64-bit floating point)", static_cast<int>(DataFormatId::Float64));
    connect(bitDepth_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this](int index) { selectedDataTypeChanged(index); });
    QLabel* channelLabel = new QLabel("Data channels");
    channels_ = new QSpinBox();
    channels_->setRange(1, 4);
    channels_->setValue(1);

    QLabel* dataRangeLabel = new QLabel("Data format range");
    dataRangeLabel->setToolTip("Data range refer to the range of the data");
    dataRangeMin_ = new QDoubleSpinBox();
    dataRangeMin_->setRange(-std::numeric_limits<double>::max(),
                            std::numeric_limits<double>::max());
    dataRangeMax_ = new QDoubleSpinBox();
    dataRangeMax_->setRange(-std::numeric_limits<double>::max(),
                            std::numeric_limits<double>::max());
    dataRangeMax_->setValue(255.0);
    QLabel* valueRangeLabel = new QLabel("Value range");
    valueRangeLabel->setToolTip(
        "Value range refer to the physical meaning of the value, i.e. Hounsfield value range for "
        "human tissue [-1000 3000]");
    valueRangeMin_ = new QDoubleSpinBox();
    valueRangeMin_->setRange(-std::numeric_limits<double>::max(),
                             std::numeric_limits<double>::max());
    valueRangeMax_ = new QDoubleSpinBox();
    valueRangeMax_->setRange(-std::numeric_limits<double>::max(),
                             std::numeric_limits<double>::max());
    QLabel* unit = new QLabel("Unit (m/s, HU, W)");
    valueUnit_ = new QLineEdit();  ///< Unit, i.e. Hounsfield/absorption/W.

    auto rowCount = 0;
    dataTypeLayout->addWidget(bitDepthLabel, rowCount, 0);
    dataTypeLayout->addWidget(bitDepth_, rowCount++, 1, 1, 2);

    dataTypeLayout->addWidget(new QLabel("Min"), rowCount, 1);
    dataTypeLayout->addWidget(new QLabel("Max"), rowCount++, 2);

    dataTypeLayout->addWidget(dataRangeLabel, rowCount, 0);
    dataTypeLayout->addWidget(dataRangeMin_, rowCount, 1);
    dataTypeLayout->addWidget(dataRangeMax_, rowCount++, 2);

    dataTypeLayout->addWidget(valueRangeLabel, rowCount, 0);
    dataTypeLayout->addWidget(valueRangeMin_, rowCount, 1);
    dataTypeLayout->addWidget(valueRangeMax_, rowCount++, 2);

    dataTypeLayout->addWidget(unit, rowCount, 0);
    dataTypeLayout->addWidget(valueUnit_, rowCount++, 1);

    dataTypeLayout->addWidget(channelLabel, rowCount, 0);
    dataTypeLayout->addWidget(channels_, rowCount++, 1);

    QGroupBox* dataTypeBox = new QGroupBox("Data type", this);
    dataTypeBox->setLayout(dataTypeLayout);
    QGridLayout* dataSizeLayout = new QGridLayout();
    QLabel* dimensionLabel = new QLabel("Dimensions");
    dimX_ = new QSpinBox();
    dimX_->setRange(0, 4096);
    dimX_->setValue(256);
    dimY_ = new QSpinBox();
    dimY_->setRange(0, 4096);
    dimY_->setValue(256);
    dimZ_ = new QSpinBox();
    dimZ_->setRange(0, 4096);
    dimZ_->setValue(256);
    QWidget* dimensions = new QWidget(this);
    QHBoxLayout* dimensionsLayout = new QHBoxLayout();
    dimensionsLayout->addWidget(dimX_);
    dimensionsLayout->addWidget(dimY_);
    dimensionsLayout->addWidget(dimZ_);
    dimensions->setLayout(dimensionsLayout);

    QLabel* spaceLabel = new QLabel("Spacing");
    spaceX_ = new QLineEdit(this);
    QLocale locale(spaceX_->locale());
    spaceX_->setText(locale.toString(0.01));
    spaceX_->setValidator(new QDoubleValidator(0.0, 1000.0, 16, spaceX_));
    spaceY_ = new QLineEdit(this);
    spaceY_->setText(locale.toString(0.01));
    spaceY_->setValidator(new QDoubleValidator(0.0, 1000.0, 16, spaceY_));
    spaceZ_ = new QLineEdit(this);
    spaceZ_->setText(locale.toString(0.01));
    spaceZ_->setValidator(new QDoubleValidator(0.0, 1000.0, 16, spaceZ_));
    QWidget* space = new QWidget(this);
    QHBoxLayout* spaceLayout = new QHBoxLayout();
    spaceLayout->addWidget(spaceX_);
    spaceLayout->addWidget(spaceY_);
    spaceLayout->addWidget(spaceZ_);
    space->setLayout(spaceLayout);

    /*
    QLabel* timeStepLabel = new QLabel("Time steps");
    timeSteps_ = new QSpinBox();
    */

    dataSizeLayout->addWidget(dimensionLabel, 0, 0);
    dataSizeLayout->addWidget(dimensions, 0, 1);
    dataSizeLayout->addWidget(spaceLabel, 1, 0);
    dataSizeLayout->addWidget(space, 1, 1);
    // dataSizeLayout->addWidget(timeStepLabel, 2, 0);
    // dataSizeLayout->addWidget(timeSteps_, 2, 1);
    QGroupBox* dataSizeBox = new QGroupBox("Data size", this);
    dataSizeBox->setLayout(dataSizeLayout);

    QGridLayout* readOptionsLayout = new QGridLayout();
    QLabel* byteOffsetLabel = new QLabel("Byte offset");
    byteOffset_ = new QSpinBox();
    byteOffset_->setRange(0, std::numeric_limits<int>::max());
    byteOffset_->setValue(0);
    byteOffset_->setSuffix(" Byte");
    /*
    QLabel* timeStepOffsetLabel = new QLabel("Time step offset");
    timeStepOffset_ = new QSpinBox();
    timeStepOffset_->setRange(0, 4096);
    timeStepOffset_->setValue(0);
    timeStepOffset_->setSuffix(" Byte");
    */
    QLabel* endianessLabel = new QLabel("Endianess");
    endianess_ = new QComboBox();
    endianess_->addItem("Little Endian");
    endianess_->addItem("Big Endian");
    readOptionsLayout->addWidget(byteOffsetLabel, 0, 0);
    readOptionsLayout->addWidget(byteOffset_, 0, 1);
    /*
    readOptionsLayout->addWidget(timeStepOffsetLabel, 1, 0);
    readOptionsLayout->addWidget(timeStepOffset_, 1, 1);
    */
    readOptionsLayout->addWidget(endianessLabel, 2, 0);
    readOptionsLayout->addWidget(endianess_, 2, 1);
    QGroupBox* readOptionsBox = new QGroupBox("Read options", this);
    readOptionsBox->setLayout(readOptionsLayout);
    QDialogButtonBox* buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(fileNameLabel, 0, 0);
    mainLayout->addWidget(fileName_, 0, 1);
    mainLayout->addWidget(dataTypeBox, 1, 0, 2, 2);
    mainLayout->addWidget(dataSizeBox, 3, 0, 2, 2);
    mainLayout->addWidget(readOptionsBox, 5, 0, 3, 2);
    mainLayout->addWidget(buttonBox, 8, 1);
    setLayout(mainLayout);
    // Set default to USHORT since we have OK default values and it is probably more common than
    // UCHAR.
    selectedDataTypeChanged(2);
}

RawDataReaderDialogQt::~RawDataReaderDialogQt() = default;

bool RawDataReaderDialogQt::show() {
    auto res = QDialog::exec();
    RenderContext::getPtr()->activateDefaultRenderContext();
    return res == QDialog::Accepted;
}

void RawDataReaderDialogQt::setFile(const std::filesystem::path& fileName) {
    fileName_->setText(utilqt::toQString(fileName));
}

const DataFormatBase* RawDataReaderDialogQt::getFormat() const {
    auto id = static_cast<DataFormatId>(bitDepth_->currentData().toInt());
    if (id == DataFormatId::NotSpecialized) {
        return DataFormatBase::get(NumericType::UnsignedInteger, channels_->value(), 16);
    } else {
        const auto df = DataFormatBase::get(id);
        return DataFormatBase::get(df->getNumericType(), channels_->value(), df->getPrecision());
    }
}

void RawDataReaderDialogQt::setFormat(const DataFormatBase* format) {
    channels_->setValue(static_cast<int>(format->getComponents()));
    const auto df = DataFormatBase::get(format->getNumericType(), 1, format->getPrecision());
    const auto index = bitDepth_->findData(static_cast<int>(df->getId()));
    bitDepth_->setCurrentIndex(index);
}

uvec3 RawDataReaderDialogQt::getDimensions() const {
    uvec3 dimensions;
    dimensions.x = dimX_->value();
    dimensions.y = dimY_->value();
    dimensions.z = dimZ_->value();
    return dimensions;
}

void RawDataReaderDialogQt::setDimensions(uvec3 dim) {
    dimX_->setValue(dim.x);
    dimY_->setValue(dim.y);
    dimZ_->setValue(dim.z);
}

dvec3 RawDataReaderDialogQt::getSpacing() const {
    QLocale locale = spaceX_->locale();
    glm::dvec3 space(0.01f);

    space.x = locale.toDouble(spaceX_->text().remove(QChar(' ')));
    space.y = locale.toDouble(spaceY_->text().remove(QChar(' ')));
    space.z = locale.toDouble(spaceZ_->text().remove(QChar(' ')));

    return space;
}

void RawDataReaderDialogQt::setSpacing(dvec3 spacing) {
    QLocale locale = spaceX_->locale();
    spaceX_->setText(locale.toString(spacing.x));
    spaceY_->setText(locale.toString(spacing.y));
    spaceZ_->setText(locale.toString(spacing.z));
}

bool RawDataReaderDialogQt::getEndianess() const { return endianess_->currentIndex() == 0; }

void RawDataReaderDialogQt::setEndianess(bool endian) {
    if (endian) {
        endianess_->setCurrentIndex(0);
    } else {
        endianess_->setCurrentIndex(1);
    }
}

DataMapper RawDataReaderDialogQt::getDataMapper() const {
    DataMapper dm;
    dm.dataRange = dvec2(dataRangeMin_->value(), dataRangeMax_->value());
    dm.valueRange = dvec2(valueRangeMin_->value(), valueRangeMax_->value());
    dm.valueAxis.unit = units::unit_from_string(utilqt::fromLocalQString(valueUnit_->text()));
    return dm;
}

void RawDataReaderDialogQt::setDataMapper(const DataMapper& datamapper) {
    dataRangeMin_->setValue(datamapper.dataRange.x);
    dataRangeMax_->setValue(datamapper.dataRange.y);
    valueRangeMin_->setValue(datamapper.valueRange.x);
    valueRangeMax_->setValue(datamapper.valueRange.y);
    valueUnit_->setText(utilqt::toLocalQString(units::to_string(datamapper.valueAxis.unit)));
}

size_t RawDataReaderDialogQt::getByteOffset() const {
    return static_cast<size_t>(byteOffset_->value());
}

void RawDataReaderDialogQt::setByteOffset(size_t offset) {
    byteOffset_->setValue(static_cast<int>(offset));
}

void RawDataReaderDialogQt::selectedDataTypeChanged(int index) {
    auto id = static_cast<DataFormatId>(bitDepth_->itemData(index).toInt());
    if (id == DataFormatId::NotSpecialized) {
        dataRangeMin_->setValue(0);
        dataRangeMax_->setValue(4095);
        valueRangeMin_->setValue(0);
        valueRangeMax_->setValue(4095);
    } else if (DataFormatBase::get(id)->getNumericType() == NumericType::Float) {
        dataRangeMin_->setValue(0);
        dataRangeMax_->setValue(1);
        valueRangeMin_->setValue(0);
        valueRangeMax_->setValue(1);
    } else {
        auto df = DataFormatBase::get(id);
        dataRangeMin_->setValue(df->getMin());
        dataRangeMax_->setValue(df->getMax());
        valueRangeMin_->setValue(df->getMin());
        valueRangeMax_->setValue(df->getMax());
    }
}

}  // namespace inviwo

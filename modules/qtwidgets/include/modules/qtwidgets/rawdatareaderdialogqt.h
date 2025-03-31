/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/datastructures/datamapper.h>  // for DataMapper
#include <inviwo/core/io/volumedatareaderdialog.h>  // for VolumeDataReaderDialog
#include <inviwo/core/util/glmvec.h>                // for dvec3, uvec3

#include <cstddef>  // for size_t
#include <string>   // for string

#include <QDialog>  // for QDialog

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QCheckBox;

namespace inviwo {

class DataFormatBase;

class IVW_MODULE_QTWIDGETS_API RawDataReaderDialogQt : public VolumeDataReaderDialog,
                                                       public QDialog {
public:
    RawDataReaderDialogQt();
    void selectedDataTypeChanged(int index);
    virtual ~RawDataReaderDialogQt();

    virtual bool show() override;

    virtual void setFile(const std::filesystem::path& fileName) override;

    virtual const DataFormatBase* getFormat() const override;
    virtual uvec3 getDimensions() const override;
    virtual dvec3 getSpacing() const override;
    virtual ByteOrder getByteOrder() const override;
    virtual DataMapper getDataMapper() const override;
    virtual size_t getByteOffset() const override;
    virtual Compression getCompression() const override;

    virtual void setFormat(const DataFormatBase* format) override;
    virtual void setDimensions(uvec3 dim) override;
    virtual void setSpacing(dvec3 spacing) override;
    virtual void setByteOrder(ByteOrder byteOrder) override;
    virtual void setDataMapper(const DataMapper& datamapper) override;
    virtual void setByteOffset(size_t offset) override;
    virtual void setCompression(Compression compression) override;

private:
    QLabel* fileName_;
    QComboBox* bitDepth_;
    QDoubleSpinBox* dataRangeMin_;
    QDoubleSpinBox* dataRangeMax_;
    QDoubleSpinBox* valueRangeMin_;
    QDoubleSpinBox* valueRangeMax_;
    QLineEdit* valueUnit_;  ///< Unit, i.e. Hounsfield/absorption/W.
    QSpinBox* channels_;
    QSpinBox* dimX_;
    QSpinBox* dimY_;
    QSpinBox* dimZ_;

    QLineEdit* spaceX_;
    QLineEdit* spaceY_;
    QLineEdit* spaceZ_;

    QSpinBox* byteOffset_;
    /*
    QSpinBox* timeSteps_;
    QSpinBox* timeStepOffset_;
    */
    QComboBox* byteOrder_;
    QCheckBox* useCompression_;
};

}  // namespace inviwo

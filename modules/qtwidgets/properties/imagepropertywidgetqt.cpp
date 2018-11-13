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

#include <modules/qtwidgets/properties/imagepropertywidgetqt.h>
#include <inviwo/core/properties/imageproperty.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QImage>
#include <QImageWriter>
#include <QString>
#include <QPixmap>
#include <QRgb>
#include <QColor>

namespace inviwo {

    QImageLabel::QImageLabel(ImageProperty* property) : property_(property) {}

    void QImageLabel::mouseReleaseEvent(QMouseEvent*) {
        property_->propertyModified();
    }

    ImagePropertyWidgetQt::ImagePropertyWidgetQt(ImageProperty* property)
        : PropertyWidgetQt(property), property_(property) {

        image_ = new QImageLabel(property);

        if (property_->getImgData()) {
            const auto imgData = property->getImgData();
            const auto imgSize = property->getImgSize();

            /*std::ofstream ppm_file{"D:/Downloads/tmp/volume_slice_after.pgm"};
            ppm_file << "P2" << std::endl;
            ppm_file << "# Volume Slice" << std::endl;
            ppm_file << imgSize.x << " " << imgSize.y << std::endl;
            ppm_file << "255" << std::endl;

            const size2_t step_size{1};
            for (size_t y{0}; y < imgSize.y; y += step_size.y) {
                for (size_t x{0}; x < imgSize.x; x += step_size.x) {
                    const auto img_value = imgData[y * imgSize.x + x];
                    ppm_file << static_cast<int>(img_value) << " ";
                }
                ppm_file << std::endl;
            }

            ppm_file.close();*/

            QImage img{static_cast<int>(imgSize.x), static_cast<int>(imgSize.y), QImage::Format_Grayscale8};
            for (int y = 0; y < imgSize.y; y++) {
                for (int x = 0; x < imgSize.x; x++) {
                    const auto value = static_cast<int>(imgData[y * imgSize.x + x]);
                    const QColor color{value, value, value};
                    img.setPixelColor(x, y, color);
                }
            }

            //const QImage img{imgData, static_cast<int>(imgSize.x), static_cast<int>(imgSize.y), QImage::Format_Grayscale8};

            image_->setPixmap(QPixmap::fromImage(img));
        } else {
            const QString filepath{"D:/Eigene Dateien/Bilder/Fractal_4.png"};
            QImage imageObject{filepath};
            image_->setPixmap(QPixmap::fromImage(imageObject));
        }

        image_->setAlignment(Qt::AlignCenter);

        // https://stackoverflow.com/questions/14563180/raw-data-to-qimage
        /*QImage* img = new QImage(imgSize.x, imgSize.y, QImage::Format_Grayscale8);
        for (int y{0}; y < img->height(); ++y) {
            memcpy(img->scanLine(y), imgData[y], img->bytesPerLine());
        }*/

        QVBoxLayout* hLayout = new QVBoxLayout();
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->setSpacing(0);
        hLayout->addWidget(image_);

        setLayout(hLayout);
        updateFromProperty();
    }
    
    /*void ImagePropertyWidgetQt::onSetDisplayName(Property*, const std::string& displayName) {
        //label_->setText(QString::fromStdString(displayName));
    }*/

    void ImagePropertyWidgetQt::updateFromProperty() {
        //label_->setText(QString::fromStdString(property_->getDisplayName()));
    }
    
} //namespace

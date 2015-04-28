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

#ifndef IVW_DIRECTORYPROPERTYWIDGETQT_H
#define IVW_DIRECTORYPROPERTYWIDGETQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/editablelabelqt.h>
#include <QLineEdit>
#include <QToolButton>

#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/qt/widgets/properties/optionpropertywidgetqt.h>
#include <inviwo/qt/widgets/properties/stringpropertywidgetqt.h>

namespace inviwo {

class IVW_QTWIDGETS_API DirectoryPropertyWidgetQt : public PropertyWidgetQt {

    Q_OBJECT

public:
    DirectoryPropertyWidgetQt(DirectoryProperty* property);

    void updateFromProperty();

public slots:
    void setPropertyValue();
    void setPropertyTreeInfo(std::string path);
    void setPropertyDisplayName();

private:
    void generateWidget();

    DirectoryProperty* property_;
    QLineEdit* lineEdit_;
    QToolButton* openButton_;
    EditableLabelQt* directoryLabel_;
};

} // namespace

#endif // IVW_DIRECTORYPROPERTYWIDGETQT_H
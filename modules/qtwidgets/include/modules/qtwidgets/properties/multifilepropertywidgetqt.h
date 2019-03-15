/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_MULTIFILEPROPERTYWIDGETQT_H
#define IVW_MULTIFILEPROPERTYWIDGETQT_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <inviwo/core/properties/fileproperty.h>

class QDropEvent;
class QDragEnterEvent;
class QDragMoveEvent;

namespace inviwo {

class MultiFileProperty;
class FilePathLineEditQt;
class EditableLabelQt;

/**
 * \class MultiFilePropertyWidgetQt
 * \brief Property widget for MultiFileProperty showing only the first file name.
 */
class IVW_MODULE_QTWIDGETS_API MultiFilePropertyWidgetQt : public PropertyWidgetQt,
                                                           public FileRequestable {
public:
    MultiFilePropertyWidgetQt(MultiFileProperty* property);
    virtual ~MultiFilePropertyWidgetQt() = default;

    virtual void updateFromProperty() override;
    virtual bool requestFile() override;

protected:
    virtual void dropEvent(QDropEvent*) override;
    virtual void dragEnterEvent(QDragEnterEvent*) override;
    virtual void dragMoveEvent(QDragMoveEvent*) override;

private:
    void setPropertyValue();

    MultiFileProperty* property_;
    FilePathLineEditQt* lineEdit_;
    EditableLabelQt* label_;
};

}  // namespace inviwo

#endif  // IVW_MULTIFILEPROPERTYWIDGETQT_H

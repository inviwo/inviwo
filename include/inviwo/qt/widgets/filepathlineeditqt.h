/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_FILEPATHLINEEDITQT_H
#define IVW_FILEPATHLINEEDITQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/lineeditqt.h>

class QLabel;

namespace inviwo {

/**
* \class FilePathLineEditQt
* \brief QLineEdit for file paths. When editing the path, i.e. the widget is focused, the full path is shown.
*        When not in focus, it shows only the file name with extension.
*        A small warning icon is shown to indicate non-existing files and paths.
*/
class IVW_QTWIDGETS_API FilePathLineEditQt : public LineEditQt {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    FilePathLineEditQt(QWidget *parent=nullptr);
    virtual ~FilePathLineEditQt() = default;

    void setPath(const std::string &path);
    const std::string& getPath() const;

    void setEditing(bool editing);
    bool isEditingEnabled() const;

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void focusInEvent(QFocusEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

    void updateContents();
    void updateIcon();

private:
    QLabel *warningLabel_; //!< warning icon which is visible if the path is invalid
    std::string path_; //!< full path including file name
    bool editingEnabled_; //!< if this flag is set, the full path is shown. Otherwise only the file name is shown
    int cursorPos_;
    bool cursorPosDirty_;
};

} // namespace iniwo

#endif // IVW_FILEPATHLINEEDITQT_H

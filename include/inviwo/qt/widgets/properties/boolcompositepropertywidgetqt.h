/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_BOOLCOMPOSITEPROPERTYWIDGETQT_H
#define IVW_BOOLCOMPOSITEPROPERTYWIDGETQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/qt/widgets/properties/collapsiblegroupboxwidgetqt.h>
#include <QLineEdit>
#include <QToolButton>

#include <inviwo/qt/widgets/properties/propertywidgetqt.h>
#include <inviwo/core/properties/compositepropertyobserver.h>

namespace inviwo {

class BoolCompositeProperty;

class IVW_QTWIDGETS_API BoolCompositePropertyWidgetQt : public CollapsibleGroupBoxWidgetQt,
                                                        public CompositePropertyObserver {
    #include <warn/push>
    #include <warn/ignore/all>
    Q_OBJECT
    #include <warn/pop>

public:
    BoolCompositePropertyWidgetQt(BoolCompositeProperty* property);
    virtual void updateFromProperty() override;

    virtual bool isChecked() const override;
    virtual bool isCollapsed() const override;

    virtual void onSetDisplayName(const std::string& displayName) override;
    virtual void onSetCollapsed(bool value) override;

    virtual void initState() override; 

protected slots:
    virtual void labelDidChange() override;

protected:
    // override from CollapsibleGroupBoxWidgetQt
    virtual void setCollapsed(bool value) override;

private:
    virtual void setChecked(bool checked) override;
    BoolCompositeProperty* boolCompProperty_;
};

}  // namespace

#endif  // IVW_BOOLCOMPOSITEPROPERTYWIDGETQT_H

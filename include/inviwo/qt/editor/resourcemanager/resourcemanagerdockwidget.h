/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_RESOURCEMANAGERDOCKWIDGET_H
#define IVW_RESOURCEMANAGERDOCKWIDGET_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/qtwidgets/inviwodockwidget.h>
#include <inviwo/core/resourcemanager/resourcemanagerobserver.h>

class QTableView;
class QCheckBox;

namespace inviwo {
class Resource;
class ResourceManager;

class ResourceManagerItemModel;

/**
 * \class ResourceManagerDockWidget
 * \brief Widget class for the Resource Manager
 */
class IVW_QTEDITOR_API ResourceManagerDockWidget : public InviwoDockWidget,
                                                   public ResourceManagerObserver {
public:
    ResourceManagerDockWidget(QWidget* parent, ResourceManager& manager);
    virtual ~ResourceManagerDockWidget();

    virtual void onResourceAdded(const std::string& key, const std::type_index& type,
                                 Resource* resource) override;
    virtual void onResourceRemoved(const std::string& key, const std::type_index& type,
                                   Resource* resource) override;

    virtual void onResourceManagerEnableStateChanged() override;

private:
    ResourceManager& manager_;

    ResourceManagerItemModel* model_;
    QTableView* tableView_;
    QCheckBox* disabledCheckBox_;
};

}  // namespace inviwo

#endif  // IVW_RESOURCEMANAGERDOCKWIDGET_H

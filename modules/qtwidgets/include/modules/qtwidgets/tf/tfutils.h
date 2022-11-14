/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <memory>

class QMenu;
class QWidget;

namespace inviwo {

class TFPrimitiveSet;
class TransferFunction;
class IsoValueCollection;
class TransferFunctionProperty;

namespace util {

/**
 * \brief Shows an InviwoFileDialog to import a TransferFunction from a file.
 *
 * @param parent         parent widget of the file dialog
 */
IVW_MODULE_QTWIDGETS_API std::shared_ptr<TransferFunction> importTransferFunctionDialog(
    QWidget* parent = nullptr);

/**
 * \brief Shows an InviwoFileDialog to export a TFPrimitiveSet to a file.
 *
 * @param primitiveSet   TransferFunction set to be exported
 * @param parent         parent widget of the file dialog
 */
IVW_MODULE_QTWIDGETS_API void exportTransferFunctionDialog(const TransferFunction& tf,
                                                           QWidget* parent = nullptr);

/**
 * \brief Shows an InviwoFileDialog to import a IsoValueCollection from a file.
 *
 * @param parent         parent widget of the file dialog
 */
IVW_MODULE_QTWIDGETS_API std::shared_ptr<IsoValueCollection> importIsoValueCollectionDialog(
    QWidget* parent = nullptr);

/**
 * \brief Shows an InviwoFileDialog to export a TFPrimitiveSet to a file.
 *
 * @param primitiveSet   IsoValueCollection set to be exported
 * @param parent         parent widget of the file dialog
 */
IVW_MODULE_QTWIDGETS_API void exportIsoValueCollectionDialog(const IsoValueCollection& iso,
                                                             QWidget* parent = nullptr);

/**
 * \brief create a submenu containing entries for TF presets of all transfer functions found in
 * the path `PathType::TransferFunctions`. Upon selecting a menu entry, the respective preset will
 * be loaded by \p property. The submenu will be disabled if \p property is read-only.
 *
 * @param parent     parent widget
 * @param menu       parent menu to which the TF preset submenu should be added
 * @param property   this property will load the TF presets once the menu entries are triggered
 * @return newly created submenu, which is owned by \p parent
 */
IVW_MODULE_QTWIDGETS_API QMenu* addTFPresetsMenu(QWidget* parent, QMenu* menu,
                                                 TransferFunctionProperty* property);

IVW_MODULE_QTWIDGETS_API QMenu* addTFColorbrewerPresetsMenu(QWidget* parent, QMenu* menu,
                                                            TransferFunctionProperty* property);

}  // namespace util

}  // namespace inviwo

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
#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/tfprimitiveset.h>

class QWidget;
class QMenu;

namespace inviwo {

class TransferFunctionProperty;

namespace util {

/**
 * \brief Shows an InviwoFileDialog to import a TFPrimitiveSet from a file.
 * Depending on the underlying type of \p primitiveSet, either TF primitives or isovalues are
 * imported.
 *
 * @param primitiveSet   target primitive set which might either be a TF or isovalues
 * @param parent         parent widget of the file dialog
 */
IVW_MODULE_QTWIDGETS_API void importFromFile(TFPrimitiveSet& primitiveSet,
                                             QWidget* parent = nullptr);

/**
 * \brief Shows an InviwoFileDialog to export a TFPrimitiveSet to a file.
 * Depending on the underlying type of \p primitiveSet, either TF primitives or isovalues are
 * exported.
 *
 * @param primitiveSet   primitive set to be exported (either TF or isovalues)
 * @param parent         parent widget of the file dialog
 */
IVW_MODULE_QTWIDGETS_API void exportToFile(const TFPrimitiveSet& primitiveSet,
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

}  // namespace util

}  // namespace inviwo

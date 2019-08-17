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

#include <modules/qtwidgets/tf/tfutils.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datawriterexception.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QAction>
#include <warn/pop>

namespace inviwo {

namespace util {

void importFromFile(TFPrimitiveSet& primitiveSet, QWidget* parent) {
    InviwoFileDialog importFileDialog(parent, "Import " + primitiveSet.getTitle(),
                                      "transferfunction");
    importFileDialog.setAcceptMode(AcceptMode::Open);
    importFileDialog.setFileMode(FileMode::ExistingFile);
    for (auto& ext : primitiveSet.getSupportedExtensions()) {
        importFileDialog.addExtension(ext);
    }
    importFileDialog.addExtension(FileExtension::all());

    if (importFileDialog.exec()) {
        const auto filename = utilqt::fromQString(importFileDialog.selectedFiles().at(0));
        try {
            NetworkLock lock;
            primitiveSet.load(filename, importFileDialog.getSelectedFileExtension());
        } catch (DataReaderException& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
        }
    }
}

void exportToFile(const TFPrimitiveSet& primitiveSet, QWidget* parent) {
    InviwoFileDialog exportFileDialog(parent, "Export " + primitiveSet.getTitle(),
                                      "transferfunction");
    exportFileDialog.setAcceptMode(AcceptMode::Save);
    exportFileDialog.setFileMode(FileMode::AnyFile);
    for (auto& ext : primitiveSet.getSupportedExtensions()) {
        exportFileDialog.addExtension(ext);
    }
    exportFileDialog.addExtension(FileExtension::all());

    if (exportFileDialog.exec()) {
        const auto filename = utilqt::fromQString(exportFileDialog.selectedFiles().at(0));
        const auto fileExt = exportFileDialog.getSelectedFileExtension();
        try {
            primitiveSet.save(filename, fileExt);
            util::log(IVW_CONTEXT_CUSTOM("util::exportToFile"),
                      "Data exported to disk: " + filename, LogLevel::Info, LogAudience::User);
        } catch (DataWriterException& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
        }
    }
}

QMenu* addTFPresetsMenu(QWidget* parent, QMenu* menu, TransferFunctionProperty* property) {
    if (!parent || !menu || !property) {
        return nullptr;
    }

    auto presets = menu->addMenu("&TF Presets");
    presets->setObjectName("TF");
    presets->setEnabled(!property->getReadOnly());
    const int iconWidth = utilqt::emToPx(presets, 11);
    // need to set the stylesheet explicitely since Qt _only_ supports 'px' for icon sizes
    presets->setStyleSheet(QString("QMenu { icon-size: %1px; }").arg(iconWidth));
    if (!property->getReadOnly()) {
        auto addPresetActions = [presets, parent, property,
                                 iconWidth](const std::string& basePath) {
            TransferFunction tf;
            auto files = filesystem::getDirectoryContentsRecursively(basePath);
            for (auto file : files) {
                for (auto& ext : property->get().getSupportedExtensions()) {
                    if (ext.matches(file)) {
                        try {
                            tf.load(file, ext);
                        } catch (DataReaderException&) {
                            // No reader found, ignore the TF
                            continue;
                        } catch (AbortException&) {
                            // Failed to load, ignore the TF
                            continue;
                        }
                        // remove basepath and trailing directory separator from filename
                        auto action = presets->addAction(
                            utilqt::toQString(file.substr(basePath.length() + 1)));
                        QObject::connect(action, &QAction::triggered, parent,
                                         [property, file, ext]() {
                                             NetworkLock lock(property);
                                             property->get().load(file, ext);
                                         });

                        action->setIcon(QIcon(utilqt::toQPixmap(tf, QSize{iconWidth, 20})));
                        break;
                    }
                }
            }
        };

        for (const auto& module : InviwoApplication::getPtr()->getModules()) {
            auto moduleTFPath = module->getPath(ModulePath::TransferFunctions);
            if (!filesystem::directoryExists(moduleTFPath)) continue;
            addPresetActions(moduleTFPath);
        }

        if (presets->actions().empty()) {
            auto action = presets->addAction("No Presets Available");
            action->setEnabled(false);
        }
    }
    return presets;
}

}  // namespace util

}  // namespace inviwo

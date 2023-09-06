/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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

#include <inviwo/core/util/colorbrewer-generated.h>  // for Category, Family, operator<<
#include <inviwo/core/common/inviwoapplication.h>    // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>         // for InviwoModule
#include <inviwo/core/common/modulepath.h>           // for ModulePath, ModulePath::Tra...
#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/datastructures/datamapper.h>        // for DataMapper
#include <inviwo/core/datastructures/tfprimitiveset.h>    // for TFPrimitiveSet
#include <inviwo/core/datastructures/transferfunction.h>  // for TransferFunction
#include <inviwo/core/io/datareaderexception.h>           // for DataReaderException
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterexception.h>               // for DataWriterException
#include <inviwo/core/network/networklock.h>                  // for NetworkLock
#include <inviwo/core/ports/volumeport.h>                     // for VolumeInport
#include <inviwo/core/properties/transferfunctionproperty.h>  // for TransferFunctionProperty
#include <inviwo/core/util/colorbrewer.h>                     // for getTransferFunction
#include <inviwo/core/util/exception.h>                       // for AbortException
#include <inviwo/core/util/filedialogstate.h>                 // for AcceptMode, FileMode, Accep...
#include <inviwo/core/util/fileextension.h>                   // for FileExtension
#include <inviwo/core/util/filesystem.h>                      // for directoryExists, getDirecto...
#include <inviwo/core/util/logcentral.h>                      // for log, LogAudience, LogAudien...
#include <inviwo/core/util/sourcecontext.h>                   // for IVW_CONTEXT_CUSTOM
#include <inviwo/core/util/stringconversion.h>                // for toString
#include <modules/qtwidgets/inviwofiledialog.h>               // for InviwoFileDialog
#include <modules/qtwidgets/inviwoqtutils.h>                  // for toQString, emToPx, toQPixmap

#include <initializer_list>  // for initializer_list
#include <limits>            // for numeric_limits
#include <memory>            // for unique_ptr, shared_ptr
#include <string>            // for basic_string, operator+
#include <string_view>       // for string_view
#include <type_traits>       // for remove_reference, remove_re...
#include <utility>           // for move
#include <vector>            // for vector

#include <QAction>           // for QAction
#include <QCheckBox>         // for QCheckBox
#include <QDialog>           // for QDialog
#include <QDialogButtonBox>  // for QDialogButtonBox, operator|
#include <QDoubleSpinBox>    // for QDoubleSpinBox
#include <QGridLayout>       // for QGridLayout
#include <QIcon>             // for QIcon
#include <QLabel>            // for QLabel
#include <QList>             // for QList
#include <QMenu>             // for QMenu
#include <QObject>           // for QObject
#include <QSize>             // for QSize
#include <QSpinBox>          // for QSpinBox
#include <QString>           // for QString
#include <QStringList>       // for QStringList
#include <QWidget>           // for QWidget
#include <fmt/core.h>        // for format

namespace inviwo {

namespace util {

std::shared_ptr<TransferFunction> importTransferFunctionDialog(QWidget* parent) {
    InviwoFileDialog importFileDialog(parent, "Import Transfer Function", "transferfunction");
    importFileDialog.setAcceptMode(AcceptMode::Open);
    importFileDialog.setFileMode(FileMode::ExistingFile);

    auto factory = util::getDataReaderFactory();

    importFileDialog.addExtensions(factory->getExtensionsForType<TransferFunction>());
    importFileDialog.addExtension(FileExtension::all());

    if (importFileDialog.exec()) {
        const auto filename = utilqt::fromQString(importFileDialog.selectedFiles().at(0));
        try {
            if (auto reader = factory->getReaderForTypeAndExtension<TransferFunction>(
                    importFileDialog.getSelectedFileExtension(), filename)) {

                return reader->readData(filename);
            } else {
                throw DataReaderException(IVW_CONTEXT_CUSTOM("importTransferFunctionDialog"),
                                          "Unable to find a Transfer function reader for {}",
                                          filename);
            }
        } catch (DataReaderException& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
        }
    }
    return nullptr;
}

std::shared_ptr<IsoValueCollection> importIsoValueCollectionDialog(QWidget* parent) {
    InviwoFileDialog importFileDialog(parent, "Import Iso Value Collection", "isovaluecollection");
    importFileDialog.setAcceptMode(AcceptMode::Open);
    importFileDialog.setFileMode(FileMode::ExistingFile);

    auto factory = util::getDataReaderFactory();

    importFileDialog.addExtensions(factory->getExtensionsForType<IsoValueCollection>());
    importFileDialog.addExtension(FileExtension::all());

    if (importFileDialog.exec()) {
        const auto filename = utilqt::fromQString(importFileDialog.selectedFiles().at(0));
        try {
            if (auto reader = factory->getReaderForTypeAndExtension<IsoValueCollection>(
                    importFileDialog.getSelectedFileExtension(), filename)) {

                return reader->readData(filename);
            } else {
                throw DataReaderException(IVW_CONTEXT_CUSTOM("importIsoValueCollectionDialog"),
                                          "Unable to find a Iso Value Collection reader for {}",
                                          filename);
            }
        } catch (DataReaderException& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
        }
    }
    return nullptr;
}

void exportTransferFunctionDialog(const TransferFunction& tf, QWidget* parent) {
    InviwoFileDialog exportFileDialog(parent, "Export Transfer Function", "transferfunction");
    exportFileDialog.setAcceptMode(AcceptMode::Save);
    exportFileDialog.setFileMode(FileMode::AnyFile);

    auto factory = util::getDataWriterFactory();

    exportFileDialog.addExtensions(factory->getExtensionsForType<TransferFunction>());
    exportFileDialog.addExtension(FileExtension::all());

    if (exportFileDialog.exec()) {
        const auto filename = utilqt::fromQString(exportFileDialog.selectedFiles().at(0));
        const auto fileExt = exportFileDialog.getSelectedFileExtension();

        try {
            if (auto writer =
                    factory->getWriterForTypeAndExtension<TransferFunction>(fileExt, filename)) {
                writer->writeData(&tf, filename);

                util::log(IVW_CONTEXT_CUSTOM("util::exportToFile"),
                          "Data exported to disk: " + filename, LogLevel::Info, LogAudience::User);
            } else {
                throw DataWriterException(IVW_CONTEXT_CUSTOM("exportTransferFunctionDialog"),
                                          "Unable to find a Transfer Function writer for {}",
                                          filename);
            }
        } catch (DataWriterException& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error, LogAudience::User);
        }
    }
}

void exportIsoValueCollectionDialog(const IsoValueCollection& iso, QWidget* parent) {
    InviwoFileDialog exportFileDialog(parent, "Export Iso Value Collection", "isovaluecollection");
    exportFileDialog.setAcceptMode(AcceptMode::Save);
    exportFileDialog.setFileMode(FileMode::AnyFile);

    auto factory = util::getDataWriterFactory();

    exportFileDialog.addExtensions(factory->getExtensionsForType<IsoValueCollection>());
    exportFileDialog.addExtension(FileExtension::all());

    if (exportFileDialog.exec()) {
        const auto filename = utilqt::fromQString(exportFileDialog.selectedFiles().at(0));
        const auto fileExt = exportFileDialog.getSelectedFileExtension();

        try {
            if (auto writer =
                    factory->getWriterForTypeAndExtension<IsoValueCollection>(fileExt, filename)) {
                writer->writeData(&iso, filename);

                util::log(IVW_CONTEXT_CUSTOM("util::exportToFile"),
                          "Data exported to disk: " + filename, LogLevel::Info, LogAudience::User);
            } else {
                throw DataWriterException(IVW_CONTEXT_CUSTOM("exportTransferFunctionDialog"),
                                          "Unable to find a Iso Value Collection writer for {}",
                                          filename);
            }
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
    // need to set the stylesheet explicitly since Qt _only_ supports 'px' for icon sizes
    presets->setStyleSheet(QString("QMenu { icon-size: %1px; }").arg(iconWidth));

    auto addPresetActions = [presets, parent, property,
                             iconWidth](const std::filesystem::path& basePath) {
        auto factory = util::getDataReaderFactory();
        auto files = filesystem::getDirectoryContentsRecursively(basePath);
        for (auto file : files) {
            if (auto reader = factory->getReaderForTypeAndExtension<TransferFunction>(file)) {
                try {
                    auto tf = reader->readData(basePath / file);
                    // remove basepath and trailing directory separator from filename
                    auto action = presets->addAction(utilqt::toQString(file.string()));
                    action->setIcon(QIcon(utilqt::toQPixmap(*tf, QSize{iconWidth, 20})));
                    QObject::connect(action, &QAction::triggered, parent,
                                     [property, tf = std::move(tf)]() {
                                         NetworkLock lock(property);
                                         property->get() = *tf;
                                     });
                } catch (DataReaderException&) {
                    // No reader found, ignore the TF
                } catch (AbortException&) {
                    // Failed to load, ignore the TF
                }
            }
        }
    };

    QObject::connect(
        presets, &QMenu::aboutToShow, presets,
        [presets, addPresetActions]() {
            for (const auto& im :
                 InviwoApplication::getPtr()->getModuleManager().getInviwoModules()) {
                auto moduleTFPath = im.getPath(ModulePath::TransferFunctions);
                if (!std::filesystem::is_directory(moduleTFPath)) continue;
                addPresetActions(moduleTFPath);
            }

            if (presets->actions().empty()) {
                auto action = presets->addAction("No Presets Available");
                action->setEnabled(false);
            }
        },
        Qt::SingleShotConnection);

    return presets;
}

namespace {

struct GenerateNDialog : QDialog {
    QCheckBox* normalized;
    QDoubleSpinBox* start;
    QDoubleSpinBox* middle;
    QDoubleSpinBox* stop;
    QSpinBox* steps;

    GenerateNDialog()
        : QDialog()
        , normalized{new QCheckBox{}}
        , start{new QDoubleSpinBox{}}
        , middle{new QDoubleSpinBox{}}
        , stop{new QDoubleSpinBox{}}
        , steps{new QSpinBox{}} {

        setWindowTitle("Specify Range");
        QGridLayout* layout = new QGridLayout(this);
        {
            const auto space = utilqt::emToPx(this, 15.0 / 9.0);
            layout->setContentsMargins(space, space, space, space);
        }

        normalized->setChecked(true);
        normalized->setDisabled(true);

        start->setMaximum(std::numeric_limits<double>::max());
        middle->setMaximum(std::numeric_limits<double>::max());
        stop->setMaximum(std::numeric_limits<double>::max());

        start->setMinimum(std::numeric_limits<double>::lowest());
        middle->setMinimum(std::numeric_limits<double>::lowest());
        stop->setMinimum(std::numeric_limits<double>::lowest());

        start->setValue(0.0);
        middle->setValue(0.5);
        stop->setValue(1.0);
        steps->setValue(10);
        steps->setMinimum(0);

        layout->addWidget(new QLabel("Normalized:"), 0, 0);
        layout->addWidget(new QLabel("Start:"), 1, 0);
        layout->addWidget(new QLabel("Middle:"), 2, 0);
        layout->addWidget(new QLabel("Stop:"), 3, 0);
        layout->addWidget(new QLabel("Steps:"), 4, 0);

        layout->addWidget(normalized, 0, 1);
        layout->addWidget(start, 1, 1);
        layout->addWidget(middle, 2, 1);
        layout->addWidget(stop, 3, 1);
        layout->addWidget(steps, 4, 1);

        QDialogButtonBox* buttonBox =
            new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &GenerateNDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &GenerateNDialog::reject);

        layout->addWidget(buttonBox, 5, 0, 1, 2);
    }
};
}  // namespace

QMenu* addTFColorbrewerPresetsMenu(QWidget* parent, QMenu* menu,
                                   TransferFunctionProperty* property) {
    if (!parent || !menu || !property) {
        return nullptr;
    }

    auto presets = menu->addMenu("&Colorbrewer Presets");
    presets->setObjectName("TF");
    presets->setEnabled(!property->getReadOnly());
    const int iconWidth = utilqt::emToPx(presets, 11);
    // need to set the stylesheet explicitely since Qt _only_ supports 'px' for icon sizes
    presets->setStyleSheet(QString("QMenu { icon-size: %1px; }").arg(iconWidth));

    auto addAction = [iconWidth, parent, property](QMenu* menu, TransferFunction tf,
                                                   std::string_view name) {
        auto action = menu->addAction(utilqt::toQString(name));
        action->setIcon(QIcon(utilqt::toQPixmap(tf, QSize{iconWidth, 20})));

        QObject::connect(action, &QAction::triggered, parent, [property, tf2 = std::move(tf)]() {
            NetworkLock lock(property);
            property->set(tf2);
        });
    };

    auto generateN = [property](colorbrewer::Category category, colorbrewer::Family family,
                                bool discrete) {
        return [category, family, discrete, property]() {
            GenerateNDialog dialog;
            if (category != colorbrewer::Category::Diverging) {
                dialog.middle->setDisabled(true);
            }

            if (property->getVolumeInport()) {
                dialog.normalized->setEnabled(true);
            }

            if (dialog.exec()) {
                auto start = dialog.start->value();
                auto middle = dialog.middle->value();
                auto stop = dialog.stop->value();
                auto steps = dialog.steps->value();

                if (!dialog.normalized->isChecked()) {
                    if (auto port = property->getVolumeInport()) {
                        const auto dataMap =
                            port->hasData() ? port->getData()->dataMap_ : DataMapper{};
                        start = dataMap.mapFromValueToNormalized(start);
                        middle = dataMap.mapFromValueToNormalized(middle);
                        stop = dataMap.mapFromValueToNormalized(stop);
                    }
                }

                property->set(colorbrewer::getTransferFunction(category, family, steps, discrete,
                                                               middle, start, stop));
            }
        };
    };

    QObject::connect(
        presets, &QMenu::aboutToShow, presets,
        [presets, iconWidth, addAction, generateN]() {
            for (auto category :
                 {colorbrewer::Category::Diverging, colorbrewer::Category::Qualitative,
                  colorbrewer::Category::Sequential}) {
                for (auto discrete : {true, false}) {

                    auto categoryMenu = presets->addMenu(utilqt::toQString(
                        toString(category) + +(discrete ? " Discrete" : " Contiguous")));
                    categoryMenu->setStyleSheet(
                        QString("QMenu { icon-size: %1px; }").arg(iconWidth));

                    for (auto family : colorbrewer::getFamiliesForCategory(category)) {
                        const auto max = colorbrewer::getMaxNumberOfColorsForFamily(family);
                        const auto min = colorbrewer::getMinNumberOfColorsForFamily(family);

                        auto familyMenu =
                            categoryMenu->addMenu(utilqt::toQString(toString(family)));
                        familyMenu->setStyleSheet(
                            QString("QMenu { icon-size: %1px; }").arg(iconWidth));
                        auto maxtf =
                            colorbrewer::getTransferFunction(category, family, max, discrete, 0.5);
                        familyMenu->setIcon(QIcon(utilqt::toQPixmap(maxtf, QSize{iconWidth, 20})));

                        for (auto n = min; n < max; ++n) {
                            addAction(familyMenu,
                                      colorbrewer::getTransferFunction(category, family, n,
                                                                       discrete, 0.5),
                                      toString(static_cast<int>(n)) + " colors");
                        }

                        auto action = familyMenu->addAction("Generate...");
                        QObject::connect(action, &QAction::triggered, presets,
                                         generateN(category, family, discrete));
                    }
                    if (!categoryMenu->actions().empty()) {
                        categoryMenu->setIcon(categoryMenu->actions().front()->icon());
                    }
                }
            }
        },
        Qt::SingleShotConnection);

    return presets;
}

}  // namespace util

}  // namespace inviwo

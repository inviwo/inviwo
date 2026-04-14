/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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

#include <inviwo/core/util/colorbrewer-generated.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/modulepath.h>
#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/tfprimitiveset.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/isovaluecollection.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterexception.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/util/colorbrewer.h>
#include <inviwo/core/util/scientificcolormaps.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/filedialogstate.h>
#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/stringconversion.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <initializer_list>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QObject>
#include <QSize>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <fmt/core.h>

namespace inviwo::util {

std::shared_ptr<TransferFunction> importTransferFunctionDialog() {
    InviwoFileDialog importFileDialog(nullptr, "Import Transfer Function", "transferfunction");
    importFileDialog.setAcceptMode(AcceptMode::Open);
    importFileDialog.setFileMode(FileMode::ExistingFile);

    auto* factory = util::getDataReaderFactory();

    importFileDialog.addExtensions(factory->getExtensionsForType<TransferFunction>());
    importFileDialog.addExtension(FileExtension::all());

    if (importFileDialog.exec()) {
        return util::exceptionGuard([&]() {
            const auto filename = utilqt::fromQString(importFileDialog.selectedFiles().at(0));
            if (auto reader = factory->getReaderForTypeAndExtension<TransferFunction>(
                    importFileDialog.getSelectedFileExtension(), filename)) {

                return reader->readData(filename);
            } else {
                throw DataReaderException(
                    SourceContext{}, "Unable to find a Transfer function reader for {}", filename);
            }
        });
    }
    return nullptr;
}

std::shared_ptr<IsoValueCollection> importIsoValueCollectionDialog() {
    InviwoFileDialog importFileDialog(nullptr, "Import Iso Value Collection", "isovaluecollection");
    importFileDialog.setAcceptMode(AcceptMode::Open);
    importFileDialog.setFileMode(FileMode::ExistingFile);

    auto* factory = util::getDataReaderFactory();

    importFileDialog.addExtensions(factory->getExtensionsForType<IsoValueCollection>());
    importFileDialog.addExtension(FileExtension::all());

    if (importFileDialog.exec()) {
        return util::exceptionGuard([&]() {
            const auto filename = utilqt::fromQString(importFileDialog.selectedFiles().at(0));
            if (auto reader = factory->getReaderForTypeAndExtension<IsoValueCollection>(
                    importFileDialog.getSelectedFileExtension(), filename)) {

                return reader->readData(filename);
            } else {
                throw DataReaderException(SourceContext{},
                                          "Unable to find a Iso Value Collection reader for {}",
                                          filename);
            }
        });
    }
    return nullptr;
}

void exportTransferFunctionDialog(const TransferFunction& tf) {
    InviwoFileDialog exportFileDialog(nullptr, "Export Transfer Function", "transferfunction");
    exportFileDialog.setAcceptMode(AcceptMode::Save);
    exportFileDialog.setFileMode(FileMode::AnyFile);

    auto* factory = util::getDataWriterFactory();

    exportFileDialog.addExtensions(factory->getExtensionsForType<TransferFunction>());
    exportFileDialog.addExtension(FileExtension::all());

    if (exportFileDialog.exec()) {
        util::exceptionGuard([&]() {
            const auto filename = utilqt::fromQString(exportFileDialog.selectedFiles().at(0));
            const auto fileExt = exportFileDialog.getSelectedFileExtension();

            if (auto writer =
                    factory->getWriterForTypeAndExtension<TransferFunction>(fileExt, filename)) {
                writer->writeData(&tf, filename);

                log::info("Data exported to disk: {}", filename);
            } else {
                throw DataWriterException(
                    SourceContext{}, "Unable to find a Transfer Function writer for {}", filename);
            }
        });
    }
}

void exportIsoValueCollectionDialog(const IsoValueCollection& iso) {
    InviwoFileDialog exportFileDialog(nullptr, "Export Iso Value Collection", "isovaluecollection");
    exportFileDialog.setAcceptMode(AcceptMode::Save);
    exportFileDialog.setFileMode(FileMode::AnyFile);

    auto* factory = util::getDataWriterFactory();

    exportFileDialog.addExtensions(factory->getExtensionsForType<IsoValueCollection>());
    exportFileDialog.addExtension(FileExtension::all());

    if (exportFileDialog.exec()) {
        util::exceptionGuard([&]() {
            const auto filename = utilqt::fromQString(exportFileDialog.selectedFiles().at(0));
            const auto fileExt = exportFileDialog.getSelectedFileExtension();

            if (auto writer =
                    factory->getWriterForTypeAndExtension<IsoValueCollection>(fileExt, filename)) {
                writer->writeData(&iso, filename);

                log::info("Data exported to disk: {}", filename);
            } else {
                throw DataWriterException(SourceContext{},
                                          "Unable to find a Iso Value Collection writer for {}",
                                          filename);
            }
        });
    }
}

namespace {

QAction* tfAction(std::string_view name, TransferFunction tf, QMenu* menu,
                  TransferFunctionProperty* property, QObject* parent) {
    auto* action = menu->addAction(utilqt::toQString(name));
    const int iconWidth = utilqt::emToPx(menu, 11);
    action->setIcon(QIcon(utilqt::toQPixmap(tf, QSize{iconWidth, 20})));
    action->setIconVisibleInMenu(true);
    QObject::connect(action, &QAction::triggered, parent,
                     util::exceptionGuarded([property, tf2 = std::move(tf)]() {
                         const NetworkLock lock(property);
                         property->set(tf2);
                     }));
    return action;
}

QMenu* addCategory(std::string_view name, QMenu* menu, const TransferFunction& tf) {
    auto* category = menu->addMenu(utilqt::toQString(name));
    const int iconWidth = utilqt::emToPx(menu, 11);
    category->setStyleSheet(QString("QMenu { icon-size: %1px; }").arg(iconWidth));
    category->setIcon(QIcon(utilqt::toQPixmap(tf, QSize{iconWidth, 20})));
    category->menuAction()->setIconVisibleInMenu(true);
    return category;
}

}  // namespace

QMenu* addTFPresetsMenu(QWidget* parent, QMenu* menu, TransferFunctionProperty* property) {
    if (!parent || !menu || !property) {
        return nullptr;
    }

    auto* presets = menu->addMenu("&TF Presets");
    presets->setObjectName("TF");
    presets->setEnabled(!property->getReadOnly());
    // need to set the stylesheet explicitly since Qt _only_ supports 'px' for icon sizes
    presets->setStyleSheet(QString("QMenu { icon-size: %1px; }").arg(utilqt::emToPx(presets, 11)));

    const auto addPresetActions = [presets, parent,
                                   property](const std::filesystem::path& basePath) {
        auto* factory = util::getDataReaderFactory();
        auto files = filesystem::getDirectoryContentsRecursively(basePath);
        for (const auto& file : files) {
            if (auto reader = factory->getReaderForTypeAndExtension<TransferFunction>(file)) {
                util::exceptionGuard([&]() {
                    auto tf = reader->readData(basePath / file);
                    tfAction(file.string(), std::move(*tf), presets, property, parent);
                });
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
                auto* action = presets->addAction("No Presets Available");
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
        auto* layout = new QGridLayout(this);
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

        auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
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

    auto* presets = menu->addMenu("&Colorbrewer Presets");
    presets->setObjectName("TF");
    presets->setEnabled(!property->getReadOnly());
    presets->setStyleSheet(QString("QMenu { icon-size: %1px; }").arg(utilqt::emToPx(presets, 11)));

    auto generateN = [property](colorbrewer::Category category, colorbrewer::Family family,
                                bool discrete) {
        return [category, family, discrete, property]() {
            GenerateNDialog dialog;
            if (category != colorbrewer::Category::Diverging) {
                dialog.middle->setDisabled(true);
            }

            if (property->data().getDataMap() != nullptr) {
                dialog.normalized->setEnabled(true);
            }

            if (dialog.exec()) {
                util::exceptionGuard([&]() {
                    auto start = dialog.start->value();
                    auto middle = dialog.middle->value();
                    auto stop = dialog.stop->value();
                    auto steps = dialog.steps->value();

                    if (!dialog.normalized->isChecked()) {
                        if (const auto* dataMap = property->data().getDataMap()) {
                            start = dataMap->mapFromValueToNormalized(start);
                            middle = dataMap->mapFromValueToNormalized(middle);
                            stop = dataMap->mapFromValueToNormalized(stop);
                        }
                    }
                    property->set(colorbrewer::getTransferFunction(category, family, steps,
                                                                   discrete, middle, start, stop));
                });
            }
        };
    };

    QObject::connect(
        presets, &QMenu::aboutToShow, presets,
        [presets, property, parent, generateN]() {
            using enum colorbrewer::Category;
            for (auto category : {Diverging, Qualitative, Sequential}) {
                for (auto discrete : {true, false}) {

                    auto categoryName =
                        fmt::format("{} {}", category, (discrete ? " Discrete" : " Contiguous"));
                    auto first = colorbrewer::getFamiliesForCategory(category).front();
                    auto categoryTF = colorbrewer::getTransferFunction(
                        category, first, colorbrewer::getMaxNumberOfColorsForFamily(first),
                        discrete, 0.5);
                    auto* categoryMenu = addCategory(categoryName, presets, categoryTF);

                    for (auto family : colorbrewer::getFamiliesForCategory(category)) {
                        const auto max = colorbrewer::getMaxNumberOfColorsForFamily(family);
                        const auto min = colorbrewer::getMinNumberOfColorsForFamily(family);

                        auto familyTF =
                            colorbrewer::getTransferFunction(category, family, max, discrete, 0.5);
                        auto* familyMenu = addCategory(toString(family), categoryMenu, familyTF);
                        for (auto n = min; n < max; ++n) {
                            tfAction(fmt::format("{} colors", n),
                                     colorbrewer::getTransferFunction(category, family, n, discrete,
                                                                      0.5),
                                     familyMenu, property, parent);
                        }

                        auto* action = familyMenu->addAction("Generate...");
                        QObject::connect(action, &QAction::triggered, presets,
                                         generateN(category, family, discrete));
                    }
                }
            }
        },
        Qt::SingleShotConnection);

    return presets;
}

TransferFunction colorListToTF(std::span<const glm::vec3> points, bool discrete) {
    if (discrete) {
        const auto n = static_cast<double>(points.size());
        const double delta = 1e-2 / static_cast<double>(points.size());
        return TransferFunction{
            std::views::zip(std::views::iota(0uz), points) |
            std::views::transform([&](auto&& elem) {
                auto&& [i, c] = elem;
                return std::array<TFPrimitiveData, 2>{
                    {{.pos = static_cast<double>(i) / n, .color = {c, 1.0f}},
                     {.pos = static_cast<double>(i + 1) / n - delta, .color = {c, 1.0f}}}};
            }) |
            std::views::join | std::ranges::to<std::vector>()};
    } else {
        const auto n = static_cast<double>(points.size() - 1);
        return TransferFunction{std::views::zip(std::views::iota(0uz), points) |
                                std::views::transform([&](auto&& elem) {
                                    auto&& [i, c] = elem;
                                    return TFPrimitiveData{.pos = static_cast<double>(i) / n,
                                                           .color = {c, 1.0f}};
                                }) |
                                std::ranges::to<std::vector>()};
    }
}

QMenu* addScientificColorMapsPresetsMenu(QWidget* parent, QMenu* menu,
                                         TransferFunctionProperty* property) {

    if (!parent || !menu || !property) {
        return nullptr;
    }

    auto* presets = menu->addMenu("&Scientific Colormap Presets");
    presets->setObjectName("TF");
    presets->setEnabled(!property->getReadOnly());
    presets->setStyleSheet(QString("QMenu { icon-size: %1px; }").arg(utilqt::emToPx(presets, 11)));

    const auto addAction = [parent, property](QMenu* menu, std::span<const glm::vec3> points,
                                              std::string_view name, bool discrete) {
        auto tf = colorListToTF(points, discrete);
        tfAction(name, tf, menu, property, parent);
    };

    QObject::connect(
        presets, &QMenu::aboutToShow, presets,
        [presets, addAction]() {
            {
                auto* category =
                    addCategory("Continuous", presets,
                                colorListToTF(scm::get(scm::allContinuous().front()), false));
                QObject::connect(
                    category, &QMenu::aboutToShow, category,
                    [category, addAction]() {
                        for (auto cm : scm::allContinuous()) {
                            addAction(category, scm::get(cm), format_as(cm), false);
                        }
                    },
                    Qt::SingleShotConnection);
            }
            {
                auto* category = addCategory(
                    "Cyclic", presets, colorListToTF(scm::get(scm::allCyclic().front()), false));
                QObject::connect(
                    category, &QMenu::aboutToShow, category,
                    [category, addAction]() {
                        for (auto cm : scm::allCyclic()) {
                            addAction(category, scm::get(cm), format_as(cm), false);
                        }
                    },
                    Qt::SingleShotConnection);
            }
            {
                auto* category =
                    addCategory("Categorical", presets,
                                colorListToTF(scm::get(scm::allCategorical().front()), true));
                QObject::connect(
                    category, &QMenu::aboutToShow, category,
                    [category, addAction]() {
                        for (auto cm : scm::allCategorical()) {
                            addAction(category, scm::get(cm), format_as(cm), true);
                        }
                    },
                    Qt::SingleShotConnection);
            }

            for (auto size : scm::allDiscreteSize()) {
                auto* category =
                    addCategory(fmt::format("Discrete {}", size), presets,
                                colorListToTF(scm::get(scm::allDiscrete().front(), size), true));
                QObject::connect(
                    category, &QMenu::aboutToShow, category,
                    [category, addAction, size]() {
                        for (auto cm : scm::allDiscrete()) {
                            addAction(category, scm::get(cm, size), format_as(cm), true);
                        }
                    },
                    Qt::SingleShotConnection);
            }
        },
        Qt::SingleShotConnection);

    return presets;
}
}  // namespace inviwo::util

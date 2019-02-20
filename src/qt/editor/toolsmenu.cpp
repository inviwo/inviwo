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

#include <inviwo/qt/editor/toolsmenu.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/network/processornetwork.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#ifdef IVW_INVIWO_META
#include <inviwo/qt/editor/toolsmetamenu.h>
#endif

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QInputDialog>
#include <QClipboard>
#include <QFile>
#include <QApplication>

#include <iostream>
#include <algorithm>
#include <warn/pop>

namespace inviwo {

namespace {

void createProcessorDocMenu(InviwoApplication *app, QMenu *docsMenu) {
    auto factory = app->getProcessorFactory();

    for (auto &m : app->getModules()) {
        auto &processors = m->getProcessors();
        if (processors.empty()) continue;

        auto modMenu = docsMenu->addMenu(utilqt::toQString(m->getIdentifier()));
        for (auto &pfo : processors) {
            auto action = modMenu->addAction(utilqt::toQString(pfo->getDisplayName()));
            docsMenu->connect(action, &QAction::triggered, [pfo, factory]() {
                const auto processor = factory->create(pfo->getClassIdentifier());
                const auto &inports = processor->getInports();
                const auto &outports = processor->getOutports();
                const auto &properties = processor->getPropertiesRecursive();
                const auto classID = processor->getClassIdentifier();
                const auto &dispName = processor->getDisplayName();

                std::ostringstream oss;
                oss << "/** \\docpage{" << classID << ", " << dispName << "}" << std::endl;
                oss << "* ![](" << classID << ".png?classIdentifier=" << classID << ")"
                    << std::endl;
                oss << "* " << std::endl;
                oss << "* Description of the processor" << std::endl;
                oss << "* " << std::endl;
                oss << "* " << std::endl;
                if (!inports.empty()) {
                    oss << "* ### Inports" << std::endl;
                    for (const auto &port : inports) {
                        oss << "*   * __" << port->getIdentifier() << "__ Describe port.\n";
                    }
                    oss << "* \n";
                }

                if (!outports.empty()) {
                    oss << "* ### Outports" << std::endl;
                    for (const auto &port : outports) {
                        oss << "*   * __" << port->getIdentifier() << "__ Describe port.\n";
                    }
                    oss << "* \n";
                }

                if (!properties.empty()) {
                    oss << "* ### Properties" << std::endl;
                    for (const auto &prop : properties) {
                        oss << "*   * __" << prop->getDisplayName() << "__ Describe property.\n";
                    }
                    oss << "* \n";
                }
                oss << "*/\n";

                QApplication::clipboard()->setText(utilqt::toQString(oss.str()));
                LogInfoCustom("ToolMenu", "DOXYGEN Template code for processor "
                                              << dispName << " copied to clipboard");
                LogInfoCustom("ToolMenu", oss.str());
            });
        }
    }
}

void createRegressionActions(QWidget *parent, InviwoApplication *app, QMenu *menu) {
    for (const auto &module : app->getModules()) {
        auto action = menu->addAction(utilqt::toQString(module->getIdentifier()));

        QObject::connect(
            action, &QAction::triggered, [modulename = module->getIdentifier(), parent, app]() {
                bool ok;
                const auto name = QInputDialog::getText(
                    parent, "Create Regression Test", "Regression test name", QLineEdit::Normal, "",
                    &ok, Qt::WindowFlags() | Qt::MSWindowsFixedSizeDialogHint);
                if (ok) {
                    const auto lname = toLower(utilqt::fromQString(name));

                    LogInfoCustom("ToolMenu", "Creating regression test " << lname);
                    if (auto module = app->getModuleByIdentifier(modulename)) {
                        const auto regressiondir = module->getPath(ModulePath::RegressionTests);
                        const auto testdir = regressiondir + "/" + lname;
                        if (filesystem::directoryExists(testdir)) {
                            LogErrorCustom(
                                "ToolMenu",
                                "Dir: \"" << testdir << "\" already exits. use a different name");
                            return;
                        }
                        filesystem::createDirectoryRecursively(testdir);

                        const auto workspacename = testdir + "/" + lname + ".inv";
                        LogInfoCustom("ToolMenu",
                                      "Saving regression workspace to: " << workspacename);
                        util::saveNetwork(app->getProcessorNetwork(), workspacename);
                        util::saveAllCanvases(app->getProcessorNetwork(), testdir);
                    }
                }
            });
    }
}

}  // namespace

ToolsMenu::ToolsMenu(InviwoMainWindow *win) : QMenu(tr("&Tools"), win) {
    auto docsMenu = addMenu("Create &Processors Docs");
    connect(docsMenu, &QMenu::aboutToShow, [docsMenu, win]() {
        docsMenu->clear();
        createProcessorDocMenu(win->getInviwoApplication(), docsMenu);
    });

    auto regressionMenu = addMenu("Create &Regression Test");
    connect(regressionMenu, &QMenu::aboutToShow, [regressionMenu, win]() {
        regressionMenu->clear();
        createRegressionActions(win, win->getInviwoApplication(), regressionMenu);
    });

#ifdef IVW_INVIWO_META
    auto sourceMenu = addMenu("Create &Sources");
    addInviwoMetaAction(sourceMenu);
#endif
}

}  // namespace inviwo

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

#include <inviwo/qt/editor/toolsmenu.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/exporter.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/workspaceutils.h>
#include <inviwo/core/network/lambdanetworkvisitor.h>

#include <modules/qtwidgets/inviwofiledialog.h>
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

void createProcessorDocMenu(InviwoApplication* app, QMenu* docsMenu) {
    auto factory = app->getProcessorFactory();

    std::vector<InviwoModule*> modules;
    for(auto& m : app->getModuleManager().getInviwoModules()){
        modules.push_back(&m);
    }
    auto order = util::ordering(modules, [](auto& a, auto& b) {
        return iCaseLess(a->getIdentifier(), b->getIdentifier());
    });
    for (size_t i : order) {
        const auto& m = modules[i];

        auto& processors = m->getProcessors();
        if (processors.empty()) continue;

        auto modMenu = docsMenu->addMenu(utilqt::toQString(m->getIdentifier()));
        for (auto& pfo : processors) {
            auto action = modMenu->addAction(utilqt::toQString(pfo->getDisplayName()));
            docsMenu->connect(action, &QAction::triggered, [pfo, factory]() {
                const auto processor = factory->create(pfo->getClassIdentifier());
                const auto& inports = processor->getInports();
                const auto& outports = processor->getOutports();
                const auto& properties = processor->getPropertiesRecursive();
                const auto classID = processor->getClassIdentifier();
                const auto& dispName = processor->getDisplayName();

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
                    for (const auto& port : inports) {
                        oss << "*   * __" << port->getIdentifier() << "__ Describe port.\n";
                    }
                    oss << "* \n";
                }

                if (!outports.empty()) {
                    oss << "* ### Outports" << std::endl;
                    for (const auto& port : outports) {
                        oss << "*   * __" << port->getIdentifier() << "__ Describe port.\n";
                    }
                    oss << "* \n";
                }

                if (!properties.empty()) {
                    oss << "* ### Properties" << std::endl;
                    for (const auto& prop : properties) {
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

void createRegressionActions(QWidget* parent, InviwoApplication* app, QMenu* menu) {
    std::vector<InviwoModule*> modules;
    for(auto& m : app->getModuleManager().getInviwoModules()){
        modules.push_back(&m);
    }
    auto order = util::ordering(modules, [](auto& a, auto& b) {
        return iCaseLess(a->getIdentifier(), b->getIdentifier());
    });
    for (size_t i : order) {
        const auto& module = modules[i];
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
                        const auto testdir = regressiondir / lname;
                        if (std::filesystem::is_directory(testdir)) {
                            LogErrorCustom(
                                "ToolMenu",
                                "Dir: \"" << testdir << "\" already exits. use a different name");
                            return;
                        }
                        try {
                            std::filesystem::create_directories(testdir);

                            const auto workspaceName = testdir / (lname + ".inv");
                            LogInfoCustom("ToolMenu",
                                          "Saving regression workspace to: " << workspaceName);

                            app->getWorkspaceManager()->save(workspaceName);
                            util::exportAllFiles(
                                *app->getProcessorNetwork(), testdir, "UPN",
                                {FileExtension{"png", ""}, FileExtension{"csv", ""},
                                 FileExtension{"txt", ""}},
                                Overwrite::Yes);

                        } catch (const Exception& error) {
                            LogErrorCustom("ToolsMenu",
                                           fmt::format("Failed to create regression test: {}",
                                                       error.getMessage()));
                            // TODO delete testdir here
                        }
                    }
                }
            });
    }
}

}  // namespace

ToolsMenu::ToolsMenu(InviwoMainWindow* win) : QMenu(tr("&Tools"), win) {
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

    auto workspaceMenu = addMenu("Workspaces");
    workspaceMenu->setToolTipsVisible(true);

    auto loadExampleWorkspaces = workspaceMenu->addAction("Load Example Workspaces");
    loadExampleWorkspaces->setToolTip(
        "Load each workspace after each other, useful to see if there are any problems when load "
        "the workspaces");
    connect(loadExampleWorkspaces, &QAction::triggered, [win]() {
        try {
            util::updateExampleWorkspaces(win->getInviwoApplication(), util::DryRun::Yes, []() {
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            });
        } catch (const Exception& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error);
        }
    });

    auto updateExampleWorkspaces = workspaceMenu->addAction("Update Example Workspaces");
    updateExampleWorkspaces->setToolTip(
        "Load and save each workspace after each other, useful to update workspace versions");
    connect(updateExampleWorkspaces, &QAction::triggered, [win]() {
        try {
            util::updateExampleWorkspaces(win->getInviwoApplication(), util::DryRun::No, []() {
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            });
        } catch (const Exception& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error);
        }
    });

    auto loadRegressionWorkspaces = workspaceMenu->addAction("Load Regression Workspaces");
    loadRegressionWorkspaces->setToolTip(
        "Load each workspace after each other, useful to see if there are any problems when load "
        "the workspaces");
    connect(loadRegressionWorkspaces, &QAction::triggered, [win]() {
        try {
            util::updateRegressionWorkspaces(win->getInviwoApplication(), util::DryRun::Yes, []() {
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            });
        } catch (const Exception& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error);
        }
    });

    auto updateRegressionWorkspaces = workspaceMenu->addAction("Update Regression Workspaces");
    updateRegressionWorkspaces->setToolTip(
        "Load and save each workspace after each other, useful to update workspace versions");

    connect(updateRegressionWorkspaces, &QAction::triggered, [win]() {
        try {
            util::updateRegressionWorkspaces(win->getInviwoApplication(), util::DryRun::No, []() {
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            });
        } catch (const Exception& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error);
        }
    });

    auto loadWorkspaces = workspaceMenu->addAction("Load Workspaces In Folder...");
    loadWorkspaces->setToolTip(
        "Load each workspace after each other, useful to see if there are any problems when load "
        "the workspaces. Loads all workspace found in the folder recursively.");
    connect(loadWorkspaces, &QAction::triggered, [win]() {
        InviwoFileDialog dialog(nullptr, "workspace", "Workspace Directory");
        dialog.setFileMode(FileMode::Directory);
        dialog.setAcceptMode(AcceptMode::Open);

        if (dialog.exec()) {
            QString qpath = dialog.selectedFiles().at(0);
            const auto path = utilqt::fromQString(qpath);
            try {
                util::updateWorkspaces(win->getInviwoApplication(), path, util::DryRun::Yes, []() {
                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                });
            } catch (const Exception& e) {
                util::log(e.getContext(), e.getMessage(), LogLevel::Error);
            }
        }
    });

    auto updateWorkspaces = workspaceMenu->addAction("Update Workspaces In Folder...");
    updateWorkspaces->setToolTip(
        "Load and save each workspace after each other, useful to update workspace versions. Loads "
        "all workspace found in the folder recursively.");

    connect(updateWorkspaces, &QAction::triggered, [win]() {
        InviwoFileDialog dialog(nullptr, "workspace", "Workspace Directory");
        dialog.setFileMode(FileMode::Directory);
        dialog.setAcceptMode(AcceptMode::Open);

        if (dialog.exec()) {
            QString qpath = dialog.selectedFiles().at(0);
            const auto path = utilqt::fromQString(qpath);
            try {
                util::updateWorkspaces(win->getInviwoApplication(), path, util::DryRun::No, []() {
                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                });
            } catch (const Exception& e) {
                util::log(e.getContext(), e.getMessage(), LogLevel::Error);
            }
        }
    });

    auto propertyPaths = workspaceMenu->addAction("Load and log all properties");
    connect(propertyPaths, &QAction::triggered, [win]() {
        auto app = win->getInviwoApplication();
        auto factory = app->getProcessorFactory();

        std::string buffer;

        for (auto&& key : factory->getKeyView()) {
            try {
                auto processor = factory->create(key, app);

                size_t indent = 0;
                LambdaNetworkVisitor visitor{
                    [&](CompositeProperty& p, NetworkVisitorEnter) {
                        fmt::format_to(std::back_inserter(buffer), "{:{}}{}\n", "", indent,
                                       p.getIdentifier());
                        indent += 4;
                    },
                    [&](CompositeProperty&, NetworkVisitorExit) { indent -= 4; },
                    [&](Processor& p, NetworkVisitorEnter) {
                        fmt::format_to(std::back_inserter(buffer), "{:{}}{}\n", "", indent,
                                       p.getIdentifier());
                        indent += 4;
                    },
                    [&](Processor&, NetworkVisitorExit) { indent -= 4; },

                    [&](Property& p) {
                        fmt::format_to(std::back_inserter(buffer), "{:{}}{}\n", "", indent,
                                       p.getIdentifier());
                    }};
                processor->accept(visitor);
            } catch (const Exception& e) {
                util::log(e.getContext(), e.getMessage());
            }
        }
        LogInfo(buffer);
    });

}  // namespace inviwo

}  // namespace inviwo

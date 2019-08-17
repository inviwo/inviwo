/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif

#include <inviwo/core/common/defaulttohighperformancegpu.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/workspacemanager.h>

#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/consolelogger.h>

#include <inviwo/core/moduleregistration.h>

#include <QtWidgets>

#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>

#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>
#include <modules/qtwidgets/properties/texteditorwidgetqt.h>
#include <modules/qtwidgets/properties/filepropertywidgetqt.h>
#include <modules/qtwidgets/propertylistwidget.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/properties/compositeproperty.h>

#include <filesystem>

/**
 * Represents processor description in config file.
 */
struct ProcDesc {
    ProcDesc(std::string id) : id(id) {}
    std::string id;  // proc identifier
    bool operator==(ProcDesc& other) { return this->id == other.id; }
    bool operator!=(ProcDesc& other) { return this->id != other.id; }
};

/**
 * Represents property description in config file.
 * Has references to form tree structures, where processors are roots,
 * composites are inner nodes and properties are leaf nodes.
 * Note that this is not a tree like you would expect,
 * because properties have a global order, independent of what processors they belong to.
 */
struct PropDesc {
    PropDesc(size_t procIndex, int compositeIndex, std::string id, std::string displayName = "")
        : procIndex(procIndex), compositeIndex(compositeIndex), id(id), displayName(displayName) {}
    size_t procIndex;    // prop must belong to processor
    int compositeIndex;  // prop can belong to composite prop
    std::string id;      // prop identifier
    std::string displayName;
};

/**
 * Represents config file.
 * Defines properties that appear in the GUI, including their order.
 * Points to a workspace file, that it can be applied to.
 */
struct Config {
    bool parseSuccess = true;
    std::vector<ProcDesc> procs;
    std::vector<PropDesc> props;
    std::string workspace;
};

// test config file under: \inviwo\build\apps\minimals\qt

/**
 * Load config from file,
 * where first line contains workspace
 * and all following lines the properties with the following syntax:
 * Processor.Composite.Property:DisplayName
 */
Config loadConfig(std::string file) {
    Config config;
    std::ifstream in(file);

    if (!in.good()) {
        config.parseSuccess = false;
        return config;
    }

    std::string line;
    for (size_t lineIndex = 0; std::getline(in, line); lineIndex++) {

        // Trim empty lines
        if (line.empty()) continue;
        // Do not trim other whitespace since it may be part of identifiers

        // First line contains workspace path (relative to config file path)
        if (lineIndex == 0) {
            std::string dir = inviwo::filesystem::getFileDirectory(file);
            config.workspace = dir + "/" + line;
            continue;
        }

        size_t pos, last_pos;
        int procIndex;

        // The path to a property is separated by dots
        if ((pos = line.find('.')) != std::string::npos) {

            // There is one processor in each line...
            ProcDesc proc(line.substr(0, pos));

            // ... which maybe already appeared in previous lines...
            procIndex = -1;
            for (int i = 0; i < config.procs.size(); i++)
                if (config.procs[i] == proc) procIndex = i;

            // ... or not.
            if (procIndex < 0) {
                procIndex = (int)config.procs.size();
                config.procs.push_back(proc);
            }

            // At this position follow the properties...
            last_pos = pos;

            // ... possibly multiple composited properties...
            int last_propIndex = -1;  // points to parent composite if existing
            while ((pos = line.find('.', last_pos + 1)) != std::string::npos) {
                PropDesc prop(procIndex, last_propIndex,
                              line.substr(last_pos + 1, pos - last_pos - 1));
                last_propIndex = (int)config.props.size();
                config.props.push_back(prop);
                last_pos = pos;
            }

            // ... but at least one.
            // At the end may be an optional display name.
            const auto propStr = line.substr(last_pos + 1);
            const size_t colonPos = propStr.find(':');
            std::string propIdentifier, propDisplayName;
            if (colonPos != std::string::npos) {
                propIdentifier = propStr.substr(0, colonPos);
                propDisplayName = propStr.substr(colonPos + 1);
            } else {
                propIdentifier = propStr;
            }
            PropDesc prop(procIndex, last_propIndex, propIdentifier, propDisplayName);
            config.props.push_back(prop);
        }
    }

    // Useful config has at least one processor and at least one property per processor...
    if (config.procs.size() < 1 || config.props.size() < config.procs.size()) {
        config.parseSuccess = false;
    }
    // ... and all ids are non-empty strings
    for (const auto& proc : config.procs) {
        if (proc.id.empty()) {
            config.parseSuccess = false;
            break;
        }
    }
    for (const auto& prop : config.props) {
        if (prop.id.empty()) {
            config.parseSuccess = false;
            break;
        }
    }

    return config;
}

using namespace inviwo;

/**
 * Searches the property object for the given descriptor in the processor network.
 * Uses parsed config to find property in correct subtree.
 * Returns pointer to the property and optionally pointer to its owner.
 * If property not found, nullptr is returned.
 */
Property* getProp(const PropDesc& desc, const Config& config, ProcessorNetwork* network,
                  PropertyOwner** outOwner = 0) {
    if (desc.compositeIndex < 0) {
        Processor* proc = network->getProcessorByIdentifier(config.procs[desc.procIndex].id);
        if (!proc) return nullptr;
        if (outOwner) *outOwner = proc;
        return proc->getPropertyByIdentifier(desc.id);
    }
    CompositeProperty* composite = dynamic_cast<CompositeProperty*>(
        getProp(config.props[desc.compositeIndex], config, network, outOwner));
    if (!composite) return nullptr;
    if (outOwner) *outOwner = composite;
    return composite->getPropertyByIdentifier(desc.id);
}

/**
 * Load workspace from filepath using workspace manager
 */
bool loadWorkspace(std::string workspace, WorkspaceManager* manager) {
    try {
        if (!workspace.empty()) {
            manager->load(workspace, [&](ExceptionContext ec) {
                try {
                    throw;
                } catch (const IgnoreException& e) {
                    util::log(
                        e.getContext(),
                        "Incomplete network loading " + workspace + " due to " + e.getMessage(),
                        LogLevel::Error);
                }
            });
        }
    } catch (const AbortException& exception) {
        util::log(exception.getContext(),
                  "Unable to load network " + workspace + " due to " + exception.getMessage(),
                  LogLevel::Error);
        return false;
    } catch (const IgnoreException& exception) {
        util::log(exception.getContext(),
                  "Incomplete network loading " + workspace + " due to " + exception.getMessage(),
                  LogLevel::Error);
        return false;
    } catch (const ticpp::Exception& exception) {
        LogErrorCustom("qtminimum", "Unable to load network " + workspace +
                                        " due to deserialization error: " + exception.what());
        return false;
    }
    return true;
}

/**
 * Qt widget to show properties
 * (similar to PropertyListWidget, but able to show individual properties instead of processor
 * groups)
 */
class CustomPropertyListWidget : public QDockWidget {
    QVBoxLayout* listLayout_;
    QWidget* listWidget_;
    QScrollArea* scrollArea_;
    std::vector<QWidget*> elements_;

public:
    CustomPropertyListWidget(QWidget* parent) : QDockWidget("Properties", parent) {

        setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        resize(utilqt::emToPx(this, QSizeF(45, 80)));  // default size

        QSizePolicy sp(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        sp.setVerticalStretch(1);
        sp.setHorizontalStretch(1);
        setSizePolicy(sp);

        const auto space = utilqt::refSpacePx(this);

        scrollArea_ = new QScrollArea(this);
        scrollArea_->setWidgetResizable(true);
        scrollArea_->setMinimumWidth(utilqt::emToPx(this, 30));
        scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#ifdef __APPLE__
        // Scrollbars are overlayed in different way on mac...
        scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#else
        scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
#endif
        scrollArea_->setFrameShape(QFrame::NoFrame);
        scrollArea_->setContentsMargins(0, space, 0, space);

        listWidget_ = new PropertyListFrame(this);
        listLayout_ = new QVBoxLayout();
        listWidget_->setLayout(listLayout_);
        listLayout_->setAlignment(Qt::AlignTop);
#ifdef __APPLE__
        // Add some space for the scrollbar on mac
        listLayout_->setContentsMargins(0, space, 10, space);
#else
        listLayout_->setContentsMargins(0, space, 0, space);
#endif
        listLayout_->setSpacing(space);
        listLayout_->setSizeConstraint(QLayout::SetMinAndMaxSize);

        scrollArea_->setWidget(listWidget_);
        setWidget(scrollArea_);
        setFeatures(QDockWidget::NoDockWidgetFeatures);
    }

    ~CustomPropertyListWidget() = default;

    int addProperty(Property* prop) {
        auto factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
        if (auto propertyWidget = static_cast<PropertyWidgetQt*>(factory->create(prop).release())) {
            listLayout_->insertWidget(-1, propertyWidget, 0, Qt::AlignTop);
            int index = (int)elements_.size();
            elements_.push_back(propertyWidget);
            return index;
        }
        return -1;
    }

    int addPropertyGroup(Processor* proc) {
        // The collapsible group widget is for interacting with property tree,
        // where processor is root, composites are inner nodes and properties are leaf nodes.
        auto widget = new CollapsibleGroupBoxWidgetQt(proc);
        widget->hide();

        // Insert at end
        int index = (int)elements_.size();
        elements_.push_back(widget);
        listLayout_->insertWidget(-1, widget, 0, Qt::AlignTop);

        for (auto prop : proc->getProperties()) {
            widget->addProperty(prop);
        }
        widget->show();
        return index;
    }

    int addPropertyGroup(CompositeProperty* composite) {
        auto widget = new CollapsibleGroupBoxWidgetQt(composite);
        widget->hide();
        int index = (int)elements_.size();
        elements_.push_back(widget);
        listLayout_->insertWidget(-1, widget, 0, Qt::AlignTop);
        for (auto prop : composite->getProperties()) {
            widget->addProperty(prop);
        }
        widget->show();
        return index;
    }

    int addPlaceholder(std::string str) {
        QLabel* label = new QLabel(this);
        label->setText(QString(str.c_str()));
        listLayout_->insertWidget(-1, label, 0, Qt::AlignTop);
        int index = (int)elements_.size();
        elements_.push_back(label);
        return index;
    }

    int addButton(std::string label, std::function<void()> onclick) {
        auto button = new QPushButton(this);
        button->setText(QString(label.c_str()));
        connect(button, &QPushButton::clicked, onclick);
        listLayout_->insertWidget(-1, button, 0, Qt::AlignTop);
        int index = (int)elements_.size();
        elements_.push_back(button);
        return index;
    }

    void setAtIndex(unsigned int index, Property* prop) {
        auto factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
        if (auto propertyWidget = static_cast<PropertyWidgetQt*>(factory->create(prop).release())) {
            delete elements_[index];  // also removes it from the layout
            elements_[index] = propertyWidget;
            listLayout_->insertWidget(index, propertyWidget, 0, Qt::AlignTop);
        }
    }
};

/**
 * Class for property observers, that get notified when properties are dynamically added or removed.
 * Can be registered on property owners, i.e. processors or composite properties.
 * The observer is only interested in one property of the owner.
 */
class DynamicPropObserver : public PropertyOwnerObserver {

    const PropDesc propDesc_;             // description of prop to be observed
    const Config config_;                 // config object containing prop relations
    ProcessorNetwork* network_;           // processor network where the prop is searched
    CustomPropertyListWidget* propList_;  // Qt widget where the prop widget is shown
    unsigned int index_;                  // position in the Qt widget

public:
    DynamicPropObserver(const PropDesc propDesc, const Config config, ProcessorNetwork* network,
                        CustomPropertyListWidget* propList, unsigned int index)
        : propDesc_(propDesc)  // description of prop to be observed
        , config_(config)      // config object containing all prop relations
        , network_(network)    // processor network where the prop is searched
        , propList_(propList)  // Qt widget where the prop widget is shown
        , index_(index) {}     // position in the Qt widget

protected:
    // Dynamic addition and removal methods (overridden from PropertyOwnerObserver)
    void onDidAddProperty(Property* property, size_t index) override;
    void onWillRemoveProperty(Property* property, size_t index) override;
};

void DynamicPropObserver::onDidAddProperty(Property* addedProp, size_t index) {
    // Check if interested in the added property
    bool interested = addedProp->getIdentifier() == propDesc_.id;
    if (!interested) {
        CompositeProperty* composite = dynamic_cast<CompositeProperty*>(addedProp);
        if (composite) interested = composite->getPropertyByIdentifier(propDesc_.id) != nullptr;
    }
    if (interested) {
        Property* myProp = getProp(propDesc_, config_, network_);
        if (myProp) propList_->setAtIndex(index_, myProp);
    }
}

void DynamicPropObserver::onWillRemoveProperty(Property* property, size_t index) {
    LogCentral::getPtr()->log("GUI", LogLevel::Info, LogAudience::Developer, "", "", 0,
                              property->getIdentifier() + " removed");
    // TODO implement and test dynamic removal
}

/**
 * Configurable GUI:
 * Can be constructed from InviwoApplicationQt and a config file.
 * Shows a small property list based on a workspace and a config,
 * besides the canvas and the console.
 */
class ConfigGUI : public QMainWindow {

    enum Status { OK, JUST_STARTED, CONFIG_ERROR, WORKSPACE_ERROR };

    Status currentStatus_;

    InviwoApplicationQt* inv_;

    std::string configFile_;
    std::string workspaceFile_;

    Config currentConfig_;

    CustomPropertyListWidget* propList_ = nullptr;
    std::vector<DynamicPropObserver*> observers_;

    std::string getWorkspaceName() {
        return std::filesystem::path(workspaceFile_).filename().string();
    }
    std::string getConfigName() { return std::filesystem::path(configFile_).filename().string(); }

    void logWarn(std::string str) {
        LogCentral::getPtr()->log("ConfigGUI", LogLevel::Warn, LogAudience::Developer, "", "", 0,
                                  str + " (" + getWorkspaceName() + ", " + getConfigName() + ")");
    }

    void clear() {
        for (const auto& observer : observers_) {
            delete observer;
        }
        observers_.clear();

        if (propList_ != nullptr) {
            // Delete safe to use here (Qt destructur cleans up all child widgets)
            delete propList_;
            propList_ = nullptr;
        }
    }

    bool rebuild() {

        clear();

        propList_ = new CustomPropertyListWidget(this);

        if (currentStatus_ != Status::OK) {
            if (currentStatus_ == Status::JUST_STARTED)
                propList_->addPlaceholder("Welcome.");
            else if (currentStatus_ == Status::CONFIG_ERROR)
                propList_->addPlaceholder("Config could not be loaded.");
            else if (currentStatus_ == Status::WORKSPACE_ERROR)
                propList_->addPlaceholder("Workspace could not be loaded. Infos in console.");
            propList_->addButton("Open Workpace...", [this]() { this->slot_loadWorkspace(); });
            propList_->addButton("Load Config...", [this]() { this->slot_loadConfig(); });
            propList_->addButton("Reload Config", [this]() { this->slot_reloadConfig(); });
            return false;
        }

        const std::string title = getWorkspaceName() + " - " + getConfigName();
        this->setWindowTitle(title.c_str());

        return true;
    }

    void rebuildFromConfig(const Config config) {

        if (!rebuild()) return;

        const auto network = inv_->getProcessorNetwork();

        for (const auto& propDesc : config.props) {

            // Don't show composites with children to be shown
            bool hasChildren = false;
            for (const auto& propDesc_j : config.props) {
                if (propDesc_j.compositeIndex < 0) continue;
                if (config.props[propDesc_j.compositeIndex].id == propDesc.id) {
                    hasChildren = true;
                    break;
                }
            }
            if (hasChildren) continue;

            const auto procDesc = config.procs[propDesc.procIndex];
            const auto proc = network->getProcessorByIdentifier(procDesc.id);
            PropertyOwner* owner = proc;
            int index = -1;

            if (!proc)
                logWarn(procDesc.id +
                        " - processor declared in config file, but not found in workspace");
            else if (propDesc.id == "*")
                index = propList_->addPropertyGroup(proc);
            else {
                Property* prop = getProp(propDesc, config, network, &owner);

                if (!propDesc.displayName.empty()) prop->setDisplayName(propDesc.displayName);

                const auto composite = dynamic_cast<CompositeProperty*>(prop);

                if (!prop) {
                    // Prop from config not found in network
                    logWarn(propDesc.id +
                            " - property declared in config file, but not found in workspace");
                    index = propList_->addPlaceholder(propDesc.id + " - property not found");
                } else if (composite) {
                    // Add all props of composite
                    index = propList_->addPropertyGroup(composite);
                } else {
                    // Add single prop
                    index = propList_->addProperty(prop);
                }
            }

            if (index < 0) {
                logWarn(procDesc.id + " - widget could not be added");
            } else {
                observers_.push_back(
                    new DynamicPropObserver(propDesc, config, network, propList_, index));
                owner->addObserver(observers_.back());
            }
        }
    }

private:
    // Reload current config
    void slot_reloadConfig() {

        const auto config = loadConfig(configFile_);

        if (config.parseSuccess) {
            currentStatus_ = Status::OK;

            // First get workspace
            workspaceFile_ = config.workspace;
            slot_reloadWorkspace();

            // Then display props
            rebuildFromConfig(config);

            currentConfig_ = config;
        } else {
            currentStatus_ = Status::CONFIG_ERROR;
            rebuild();
        }

        this->addDockWidget(Qt::RightDockWidgetArea, propList_);
    }

    // Load config with file chooser
    void slot_loadConfig() {
        const auto dir = configFile_.empty() ? "." : configFile_.c_str();
        const auto fileName = QFileDialog::getOpenFileName(
            this, tr("Load Config..."), dir, tr("Inviwo Application GUI Config (*.cfg *.txt)"));
        const auto fileNameUTF8 = std::string(fileName.toUtf8().constData());

        if (fileNameUTF8.empty()) return;  // chooser was closed/cancelled

        configFile_ = fileNameUTF8;
        slot_reloadConfig();
    }

    // Reload current workspace
    void slot_reloadWorkspace() {

        // Clear property widgets now, because successful workspace loading will destroy processors
        // (property owners) and properties, which would cause the widget destructor to dereference
        // deleted properties
        clear();

        // Destroy all processors and properties in the old workspace (hopefully releasing all taken
        // memory)
        inv_->getProcessorNetwork()->clear();
        inv_->getWorkspaceManager()->clear();

        if (loadWorkspace(workspaceFile_, inv_->getWorkspaceManager())) {
            currentStatus_ = Status::OK;
            rebuildFromConfig(currentConfig_);
        } else {
            currentStatus_ = Status::WORKSPACE_ERROR;
            rebuild();
        }
    }

    // Load workspace with file chooser
    void slot_loadWorkspace() {

        // Continue from last path
        const auto dir = workspaceFile_.empty() ? "." : workspaceFile_.c_str();
        const auto fileName = QFileDialog::getOpenFileName(this, tr("Load Workspace..."), dir,
                                                           tr("Inviwo Workspace (*.inv)"));
        const auto fileNameUTF8 = std::string(fileName.toUtf8().constData());

        if (fileNameUTF8.empty()) return;  // chooser was closed/cancelled

        clear();
        inv_->getProcessorNetwork()->clear();
        inv_->getWorkspaceManager()->clear();

        // Try to load
        if (loadWorkspace(fileNameUTF8, inv_->getWorkspaceManager())) {
            currentStatus_ = Status::OK;
            rebuildFromConfig(currentConfig_);
        } else {
            currentStatus_ = Status::WORKSPACE_ERROR;
            workspaceFile_ = fileNameUTF8;
            rebuild();
        }
    }

    // Save workspace with file chooser
    void slot_saveWorkspaceAs() {
        const auto fileName = QFileDialog::getSaveFileName(this, tr("Save Workspace As..."), ".",
                                                           tr("Inviwo Workspace (*.inv)"));
        const auto fileNameUTF8 = std::string(fileName.toUtf8().constData());
        inv_->getWorkspaceManager()->save(fileNameUTF8);
    }

public:
    ConfigGUI(InviwoApplicationQt* inv, std::string configFile = "")
        : QMainWindow(), currentStatus_(Status::JUST_STARTED), inv_(inv), configFile_(configFile) {

        inv->setMainWindow(this);

        // TODO canvas as dockwidget

        QMenuBar* menuBar = new QMenuBar(this);

        QMenu* fileMenu = menuBar->addMenu("File");

        QAction* loadConfig = fileMenu->addAction("Load Config...");
        connect(loadConfig, &QAction::triggered, this, &ConfigGUI::slot_loadConfig);

        QAction* reloadConfig = fileMenu->addAction("Reload Config!");
        connect(reloadConfig, &QAction::triggered, this, &ConfigGUI::slot_reloadConfig);

        fileMenu->addSeparator();

        QAction* reloadWorkspace = fileMenu->addAction("Reload Workspace!");
        connect(reloadWorkspace, &QAction::triggered, this, &ConfigGUI::slot_reloadWorkspace);

        QAction* loadWorkspace = fileMenu->addAction("Open Workspace...");
        connect(loadWorkspace, &QAction::triggered, this, &ConfigGUI::slot_loadWorkspace);
        loadWorkspace->setShortcuts(QKeySequence::Open);

        QAction* saveWorkspaceAs = fileMenu->addAction("Save Workspace As...");
        connect(saveWorkspaceAs, &QAction::triggered, this, &ConfigGUI::slot_saveWorkspaceAs);
        saveWorkspaceAs->setShortcuts(QKeySequence::Save);

        this->setMenuBar(menuBar);

        if (!configFile.empty())
            slot_reloadConfig();
        else
            rebuild();

        this->setWindowTitle("Inviwo Mini");
        this->setMinimumHeight(700);
        this->show();
    }

    ~ConfigGUI() { clear(); }
};

/**
 * Program entry
 */
int main(int argc, char** argv) {

    LogCentral::init();
    inviwo::util::OnScopeExit deleteLogcentral([]() { inviwo::LogCentral::deleteInstance(); });
    auto logger = std::make_shared<inviwo::ConsoleLogger>();
    LogCentral::getPtr()->registerLogger(logger);

    InviwoApplicationQt inviwoApp(argc, argv, "Inviwo-Qt");
    inviwoApp.printApplicationInfo();
    inviwoApp.setAttribute(Qt::AA_NativeWindows);
    inviwoApp.setProgressCallback([](std::string m) {
        LogCentral::getPtr()->log("InviwoApplication", LogLevel::Info, LogAudience::User, "", "", 0,
                                  m);
    });

    inviwoApp.setWindowIcon(QIcon(":/inviwo/inviwo_light.png"));
    inviwoApp.setAttribute(Qt::AA_NativeWindows);
    QFile styleSheetFile(":/stylesheets/inviwo.qss");
    styleSheetFile.open(QFile::ReadOnly);
    QString styleSheet = QString::fromUtf8(styleSheetFile.readAll());
    inviwoApp.setStyleSheet(styleSheet);
    styleSheetFile.close();

    // Initialize all modules
    inviwoApp.registerModules(inviwo::getModuleList());

    auto& cmdparser = inviwoApp.getCommandLineParser();
    TCLAP::ValueArg<std::string> snapshotArg(
        "s", "snapshot",
        "Specify default name of each snapshot, or empty string for processor name.", false, "",
        "Snapshot default name: UPN=Use Processor name.");

    cmdparser.add(&snapshotArg,
                  [&]() {
                      std::string path = cmdparser.getOutputPath();
                      if (path.empty()) path = inviwoApp.getPath(PathType::Images);
                      util::saveAllCanvases(inviwoApp.getProcessorNetwork(), path,
                                            snapshotArg.getValue());
                  },
                  1000);

    // Do this after registerModules if some arguments were added
    cmdparser.parse(inviwo::CommandLineParser::Mode::Normal);

    // Need to clear the network and (will delete processors and processorwidgets)
    // before QMainWindoes is deleted, otherwise it will delete all processorWidgets
    // before Processor can delete them.
    // TODO investigate crash on closing window after having loaded new workspace
    util::OnScopeExit clearNetwork([&]() { inviwoApp.getProcessorNetwork()->clear(); });

    inviwoApp.getProcessorNetwork()->lock();

    const std::string workspace = cmdparser.getLoadWorkspaceFromArg()
                                      ? cmdparser.getWorkspacePath()
                                      : inviwoApp.getPath(PathType::Workspaces, "/boron.inv");

    if (!loadWorkspace(workspace, inviwoApp.getWorkspaceManager())) {
        QMessageBox msgBox;
        msgBox.setText("Workspace could not be loaded. Infos in console.");
        msgBox.exec();
    }

    ConfigGUI gui(&inviwoApp);

    inviwoApp.processFront();
    inviwoApp.getProcessorNetwork()->unlock();

    cmdparser.processCallbacks();  // run any command line callbacks from modules.

    if (cmdparser.getQuitApplicationAfterStartup()) {
        inviwoApp.closeInviwoApplication();
        inviwoApp.quit();
        return 0;
    } else {
        return inviwoApp.exec();
    }
}

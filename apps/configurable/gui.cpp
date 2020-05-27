#include "gui.h"

#include <inviwo/core/util/filesystem.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <ticpp/ticpp.h>

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
            return true;
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
    return false;
}

// =============================================================================================

CustomPropertyListWidget::CustomPropertyListWidget(QWidget* parent)
    : QDockWidget("Properties", parent) {

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

int CustomPropertyListWidget::addProperty(Property* prop) {
    auto factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
    if (auto propertyWidget = static_cast<PropertyWidgetQt*>(factory->create(prop).release())) {
        listLayout_->insertWidget(-1, propertyWidget, 0, Qt::AlignTop);
        int index = (int)elements_.size();
        elements_.push_back(propertyWidget);
        return index;
    }
    return -1;
}

int CustomPropertyListWidget::addPropertyGroup(Processor* proc) {
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

int CustomPropertyListWidget::addPropertyGroup(CompositeProperty* composite) {
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

int CustomPropertyListWidget::addPlaceholder(std::string str) {
    QLabel* label = new QLabel(this);
    label->setText(QString(str.c_str()));
    listLayout_->insertWidget(-1, label, 0, Qt::AlignTop);
    int index = (int)elements_.size();
    elements_.push_back(label);
    return index;
}

int CustomPropertyListWidget::addButton(std::string label, std::function<void()> onclick) {
    auto button = new QPushButton(this);
    button->setText(QString(label.c_str()));
    connect(button, &QPushButton::clicked, onclick);
    listLayout_->insertWidget(-1, button, 0, Qt::AlignTop);
    int index = (int)elements_.size();
    elements_.push_back(button);
    return index;
}

void CustomPropertyListWidget::setAtIndex(unsigned int index, Property* prop) {
    auto factory = InviwoApplication::getPtr()->getPropertyWidgetFactory();
    if (auto propertyWidget = static_cast<PropertyWidgetQt*>(factory->create(prop).release())) {
        delete elements_[index];  // also removes it from the layout
        elements_[index] = propertyWidget;
        listLayout_->insertWidget(index, propertyWidget, 0, Qt::AlignTop);
    }
}

// =============================================================================================

DynamicPropObserver::DynamicPropObserver(const PropDesc propDesc, const Config config,
                                         ProcessorNetwork* network,
                                         CustomPropertyListWidget* propList, unsigned int index)
    : propDesc_(propDesc)  // description of prop to be observed
    , config_(config)      // config object containing all prop relations
    , network_(network)    // processor network where the prop is searched
    , propList_(propList)  // Qt widget where the prop widget is shown
    , index_(index) {}     // position in the Qt widget

void DynamicPropObserver::onDidAddProperty(Property* addedProp, size_t) {
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

void DynamicPropObserver::onWillRemoveProperty(Property* property, size_t) {
    LogCentral::getPtr()->log("GUI", LogLevel::Info, LogAudience::Developer, "", "", 0,
                              property->getIdentifier() + " removed");
    // TODO implement and test dynamic removal
}

// =============================================================================================

ConfigGUI::ConfigGUI(InviwoApplicationQt* inv, std::string configFile)
    : QMainWindow()
    , currentStatus_(Status::WELCOME)
    , inv_(inv)
    , basePath_(inv->getBasePath())
    , configFile_(configFile) {

    QMenuBar* menuBar = new QMenuBar(this);

    QMenu* fileMenu = menuBar->addMenu("File");

    QAction* loadConfigAction = fileMenu->addAction("Load Config...");
    connect(loadConfigAction, &QAction::triggered, this, &ConfigGUI::openConfig);

    QAction* reloadConfigAction = fileMenu->addAction("Reload Config");
    connect(reloadConfigAction, &QAction::triggered, this, &ConfigGUI::reloadConfig);

    fileMenu->addSeparator();

    QAction* loadWorkspaceAction = fileMenu->addAction("Open Workspace...");
    connect(loadWorkspaceAction, &QAction::triggered, this, &ConfigGUI::openWorkspace);
    loadWorkspaceAction->setShortcuts(QKeySequence::Open);

    QAction* saveWorkspaceAction = fileMenu->addAction("Save Workspace As...");
    connect(saveWorkspaceAction, &QAction::triggered, this, &ConfigGUI::saveWorkspaceAs);
    saveWorkspaceAction->setShortcuts(QKeySequence::Save);

    reloadConfig();

    inv->setMainWindow(this);

    this->setMenuBar(menuBar);
    this->setWindowTitle("Inviwo Mini");
    this->setMinimumHeight(500);
    this->setMinimumWidth(500);
    this->show();
}

ConfigGUI::~ConfigGUI() {
    inv_->getProcessorNetwork()->clear();
    clear();
}

void ConfigGUI::saveWorkspaceAs() {
    const auto fileName = QFileDialog::getSaveFileName(this, tr("Save Workspace As..."), ".",
                                                       tr("Inviwo Workspace (*.inv)"));
    inv_->getWorkspaceManager()->save(inviwo::utilqt::fromQString(fileName));
}

void ConfigGUI::openWorkspace() {

    // Continue from last path
    const auto defaultDir = configFile_.empty() ? basePath_.c_str() : configFile_.c_str();
    const auto fileName = QFileDialog::getOpenFileName(this, tr("Load Workspace..."), defaultDir,
                                                       tr("Inviwo Workspace (*.inv)"));
    const auto fileNameUTF8 = inviwo::utilqt::fromQString(fileName);
    if (fileNameUTF8.empty()) return;  // chooser was closed/cancelled

    // Clear property widgets now, because successful workspace loading will destroy processors
    // (property owners) and properties, which would cause the widget destructor to dereference
    // deleted properties
    clear();

    // Destroy all processors and properties in the old workspace (hopefully releasing all taken
    // memory)
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

void ConfigGUI::openConfig() {
    const auto defaultDir = configFile_.empty() ? basePath_.c_str() : configFile_.c_str();
    const auto fileName = QFileDialog::getOpenFileName(
        this, tr("Load Config..."), defaultDir, tr("Inviwo Application GUI Config (*.cfg *.txt)"));
    const auto fileNameUTF8 = std::string(fileName.toUtf8().constData());
    if (fileNameUTF8.empty()) return;  // chooser was closed/cancelled
    configFile_ = fileNameUTF8;
    reloadConfig();
}

void ConfigGUI::reloadConfig() {

    const auto config = loadConfig(configFile_);

    if (config.parseSuccess) {
        currentStatus_ = Status::OK;

        // First get workspace
        workspaceFile_ = config.workspace;

        // Destroy old widgets and network
        clear();
        inv_->getProcessorNetwork()->clear();
        inv_->getWorkspaceManager()->clear();

        // Load new network
        if (loadWorkspace(workspaceFile_, inv_->getWorkspaceManager())) {
            rebuildFromConfig(currentConfig_);
        } else {
            currentStatus_ = Status::WORKSPACE_ERROR;
            rebuild();
        }

        // Then display props
        rebuildFromConfig(config);

        currentConfig_ = config;
    } else {
        currentStatus_ = Status::CONFIG_ERROR;
        rebuild();
    }

    this->addDockWidget(Qt::RightDockWidgetArea, propList_);
}

void ConfigGUI::clear() {
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

bool ConfigGUI::rebuild() {
    clear();

    propList_ = new CustomPropertyListWidget(this);

    if (currentStatus_ != Status::OK) {
        if (currentStatus_ == Status::WELCOME)
            propList_->addPlaceholder("Welcome.");
        else if (currentStatus_ == Status::CONFIG_ERROR)
            propList_->addPlaceholder("Config could not be loaded.");
        else if (currentStatus_ == Status::WORKSPACE_ERROR)
            propList_->addPlaceholder("Workspace could not be loaded. Infos in console.");
        propList_->addButton("Open Workpace...", [this]() { this->openWorkspace(); });
        propList_->addButton("Load Config...", [this]() { this->openConfig(); });
        propList_->addButton("Reload Config", [this]() { this->reloadConfig(); });
        return false;
    }

    const std::string title = getWorkspaceName() + " - " + getConfigName();
    this->setWindowTitle(title.c_str());

    return true;
}

void ConfigGUI::rebuildFromConfig(const Config config) {

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
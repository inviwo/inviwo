#ifndef GUI_H
#define GUI_H

#include "configparser.h"

#include <QtWidgets>

#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>

#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>
#include <modules/qtwidgets/properties/filepropertywidgetqt.h>
#include <modules/qtwidgets/propertylistwidget.h>

#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/properties/compositeproperty.h>

#include <filesystem>

using namespace inviwo;

// =============================================================================================

/**
 * Load workspace from filepath using workspace manager
 */
bool loadWorkspace(std::string workspace, WorkspaceManager* manager);

// =============================================================================================

/**
 * Qt widget to show properties:
 * Based on PropertyListWidget, but able to show individual properties instead of processor
 * groups.
 */
class CustomPropertyListWidget : public QDockWidget {
    QVBoxLayout* listLayout_;
    QWidget* listWidget_;
    QScrollArea* scrollArea_;
    std::vector<QWidget*> elements_;

public:
    CustomPropertyListWidget(QWidget* parent);

    ~CustomPropertyListWidget() = default;  // qt widget automatically deletes its children

    int addProperty(Property* prop);

    int addPropertyGroup(Processor* proc);

    int addPropertyGroup(CompositeProperty* composite);

    int addPlaceholder(std::string str);

    int addButton(std::string label, std::function<void()> onclick);

    void setAtIndex(unsigned int index, Property* prop);
};

// =============================================================================================

/**
 * Property observers are notified when properties are added or removed.
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
                        CustomPropertyListWidget* propList, unsigned int index);

protected:
    // Dynamic addition and removal methods (overridden from PropertyOwnerObserver)
    void onDidAddProperty(Property* property, size_t index) override;
    void onWillRemoveProperty(Property* property, size_t index) override;
};

// =============================================================================================

/**
 * Configurable GUI:
 * Can be constructed from InviwoApplicationQt and a config file.
 * Shows a small property list based on workspace and config,
 * besides the canvas and the console.
 * TODO Canvas as dock widget and console only optional
 */
class ConfigGUI : public QMainWindow {

    enum Status { OK, WELCOME, CONFIG_ERROR, WORKSPACE_ERROR };

    Status currentStatus_;

    // Inviwo application that stores processor network
    InviwoApplicationQt* inv_;

    std::string basePath_;
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

    // Destroy property widgets
    void clear();

    // Rebuild and show default state
    bool rebuild();

    // Rebuild and show properties from config
    void rebuildFromConfig(const Config config);

    // Reload current config
    void reloadConfig();

    // Load config with file chooser
    void openConfig();

    // Load workspace with file chooser
    void openWorkspace();

    // Save workspace with file chooser
    void saveWorkspaceAs();

public:
    // Start GUI with no config by default (can be opened via menu)
    ConfigGUI(InviwoApplicationQt* inv, std::string configFile = "");

    ~ConfigGUI();
};

#endif
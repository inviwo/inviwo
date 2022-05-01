/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/gamepad/processors/gamepadinput.h>

#include <inviwo/core/network/networklock.h>
#include <QGamepadManager>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GamepadInput::processorInfo_{
    "org.inviwo.GamepadInput",  // Class identifier
    "Gamepad Input",            // Display name
    "Gamepad",                  // Category
    CodeState::Experimental,    // Code state
    Tags::CPU,                  // Tags
};
const ProcessorInfo GamepadInput::getProcessorInfo() const { return processorInfo_; }

GamepadInput::GamepadInput()
    : Processor()
    , propGamepadSelect("gamepadSelect", "Gamepad")
    , propReadoutGroups("readoutGroups", "Readout Groups", 10)
    , propGroupSelect("groupSelect", "Active")
    , propKeyboardShortcuts("keyboardShortcuts", "Keyboard Shortcuts")
    , propEventNextGroup(
          "eventNextGroup", "Next Group",
          [this](Event* e) {
              nextReadoutGroup();
              e->markAsUsed();
          },
          IvwKey::Period, KeyState::Press)
    , propEventPreviousGroup(
          "eventPreviousGroup", "Previous Group",
          [this](Event* e) {
              previousReadoutGroup();
              e->markAsUsed();
          },
          IvwKey::Comma, KeyState::Press)
    , propText("text", "Help and Notifications")
    , Pad(0, this)
    , pCR(nullptr) {
    // Selection of gamepads
    propGamepadSelect.onChange([this]() { connectToSelectedGamepad(); });
    addProperty(propGamepadSelect);
    updateGamepads();
    connect(QGamepadManager::instance(), &QGamepadManager::connectedGamepadsChanged, this,
            &GamepadInput::updateGamepads);

    // Connect the Select button on the gamepad to switching to the next readout group
    connect(&Pad, &QGamepad::buttonSelectChanged, this, [this](bool value) {
        if (!value) nextReadoutGroup();
    });

    // Readout groups: prefab
    GamepadValuesProperty Template("group1", "Group 1");
    Template.setReadOnlyContent(true).setCollapsed(true).setCurrentStateAsDefault();
    propReadoutGroups.addPrefab(std::make_unique<GamepadValuesProperty>(Template));

    // Readout groups: selection
    propGroupSelect.onChange([this]() { connectToSelectedGroup(); });
    addProperties(propReadoutGroups,
                  propGroupSelect);  // This order is important for deserialization.
    propReadoutGroups.PropertyOwnerObservable::addObserver(this);

    // Keyboard shortcuts for switching groups
    propKeyboardShortcuts.addProperties(propEventNextGroup, propEventPreviousGroup);
    propKeyboardShortcuts.setCollapsed(true).setCurrentStateAsDefault();
    addProperty(propKeyboardShortcuts);

    // Add an initial readout
    propReadoutGroups.constructProperty(0);

    // Help texts
    propText.setCollapsed(true)
        .setReadOnly(true)
        .setSerializationMode(PropertySerializationMode::None)
        .setCurrentStateAsDefault();
    addProperty(propText);
}

void GamepadInput::nextReadoutGroup() {
    size_t next = propGroupSelect.getSelectedIndex() + 1;
    next = next % propGroupSelect.size();
    propGroupSelect.setSelectedIndex(next);
}

void GamepadInput::previousReadoutGroup() {
    if (propGroupSelect.getSelectedIndex() > 0) {
        propGroupSelect.setSelectedIndex(propGroupSelect.getSelectedIndex() - 1);
    } else {
        propGroupSelect.setSelectedIndex(propGroupSelect.size() - 1);
    }
}

GamepadInput::~GamepadInput() {
    // Remove us from observing the destruction of the ListProperty.
    // We would get calls leading us to connect to a different group,
    // which (could) lead to Qt being called on an already destructed QWindow.
    propReadoutGroups.PropertyOwnerObservable::removeObserver(this);
}

void GamepadInput::updateGamepads() {
    QGamepadManager* pGamepadManager = QGamepadManager::instance();
    if (pGamepadManager) {
        propGamepadSelect.clearOptions();
        QList<int> GamepadIDs = pGamepadManager->connectedGamepads();
        // std::string AllGamepadNames("");
        for (int i = 0; i < GamepadIDs.count(); i++) {
            const int ThisID = GamepadIDs[i];
            QString GamepadName = pGamepadManager->gamepadName(i);
            const std::string Identifier = GamepadName.length() > 0
                                               ? GamepadName.toStdString()
                                               : "gamepad" + std::to_string(ThisID);
            const std::string DisplayName = GamepadName.length() > 0
                                                ? GamepadName.toStdString()
                                                : "Gamepad " + std::to_string(ThisID);
            propGamepadSelect.addOption(Identifier, DisplayName, ThisID);
            // AllGamepadNames += " \"" + DisplayName + "\"";
        }
        if (GamepadIDs.count() > 0) {
            // propGamepadSelect.setReadOnly(false);
            // LogInfo("Found " << GamepadIDs.count() << (GamepadIDs.count() == 1 ? " gamepad:" : "
            // gamepads:") << AllGamepadNames);
        } else {
            propGamepadSelect.addOption("invalid", "No gamepad connected", 0);
            // propGamepadSelect.setReadOnly(true); //This makes the property sometimes read-only
            // even when an option is in it LogInfo("No gamepad connected.")
        }
    }
}

void GamepadInput::updateGroupSelect() {
    // Update selection of readout groups
    NetworkLock lock(&propGroupSelect);

    if (propReadoutGroups.size()) {
        std::vector<OptionPropertyStringOption> Options;
        for (auto p : propReadoutGroups.getProperties()) {
            Options.emplace_back(p->getIdentifier(), p->getDisplayName());
        }
        propGroupSelect.replaceOptions(Options);
    } else {
        propGroupSelect.clearOptions();
    }
}

void GamepadInput::connectToSelectedGamepad() {
    // Get manager and selected gamepad ID
    QGamepadManager* pGamepadManager = QGamepadManager::instance();
    if (!pGamepadManager) return;
    QList<int> GamepadIDs = pGamepadManager->connectedGamepads();
    const int idSelected = propGamepadSelect.getSelectedValue();
    if (idSelected >= GamepadIDs.count()) return;

    // Tell our member variable Pad, which physical gamepad it shall reflect
    Pad.setDeviceId(idSelected);
    // LogInfo("Selected Gamepad: " << idSelected);
}

void GamepadInput::deserialize(Deserializer& d) {
    Processor::deserialize(d);

    // ensure that option property is up to date
    updateGroupSelect();
    // make processor aware of changes in display names of the readout groups
    for (auto p : propReadoutGroups.getProperties()) {
        p->Property::addObserver(this);
    }

    // make sure that we know how many gamepads are connected
    // this may have changed after reloading a workspace
    updateGamepads();
}

void GamepadInput::onSetDisplayName(Property*, const std::string&) { updateGroupSelect(); }

void GamepadInput::onDidAddProperty(Property* property, size_t) {
    updateGroupSelect();
    property->Property::addObserver(this);
}

void GamepadInput::onDidRemoveProperty(Property*, size_t) {
    // Do not call this when GamepadInput is being destroyed (see our destructor).
    updateGroupSelect();
}

void GamepadInput::connectToSelectedGroup() {
    if (propGroupSelect.size() < 1 || propReadoutGroups.size() < 1) return;

    // Disconnect previous connections
    for (int i(0); i < readoutConnections.size(); i++) {
        QObject::disconnect(readoutConnections[i]);
    }
    readoutConnections.clear();

    // Get active readout group
    pCR = dynamic_cast<GamepadValuesProperty*>(
        propReadoutGroups.getPropertyByIdentifier(propGroupSelect.getSelectedIdentifier()));
    if (!pCR) return;

    // Notify
    propText.propNotification = propGroupSelect.getSelectedDisplayName();

    // Connect to active readout group
    // - floats
    readoutConnections << connect(&Pad, &QGamepad::axisLeftXChanged, this,
                                  [this](double value) { pCR->propAxisLeftX = value; });
    readoutConnections << connect(&Pad, &QGamepad::axisLeftYChanged, this,
                                  [this](double value) { pCR->propAxisLeftY = value; });
    readoutConnections << connect(&Pad, &QGamepad::axisRightXChanged, this,
                                  [this](double value) { pCR->propAxisRightX = value; });
    readoutConnections << connect(&Pad, &QGamepad::axisRightYChanged, this,
                                  [this](double value) { pCR->propAxisRightY = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonL2Changed, this,
                                  [this](double value) { pCR->propButtonL2 = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonR2Changed, this,
                                  [this](double value) { pCR->propButtonR2 = value; });
    // - booleans
    readoutConnections << connect(&Pad, &QGamepad::buttonAChanged, this,
                                  [this](bool value) { pCR->propButtonA = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonBChanged, this,
                                  [this](bool value) { pCR->propButtonB = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonXChanged, this,
                                  [this](bool value) { pCR->propButtonX = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonYChanged, this,
                                  [this](bool value) { pCR->propButtonY = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonLeftChanged, this,
                                  [this](bool value) { pCR->propButtonLeft = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonRightChanged, this,
                                  [this](bool value) { pCR->propButtonRight = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonUpChanged, this,
                                  [this](bool value) { pCR->propButtonUp = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonDownChanged, this,
                                  [this](bool value) { pCR->propButtonDown = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonL1Changed, this,
                                  [this](bool value) { pCR->propButtonL1 = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonR1Changed, this,
                                  [this](bool value) { pCR->propButtonR1 = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonL3Changed, this,
                                  [this](bool value) { pCR->propButtonL3 = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonR3Changed, this,
                                  [this](bool value) { pCR->propButtonR3 = value; });
    // readoutConnections << connect(&Pad, &QGamepad::buttonSelectChanged, this, [this](bool value)
    // { pCR->propButtonSelect = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonStartChanged, this,
                                  [this](bool value) { pCR->propButtonStart = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonCenterChanged, this,
                                  [this](bool value) { pCR->propButtonCenter = value; });
    readoutConnections << connect(&Pad, &QGamepad::buttonGuideChanged, this,
                                  [this](bool value) { pCR->propButtonGuide = value; });
}

}  // namespace inviwo

/*********************************************************************
 *  Author  : Raphael Rey
 *  Init    : Tuesday, May 21, 2019 - 14:27:27
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <gamepad/gamepadcontroller.h>

#include <QGamepadManager>
#include <inviwo/core/network/networklock.h>

namespace inviwo {
namespace kth {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GamepadController::processorInfo_{
    "org.inviwo.GamepadController",  // Class identifier
    "Gamepad Controller",            // Display name
    "Undefined",                     // Category
    CodeState::Experimental,         // Code state
    Tags::None,                      // Tags
};

const ProcessorInfo GamepadController::getProcessorInfo() const { return processorInfo_; }

GamepadController::GamepadController()
	: Processor()
	, propCamera("camera","Camera")
	,rotationSensitivity("rotationSensitivity", "Rotation Sensitivity",0.1,0.001,1)
	,movementSensitivity("movementSensitivity", "Movement Sensitivity",0.1,0.001,1)
	,newMode("newMode","New Mode")
	, timer_(std::chrono::milliseconds{ 10 }, [this]() {

	rotate();
	transfer();
	for (size_t i = 0; i < 12; i++)
	{
		if (buttonHeld[i]) {
			buttonPressed(buttonIdentifiers[i],true);
		}
	}
	for (size_t i = 0; i < 6; i++)
	{
		if (joyStickValues[i] != 0) {
			joyStickTouched(joyStickIdentifiers[i], joyStickValues[i]);
		}
	}
})
{
	newMode.onChange([&]() {
		auto mode = new ModeProperty("mode", "Mode");

		size_t count = 1;
		const auto& base = mode->getIdentifier();
		auto id = base;
		auto displayname = base;
		while (getPropertyByIdentifier(id) != nullptr) {
			id = base + toString(count);
			displayname = base + " " + toString(count);
			++count;
		}
		mode->setIdentifier(id);
		mode->setDisplayName(displayname);

		modes.push_back(mode);
		addProperty(mode);
	});
    // addPort();
     addProperty(propCamera);
	 addProperty(movementSensitivity);
	 addProperty(rotationSensitivity);
	 addProperty(newMode);
    connectButtons();
    connect(QGamepadManager::instance(), &QGamepadManager::connectedGamepadsChanged, this, [this]()
	{
		lookFromRDelta = 0; 
		horizontalMovement = 0;
		verticalMovement = 0;
		verticalRotateDelta = 0;
		horizontalRotateDelta = 0;
		connectButtons();

		//Sets variations to zero in case gamepad gets disconnected while being used
	}); // Connection needed to handle windows behaviour
	}

// connect(radiusSpinBox_, 
//        static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
//        &LightPropertyWidgetQt::onRadiusSpinBoxChanged);

void GamepadController::process() {}

void GamepadController::transfer()
{
	const vec3 right = glm::normalize(glm::cross(propCamera.getLookTo() - propCamera.getLookFrom(), propCamera.getLookUp()));
    propCamera.setLook(propCamera.getLookFrom() - horizontalMovement * right + verticalMovement * propCamera.getLookUp(),
            propCamera.getLookTo() - horizontalMovement * right + verticalMovement * propCamera.getLookUp(), propCamera.getLookUp());
}

void GamepadController::rotate()
{
    const auto& to = propCamera.getLookTo();
    const auto& from = propCamera.getLookFrom();
    const auto& up = propCamera.getLookUp();

        // Compute coordinates on a sphere to rotate from and to
            const auto rot = glm::half_pi<float>() * (vec3(horizontalRotateDelta,verticalRotateDelta,0));
            const auto Pa = glm::normalize(from - to);
            const auto Pc = glm::rotate(glm::rotate(Pa, rot.y, glm::cross(Pa, up)), rot.x, up);
            auto lastRot = glm::quat(Pc, Pa);

        propCamera.setLook(to + glm::rotate(lastRot, from - to), to, glm::rotate(lastRot, up));
}

void GamepadController::connectButtons() {
    auto gamepads = QGamepadManager::instance()->connectedGamepads();
    LogInfo(gamepads.isEmpty());
    if (!gamepads.isEmpty()) {
        pPad = new QGamepad(*(gamepads.begin()));
		if (!buttonsConnected) {
			connect(pPad, &QGamepad::axisLeftXChanged, this, [this](double value) {
				horizontalRotateDelta = value * rotationSensitivity;
				joyStickValues[JoySticks(LeftX)] = value;
			});
			connect(pPad, &QGamepad::axisLeftYChanged, this, [this](double value) {
				verticalRotateDelta = -value * rotationSensitivity;
				joyStickValues[JoySticks(LeftY)] = value;
			});
			connect(pPad, &QGamepad::axisRightXChanged, this, [this](double value) {
				horizontalMovement = value * movementSensitivity;
				joyStickValues[JoySticks(RightX)] = value;
			});
			connect(pPad, &QGamepad::axisRightYChanged, this, [this](double value) {
				verticalMovement = value * movementSensitivity;
				joyStickValues[JoySticks(RightY)] = value;
			});
			connect(pPad, &QGamepad::buttonAChanged, this, [this](bool value) {
				buttonHeld[Buttons(A)] = value;
				if (value) {
					buttonPressed("A", false);
				}
			});
			connect(pPad, &QGamepad::buttonBChanged, this, [this](bool value) {
				buttonHeld[Buttons(B)] = value;
				if (value) {
					buttonPressed("B", false);
				}
			});
			connect(pPad, &QGamepad::buttonYChanged, this, [this](bool value) {
				buttonHeld[Buttons(Y)] = value;
				if (value) {
					buttonPressed("Y", false);
				}
			});
			connect(pPad, &QGamepad::buttonXChanged, this, [this](bool value) {
				buttonHeld[Buttons(X)] = value;
				if (value) {
					buttonPressed("X", false);
				}
			});
			connect(pPad, &QGamepad::buttonUpChanged, this, [this](bool value) {
				buttonHeld[Buttons(Up)] = value;
				if (value) {
					buttonPressed("Up", false);
				}
			});
			connect(pPad, &QGamepad::buttonDownChanged, this, [this](bool value) {
				buttonHeld[Buttons(Down)] = value;
				if (value) {
					buttonPressed("Down", false);
				}
			});
			connect(pPad, &QGamepad::buttonLeftChanged, this, [this](bool value) {
				buttonHeld[Buttons(Left)] = value;
				if (value) {
					buttonPressed("Left", false);
				}
			});
			connect(pPad, &QGamepad::buttonRightChanged, this, [this](bool value) {
				buttonHeld[Buttons(Right)] = value;
				if (value) {
					buttonPressed("Right", false);
				}
			});
			connect(pPad, &QGamepad::buttonStartChanged, this, [this](bool value) {
				if (timer_.isRunning() && value) {
					timer_.stop();
				}
				else {
					if (value) {
						timer_.start();
						LogInfo("Timer start");
					}
				}
			});
			connect(pPad, &QGamepad::buttonSelectChanged, this, [this](bool value) {
				if (value) {
					if (currentMode == modes.size() - 1) {
						currentMode = 0;
					}
					else {
						currentMode++;
					}
				}
			});
			connect(pPad, &QGamepad::buttonR1Changed, this, [this](bool value) {
				buttonHeld[Buttons(R1)] = value;
				if (value) {
					buttonPressed("R1", false);
				}
			});
			connect(pPad, &QGamepad::buttonL1Changed, this, [this](bool value) {
				buttonHeld[Buttons(L1)] = value;
				if (value) {
					buttonPressed("L1", false);
				}
			});
			connect(pPad, &QGamepad::buttonR3Changed, this, [this](bool value) {
				buttonHeld[Buttons(R3)] = value;
				if (value) {
					buttonPressed("R3", false);
				}
			});
			connect(pPad, &QGamepad::buttonL3Changed, this, [this](bool value) {
				buttonHeld[Buttons(L3)] = value;
				if (value) {
					buttonPressed("L3", false);
				}
			});
			connect(pPad, &QGamepad::buttonR2Changed, this, [this](double value) {
				joyStickValues[JoySticks(R2)] = value;
				if (lookFromRDelta >= 0) {
					lookFromRDelta = value;
				}
			});
			connect(pPad, &QGamepad::buttonL2Changed, this, [this](double value) {
				joyStickValues[JoySticks(L2)] = value;
				if (lookFromRDelta <= 0) {
					lookFromRDelta = -value;
				}
			});
			buttonsConnected = true;
		}
    }
}

void GamepadController::buttonPressed(std::string button,bool isHeld)
{
	if (!modes.empty()) {
		modes[currentMode]->buttonPressed(button,isHeld);
	}
}

void GamepadController::joyStickTouched(std::string joyStick, double value)
{
		if (!modes.empty()) {
		modes[currentMode]->joyStickTouched(joyStick,value);
	}
}

}  // namespace kth
}  // namespace inviwo

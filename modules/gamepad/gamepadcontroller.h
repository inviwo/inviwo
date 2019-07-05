/*********************************************************************
 *  Author  : Raphael Rey
 *  Init    : Tuesday, May 21, 2019 - 14:27:27
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <gamepad/gamepadmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
//#include <inviwo/core/ports/volumeport.h>
//#include <inviwo/core/ports/meshport.h>
//#include <inviwo/core/properties/boolcompositeproperty.h>
//#include <inviwo/core/properties/boolproperty.h>
//#include <inviwo/core/properties/buttonproperty.h>
//#include <inviwo/core/properties/compositeproperty.h>
//#include <inviwo/core/properties/fileproperty.h>
//#include <inviwo/core/properties/minmaxproperty.h>
//#include <inviwo/core/properties/optionproperty.h>
//#include <inviwo/core/properties/ordinalproperty.h>
//#include <inviwo/core/properties/stringproperty.h>
//#include <inviwo/core/properties/transferfunctionproperty.h>

#include <QGamepad>
#include <qwindow.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/util/timer.h>
#include <gamepad/gamepadcontrolledproperty.h>
#include <gamepad/ordinalcontrolledproperty.h>
#include <gamepad/modeproperty.h>

namespace inviwo {
namespace kth {

/** \docpage{org.inviwo.GamepadController, Gamepad Controller}
    ![](org.inviwo.GamepadController.png?classIdentifier=org.inviwo.GamepadController)

    Explanation of how to use the processor.
    

    ### Inports
      * __<Inport1>__ <description>.
    

    ### Outports
      * __<Outport1>__ <description>.
    

    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/

/** \class GamepadController
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    

    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Raphael Rey
*/
class IVW_MODULE_GAMEPAD_API GamepadController : public Processor, public QWindow {
    // Friends
    // Types
public:
    // Construction / Deconstruction
public:
    GamepadController();
    virtual ~GamepadController() = default;

    // Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    /// Our main computation function
    virtual void process() override;

// Ports
public:

// Properties
public:
    CameraProperty propCamera;
	FloatProperty rotationSensitivity;
	FloatProperty movementSensitivity;
	ButtonProperty newMode;
	std::vector<ModeProperty*> modes;



 
// Attributes
private:
	bool buttonsConnected = false;
    QGamepad* pPad;
	Timer timer_;
	int currentMode;
	double lookFromRDelta = 0;


	double horizontalRotateDelta = 0;
	double verticalRotateDelta = 0;

	double horizontalMovement = 0;
	double verticalMovement = 0;

	void transfer();
	void rotate();

    void connectButtons();
	void buttonPressed(std::string button,bool isHeld);
	void joyStickTouched(std::string joyStick, double value);
	enum Buttons { A, B, X, Y, R1, L1, Up, Left, Right, Down, L3, R3 };
	enum JoySticks {LeftX, LeftY, RightX, RightY,R2,L2};
	bool buttonHeld[12];
	std::string buttonIdentifiers[12] = { "A", "B", "X", "Y", "R1", "L1", "Up", "Left", "Right", "Down", "L3", "R3" };
	std::string joyStickIdentifiers[6] = { "Left Joystick X","Left Joystick Y","Right Joystick X", "Right Joystick Y", "R2","L2" };
	double joyStickValues[6];
};

}  // namespace kth
}  // namespace inviwo

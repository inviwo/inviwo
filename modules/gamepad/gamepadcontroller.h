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

constexpr auto NUMBER_OF_BUTTONS = 12;;
constexpr auto NUMBER_OF_JOYSTICKS = 6;

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
    \brief A processor allowing the use of a controller, the user has to add the type of property he wants to control and then bind them
	and connect them to the corresponding properties in the processor network.

	It is possible to create different configurations called modes and to switch between modes using select.
    


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
	void deserialize(Deserializer& d) override;

protected:
    /// Our main computation function
    virtual void process() override;

// Ports
public:

// Properties
public:
	ButtonProperty newMode;
	std::vector<ModeProperty*> modes;
	StringProperty modeNotifier;
	StringProperty text_;



 
// Attributes
private:
	bool buttonsConnected = false;
    QGamepad* pPad;
	Timer timer_;
	int currentMode =0 ;

    void connectButtons();
	void buttonPressed(std::string button,bool isHeld);
	void joyStickTouched(std::string joyStick, double value);
	enum Buttons { A, B, X, Y, R1, L1, Up, Left, Right, Down, L3, R3 };
	enum JoySticks {LeftX, LeftY, RightX, RightY,R2,L2};
	bool buttonHeld[NUMBER_OF_BUTTONS];
	std::string buttonIdentifiers[NUMBER_OF_BUTTONS] = { "A", "B", "X", "Y", "R1", "L1", "Up", "Left", "Right", "Down", "L3", "R3" };
	std::string joyStickIdentifiers[NUMBER_OF_JOYSTICKS] = { "Left Joystick X","Left Joystick Y","Right Joystick X", "Right Joystick Y", "R2","L2" };
	double joyStickValues[NUMBER_OF_JOYSTICKS];
};

}  // namespace kth
}  // namespace inviwo

/*********************************************************************
 *  Author  : Raphael Rey
 *  Init    : Tuesday, May 21, 2019 - 14:25:16
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <gamepad/gamepadmodule.h>
#include <gamepad/gamepadcontroller.h>

namespace inviwo
{

using namespace kth;

GamepadModule::GamepadModule(InviwoApplication* app) : InviwoModule(app, "gamepad")
{
    // Add a directory to the search path of the Shadermanager
    // ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:
    
    // Processors
    // registerProcessor<gamepadProcessor>();
    registerProcessor<GamepadController>();
    
    // Properties
    // registerProperty<gamepadProperty>();
    
    // Readers and writes
    // registerDataReader(util::make_unique<gamepadReader>());
    // registerDataWriter(util::make_unique<gamepadWriter>());
    
    // Data converters
    // registerRepresentationConverter(util::make_unique<gamepadDisk2RAMConverter>());

    // Ports
    // registerPort<gamepadOutport>();
    // registerPort<gamepadInport>();

    // PropertyWidgets
    // registerPropertyWidget<gamepadPropertyWidget, gamepadProperty>("Default");
    
    // Dialogs
    // registerDialog<gamepadDialog>(gamepadOutport);
    
    // Other varius things
    // registerCapabilities(util::make_unique<gamepadCapabilities>());
    // registerSettings(util::make_unique<gamepadSettings>());
    // registerMetaData(util::make_unique<gamepadMetaData>());   
    // registerPortInspector("gamepadOutport", "path/workspace.inv");
    // registerProcessorWidget(std::string processorClassName, std::unique_ptr<ProcessorWidget> processorWidget);
    // registerDrawer(util::make_unique_ptr<gamepadDrawer>());  
}

} // namespace

/*********************************************************************
 *  Author  : Raphael Rey
 *  Init    : Tuesday, May 21, 2019 - 14:25:16
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <gamepad/gamepadmoduledefine.h>
#include <inviwo/core/common/inviwomodule.h>

namespace inviwo
{

class IVW_MODULE_GAMEPAD_API GamepadModule : public InviwoModule
{
public:
    GamepadModule(InviwoApplication* app);
    virtual ~GamepadModule() = default;
};

} // namespace

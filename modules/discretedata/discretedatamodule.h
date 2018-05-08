/*********************************************************************
 *  Author  : Tino Weinkauf
 *  Init    : Sunday, October 01, 2017 - 05:17:04
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <discretedata/discretedatamoduledefine.h>
#include <inviwo/core/common/inviwomodule.h>

namespace inviwo
{

class IVW_MODULE_DISCRETEDATA_API DiscreteDataModule : public InviwoModule {

public:
    DiscreteDataModule(InviwoApplication* app);
    virtual ~DiscreteDataModule() = default;
};

} // namespace

// Automatically generated file do not change! (see filegeneration.cmake, ivw_private_generate_module_registration_file)

#include <inviwo/sys/moduleregistration.h>

@MODULE_HEADERS@

namespace inviwo {

/**
 * \brief Creates factories for each enabled module.
 *
 * @return std::vector<std::unique_ptr<InviwoModuleFactoryObject>> Factories of enabled modules
 */
std::vector<std::unique_ptr<InviwoModuleFactoryObject>> getModuleList() {
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>> modules;

@MODULE_CLASS_FUNCTIONS@

    return modules;
}

}  //namespace

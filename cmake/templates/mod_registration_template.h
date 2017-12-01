// Automatically generated file do not change! (see globalmacros.cmake, ivw_private_generate_module_registration_file)
#ifdef IVW_RUNTIME_MODULE_LOADING

#include <inviwo/core/common/runtimemoduleregistration.h>
namespace inviwo {
RuntimeModuleLoading getModuleList() { return RuntimeModuleLoading{}; }
}  //namespace

#else

#include <inviwo/core/common/inviwomodulefactoryobject.h>

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

#endif // IVW_RUNTIME_MODULE_LOADING

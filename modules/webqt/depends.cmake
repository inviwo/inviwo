# Inviwo module dependencies for current module
# List modules on the format "Inviwo<ModuleName>Module"
set(dependencies
    InviwoQtWidgetsModule
    InviwoWebBrowserModule
)

set(EnableByDefault OFF)

if(IVW_CFG_MSVC_ADDRESS_SANITIZER)
    set(Disabled ON)
    set(DisabledReason "Disabled since IVW_CFG_MSVC_ADDRESS_SANITIZER is ON")
endif()
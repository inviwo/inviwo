#--------------------------------------------------------------------
# Dependencies for current module
# List modules on the format "Inviwo<ModuleName>Module"
set(dependencies
    InviwoPython3Module
    InviwoQtWidgetsModule
)

if(Python3_Development_FOUND)
    set(EnableByDefault ON)
endif()

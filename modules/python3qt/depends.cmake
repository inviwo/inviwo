#--------------------------------------------------------------------
# Dependencies for current module
# List modules on the format "Inviwo<ModuleName>Module"
set(dependencies
    InviwoPython3Module
    InviwoQtWidgetsModule
)

if(PYTHONLIBS_FOUND)
    set(EnableByDefault ON)
endif()

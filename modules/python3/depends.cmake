#--------------------------------------------------------------------
# Dependencies for current module
# List modules on the format "Inviwo<ModuleName>Module"
set(dependencies
    InviwoBaseModule
)
set(protected ON)

if(PYTHONLIBS_FOUND)
    set(EnableByDefault ON)
endif()

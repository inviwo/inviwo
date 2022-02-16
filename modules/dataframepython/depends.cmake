# Inviwo module dependencies for current module
# List modules on the format "Inviwo<ModuleName>Module"
set(dependencies
    InviwoDataFrameModule
    InviwoPython3Module
)

if(Python3_Development_FOUND)
    set(EnableByDefault ON)
endif()
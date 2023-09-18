#--------------------------------------------------------------------
# Dependencies for current module
set(dependencies
)
set(protected ON)

if(NOT IVW_ENABLE_PYTHON) 
    set(Disabled ON)
    set(DisabledReason "Disabled since IVW_ENABLE_PYTHON is OFF")
endif()

set(EnableByDefault ON)

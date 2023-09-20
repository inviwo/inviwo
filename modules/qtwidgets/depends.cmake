#--------------------------------------------------------------------
# Dependencies for QtWidgets module
set(dependencies 
)

if(NOT IVW_ENABLE_QT) 
    set(Disabled ON)
    set(DisabledReason "Disabled since IVW_ENABLE_QT is OFF")
endif()

set(EnableByDefault ON)

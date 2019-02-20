 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2012-2019 Inviwo Foundation
 # All rights reserved.
 # 
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions are met: 
 # 
 # 1. Redistributions of source code must retain the above copyright notice, this
 # list of conditions and the following disclaimer. 
 # 2. Redistributions in binary form must reproduce the above copyright notice,
 # this list of conditions and the following disclaimer in the documentation
 # and/or other materials provided with the distribution. 
 # 
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 # ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 # DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 # ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 # (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 # LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 # ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 # SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 # 
 #################################################################################

function(ivw_locateqt retval)
    if(NOT QT_QMAKE_EXECUTABLE)
        find_program(QT_QMAKE_EXECUTABLE_FINDQT NAMES qmake qmake4 qmake-qt4 qmake5 qmake-qt5
           PATHS "${QT_SEARCH_PATH}/bin" "$ENV{QTDIR}/bin")
        set(QT_QMAKE_EXECUTABLE ${QT_QMAKE_EXECUTABLE_FINDQT} CACHE PATH "Qt qmake program.")
    endif()

    if(QT_QMAKE_EXECUTABLE)
        execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query "QT_INSTALL_PREFIX" OUTPUT_VARIABLE QT5_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
        if(APPLE) # On OSX look for the usual brew installation. 
            foreach(path "/usr/local/Cellar/qt" "/usr/local/Cellar/qt5")
                if(EXISTS ${path})
                    file(GLOB qtversions RELATIVE "${path}/" "${path}/?.*.*")
                    list(LENGTH qtversions len)
                    if(${len} GREATER 0)
                        list(GET qtversions 0 qtlatest)
                        foreach(ver IN LISTS qtversion)
                            if(ver VERSION_GREATER qtlatest)
                                set(qtlatest ${ver})
                            endif()
                        endforeach()
                        if(EXISTS "${path}/${qtlatest}")
                            set(QT5_PATH "${path}/${qtlatest}")
                            break()
                        endif()
                    endif()
                endif()
            endforeach()
        endif()
    endif()
    set(${retval} ${QT5_PATH} PARENT_SCOPE)
endfunction()
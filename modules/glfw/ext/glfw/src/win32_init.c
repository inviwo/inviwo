//========================================================================
// GLFW 3.1 Win32 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "internal.h"

#include <stdlib.h>
#include <malloc.h>

#ifdef __BORLANDC__
// With the Borland C++ compiler, we want to disable FPU exceptions
#include <float.h>
#endif // __BORLANDC__


#if defined(_GLFW_USE_OPTIMUS_HPG)

// Applications exporting this symbol with this value will be automatically
// directed to the high-performance GPU on nVidia Optimus systems
//
GLFWAPI DWORD NvOptimusEnablement = 0x00000001;

#endif // _GLFW_USE_OPTIMUS_HPG

#if defined(_GLFW_BUILD_DLL)

// GLFW DLL entry point
//
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    return TRUE;
}

#endif // _GLFW_BUILD_DLL

// Load necessary libraries (DLLs)
//
static GLboolean initLibraries(void)
{
#ifndef _GLFW_NO_DLOAD_WINMM
    // winmm.dll (for joystick and timer support)

    _glfw.win32.winmm.instance = LoadLibraryW(L"winmm.dll");
    if (!_glfw.win32.winmm.instance)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to load winmm.dll");
        return GL_FALSE;
    }

    _glfw.win32.winmm.joyGetDevCaps = (JOYGETDEVCAPS_T)
        GetProcAddress(_glfw.win32.winmm.instance, "joyGetDevCapsW");
    _glfw.win32.winmm.joyGetPos = (JOYGETPOS_T)
        GetProcAddress(_glfw.win32.winmm.instance, "joyGetPos");
    _glfw.win32.winmm.joyGetPosEx = (JOYGETPOSEX_T)
        GetProcAddress(_glfw.win32.winmm.instance, "joyGetPosEx");
    _glfw.win32.winmm.timeGetTime = (TIMEGETTIME_T)
        GetProcAddress(_glfw.win32.winmm.instance, "timeGetTime");

    if (!_glfw.win32.winmm.joyGetDevCaps ||
        !_glfw.win32.winmm.joyGetPos ||
        !_glfw.win32.winmm.joyGetPosEx ||
        !_glfw.win32.winmm.timeGetTime)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Win32: Failed to load winmm functions");
        return GL_FALSE;
    }
#endif // _GLFW_NO_DLOAD_WINMM

    _glfw.win32.user32.instance = LoadLibraryW(L"user32.dll");
    if (_glfw.win32.user32.instance)
    {
        _glfw.win32.user32.SetProcessDPIAware = (SETPROCESSDPIAWARE_T)
            GetProcAddress(_glfw.win32.user32.instance, "SetProcessDPIAware");
        _glfw.win32.user32.ChangeWindowMessageFilterEx = (CHANGEWINDOWMESSAGEFILTEREX_T)
            GetProcAddress(_glfw.win32.user32.instance, "ChangeWindowMessageFilterEx");
    }

    _glfw.win32.dwmapi.instance = LoadLibraryW(L"dwmapi.dll");
    if (_glfw.win32.dwmapi.instance)
    {
        _glfw.win32.dwmapi.DwmIsCompositionEnabled = (DWMISCOMPOSITIONENABLED_T)
            GetProcAddress(_glfw.win32.dwmapi.instance, "DwmIsCompositionEnabled");
    }

    return GL_TRUE;
}

// Unload used libraries (DLLs)
//
static void terminateLibraries(void)
{
#ifndef _GLFW_NO_DLOAD_WINMM
    if (_glfw.win32.winmm.instance != NULL)
    {
        FreeLibrary(_glfw.win32.winmm.instance);
        _glfw.win32.winmm.instance = NULL;
    }
#endif // _GLFW_NO_DLOAD_WINMM

    if (_glfw.win32.user32.instance)
        FreeLibrary(_glfw.win32.user32.instance);

    if (_glfw.win32.dwmapi.instance)
        FreeLibrary(_glfw.win32.dwmapi.instance);
}

static void createKeyTables(void)
{
    int i;

    for (i = 0;  i < 512;  i++)
    {
        _glfw.win32.publicKeys[i] = GLFW_KEY_UNKNOWN;
        _glfw.win32.nativeKeys[i] = GLFW_KEY_UNKNOWN;
    }

    _glfw.win32.nativeKeys[GLFW_KEY_0] = 0x0B;
    _glfw.win32.nativeKeys[GLFW_KEY_1] = 0x02;
    _glfw.win32.nativeKeys[GLFW_KEY_2] = 0x03;
    _glfw.win32.nativeKeys[GLFW_KEY_3] = 0x04;
    _glfw.win32.nativeKeys[GLFW_KEY_4] = 0x05;
    _glfw.win32.nativeKeys[GLFW_KEY_5] = 0x06;
    _glfw.win32.nativeKeys[GLFW_KEY_6] = 0x07;
    _glfw.win32.nativeKeys[GLFW_KEY_7] = 0x08;
    _glfw.win32.nativeKeys[GLFW_KEY_8] = 0x09;
    _glfw.win32.nativeKeys[GLFW_KEY_9] = 0x0A;
    _glfw.win32.nativeKeys[GLFW_KEY_A] = 0x1E;
    _glfw.win32.nativeKeys[GLFW_KEY_B] = 0x30;
    _glfw.win32.nativeKeys[GLFW_KEY_C] = 0x2E;
    _glfw.win32.nativeKeys[GLFW_KEY_D] = 0x20;
    _glfw.win32.nativeKeys[GLFW_KEY_E] = 0x12;
    _glfw.win32.nativeKeys[GLFW_KEY_F] = 0x21;
    _glfw.win32.nativeKeys[GLFW_KEY_G] = 0x22;
    _glfw.win32.nativeKeys[GLFW_KEY_H] = 0x23;
    _glfw.win32.nativeKeys[GLFW_KEY_I] = 0x17;
    _glfw.win32.nativeKeys[GLFW_KEY_J] = 0x24;
    _glfw.win32.nativeKeys[GLFW_KEY_K] = 0x25;
    _glfw.win32.nativeKeys[GLFW_KEY_L] = 0x26;
    _glfw.win32.nativeKeys[GLFW_KEY_M] = 0x32;
    _glfw.win32.nativeKeys[GLFW_KEY_N] = 0x31;
    _glfw.win32.nativeKeys[GLFW_KEY_O] = 0x18;
    _glfw.win32.nativeKeys[GLFW_KEY_P] = 0x19;
    _glfw.win32.nativeKeys[GLFW_KEY_Q] = 0x10;
    _glfw.win32.nativeKeys[GLFW_KEY_R] = 0x13;
    _glfw.win32.nativeKeys[GLFW_KEY_S] = 0x1F;
    _glfw.win32.nativeKeys[GLFW_KEY_T] = 0x14;
    _glfw.win32.nativeKeys[GLFW_KEY_U] = 0x16;
    _glfw.win32.nativeKeys[GLFW_KEY_V] = 0x2F;
    _glfw.win32.nativeKeys[GLFW_KEY_W] = 0x11;
    _glfw.win32.nativeKeys[GLFW_KEY_X] = 0x2D;
    _glfw.win32.nativeKeys[GLFW_KEY_Y] = 0x15;
    _glfw.win32.nativeKeys[GLFW_KEY_Z] = 0x2C;

    _glfw.win32.nativeKeys[GLFW_KEY_APOSTROPHE]    = 0x28;
    _glfw.win32.nativeKeys[GLFW_KEY_BACKSLASH]     = 0x2B;
    _glfw.win32.nativeKeys[GLFW_KEY_COMMA]         = 0x33;
    _glfw.win32.nativeKeys[GLFW_KEY_EQUAL]         = 0x0D;
    _glfw.win32.nativeKeys[GLFW_KEY_GRAVE_ACCENT]  = 0x29;
    _glfw.win32.nativeKeys[GLFW_KEY_LEFT_BRACKET]  = 0x1A;
    _glfw.win32.nativeKeys[GLFW_KEY_MINUS]         = 0x0C;
    _glfw.win32.nativeKeys[GLFW_KEY_PERIOD]        = 0x34;
    _glfw.win32.nativeKeys[GLFW_KEY_RIGHT_BRACKET] = 0x1B;
    _glfw.win32.nativeKeys[GLFW_KEY_SEMICOLON]     = 0x27;
    _glfw.win32.nativeKeys[GLFW_KEY_SLASH]         = 0x35;
    _glfw.win32.nativeKeys[GLFW_KEY_WORLD_2]       = 0x56;

    _glfw.win32.nativeKeys[GLFW_KEY_BACKSPACE]     = 0x0E;
    _glfw.win32.nativeKeys[GLFW_KEY_DELETE]        = 0x153;
    _glfw.win32.nativeKeys[GLFW_KEY_END]           = 0x14F;
    _glfw.win32.nativeKeys[GLFW_KEY_ENTER]         = 0x1C;
    _glfw.win32.nativeKeys[GLFW_KEY_ESCAPE]        = 0x01;
    _glfw.win32.nativeKeys[GLFW_KEY_HOME]          = 0x147;
    _glfw.win32.nativeKeys[GLFW_KEY_INSERT]        = 0x152;
    _glfw.win32.nativeKeys[GLFW_KEY_MENU]          = 0x15D;
    _glfw.win32.nativeKeys[GLFW_KEY_PAGE_DOWN]     = 0x151;
    _glfw.win32.nativeKeys[GLFW_KEY_PAGE_UP]       = 0x149;
    _glfw.win32.nativeKeys[GLFW_KEY_PAUSE]         = 0x45;
    _glfw.win32.nativeKeys[GLFW_KEY_SPACE]         = 0x39;
    _glfw.win32.nativeKeys[GLFW_KEY_TAB]           = 0x0F;
    _glfw.win32.nativeKeys[GLFW_KEY_CAPS_LOCK]     = 0x3A;
    _glfw.win32.nativeKeys[GLFW_KEY_NUM_LOCK]      = 0x145;
    _glfw.win32.nativeKeys[GLFW_KEY_SCROLL_LOCK]   = 0x46;
    _glfw.win32.nativeKeys[GLFW_KEY_F1]            = 0x3B;
    _glfw.win32.nativeKeys[GLFW_KEY_F2]            = 0x3C;
    _glfw.win32.nativeKeys[GLFW_KEY_F3]            = 0x3D;
    _glfw.win32.nativeKeys[GLFW_KEY_F4]            = 0x3E;
    _glfw.win32.nativeKeys[GLFW_KEY_F5]            = 0x3F;
    _glfw.win32.nativeKeys[GLFW_KEY_F6]            = 0x40;
    _glfw.win32.nativeKeys[GLFW_KEY_F7]            = 0x41;
    _glfw.win32.nativeKeys[GLFW_KEY_F8]            = 0x42;
    _glfw.win32.nativeKeys[GLFW_KEY_F9]            = 0x43;
    _glfw.win32.nativeKeys[GLFW_KEY_F10]           = 0x44;
    _glfw.win32.nativeKeys[GLFW_KEY_F11]           = 0x57;
    _glfw.win32.nativeKeys[GLFW_KEY_F12]           = 0x58;
    _glfw.win32.nativeKeys[GLFW_KEY_F13]           = 0x64;
    _glfw.win32.nativeKeys[GLFW_KEY_F14]           = 0x65;
    _glfw.win32.nativeKeys[GLFW_KEY_F15]           = 0x66;
    _glfw.win32.nativeKeys[GLFW_KEY_F16]           = 0x67;
    _glfw.win32.nativeKeys[GLFW_KEY_F17]           = 0x68;
    _glfw.win32.nativeKeys[GLFW_KEY_F18]           = 0x69;
    _glfw.win32.nativeKeys[GLFW_KEY_F19]           = 0x6A;
    _glfw.win32.nativeKeys[GLFW_KEY_F20]           = 0x6B;
    _glfw.win32.nativeKeys[GLFW_KEY_F21]           = 0x6C;
    _glfw.win32.nativeKeys[GLFW_KEY_F22]           = 0x6D;
    _glfw.win32.nativeKeys[GLFW_KEY_F23]           = 0x6E;
    _glfw.win32.nativeKeys[GLFW_KEY_F24]           = 0x76;
    _glfw.win32.nativeKeys[GLFW_KEY_LEFT_ALT]      = 0x38;
    _glfw.win32.nativeKeys[GLFW_KEY_LEFT_CONTROL]  = 0x1D;
    _glfw.win32.nativeKeys[GLFW_KEY_LEFT_SHIFT]    = 0x2A;
    _glfw.win32.nativeKeys[GLFW_KEY_LEFT_SUPER]    = 0x15B;
    _glfw.win32.nativeKeys[GLFW_KEY_PRINT_SCREEN]  = 0x137;
    _glfw.win32.nativeKeys[GLFW_KEY_RIGHT_ALT]     = 0x138;
    _glfw.win32.nativeKeys[GLFW_KEY_RIGHT_CONTROL] = 0x11D;
    _glfw.win32.nativeKeys[GLFW_KEY_RIGHT_SHIFT]   = 0x36;
    _glfw.win32.nativeKeys[GLFW_KEY_RIGHT_SUPER]   = 0x15C;
    _glfw.win32.nativeKeys[GLFW_KEY_DOWN]          = 0x150;
    _glfw.win32.nativeKeys[GLFW_KEY_LEFT]          = 0x14B;
    _glfw.win32.nativeKeys[GLFW_KEY_RIGHT]         = 0x14D;
    _glfw.win32.nativeKeys[GLFW_KEY_UP]            = 0x148;

    _glfw.win32.nativeKeys[GLFW_KEY_KP_0]          = 0x52;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_1]          = 0x4F;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_2]          = 0x50;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_3]          = 0x51;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_4]          = 0x4B;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_5]          = 0x4C;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_6]          = 0x4D;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_7]          = 0x47;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_8]          = 0x48;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_9]          = 0x49;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_ADD]        = 0x4E;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_DECIMAL]    = 0x53;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_DIVIDE]     = 0x135;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_ENTER]      = 0x11C;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_MULTIPLY]   = 0x37;
    _glfw.win32.nativeKeys[GLFW_KEY_KP_SUBTRACT]   = 0x4A;

    for (i = 0;  i <= GLFW_KEY_LAST;  i++)
    {
        if (_glfw.win32.nativeKeys[i] != GLFW_KEY_UNKNOWN)
            _glfw.win32.publicKeys[_glfw.win32.nativeKeys[i]] = i;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Returns whether desktop compositing is enabled
//
BOOL _glfwIsCompositionEnabled(void)
{
    BOOL enabled;

    if (!_glfw_DwmIsCompositionEnabled)
        return FALSE;

    if (_glfw_DwmIsCompositionEnabled(&enabled) != S_OK)
        return FALSE;

    return enabled;
}

// Returns a wide string version of the specified UTF-8 string
//
WCHAR* _glfwCreateWideStringFromUTF8(const char* source)
{
    WCHAR* target;
    int length;

    length = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
    if (!length)
        return NULL;

    target = calloc(length + 1, sizeof(WCHAR));

    if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, length + 1))
    {
        free(target);
        return NULL;
    }

    return target;
}

// Returns a UTF-8 string version of the specified wide string
//
char* _glfwCreateUTF8FromWideString(const WCHAR* source)
{
    char* target;
    int length;

    length = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
    if (!length)
        return NULL;

    target = calloc(length + 1, sizeof(char));

    if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, length + 1, NULL, NULL))
    {
        free(target);
        return NULL;
    }

    return target;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformInit(void)
{
    // To make SetForegroundWindow work as we want, we need to fiddle
    // with the FOREGROUNDLOCKTIMEOUT system setting (we do this as early
    // as possible in the hope of still being the foreground process)
    SystemParametersInfoW(SPI_GETFOREGROUNDLOCKTIMEOUT, 0,
                          &_glfw.win32.foregroundLockTimeout, 0);
    SystemParametersInfoW(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, UIntToPtr(0),
                          SPIF_SENDCHANGE);

    if (!initLibraries())
        return GL_FALSE;

    createKeyTables();

    if (_glfw_SetProcessDPIAware)
        _glfw_SetProcessDPIAware();

#ifdef __BORLANDC__
    // With the Borland C++ compiler, we want to disable FPU exceptions
    // (this is recommended for OpenGL applications under Windows)
    _control87(MCW_EM, MCW_EM);
#endif

    if (!_glfwInitContextAPI())
        return GL_FALSE;

    _glfwInitTimer();
    _glfwInitJoysticks();

    return GL_TRUE;
}

void _glfwPlatformTerminate(void)
{
    if (_glfw.win32.classAtom)
    {
        UnregisterClassW(_GLFW_WNDCLASSNAME, GetModuleHandleW(NULL));
        _glfw.win32.classAtom = 0;
    }

    // Restore previous foreground lock timeout system setting
    SystemParametersInfoW(SPI_SETFOREGROUNDLOCKTIMEOUT, 0,
                          UIntToPtr(_glfw.win32.foregroundLockTimeout),
                          SPIF_SENDCHANGE);

    free(_glfw.win32.clipboardString);
    free(_glfw.win32.keyName);

    _glfwTerminateJoysticks();
    _glfwTerminateContextAPI();
    terminateLibraries();
}

const char* _glfwPlatformGetVersionString(void)
{
    const char* version = _GLFW_VERSION_NUMBER " Win32"
#if defined(_GLFW_WGL)
        " WGL"
#elif defined(_GLFW_EGL)
        " EGL"
#endif
#if defined(__MINGW32__)
        " MinGW"
#elif defined(_MSC_VER)
        " VisualC"
#elif defined(__BORLANDC__)
        " BorlandC"
#endif
#if !defined(_GLFW_NO_DLOAD_WINMM)
        " LoadLibrary(winmm)"
#endif
#if defined(_GLFW_BUILD_DLL)
        " DLL"
#endif
        ;

    return version;
}


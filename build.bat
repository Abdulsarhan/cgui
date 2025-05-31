@echo off
REM Build script for MSVC command line

REM Set your source files
set SOURCES=cgui.c glad.c

REM Output executable name
set OUTPUT=cgui.exe

REM Include directory for glad.h etc.
set INCLUDE_DIR=win32_thirdparty

REM Compile and link with OpenGL and Windows libs
cl /nologo /EHsc /I %INCLUDE_DIR% %SOURCES% /Fe:%OUTPUT% opengl32.lib user32.lib gdi32.lib

if %ERRORLEVEL% neq 0 (
    echo Build failed.
    pause
    exit /b %ERRORLEVEL%
) else (
    echo Build succeeded: %OUTPUT%
)

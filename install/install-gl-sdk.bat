
set THIS_DIR=%~dp0
set PATH_7Z_EXE="C:\Program Files\7-Zip\7z.exe"
set SDL2_SOURCE=https://www.libsdl.org/release/SDL2-devel-2.0.8-VC.zip
set GLEW_SOURCE=https://sourceforge.net/projects/glew/files/glew/2.0.0/glew-2.0.0-win32.zip/download
set GLUT_SOURCE=https://user.xmission.com/~nate/glut/glut-3.7.6-bin.zip

echo off
REM bitsadmin /transfer myDownloadJob1 /download /priority normal %SDL2_SOURCE% %THIS_DIR%\sdl2.zip
REM bitsadmin /transfer myDownloadJob2 /download /priority normal %GLEW_SOURCE% %THIS_DIR%\glew.zip
REM bitsadmin /transfer myDownloadJob3 /download /priority normal %GLUT_SOURCE% %THIS_DIR%\glut.zip

powershell "Import-Module BitsTransfer; Start-BitsTransfer '%SDL2_SOURCE%' '%THIS_DIR%\sdl2.zip'"
powershell "Import-Module BitsTransfer; Start-BitsTransfer '%GLEW_SOURCE%' '%THIS_DIR%\glew.zip'"
powershell "Import-Module BitsTransfer; Start-BitsTransfer '%GLUT_SOURCE%' '%THIS_DIR%\glut.zip'"

cd ..
mkdir devsdk
cd devsdk
%PATH_7Z_EXE% x ..\install\sdl2.zip -aoa
%PATH_7Z_EXE% x ..\install\glew.zip -aoa
%PATH_7Z_EXE% x ..\install\glut.zip -aoa
cd %THIS_DIR%












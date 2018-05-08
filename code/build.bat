@echo off

set CodeDir=..\code
set OutputDir=..\build_win32

set CommonCompilerFlags=-O2 -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4127 -wd4201 -wd4100 -wd4189 -wd4505 -Z7 -FC
set CommonCompilerFlags=-DKIWI_DEBUG=1 -DKIWI_WIN32=1 %CommonCompilerFlags%
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib Winmm.lib opengl32.lib

IF NOT EXIST %OutputDir% mkdir %OutputDir%

pushd %OutputDir%

del *.pdb > NUL 2> NUL

REM Asset File Builder
cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS %CodeDir%\kiwi_asset_builder.cpp /link %CommonLinkerFlags%

REM 64-bit build
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% %CodeDir%\kiwi.cpp -Fmkiwi.map -LD /link %CommonLinkerFlags% -incremental:no -opt:ref -PDB:kiwi_%random%.pdb -EXPORT:GameUpdateAndRender
del lock.tmp
cl %CommonCompilerFlags% %CodeDir%\win32_kiwi.cpp -I C:\VulkanSDK\1.1.73.0\Include\vulkan\ -Fmwin32_kiwi.map /link %CommonLinkerFlags%

popd

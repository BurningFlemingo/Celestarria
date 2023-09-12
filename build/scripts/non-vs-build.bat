@echo off

set CommonCompileFlags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4100 -wd4189 -wd4065 -wd6387 -wd4101 -wd4505 -DDEBUG=1 /FC -Z7
set CommonLinkFlags=-incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST .\binarys mkdir .\binarys
pushd .\binarys

del *.pdb
cl %CommonCompileFlags% ..\..\src\Celestarria\Game.cpp /LD /link /PDB:Celestarria_dll.pdb /EXPORT:getSoundSamples /EXPORT:updateAndRender -subsystem:windows %CommonLinkFlags% /OUT:Celestarria.dll
cl %CommonCompileFlags% ..\..\src\Platforms\Windows\Main.cpp /link /PDB:Celestarria_exe.pdb -subsystem:windows %CommonLinkFlags% /OUT:Celestarria.exe

popd

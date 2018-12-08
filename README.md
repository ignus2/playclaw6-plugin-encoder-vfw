# playclaw6-plugin-encoder-vfw
VFW encoder plugin for PlayClaw 6

## Installation
1. Create a folder called "plugins" under C:\Program Files (x86)\PlayClaw 6\
2. Copy the encoder-vfw.dll file to the plugins folder: C:\Program Files (x86)\PlayClaw 6\plugins\

## Compiling
Compilation requires the PlayClaw 6 SDK: https://github.com/CyberDemonLord/playclaw6-sdk
1. Copy the VFW encoder plugin source files into playclaw6-sdk\source code\plugins\encoder-vfw\ 
2. Add the encoder-vfw.vcxproj file to the playclaw6-plugins.sln solution
3. Open the solution with Visual Studio 2015/2017, migrate/retarget as necessary then compile

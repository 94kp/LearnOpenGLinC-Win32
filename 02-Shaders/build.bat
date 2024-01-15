cls
DEL OGL.exe
DEL OGL.obj
DEL OGL.res
cl.exe /c /EHsc /I C:\glew\include OGL.cpp
rc.exe OGL.rc
link.exe OGL.obj OGL.res /LIBPATH:C:\glew\lib\Release\x64 gdi32.lib user32.lib /SUBSYSTEM:WINDOWS
OGL.exe

type Log.txt

## Command Library

To learn C++, Qt and Git I have programmed a useful tool, where you can add entered commands to a library.
The added commands can called from the library and then you can execute them directly in the program or you
can copy and paste them to a terminal window. Not all commands are executable in the program's terminal window.

Building the program:
* start a terminal (Linux) or the MinGW terminal for Qt (Windows)
* download the code to your computer
* go into the _build_ directory
* execute _qmake ../CmdLib-linux.pro_ (Linux) or _qmake ..\CmdLib-windows.pro_ (Windows)
* execute _make_ (Linux) or _mingw32-make_ (Windows)
* change to the _final_ directory under _CmdLib_
* on Linux you can run the program with _./cmdlib_
* on Windows execute _windeployqt cmdlib.exe_
* now you can run the program on Windows too

You need at least Qt5.15 and the GNU toolchain (Linux) or MinGW (GNU toolchain on Windows) for compiling the program.

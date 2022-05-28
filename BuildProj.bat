@echo off

rem 1> is standard out and 2> is standard error.

cd \Eric\Main\NetSrv

rem @CommandLine.txt
rem Use -v to see how it gets called.
rem Using -v shows that Clang is using
rem Visual Studio tools:
rem \Community\\VC\\Tools\\MSVC\
rem and \\Windows Kits\\10\\include\\
rem Visual C++ C Runtime (CRT)
rem Universal C Runtime

rem C:\LLVM\bin\clang++ --help

rem -Weverything
rem -Wall

rem Don't use old style casting.
rem -Wno-old-style-cast

rem -Ofast

rem I don't care about being compatible with
rem C++ 98.
rem -Wno-c++98-compat-pedantic

rem -lWs2_32.lib
rem -lwsock32.lib

C:\LLVM\bin\clang++ *.cpp \Eric\Main\CppBase\*.cpp \Eric\Main\Network\*.cpp \Eric\Main\CryptoBase\*.cpp \Eric\Main\WinApi\*.cpp -o NetSrv.exe -Weverything -Wno-c++98-compat-pedantic -ftrapv -Ofast -Wl,-MAP -Wl,-STACK:10000000 -lWs2_32.lib 2> Build.log

rem Show macros:
rem Over 54,000 macros defined.
// duplicates?
rem C:\LLVM\bin\clang++ *.cpp \Eric\Main\CppBase\*.cpp \Eric\Main\CryptoBase\*.cpp \Eric\Main\WinApi\*.cpp -E -dM -Weverything -Wno-c++98-compat-pedantic -ftrapv -Ofast > Build.log


rem https://docs.microsoft.com/en-us/cpp/build/reference/stack-stack-allocations?view=msvc-160


rem Tell the linker to use this max stack size.
rem -Wl,-STACK:10000000
rem -Wl,"-stack_size 0x100000"
rem On Windows, clang calls link.exe.

rem -Wl,<arg>
rem Pass the comma separated arguments in
rem <arg> to the linker

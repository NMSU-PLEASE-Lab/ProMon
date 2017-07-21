##Printer
This program can be used to print the loop present in the binary executable.
It takes two arguments:
 1.BinaryExecutableName
 2.FunctionName (optional): If function name is not supplied then loops for all functions will be printed.

g++  -o Printer Printer.cc  -I/home/ujjwal/Build/DyninstAPI-9.3.2/include -L/home/ujjwal/Build/DyninstAPI-9.3.2/lib -ldyninstAPI -lrt -lpatchAPI -lsymtabAPI -lpcontrol -linstructionAPI -lparseAPI -ldyninstAPI_RT -ldynElf -ldynDwarf -lcommon -lstackwalk -lsymLite

./Printer testapp mainBody

## Test app
testapp is test application for testing ProMon. Following sample creates promon executable from testapp executable.

/home/ujjwal/Source/ProMon2/run/pmsetup.sh /home/ujjwal/Source/ProMon2/test/testapp  /home/ujjwal/Source/ProMon2/test/testapp.xml

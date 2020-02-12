
First, install in your system:
build-essential, libglfw3-dev, libglfw3

Then enter "make" from this directory to build libraries and examples.

To force compilation in 32 bits type 'make ARCH=32'

makefile commands:
all :  build libs and executables (default cmd)
libs :  build only libs
clean : clean all temporary files
cleanexe : clean executables and their temporary files
cleanall : clean libraries, executables and temporary files

For developing your own application, start from the project in
../examples/sigapp.7z, which is prepared to be compiled outside
of the SIG distribution folder.


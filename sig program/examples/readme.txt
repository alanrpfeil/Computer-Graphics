
Each folder here contains source files of example applications
which are to be compiled using SIG projects or makefiles.

Examples to be compiled outside the SIG main folder are also
provided:

- File sigapp.7z contains a project that can be used
as a starting point to build your own SIG application
outside the sig folder. Follow these steps:

1. Unzip sigapp.7z in the same folder as your sig installation.
   Sigapp uses relative paths to find SIG's libraries and it
   assumes folders sig/ and sigapp/ are on the same directory.

2. Windows: you can now open sigapp/vs2017/sigapp.sln with 
            Visual Studio to work on your new project.
   Linux: type make from inside the make/ folder.

3. Rename your sigapp folder to your project's name.

- File sigmynode.7z also contains a project ready to be compiled
outside the SIG folder - just follow the same steps given above.
Project sigmynode.7z provides an example of how to test/build a
new scene graph node and node renderer. 

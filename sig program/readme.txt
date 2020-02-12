===============================================================
       Standalone Interactive Graphics (SIG) Toolkit
          Copyright (c) 2018-2019 Marcelo Kallmann
===============================================================

SIG is a C++ class toolkit for the development of applications displaying
interactive graphics. SIG is designed to be small, flexible, fast, portable,
and standalone. SIG includes a simple and extendible scene graph, a flexible
skeleton structure supporting a variety of joint definitions, classes for 
loading and manipulating bvh motion files and obj geometry files, several 
utilities for developing motion planners, and an analytical Inverse Kinematics
solver. It also includes functionality to manage resources such as textures
and fonts. Being standalone the entire functionality is exposed making SIG
a great tool to learn OpenGL and to develop research in computer graphics.

==== TERMS OF DISTRIBUTION ====

This software is distributed under the Apache License, Version 2.0.
All copies must contain the full copyright notice licence.txt located
at the base folder of the distribution.

Sig is available from: https://bitbucket.org/mkallmann/sig

==== INSTALLATION GUIDELINES ====

Windows: use the visual studio solution(s) in the vs20NN folder
Linux:   call make from the make/ folder (in development)
Other:   a port to glfw is also under development

==== DIRECTORIES ====

- data:     example model and skeleton files
- doc:      documentation files
- examples: example programs using SIG
- fonts:    font files that can be used by SIG
- include:  header files of the SIG libraries
- lib:      after compilation will contain compiled libraries
- make:     makefiles for g++ SIG compilation
- scripts:  python scripts to facilitate development
- shaders:  glsl shaders used by SIG
- src:      C++ sources for the SIG libraries
- vs2017:   projects for Microsoft Visual Studio 2017 (Community Version)

==== GETTING STARTED ====

- Read doc/sigdoc.pdf for an overview of SIG
- Study the provided example projects
- Use project "sigapp" to start your own project (read examples/readme.txt)
- Read roadmap.txt for a summary of the development state
 
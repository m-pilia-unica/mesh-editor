Mesh Editor - PR1 Final Project
===============================
This program is part of the programming class final project. It permits to do 
simple edits to a triangular mesh 3D model, like colour the model or apply a 
rotation to it.

Features
========
The editor permits to color the model in various ways:
- gradient coloration along principal axis;
- distance based coloration respect to an arbitrary point;
- flat coloration;
- random coloration.

Another extra feature is the possibility to rotate the model around an arbitrary
axis. 

The program can show some informations about the model (vertices and faces number, total surface, volume, biggest and smallest face, extreme vertices 
for each coordinate).

Overall Design
==============
The control flow is handled inside the main subroutine. The application is 
designed with a main do-while loop, each cicle of wich consists into:
1. get a filename;
2. open the file;
3. parse model data from file;
4. do some action (with at least one coloration);
5. get a output filename;
6. save the modified model in the new file.

In order to provide the possibility of doing more than one
action (e.g. coloration and rotation or info showing) the main menu 
is shown inside a nested do-while loop, and for each of its cycles it
asks the user for an action, complete the request and then iterates, until
the user chose to save the model or exit discarding changes.

![flowchart](/media/flowchart.png)

Modularization
==============

The program was structured dividing the back-end code (which actually 
implements algorithms and functionalities, and it is potentially reusable) 
from front-end code implementing the program interace and handling the 
interation with the user.

Build and run
=============
To build the project with gcc or a compatible compiler, launch the following 
command in the project root directory
~~~~{.sh}
gcc -o ./bin/main main.c frontend.c backend.c -lm 
~~~~
or similar command for other compilers. When compiled with the `__DEBUG__` 
macro defined (e.g. through the gcc's -D parameter) the application 
provides extra debug output.

If you have a working GNU make installation, you can build the project using
the makefile. Launch in the root folder:
~~~~{.sh}
make
~~~~

To build the project with extra debug output:
~~~~{.sh}
make debug
~~~~

To build the project documentation:
~~~~{.sh}
make doc
~~~~

License
=======
The project is licensed under GPL 3. See [LICENSE](/LICENSE) file for the full 
license.

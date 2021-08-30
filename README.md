# cyclo
Author: Guillaume (Bill) ARRECKX

Intro
=====
As a professional embedded software architect, I always work on small pet
projects to keep myself up-to-date with the latest changes in languages.
This project was the perfect excuse to get to try the latest C++20 features.
Using a small, resource constrained microcontroller, is far more very
revealing of the compilers performance. 

The project initially tried to create a meta modelled framework (using concepts et al.), but
the effort required, the debug, the maintenability did not seem reasonable for now. C++20 brings
many good things, but they perform best in libraries, and should not be used solely because they
exist and are new! I also had some issues compiling existing code, so in the end, downgraded
to a C++17 compiler.

For good measured, the project uses a AVR xMega port of the latest FreeRTOS,
the excellent ETL library, a message framework (on top of ETL messages), an RTOS C++
abstraction and the excellent boost::sml state machine library.
The software architecture was done in UML.
No dynamic memory is used.

Brief
=====
Cyclo is a local and remote controllable relay capable of handling 8A over 240V AC.
It is housed in a DIN mountable enclosure, and offers 2 banana sockets as well as Molex connection.
It is powered through a USB socket, which also allow to drive it remotely, using a VT100 terminal.

Cyclo can be used to cycle some systems on/off many times to validate proper boot up of embedded
equipments. It can be used to turn on/off any devices, down to 10ms to test power supplies resilence for example.
The remote control and computer friendly, and allow to control the relay directly.

The system can start automatically too.

The joystick on the side, allow configuring Cyclo manually to create a simple repeating on/off program,
with timings from 1 second to 100 minutes - on and off.

The toggle switch allow to make it normally opened or close when powered off or idle.

Up to 10 programs can be created for different scenari, single run, or repeating sequences.
When driving it from a computer, it is possible to create some custom profiles,
using random numbers, or ramps to vary the timing to look for a specific problem.

Software
========
The software has been written for a gcc C++17 compiler. It can be compiled
for an AVR xMega, or for a Linux PC, to run the simulator.
The need for the simulator is to allow proper debugging, which is not
possible using the Microchip Studio since the latest supported compiler is
a gcc 4.7 (the latest at the time of writting this is the 11), but to get
the c++17 feature, the compiler must be updated, and the debugging no
longer works correctly.
A makefile is also provided to compile in Linux or WSL2.
The layout keeps the Microchip Studio layout - which is not ideal IMHO. 
A makefile is added to build from the command line.
The format of the files is managed by clang-format-10 which the clang config
file added.
Note: Gcc 8 or greater is required. I am using gcc 10.

Licence
=======
Cyclo is licenced under MIT licence.

Hardware
============================================================================
Cyclo has been designed with Cadsoft Eagle (v7) using a non-profile licence.
Since, CadSoft was acquired by Autodesk, but the files remain compatible with
the latest version of Eagle.

The case was designed with open source CAD and is made available as 
STL and STEP. It is 3D printer friendly.

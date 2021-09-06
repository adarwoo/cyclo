# cyclo
Author: Guillaume (Bill) ARRECKX

Brief
=====
Cyclo is a remote controllable relay capable of handling 16A over 240V AC or 3A over 120V DC.
It is housed in a DIN mountable enclosure, and offers 2 banana sockets as well as Molex connection.
It is powered through a USB socket, which also allow to drive it remotely, using a VT100 terminal.
It features a small OLED screen and a joystick to drive it manually, and display operation stats.

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
exist and are new! I also had some issues compiling existing code, so in the end, I downgraded
the project to a C++17 compiler.

Nontheless, the project features many interesting features, such as:
 * An AVR xMega (ATxMega128A4U) with 128K of flash and 8K of RAM, 2K of EEProm and a USB port
 * A XMega port of the latest FreeRTOS (10.4.4)
 * A C++ wrapper for FreeRTOS (home brew using typestrings from https://github.com/irrequietus/typestring)
 * The excellent ETL library https://www.etlcpp.com/ (I added a Tokenizer class)
 * A message framework (on top of ETL messages and FreeRTOS queues)
 * The boost::sml state machine library

Product description
===================
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
The software architecture was done in UML, with class diagram, sequence diagram and state machines.
The emphasis is on using as much C++ as required.

No dynamic memory is used - and the stack is used as little as possible.

Building the software
=====================
The software has been written for a gcc C++17 compiler. It can be compiled
for an AVR xMega, or for a Linux PC, to run the simulator.
The need for the simulator is to allow proper debugging, which is not
possible using the Microchip Studio since the latest supported compiler is
a gcc 4.7 (the latest at the time of writting this is the 11), but to get
the c++17 feature, the compiler must be updated, and the debugging no
longer works correctly.
A makefile is also provided to compile in Linux or WSL2.
Since it uses a recent compiler, I have added a Dockerfile to create a container
to rebuild all. If you're running Linux or Windows 10/11, use the Docker container
and save yourselve plenty of pain.
A convenient script will build the docker container and launch the build.
The layout keeps the Microchip Studio layout - which is not ideal IMHO. The project can
still be opened with Microchip studio - but you'll need to ugrade the compiler.
A makefile is added to build from the command line.
The format of the files is managed by clang-format-10 which the clang config
file added.
Note: Gcc 8 or greater is required. I am using gcc 11.
I use Microsoft VSCode for all the developement - including debugging (the simulated version).
I find it is a fantastic IDE with none of the Eclipse heavyness and frustrations.
The .vs workspace is part of the distribution.

Building the hardware
=====================
The PCB can be done as a single or double sided !
Cyclo has been designed with an old Cadsoft Eagle (v7.x) using a non-profile licence.
Since, CadSoft was acquired by Autodesk, but the files remain compatible with
the latest version of Eagle.

I have added the .brd and .sch which must PCB house will take.
If you plan on making the PCB yourself - you will need a production method which can achieve 
at least 10mil tracks and isolation. A good CNC will do that. If you're ething, you need to
know what you're doint.
I personnaly still etch my PCB and also do PTH (Yes!). But I have a CNC for drilling, and 
a lot of chemicals like paladium... and 15 years experience doing it.

Building the case
=================
The case was designed with open source CAD and is made available as 
STL and STEP. It is 3D printer friendly.
I use a Prusa MK3 with PLA or PETG.
The PCB once cut to size will clip in place. No screews required.

Licence
=======
Cyclo is licenced under MIT licence.

External links
==============
FreeRTOS              : https://freertos.org
ETL                   : https://www.etlcpp.com
Boost SML             : https://boost-ext.github.io/sml/index.html
Typestring library    : https://github.com/irrequietus/typestring
Toolchain for Windows : https://blog.zakkemble.net/avr-gcc-builds/
Toolchain for Linux   : https://avr.lumito.net/downloads/
Docker                : https://www.docker.com
VSCode                : https://code.visualstudio.com
Git                   : https://git-scm.com
GNU Make              : https://www.gnu.org/software/make/

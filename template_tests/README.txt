# Command set

Commands
--------
[o]pen
[c]lose

Sequence control
----------------
[s]ave <n> Write the last sequence into the slot n, where n 1->9.
[l]run <n> Run. Run the program number n (0->9), 0 is the manual program. 
           If the program slot is empty, generates an error
[d]efault <n> Make the sequence start on power-up.
           If d is empty, remove the default. The command 'o' becomes the default.

Time units
----------
H heure
M minutes
s seconds
m milliseconds

Example:
--------
$ o 1s 500m o 4s c
> [00012] / NO : Open (34s)
$ open 1s close 60s
Close the relay
$ c

A minimum pause of 1 second as added to all open/close. All sequences are looped.

A sequence grab the console. Enter to exit.
The prompt is $
The info line starts with >

The console waits for a user input <enter> to start.
The console is ready to accepts commmand when $ is displayed.
The console is preloaded with all programs in the history.

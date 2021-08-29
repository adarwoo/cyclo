Some quick thoughts for the tracing.

The tracing library has been split so it can easily run on multiple OSes.

The one size fits all approach is defuncts.

For each OS, you should have:
	A DLL/SharedLib entry point function log_init_[OS_NAME].c
	A OS specific implementation : log_os_[OS_NAME].cpp
	
To add a new OS, you must provide all methods declared in log_os.h.

To figure out what they do, fire Doxygen on the code - and things will become clear.

Guillaume


# multima
Long term project now even taking longer time!

## Build prerequsitives
The build system (build.bat) only supports Windows at the moment, but I'm planning to add Linux support as well.

This is what I use:
 * Visual Studio 2013 (the free version)
 
Running `build.bat` assumes that the cmd prompt was started through `misc/shell.bat`, which sets up paths and so on, especially the one that points to your VC++ installation.
But you could easily change `build.bat` to just point to that directly instead.

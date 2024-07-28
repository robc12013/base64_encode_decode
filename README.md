A simple base64 encoder and decoder written in C, for use within other C programs on Linux systems.

To use this in another C program, add this line in said program:

`#include "./base64.h"`

And then compile it something like this:

`gcc someOtherProgram.c base64.c -lm`
# dcf77-decode
Decode the DCF77 time signal on a PC. Tested on Linux, should work on most operating systems.

# Building and usage
Compile with
```
gcc dcf77_decode.c -o dcf77_decode
```

Run with
```
./dcf77_decode datafile
```

Due to the differences in receiving hardware, the data is read from a file.
If "-" is specified as the datafile, input is read from stdin.

## Datafile format
- Empty line ("\n") = Missing pulse at the start of a minute
- Line only with a 1 ("1\n") = Bit 1
- Line only with a 0 ("0\n") = Bit 0

An example can be found in ``data-example.txt``

# Arduino example
The arduino_dcf77_to_serial directory contains Arduino code that can be used to send the data coming from a DCF77 receiver module over serial to a PC. In this case the serial device file (e.g. /dev/ttyUSB0) can be specified as the datafile.

# License
GNU GPL v3 or later

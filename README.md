G35marquee
==========

A serial-connected LED scrolling marquee display for Teensy 2.0 (AVR 32u4).
---------------------------------------------------------------------------

Distribute freely and use for any purpose. No warranty expressed or implied.
Full License at: http://creativecommons.org/licenses/by/3.0/

GitHub home: https://github.com/kc0tfb/G35marquee
Demo video: http://youtu.be/NIbXecUxjb0

This Arduino/Teensyduino-compatible project uses 3 strands of GE-35
ColorEffects addressable LEDs as a marquee.  The marquee simultaneously
receives both USB and UART serial streams from a PC or other controller. 
When a CR or LF is received, it displays the string on the strands of LEDs
attached to a Teensy 2.0 (an inexpensive AVR 32u4 dev board from PJRC.com).

Please see the ACKNOWLEDGEMENTS section.  This project wouldn't exist
without knowledge gained from the others who tread this path before me.

HARDWARE:
---------

* a Teensy 2.0 AVR Mega32u4 dev board (PJRC.com)
* 3 strands of GE G35 ColorEffects LED holiday lights
* a pair of APC220 data radios (DFrobot.com, configured for 9600N81)
* (or) a microcontroller that sends TTL serial messages to the 32u4.
* (or) a PC sending USB serial messages to the 32u4.

The LEDs strands (we'll call them A, B and C) are attached to three pins on
the microcontroller and are laid out in 6 rows, as follows:

A01 A02 A03 A04 A05 A06 A07 A08 A09 A10 A11
A22 A21 A20 A19 A18 A17 A16 A15 A14 A13 A12

B01 B02 B03 B04 B05 B06 B07 B08 B09 B10 B11
B22 B21 B20 B19 B18 B17 B16 B15 B14 B13 B12

C01 C02 C03 C04 C05 C06 C07 C08 C09 C10 C11
C22 C21 C20 C19 C18 C17 C16 C15 C14 C13 C12

The code to initialize each LED pixel turns them into an [X][Y] matrix used
to create the scrolling text display. See the "USAGE" file for more info.

IMPORTANT NOTE:
---------------

When connecting the strands to your microcontroller or dev board, you'll
need a common ground as well as a data line connection to each strand.  DO
NOT intermingle the +5V power supply lines, however.  If you're using the
stock power supplies with these lights, each strand should be kept on its
own power supply.

Also, do not connect both your computer's USB connection and the +5V rail
from any other power supply to the microcontroller at the same time.  The
voltage differential between the power supply and the computer will allow
current to flow where it shouldn't (from the LED's power supply through USB
to your computer), potentially damaging the PC, microcontroller, power
supply or all of the above.

ACKNOWLEDGEMENTS:
-----------------

This project would not have been possible without the pioneering work done
by several others:

DeepDarc for discovering the LED control protocol and providing the original
implementation of G35 LED third-party control software.

	www.deepdarc.com/2010/11/27/hacking-christmas-lights/

John Riney and Jason Beeland's project provided a starting point for my code
and gave me ideas for assembling the hardware.  I offer my apologies to
them, as a lot of Jason's fine work has been irreperably mangled in my
pursuit of minimized memory footprint.

	https://github.com/riney/g35_arduino_marquee

Sowbug's G35Arduino library provided the data transmission routine.  There
are a number of other Arduino-compatible G35 LED resources out there, but
G35Arduino is pretty much the gold standard, in that it offers control of
multiple strands with individual queues and proper timing management,
exposing the novice programmer to only minimal complexity. If you want a
general-purpose G35 strand control library, this is it:

	https://github.com/sowbug/G35Arduino

BACKSTORY:
----------

I wanted to create an AVR-based serial display with three 50-LED strands of
GE ColorEffects lights.  I tried to use Jason Beeland's code, however I
didn't want to use a Mega for this and he used structs that couldn't be
stuffed into much less RAM and still handle 150 pixels.  I really liked the
double-buffered approach he took to updates, as it made the display
amazingly fast, so I knew I had to keep that part.

I started by throwing a lot of those big structs out the window.  I divorced
pixel matrix/address data into its own array since it didn't need to be
double-buffered like the pixels were.  I further reduced the footprint of
the pixel address array by stuffing the IO pin data into the top two bits,
since the G35 protocol only uses 6 bit addresses for bulbs.  I turned to a
palette-based approach to handle color and intensity, shrinking the output
and display buffers down to just one byte for each pixel.

All those changes got me code that compiled for the 32u4 (in the form of a
Teensy 2.0 from PJRC.com), with plenty of room to spare, but I was on a
roll.  It was time to add features to make the project complete.  Next, I
moved the font table into flash memory, taking advantage of the additional
room to add a few more characters and to make it proportional width.  Then I
added code to accept serial data and put it up on the display.  This gave me
what I needed for the next phase.

I spent an evening annoying my girlfriend by making the display turn lots of
ugly colors in search of 14 nice ones (plus black and white) to form the
palette defined in G35font.h.  She still thinks "peach" is a little off,
but oh, well. ;)   You're welcome to comment out the section of
G35_textDisplay() that handles special effects and replace it with the one
that allows fine-tuning of colors, left in for this purpose should you wish
to alter or add to the color palette. Once you're happy with your color
tweaks, immortalize them in the top section of G35font.h.

Speaking of special effects, within a string, you may: change the foreground
color, fill the buffer with foreground or background color, swap
foreground/background color, change scroll speed, pause the display and
select loop-forever or run-once mode.  This covers pretty much everything I
wanted to do with this display.

With all this, there's still plenty of RAM to write your own loop() code to
read sensors or otherwise obtain information to display.  Feel free to add
your own contributions.  If you think of nifty effects to add, drop me a
line and I'll consider adding them to my code.

Thank you and happy hacking!

Peter Gamache, KC0TFB

P.S.  The ampersand glyph is ugly.  Can you do better?  Please share!

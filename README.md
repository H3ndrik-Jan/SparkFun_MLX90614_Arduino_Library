C Library for utilizing the MLX90614 IR sensor with an Xmega
========================================
I changed sparkfuns library for use with an Xmega, without using the arduino environment. Not all functions of the sparkfun library are implemented for the xmega. Instead of using the Arduino Wire library for I2C communication, the library utilizes an i2c library which can be found in this book: https://www.dolman-wim.nl/xmega/index.php

Repository Contents
-------------------
* * **/src** - Source files for the library (.c, .h).

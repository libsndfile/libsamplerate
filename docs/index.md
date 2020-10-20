---
layout: default
title: Libsamplerate home page
---

{: .indent_block}
*"Choosing a sample rate coverter wasn't easy. We ran numerous tests with Secret
Rabbit Code and other sample rate converters, then compared them all. In the
end, SRC outperformed the others, including some extremely well known and
popular software. We had one issue with SRC, but after emailing Erik, he got
back to us immediately with an answer. Choosing SRC was a no brainer."*  
Ryan Smith, International Marketing Manager,  
[Emersys Corp.](http://emersys.co.kr/), South Korea.  
Product : [Maven3D](http://maven3d.com)

**S**ecret **R**abbit **C**ode (aka libsamplerate) is a **S**ample **R**ate
**C**onverter for audio. One example of where such a thing would be useful is
converting audio from the CD sample rate of 44.1kHz to the 48kHz sample rate
used by DAT players.

**SRC** is capable of arbitrary and time varying conversions; from downsampling
by a factor of 256 to upsampling by the same factor. Arbitrary in this case
means that the ratio of input and output sample rates can be an irrational
number. The conversion ratio can also vary with time for speeding up and slowing
down effects.

**SRC** provides a small set of converters to allow quality to be traded off
against computation cost. The current best converter provides a signal-to-noise
ratio of 145dB with -3dB passband extending from DC to 96% of the theoretical
best bandwidth for a given pair of input and output sample rates.

Since the library has few dependencies beyond that provided by the standard C
library, it should compile and work on just about any operating system. It is
known to work on Linux, MacOSX, [Win32](win32.md) and Solaris. With some
relatively minor hacking it should also be relatively easy to port it to
embedded systems and digital signal processors.

In addition, the library comes with a comprehensive test suite which can
validate the performance of the library on new platforms.

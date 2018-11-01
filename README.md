# AEOLUS FOR CSOUND

Michael Gogins
http://michaelgogins.tumblr.com

## Introduction

This project provides a set of Csound plugin opcodes encapsulating the 
Aeolus software organ by Fons Adriaensen. I have always loved how this 
instrument actually sounds, and how it uses the whole battery of pipes and 
stops from organ history. I am now making it available in Csound.

Aeolus can of course already be used with Csound via Jack, but these 
opcodes are considerably easier to use.

See:

http://www.muse-sequencer.org/index.php/Aeolus
http://kokkinizita.linuxaudio.org/linuxaudio/aeolus/index.html
https://github.com/fugalh/aeolus

I take this as canonical:

https://salsa.debian.org/multimedia-team/aeolus

Aeolus and supporting libraries are licensed as GPLv3. See:

https://www.gnu.org/licenses/gpl-faq.en.html#WhatDoesCompatMean

The vexing question of using GPLv3 code to create a plugin for LGPv2.1 Csound 
is as follows:

https://www.gnu.org/licenses/quick-guide-gplv3.html

"GPLv2 is compatible with GPLv3 if the program allows you to choose 'any later 
version' of the GPL, which is the case for most software released under this 
license." This "any later version" language appears in the COPYING file of the 
Csound repository, and in the license text of csound.h.

I take this to mean I can build and run the Aeolus plugin with Csound, but the 
plugin itself must be GPLv3. 

# Installation

On Linux, download the Debian package from this repository and install it in 
the usual way, e.g. `sudo apt install ./csound-aeolus-1.0.0-Linux.deb`. A 
set of organ stops are installed in `usr/share/csound-aeolus/stops-0.3.0`.

# Building

Install these Debian packages by running `bash install-dependencies.bash`:
```
libasound2-dev
libclthreads-dev
libclxclient-dev
libjack-dev
libreadline-dev
libzita-alsa-pcmi-dev
```
Change to the `aeolus/source` directory and execute `make` and `sudo make 
install`. You should now be able to run the regular Aeolus application as a 
test of your system.

To build the Csound opcodes, change to the root directory of the repository
and execute:
```
mkdir build
cd build
cmake ..
make
sudo make install
```

# Usage

Aeolus stops and instruments must defined using the Aeolus program. The Csound 
opcodes can load stops, instruments, and presets, and select and modify stops 
and presets.

See `doc/csound_aeolus.pdf` for documentation of the opcodes.

# Design notes

The Csound opcodes are based on the Aeolus application's `aeolus_txt.so`
loadable module and the text mode's `main` function.

All of the actual opcodes are defined in `src/csound_aeolus.cpp`.
Customized versions of some of the Aeolus source files from the 
`aeolus/source` directory have been created in the `src` directory. As few 
changes as possible have been made to these files, for the purpose of 
maintainability in the future.

Internally, Aeolus uses several threads that communicate with each other by 
message passing. The Csound opcodes follow the same design.








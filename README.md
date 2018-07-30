# AEOLUS FOR CSOUND
## Michael Gogins

This project provides a set of Csound plugin opcodes encapsulating the 
Aeolus software organ by Fons Adriaensen. I have always loved how this 
instrument actually sounds, and how it uses the whole battery of pipes and 
stops from organ history. I would like to make it available in Csound.

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

# Building

Install these Debian packages by running `bash install-dependencies.bash`:

libasound2-dev
libclthreads-dev
libclxclient-dev
libjack-dev
libreadline-dev
libzita-alsa-pcmi-dev

Change to the source directory and execute `make`.

# Design

Aeolus stops and instruments must defined using the Aeolus program. The Csound 
opcodes can select and modify presets.

The text-mode Aeolus is completely encapsulateed in a loadable module. Its  
command-line options are:
```
Options:
  -h                 Display this text
  -t                 Text mode user interface
  -u                 Use presets file in user's home dir
  -N <name>          Name to use as JACK and ALSA client [aeolus]
  -S <stops>         Name of stops directory [stops]
  -I <instr>         Name of instrument directory [Aeolus]
  -W <waves>         Name of waves directory [waves]
  -J                 Use JACK (default), with options:
    -s               Select JACK server
    -B               Ambisonics B format output
  -A                 Use ALSA, with options:
    -d <device>        Alsa device [default]
    -r <rate>          Sample frequency [48000]
    -p <period>        Period size [1024]
    -n <nfrags>        Number of fragments [2]
```

Actual control of the organ for performance, most importantly setting stops, 
is done with MIDI controller number 0x98.

The corresponding Csound opcodes are as follows.

## Initialization

`S_aeolus aeolus_init S_stops_directory, S_instruments_directory,_S_waves_directory, i_bform`

## Select Bank and Preset

`aeolus_preset S_aeolus, i_bank, i_preset, S_presets_directory`

## Low-level MIDI

`aeolus_midi S_aeolus, k_status, k_channel, k_data1 [, k_data2]`

## Group Mode

`aeolus_group_mode S_aeolus, i_group, i_mode`

## Select Stop

`aeolus_stop S_aeolus, k_stop_button`

## Play Note

`aeolus_note S_aeolus, i_midi_channel, i_midi_key, i_midi_velocity`

## Audio Output

Aeolus computes audio in first order Ambisonic or stereo format.
In either case this should be used in one always-on instrument per 
Aeolus instance. a_out[] must be integer size 4 for Ambisonic or size 2 
for stereo.

`a_out[] aeolus_out S_aeolus`

# Implementation

The class hierarchy in Aeolus is not based on abstract interfaces as much as 
it should be. Things must be set up as in main.cc. The Audio class hardwires
ALSA or Jack. MIDI events also come via the ALSA sequencer or via Jack. 

I have created a new CsoundAudio class that started out as a copy of the 
existing Audio class. Processing in CsoundAudio is modeled on Jack but the 
Jack callback is replaced by a Csound callback invoked from the output 
opcode's krate function. All control and key events are enqueues using a 
modification of Aeolus' Jack MIDI dispatcher.

Audio is always synthesized in Ambisonic B-format, but the output opcodes 
can generate either Ambisonic or stereo audio.

In Aeolus, the output buffer is indexed [channel][frame].

In Csound, the output array elements are indexed [channel][frame].

## MIDI Logic

MIDI logic seems a bit hackish. 

Division I	    Channel 0
Division II	    Channel 1
Division III	Channel 2
Pedal	        Channel 3

There are 32 presets in 32 banks. 

Presets are sent as program change events (status 0xC0) of value < 32.

Other things are sent as controllers (status 0xB0) as follows:

Bank select     Controller 32   Value mask 000bbbbb (b bank)
Group mode      Controller 98   Value mask 01mm0ggg (m mode g group)
Stop select     Controller 98   Value mask 000bbbbb (b stop button)



<CsoundSynthesizer>
<CsOptions>
; Credits: Adapted by Michael Gogins 
; from code by David Horowitz and Lian Cheung. 
; the Aeolus GUI to dispatch events and display properly.
-m3 --displays -odac
</CsOptions>
<CsInstruments>
sr = 44100
ksmps = 20
nchnls = 2 ; Changed for WebAssembly output from: = 2

// S_aeolus aeolus_init S_stops_directory, S_instruments_directory,_S_waves_directory, i_bform

gS_aeolus aeolus_init "/home/mkg/stops-0.3.0", "Aeolus", "waves", 0

// aeolus_preset S_aeolus, i_bank, i_preset, S_presets_directory

aeolus_preset gS_aeolus, 1, 1, "~/.aeolus-presets"

; Send notes from the score to the Aeolus.
instr 1 
imidichannel init 0
aeolus_note gS_aeolus, imidichannel, p4, p5
endin

; Send audio from the Aeolus to the output.
instr 3 
a_out[] init 2
a_out aeolus_out gS_aeolus
out a_out
endin

</CsInstruments>
<CsScore>
; Turn on the instrument that receives audio from the Aeolus indefinitely.
i 3 0 -1
; Send a C major 7th arpeggio to the Aeolus.
i 1 1 10 60 76
i 1 2 10 64 73
i 1 3 10 67 70 
i 1 4 10 71 67
i 1 11 10 60 76
i 1 12 10 64 73
i 1 13 10 67 70 
i 1 14 10 71 67
i 1 21 10 60 76
i 1 22 10 64 73
i 1 23 10 67 70 
i 1 24 10 71 67
i 1 31 10 60 76
i 1 32 10 64 73
i 1 33 10 67 70 
i 1 34 10 71 67
; End the performance, leaving some time 
; for the Aeolus to finish sending out its audio,
; or for the user to play with the Aeolus virtual keyboard.
e 20
</CsScore>
</CsoundSynthesizer>

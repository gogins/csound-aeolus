<CsoundSynthesizer>
<CsOptions>
; Credits: Adapted by Michael Gogins 
; from code by David Horowitz and Lian Cheung. 
; the Aeolus GUI to dispatch events and display properly.
-m3 --displays -odac
</CsOptions>
<CsInstruments>
sr = 48000
ksmps = 64
nchnls = 2 
0dbfs = 1

gi_aeolus aeolus_init "/home/mkg/stops-0.3.0", "Aeolus", "waves", 0, 10

; Send notes from the score to the Aeolus.
instr 1 
aeolus_note gi_aeolus, p4, p5, p6
endin

; Send audio from the Aeolus to the output.
instr 2 
print p1, p2, p3
aeolus_preset gi_aeolus, 1, 1
a_out[] init 2
a_out aeolus_out gi_aeolus
out a_out
endin

</CsInstruments>
<CsScore>
; Turn on the instrument that receives audio from the Aeolus indefinitely.
i 2 0 -1
; Send a C major 7th arpeggio to the Aeolus.
i 1  1 10 1 60 76
i 1  2 10 1 64 73
i 1  3 10 1 67 70 
i 1  4 10 1 71 67
i 1 11 10 2 60 76
i 1 12 10 2 64 73
i 1 13 10 2 67 70 
i 1 14 10 2 71 67
i 1 21 10 3 60 76
i 1 22 10 3 64 73
i 1 23 10 3 67 70 
i 1 24 10 3 71 67
i 1 31 10 4 60 76
i 1 32 10 4 64 73
i 1 33 10 4 67 70 
i 1 34 10 4 71 67
; End the performance, leaving some time 
; for the Aeolus to finish sending out its audio,
; or for the user to play with the Aeolus virtual keyboard.
e 20
</CsScore>
</CsoundSynthesizer>

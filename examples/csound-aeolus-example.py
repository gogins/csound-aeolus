import ctcsound
csound = ctcsound.Csound()
orc = '''
sr = 48000
ksmps = 64
nchnls = 2 
0dbfs = 1

gi_aeolus aeolus_init "/home/mkg/stops-0.3.0", "Aeolus", "waves", 0, 8

instr 1 
print p1, p2, p3, p4, p5
aeolus_note gi_aeolus, p1, p4, p5
endin

instr 2 
print p1, p2, p3, p4, p5
aeolus_note gi_aeolus, p1, p4, p5
endin

instr 3 
print p1, p2, p3, p4, p5
aeolus_note gi_aeolus, p1, p4, p5
endin

instr 4 
print p1, p2, p3, p4, p5
aeolus_note gi_aeolus, p1, p4, p5
endin

instr 5
print p1, p2, p3, p4, p5
aeolus_note gi_aeolus, p1, p4, p5
endin

instr 6
print p1, p2, p3, p4, p5
aeolus_note gi_aeolus, p1, p4, p5
endin

alwayson "aeolus_out"

; Send audio from the Aeolus to the output.
instr aeolus_out 
print p1, p2, p3
aeolus_preset gi_aeolus, 1, 1, "~/.aeolus-presets"
a_out[] init 2
a_out aeolus_out gi_aeolus
out a_out
endin
'''

sco = "f 0 360 \n"
c = 0.983
y = 0.5
t = 1
for i in xrange(400):
    t = t + 0.25
    d = .25 + (i % 4) / 2
    y1 = y * 4 * c *  (1 - y)
    y = y1
    k = int(36 + y * 60)
    v = 80
    sco = sco + "i %9.4f %9.4f %9.4f %9.4f %9.4f\n" % (1 + (i % 4), t, d, k, v)
print sco
csound.compileOrc(orc)
csound.readScore(sco)
csound.setOption("-odac")
csound.start()
csound.perform()
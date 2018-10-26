#include <cmath>
#include <csound.hpp>
#include <cstring>
#include <string>

int main(int argc, const char **argv)
{
    Csound csound;
    csound.Message("AeoluS Study #1\n");
    std::string orc = R"(
sr = 48000
ksmps = 64
nchnls = 2 
0dbfs = 1

gi_aeolus aeolus_init "/home/mkg/stops-0.3.0", "Aeolus", "waves", 0, 3

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
)";

std::string sco = R"(f 0 30
)";
    double c = 0.99983;
    double y = 0.56;
    double t = 1;
    for (int i = 0; i < 20; ++i) {
        char scoreline[0x200];
        double t = t + 1./6.;
        double r = std::fmod(i, 4.);
        double d = .25 + (i % 4) / 2;
        double y1 = y * 4. * c * (1. - y);
        y = y1;
        double k = std::floor(36. + y * 60.);
        double v = 80;
        std::sprintf(scoreline, "i %9.4f %9.4f %9.4f %9.4f %9.4f\n", 1. + r, t, d, k, v);
        sco += scoreline;
    }
    csound.Message(sco.c_str());
    csound.CompileOrc(orc.c_str());
    csound.ReadScore(sco.c_str());
    csound.SetOption("-odac");
    csound.Start();
    csound.Perform();
    return 0;
}

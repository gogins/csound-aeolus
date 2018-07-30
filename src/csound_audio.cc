// ----------------------------------------------------------------------------
//
//  Copyright (C) 2003-2013 Fons Adriaensen <fons@linuxaudio.org>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------------


#include <math.h>
#include "csound_audio.h"
#include "messages.h"

CsoundAudio::CsoundAudio (CSOUND *csound_, Lfq_u32 *qnote, Lfq_u32 *qcomm) :
    A_thread ("CsoundAudio"),
    csound(csound_),
    _appname ("CsoundAudio"),
    _qnote (qnote),
    _qcomm (qcomm),
    _qmidi (0),
    _running (false),
    _abspri (0),
    _relpri (0),
    _bform (0),
    _nplay (0),
    _fsamp (0),
    _fsize (0),
    _nasect (0),
    _ndivis (0)
{
}


CsoundAudio::~CsoundAudio (void)
{
    int i;
    for (i = 0; i < _nasect; i++) delete _asectp [i];
    for (i = 0; i < _ndivis; i++) delete _divisp [i];
    _reverb.fini ();
}


void CsoundAudio::init_audio (void)
{
    int i;
    _audiopar [VOLUME]._val = 0.32f;
    _audiopar [VOLUME]._min = 0.00f;
    _audiopar [VOLUME]._max = 1.00f;
    _audiopar [REVSIZE]._val = _revsize = 0.075f;
    _audiopar [REVSIZE]._min =  0.025f;
    _audiopar [REVSIZE]._max =  0.150f;
    _audiopar [REVTIME]._val = _revtime = 4.0f;
    _audiopar [REVTIME]._min =  2.0f;
    _audiopar [REVTIME]._max =  7.0f;
    _audiopar [STPOSIT]._val =  0.5f;
    _audiopar [STPOSIT]._min = -1.0f;
    _audiopar [STPOSIT]._max =  1.0f;

    _reverb.init (_fsamp);
    _reverb.set_t60mf (_revtime);
    _reverb.set_t60lo (_revtime * 1.50f, 250.0f);
    _reverb.set_t60hi (_revtime * 0.50f, 3e3f);

    _nasect = NASECT;
    for (i = 0; i < NASECT; i++)
    {
        _asectp [i] = new Asection ((float) _fsamp);
        _asectp [i]->set_size (_revsize);
    }
    _hold = KEYS_MASK;
}


void CsoundAudio::start (void)
{
    M_audio_info  *M;
    int           i;

    M = new M_audio_info ();
    M->_nasect = _nasect;
    M->_fsamp  = _fsamp;
    M->_fsize  = _fsize;
    M->_instrpar = _audiopar;
    for (i = 0; i < _nasect; i++) M->_asectpar [i] = _asectp [i]->get_apar ();
    send_event (TO_MODEL, M);
}

void CsoundAudio::thr_main (void)
{
}

void CsoundAudio::proc_queue (Lfq_u32 *Q)
{
    uint32_t  k;
    int       b, c, i, j, n;
    union     {
        uint32_t i;
        float f;
    } u;

    // Execute commands from the model thread (qcomm),
    // or from the midi thread (qnote).

    n = Q->read_avail ();
    while (n > 0)
    {
        k = Q->read (0);
        c = k >> 24;
        j = (k >> 16) & 255;
        i = (k >>  8) & 255;
        b = k & 255;

        switch (c)
        {
        case 0:
            // Single key off.
            key_off (i, b);
            Q->read_commit (1);
            break;

        case 1:
            // Single key on.
            key_on (i, b);
            Q->read_commit (1);
            break;

        case 2:
            // Conditional key off.
            cond_key_off (j, b);
            Q->read_commit (1);
            break;

        case 3:
            // Conditional key on.
            cond_key_on (j, b);
            Q->read_commit (1);
            break;

        case 4:
            // Clear bits in division mask.
            _divisp [j]->clr_div_mask (b);
            Q->read_commit (1);
            break;

        case 5:
            // Set bits in division mask.
            _divisp [j]->set_div_mask (b);
            Q->read_commit (1);
            break;

        case 6:
            // Clear bits in rank mask.
            _divisp [j]->clr_rank_mask (i, b);
            Q->read_commit (1);
            break;

        case 7:
            // Set bits in rank mask.
            _divisp [j]->set_rank_mask (i, b);
            Q->read_commit (1);
            break;

        case 8:
            // Hold off.
            _hold = KEYS_MASK;
            cond_key_off (HOLD_MASK, HOLD_MASK);
            Q->read_commit (1);
            break;

        case 9:
            // Hold on.
            _hold = KEYS_MASK | HOLD_MASK;
            cond_key_on (j, HOLD_MASK);
            Q->read_commit (1);
            break;

        case 16:
            // Tremulant on/off.
            if (b) _divisp [j]->trem_on ();
            else   _divisp [j]->trem_off ();
            Q->read_commit (1);
            break;

        case 17:
            // Per-division performance controllers.
            if (n < 2) return;
            u.i = Q->read (1);
            Q->read_commit (2);
            switch (i)
            {
            case 0:
                _divisp [j]->set_swell (u.f);
                break;
            case 1:
                _divisp [j]->set_tfreq (u.f);
                break;
            case 2:
                _divisp [j]->set_tmodd (u.f);
                break;
                break;
            }
        }
        n = Q->read_avail ();
    }
}


void CsoundAudio::proc_keys1 (void)
{
    int d, m, n;

    for (n = 0; n < NNOTES; n++)
    {
        m = _keymap [n];
        if (m & 128)
        {
            m &= 127;
            _keymap [n] = m;
            for (d = 0; d < _ndivis; d++) _divisp [d]->update (n, m);
        }
    }
}


void CsoundAudio::proc_keys2 (void)
{
    int d;

    for (d = 0; d < _ndivis; d++) _divisp [d]->update (_keymap);
}


void CsoundAudio::proc_synth (int nframes)
{
    int           j, k;
    float         W [PERIOD];
    float         X [PERIOD];
    float         Y [PERIOD];
    float         Z [PERIOD];
    float         R [PERIOD];
    float        *out [8];

    if (fabsf (_revsize - _audiopar [REVSIZE]._val) > 0.001f)
    {
        _revsize = _audiopar [REVSIZE]._val;
        _reverb.set_delay (_revsize);
        for (j = 0; j < _nasect; j++) _asectp[j]->set_size (_revsize);
    }
    if (fabsf (_revtime - _audiopar [REVTIME]._val) > 0.1f)
    {
        _revtime = _audiopar [REVTIME]._val;
        _reverb.set_t60mf (_revtime);
        _reverb.set_t60lo (_revtime * 1.50f, 250.0f);
        _reverb.set_t60hi (_revtime * 0.50f, 3e3f);
    }

    for (j = 0; j < _nplay; j++) out [j] = _outbuf [j];
    for (k = 0; k < nframes; k += PERIOD)
    {
        memset (W, 0, PERIOD * sizeof (float));
        memset (X, 0, PERIOD * sizeof (float));
        memset (Y, 0, PERIOD * sizeof (float));
        memset (Z, 0, PERIOD * sizeof (float));
        memset (R, 0, PERIOD * sizeof (float));

        for (j = 0; j < _ndivis; j++) _divisp [j]->process ();
        for (j = 0; j < _nasect; j++) _asectp [j]->process (_audiopar [VOLUME]._val, W, X, Y, R);
        _reverb.process (PERIOD, _audiopar [VOLUME]._val, R, W, X, Y, Z);

        if (_bform)
        {
            for (j = 0; j < PERIOD; j++)
            {
                out [0][j] = W [j];
                out [1][j] = 1.41 * X [j];
                out [2][j] = 1.41 * Y [j];
                out [3][j] = 1.41 * Z [j];
            }
        }
        else
        {
            for (j = 0; j < PERIOD; j++)
            {
                out [0][j] = W [j] + _audiopar [STPOSIT]._val * X [j] + Y [j];
                out [1][j] = W [j] + _audiopar [STPOSIT]._val * X [j] - Y [j];
            }
        }
        for (j = 0; j < _nplay; j++) out [j] += PERIOD;
    }
}


void CsoundAudio::proc_mesg (void)
{
    ITC_mesg *M;

    while (get_event_nowait () != EV_TIME)
    {
        M = get_message ();
        if (! M) continue;

        switch (M->type ())
        {
        case MT_NEW_DIVIS:
        {
            M_new_divis  *X = (M_new_divis *) M;
            Division     *D = new Division (_asectp [X->_asect], (float) _fsamp);
            D->set_div_mask (X->_dmask);
            D->set_swell (X->_swell);
            D->set_tfreq (X->_tfreq);
            D->set_tmodd (X->_tmodd);
            _divisp [_ndivis] = D;
            _ndivis++;
            break;
        }
        case MT_CALC_RANK:
        case MT_LOAD_RANK:
        {
            M_def_rank *X = (M_def_rank *) M;
            _divisp [X->_divis]->set_rank (X->_rank, X->_wave,  X->_sdef->_pan, X->_sdef->_del);
            send_event (TO_MODEL, M);
            M = 0;
            break;
        }
        case MT_AUDIO_SYNC:
            send_event (TO_MODEL, M);
            M = 0;
            break;
        }
        if (M) M->recover ();
    }
}

void CsoundAudio::init_csound(Lfq_u8 *qmidi, bool bform)
{
    int                 i;
    int                 opts;
    ///jack_status_t       stat;
    struct sched_param  spar;
    const char          **p;

    _bform = bform;
    _qmidi = qmidi;

    if (_bform)
    {
        _nplay = 4;
    }
    else
    {
        _nplay = 2;
    }

    _fsamp = csound->GetSr(csound);
    _fsize = csound->GetKsmps(csound);
    for (int i = 0; i < _nplay; i++) _outbuf [i] = new float [_fsize];
    init_audio ();
}

int CsoundAudio::csound_callback (int nframes, ARRAYDAT *output)
{
    int i;
    proc_queue (_qnote);
    proc_queue (_qcomm);
    proc_keys1 ();
    proc_keys2 ();
    proc_synth (nframes);
    proc_mesg ();
    // Copy from Aeolus output to Csound output.
    for (int channel = 0, channels = output->dimensions; channel < channels; ++channel) {
        for (int frame = 0, frames = output->sizes[channel]; frame < frames; ++frame) {
            int index = channel * frames + frame;
            output->data[index] = _outbuf[channel][frame];
        }
    }
    return 0;
}

void CsoundAudio::csound_midi (MYFLT *status, MYFLT *channel, MYFLT *key, MYFLT *velocity)
{
    int                 c, d, f, m, n, t, v;
    t = (int) *status;
    c = (int) *channel;
    n = (int) *key;
    v = (int) *velocity;
    m = _midimap [c] & 127;        // Keyboard and hold bits
    d = (_midimap [c] >>  8) & 7;  // Division number if (f & 2)
    f = (_midimap [c] >> 12) & 7;  // Control enabled if (f & 4)

    switch (t & 0xF0)
    {
    case 0x80:
    case 0x90:
        // Note on or off.
        if (v && (t & 0x10))
        {
            // Note on.
            if (n < 36)
            {
                if ((f & 4) && (n >= 24) && (n < 34))
                {
                    // Preset selection, sent to model thread
                    // if on control-enabled channel.
                    if (_qmidi->write_avail () >= 3)
                    {
                        _qmidi->write (0, t);
                        _qmidi->write (1, n);
                        _qmidi->write (2, v);
                        _qmidi->write_commit (3);
                    }
                }
            }
            else if (n <= 96)
            {
                if (m) key_on (n - 36, m & _hold);
            }
        }
        else
        {
            // Note off.
            if (n < 36)
            {
            }
            else if (n <= 96)
            {
                if (m) key_off (n - 36, m & KEYS_MASK);
            }
        }
        break;

    case 0xB0: // Controller
        switch (n)
        {
        case MIDICTL_HOLD:
            // Hold pedal.
            if (m & HOLD_MASK)
            {
                if (v > 63)
                {
                    _hold = KEYS_MASK | HOLD_MASK;
                    cond_key_on (m, HOLD_MASK);
                }
                else
                {
                    _hold = KEYS_MASK;
                    cond_key_off (HOLD_MASK, HOLD_MASK);
                }
            }
            break;

        case MIDICTL_ASOFF:
            // All sound off, accepted on control channels only.
            // Clears all keyboards, including held notes.
            if (f & 4) cond_key_off (ALL_MASK, ALL_MASK);
            break;

        case MIDICTL_ANOFF:
            // All notes off, accepted on channels controlling
            // a keyboard. Does not clear held notes.
            if (m) cond_key_off (m, m);
            break;

        case MIDICTL_BANK:
        case MIDICTL_IFELM:
            // Program bank selection or stop control, sent
            // to model thread if on control-enabled channel.
            if (f & 4)
            {
                if (_qmidi->write_avail () >= 3)
                {
                    _qmidi->write (0, t);
                    _qmidi->write (1, n);
                    _qmidi->write (2, v);
                    _qmidi->write_commit (3);
                }
            }
        case MIDICTL_SWELL:
        case MIDICTL_TFREQ:
        case MIDICTL_TMODD:
            // Per-division performance controls, sent to model
            // thread if on a channel that controls a division.
            if (f & 2)
            {
                if (_qmidi->write_avail () >= 3)
                {
                    _qmidi->write (0, t);
                    _qmidi->write (1, n);
                    _qmidi->write (2, v);
                    _qmidi->write_commit (3);
                }
            }
            break;
        }
        break;

    case 0xC0:
        // Program change sent to model thread
        // if on control-enabled channel.
        if (f & 4)
        {
            if (_qmidi->write_avail () >= 3)
            {
                _qmidi->write (0, t);
                _qmidi->write (1, n);
                _qmidi->write (2, 0);
                _qmidi->write_commit (3);
            }
        }
        break;
    }
}






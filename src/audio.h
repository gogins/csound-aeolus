// ----------------------------------------------------------------------------
//
//  Copyright (C) 2018 by Michael Gogins <michael.gogins@gmail.com>
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

#ifndef __AUDIO_H
#define __AUDIO_H


#include <stdlib.h>
#include <clthreads.h>
#include "asection.h"
#include "division.h"
#include "lfqueue.h"
#include "reverb.h"
#include "global.h"
#include <csdl.h>

class Audio : public A_thread
{
public:

    Audio (CSOUND *csound, Lfq_u32 *qnote, Lfq_u32 *qcomm);
    void init_csound(Lfq_u8 *qmidi, bool bform);
    void csound_midi (MYFLT *status, MYFLT *channel, MYFLT *key, MYFLT *velocity);
    int csound_callback (unsigned int &csound_frame_index, 
        unsigned int &csound_frame_count, 
        unsigned int &aeolus_frame_index, 
        unsigned int &aeolus_frame_count, 
        ARRAYDAT *output);

    virtual ~Audio (void);
    void  start (void);

    const char  *appname (void) const { return _appname; }
    uint16_t    *midimap (void) const { return (uint16_t *) _midimap; }
    int  policy (void) const { return _policy; }
    int  abspri (void) const { return _abspri; }
    int  relpri (void) const { return _relpri; }

private:
    CSOUND *csound;

   
    enum { VOLUME, REVSIZE, REVTIME, STPOSIT };

    void init_audio (void);
    virtual void thr_main (void);
    void proc_queue (Lfq_u32 *);
    void proc_synth (int);
    void proc_keys1 (void);
    void proc_keys2 (void);
    void proc_mesg (void);

    void key_off (int n, int b)
    {
        _keymap [n] &= ~b;
        _keymap [n] |= 128;
    }

    void key_on (int n, int b)
    {
        _keymap [n] |= b | 128;
    }

    void cond_key_off (int m, int b)
    {
	int            i;
	unsigned char  *p;

	for (i = 0, p = _keymap; i < NNOTES; i++, p++)
	{
            if (*p & m)
	    {
                *p &= ~b;
		*p |= 128;
	    }
	}
    }

    void cond_key_on (int m, int b)
    {
	int            i;
	unsigned char  *p;

	for (i = 0, p = _keymap; i < NNOTES; i++, p++)
	{
            if (*p & m)
	    {
                *p |= b | 128;
	    }
	}
    }

    const char     *_appname;
    uint16_t        _midimap [16];
    Lfq_u32        *_qnote; 
    Lfq_u32        *_qcomm; 
    Lfq_u8         *_qmidi;
    volatile bool   _running;
    int             _policy;
    int             _abspri;
    int             _relpri;
    int             _hold;
    bool            _bform;
    int             _nplay;
    unsigned int    _fsamp;
    unsigned int    _fsize;
    int             _nasect;
    int             _ndivis;
    Asection       *_asectp [NASECT];
    Division       *_divisp [NDIVIS];
    Reverb          _reverb;
    float          *_outbuf [8];
    unsigned char   _keymap [NNOTES];
    Fparm           _audiopar [4];
    float           _revsize;
    float           _revtime;

};

#endif


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

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include <clthreads.h>
#include <dlfcn.h>
#include "imidi.h"
#include "model.h"
#include "slave.h"
#include "iface.h"
#include "src/tiface.h"
#include "src/audio.h"
#include <csound/OpcodeBase.hpp>

struct csound_aeolus_t
{
    CSOUND *csound;
    char  optline [1024];
    bool  t_opt = false;
    bool  u_opt = false;
    bool  A_opt = false;
    bool  B_opt = false;
    int   r_val = 48000;
    int   p_val = 1024;
    int   n_val = 2;
    const char *N_val = "aeolus";
    const char *S_val = "stops";
    const char *I_val = "Aeolus";
    const char *W_val = "waves";
    const char *d_val = "default";
    const char *s_val = 0;
    Lfq_u32  note_queue  = Lfq_u32(256);
    Lfq_u32  comm_queue = Lfq_u32(256);
    Lfq_u8   midi_queue = Lfq_u8(1024);
    Tiface   *iface;
    ITC_ctrl       itcc;
    Audio         *audio;
    Imidi         *imidi;
    Model         *model;
    Slave         *slave;
    char           s [1024];
    char          *p;
    int            n;
    std::thread    thread;
    csound_aeolus_t()
    {
    }
    virtual ~csound_aeolus_t()
    {
        csound->Message(csound, "csound_aeolus_t::~csound_aeolus_t.\n");
        thread.join();
        delete audio;
        delete imidi;
        delete model;
        delete slave;
        delete iface;
    }
    virtual void run(CSOUND *csound_, const char *stops_directory, const char *instruments_directory, const char *waves_directory, bool bform)
    {
        csound = csound_;
        csound->Message(csound, "csound_aeolus_t::run...\n");
        csound->Message(csound, "csound_aeolus_t::stops_directory:       %s\n", stops_directory);
        csound->Message(csound, "csound_aeolus_t::instruments_directory: %s\n", instruments_directory);
        csound->Message(csound, "csound_aeolus_t::waves_directory:       %s\n", waves_directory);
        csound->Message(csound, "csound_aeolus_t::bform:                 %d\n", bform);
        p = getenv ("HOME");
        S_val = stops_directory;
        I_val = instruments_directory;
        W_val = waves_directory;
        r_val = csound->GetSr(csound);
        p_val = csound->GetKsmps(csound);
        n_val = csound->GetNchnls(csound);
        u_opt = true;
        if (mlockall (MCL_CURRENT | MCL_FUTURE)) csound->Message(csound, "Warning: memory lock failed.\n");
        audio = new Audio (csound, &note_queue, &comm_queue);
        audio->init_csound(&midi_queue, bform);
        model = new Model (&comm_queue, &midi_queue, audio->midimap (), audio->appname (), S_val, I_val, W_val, u_opt);
        imidi = new Imidi (&note_queue, &midi_queue, audio->midimap (), audio->appname ());
        slave = new Slave ();
        iface = new Tiface(0, nullptr);
        ITC_ctrl::connect (audio, EV_EXIT,  &itcc, EV_EXIT);
        ITC_ctrl::connect (audio, EV_QMIDI, model, EV_QMIDI);
        ITC_ctrl::connect (audio, TO_MODEL, model, FM_AUDIO);
        ITC_ctrl::connect (imidi, EV_EXIT,  &itcc, EV_EXIT);
        ITC_ctrl::connect (imidi, EV_QMIDI, model, EV_QMIDI);
        ITC_ctrl::connect (imidi, TO_MODEL, model, FM_IMIDI);
        ITC_ctrl::connect (model, EV_EXIT,  &itcc, EV_EXIT);
        ITC_ctrl::connect (model, TO_AUDIO, audio, FM_MODEL);
        ITC_ctrl::connect (model, TO_SLAVE, slave, FM_MODEL);
        ITC_ctrl::connect (model, TO_IFACE, iface, FM_MODEL);
        ITC_ctrl::connect (slave, EV_EXIT,  &itcc, EV_EXIT);
        ITC_ctrl::connect (slave, TO_AUDIO, audio, FM_SLAVE);
        ITC_ctrl::connect (slave, TO_MODEL, model, FM_SLAVE);
        ITC_ctrl::connect (iface, EV_EXIT,  &itcc, EV_EXIT);
        ITC_ctrl::connect (iface, TO_MODEL, model, FM_IFACE);
        audio->start ();
        if (imidi->thr_start (SCHED_FIFO, audio->relpri () - 20, 0x00010000))
        {
            csound->Message(csound, "Warning: can't run midi thread in RT mode.\n");
            imidi->thr_start (SCHED_OTHER, 0, 0x00010000);
        }
        if (model->thr_start (SCHED_FIFO, audio->relpri () - 30, 0x00010000))
        {
            csound->Message(csound, "Warning: can't run model thread in RT mode.\n");
            model->thr_start (SCHED_OTHER, 0, 0x00010000);
        }
        slave->thr_start (SCHED_OTHER, 0, 0x00010000);
        iface->thr_start (SCHED_OTHER, 0, 0x00020000);
        n = 4;
        csound->Message(csound, "csound_aeolus_t::run now dispatching...\n");
        while (n)
        {
            itcc.get_event (1 << EV_EXIT);
            {
                csound->Message(csound, "aeolus_csound_t::run received EV_EXIT.\n");
                if (n-- == 4)
                {
                    imidi->terminate ();
                    model->terminate ();
                    slave->terminate ();
                    iface->terminate ();
                }
            }
        }
        csound->Message(csound, "csound_aeolus_t::run.\n");
    }
};

static std::map<MYFLT, std::shared_ptr<csound_aeolus_t> > instances_for_ids;

/**
 * i_aeolus aeolus_init S_stops_directory, S_instruments_directory,_S_waves_directory, i_bform, i_wait_seconds`
 */
struct aeolus_init_t : public csound::OpcodeBase<aeolus_init_t>
{
    MYFLT *i_instance_id;
    STRINGDAT *S_stops_directory;
    STRINGDAT *S_instruments_directory;
    STRINGDAT *S_waves_directory;
    MYFLT *i_bform;
    MYFLT *i_wait_seconds;
    std::shared_ptr<csound_aeolus_t> aeolus;
    int init(CSOUND *csound)
    {
        *i_instance_id = instances_for_ids.size();
        aeolus = std::shared_ptr<csound_aeolus_t>(new csound_aeolus_t);
        instances_for_ids[*i_instance_id] = aeolus;
        log(csound, "aeolus_init: instance_id: %f aeolus: %p...\n", *i_instance_id, aeolus.get());
        aeolus->thread = std::thread(&csound_aeolus_t::run, aeolus.get(), 
            csound, 
            S_stops_directory->data, 
            S_instruments_directory->data, 
            S_waves_directory->data, 
            *i_bform);
        int wait_seconds = *i_wait_seconds;
        std::this_thread::sleep_for (std::chrono::seconds(wait_seconds));        
        log(csound, "aeolus_init.\n");
        return 0;
    }
};

/**
 * aeolus_preset i_aeolus, k_bank, k_preset, S_presets_directory
 */
struct aeolus_preset_t : public csound::OpcodeBase<aeolus_preset_t>
{
    MYFLT *i_instance_id;
    MYFLT *k_bank;
    MYFLT *k_preset;
    STRINGDAT *S_presets_directory;
    std::shared_ptr<csound_aeolus_t> aeolus;
    MYFLT k_bank_prior;
    MYFLT k_preset_prior;
    int init(CSOUND *csound)
    {
        int result = 0;
        aeolus = instances_for_ids[*i_instance_id];
        log(csound, "aeolus_preset: instance_id: %f aeolus: %p...\n", *i_instance_id, aeolus.get());
        k_bank_prior = -1;
        k_preset_prior = -1;
        aeolus->d_val = S_presets_directory->data;
        return result;
    }
    int kontrol(CSOUND *csound)
    {
        int result = 0;
        if (*k_bank != k_bank_prior || *k_preset != k_preset_prior) {
            MYFLT controller = 0xB0;
            MYFLT program_change = 0xC0;
            MYFLT channel = 0;
            MYFLT controller_number = 98;
            MYFLT zero = 0;
            log(csound, "aeolus_preset: bank %f  preset %f in %s...\n", k_bank_prior, k_preset_prior, S_presets_directory->data);
            aeolus->audio->csound_midi(&controller, &channel, &controller_number, k_bank);
            aeolus->audio->csound_midi(&program_change, &channel, k_preset, &zero);
            k_bank_prior = *k_bank;
            k_preset_prior = *k_preset;
            log(csound, "aeolus_preset: bank %f  preset %f.\n", k_bank_prior, k_preset_prior);
        }
        return result;
    }
};

/**
 * Send arbitary MIDI channel messages to Aeolus.
 *
 * aeolus_midi i_aeolus, k_status, k_channel, k_data1, k_data2
 */
struct aeolus_midi_t : public csound::OpcodeBase<aeolus_midi_t>
{
    MYFLT *i_instance_id;
    MYFLT *k_status;
    MYFLT *k_channel;
    MYFLT *k_data1;
    MYFLT *k_data2;
    MYFLT k_status_prior = -1;
    MYFLT k_channel_prior = -1;
    MYFLT k_data1_prior = -1;
    MYFLT k_data2_prior = -1;
    std::shared_ptr<csound_aeolus_t> aeolus;
    int init(CSOUND *csound)
    {
        int result = 0;
        aeolus = instances_for_ids[*i_instance_id];
        return result;
    }
    int kontrol(CSOUND *csound)
    {
        int result = 0;
        if (*k_status != k_status_prior ||
            *k_channel != k_channel_prior ||
            *k_data1 != k_data1_prior ||
            *k_data2 != k_data2_prior) {
            aeolus->audio->csound_midi(k_status, k_channel, k_data1, k_data2);
        }
        return result;
    }
};

/**
 * aeolus_group_mode i_aeolus, k_group, k_mode

  v = 01mm0ggg

      This type of messages (bit 6 set) selects a group, and either
      resets all stops in that group or sets the mode for the second
      form below.

      mm = mode. This can be:

         00  disabled, also resets all elements in the group.
         01  set off
         10  set on
         11  toggle

      ggg = group, one of the button groups as defined in the instrument
      definition. In the GUI groups start at the top, the first one (for
      division III) being group #0.

      The values of mm and ggg are stored and need not be repeated unless
      they change.

 */
struct aeolus_group_mode_t : public csound::OpcodeBase<aeolus_group_mode_t>
{
    MYFLT *i_instance_id;
    MYFLT *k_group;
    MYFLT *k_mode;
    MYFLT k_group_prior;
    MYFLT k_mode_prior;
    std::shared_ptr<csound_aeolus_t> aeolus;
    int init(CSOUND *csound)
    {
        int result = 0;
        aeolus = instances_for_ids[*i_instance_id];
        return result;
    }
    int kontrol(CSOUND *csound)
    {
        int result = 0;
        if (*k_group != k_group_prior || *k_mode != k_mode_prior) {
            MYFLT controller = 0xB0;
            MYFLT channel = 0;
            MYFLT controller_number = 98;
            int mode = (int) *k_mode;
            int group = (int) *k_group;
            int value_ = 0b01000000 + (mode << 4) + group;
            MYFLT value = value_;
            aeolus->audio->csound_midi(&controller, &channel, &controller_number, &value);
            k_mode_prior = *k_mode;
            k_group_prior = *k_group;
        }
        return result;
    }
};

/**
 * aeolus_stop i_aeolus, k_stop_button

  v = 000bbbbb

      According to the current state of mode, this command switches a
      stop on or off, or toggles its state, or does nothing at all.

      bbbbb = button index within the group.

      Buttons are numbered left to right, top to bottom within each
      group. The first one is #0.

 */
struct aeolus_stop_t : public csound::OpcodeBase<aeolus_stop_t>
{
    MYFLT *i_instance_id;
    MYFLT *k_stop;
    MYFLT k_stop_prior;
    std::shared_ptr<csound_aeolus_t> aeolus;
    int init(CSOUND *csound)
    {
        int result = 0;
        aeolus = instances_for_ids[*i_instance_id];
        return result;
    }
    int kontrol(CSOUND *csound)
    {
        int result = 0;
        if (*k_stop != k_stop_prior) {
            MYFLT controller = 0xB0;
            MYFLT channel = 0;
            MYFLT controller_number = 98;
            aeolus->audio->csound_midi(&controller, &channel, &controller_number, k_stop);
        }
        return result;
    }
};

/**
 * aeolus_note i_aeolus, i_midi_channel, i_midi_key, i_midi_velocity
 */
struct aeolus_note_t : public csound::OpcodeNoteoffBase<aeolus_note_t>
{
    MYFLT *i_instance_id;
    MYFLT *i_channel;
    MYFLT *i_midi_key;
    MYFLT *i_midi_velocity;
    std::shared_ptr<csound_aeolus_t> aeolus;
    int init(CSOUND *csound)
    {
        int result = 0;
        aeolus = instances_for_ids[*i_instance_id];
        MYFLT note_on = 0x90;
        aeolus->audio->csound_midi(&note_on, i_channel, i_midi_key, i_midi_velocity);
        return result;
    }
    int noteoff(CSOUND *csound)
    {
        int result = 0;
        MYFLT note_off = 0x80;
        aeolus->audio->csound_midi(&note_off, i_channel, i_midi_key, i_midi_velocity);
        return result;
    }
};

/**
 * aeolus_command i_aeolus, i_midi_channel, i_midi_key, i_midi_velocity
 */
struct aeolus_command_t : public csound::OpcodeBase<aeolus_command_t>
{
    MYFLT *i_instance_id;
    STRINGDAT *S_command;
    std::shared_ptr<csound_aeolus_t> aeolus;
    int init(CSOUND *csound)
    {
        int result = 0;
        aeolus = instances_for_ids[*i_instance_id];
        MYFLT note_on = 0x90;
        aeolus->iface->parse_command(S_command->data);
        return result;
    }
};

/**
 * a_out[] aeolus_out i_aeolus
 */
struct aeolus_out_t : public csound::OpcodeNoteoffBase<aeolus_out_t>
{
    ARRAYDAT *v_output;
    MYFLT *i_instance_id;
    std::shared_ptr<csound_aeolus_t> aeolus;
    unsigned int csound_frame_index;
    unsigned int csound_frame_count;
    unsigned int aeolus_frame_index;
    unsigned int aeolus_frame_count;
    int init(CSOUND *csound)
    {
        log(csound, "Began aeolus_out...\n");
        int result = 0;
        aeolus = instances_for_ids[*i_instance_id];
        csound_frame_index = 0;
        csound_frame_count = ksmps();
        aeolus_frame_index = PERIOD;
        aeolus_frame_count = PERIOD;
        return result;
    }
    int kontrol(CSOUND *csound)
    {
        int result = 0;
        aeolus->audio->csound_callback(csound_frame_index, csound_frame_count, aeolus_frame_index, aeolus_frame_count, v_output);
        return result;
    }
    int noteoff(CSOUND *csound)
    {
        int result = 0;
        log(csound, "Ended aeolus_out.\n");
        return result;
    }
};

extern "C"
{
    OENTRY oentries[] =
    {
        // i_aeolus aeolus_init S_stops_directory, S_instruments_directory,_S_waves_directory, i_bform, i_wait_seconds
        {
            (char*)"aeolus_init",
            sizeof(aeolus_init_t),
            0,
            1,
            (char*)"i",
            (char*)"SSSii",
            (SUBR) aeolus_init_t::init_,
            0,
            0,
        },
        // aeolus_preset i_aeolus, k_bank, k_preset, S_presets_directory
        {
            (char*)"aeolus_preset",
            sizeof(aeolus_preset_t),
            0,
            3,
            (char*)"",
            (char*)"ikkS",
            (SUBR) aeolus_preset_t::init_,
            (SUBR) aeolus_preset_t::kontrol_,
            0,
        },
        // aeolus_midi i_aeolus, k_status, k_channel, k_data1 [, k_data2]
        {
            (char*)"aeolus_midi",
            sizeof(aeolus_midi_t),
            0,
            3,
            (char*)"",
            (char*)"ikkkj",
            (SUBR) aeolus_midi_t::init_,
            (SUBR) aeolus_midi_t::kontrol_,
            0,
        },
        // aeolus_group_mode i_aeolus, k_group, k_mode
        {
            (char*)"aeolus_group_mode",
            sizeof(aeolus_group_mode_t),
            0,
            3,
            (char*)"",
            (char*)"ikk",
            (SUBR) aeolus_group_mode_t::init_,
            (SUBR) aeolus_group_mode_t::kontrol_,
            0,
        },
        // aeolus_stop i_aeolus, k_stop_button
        {
            (char*)"aeolus_stop",
            sizeof(aeolus_stop_t),
            0,
            3,
            (char*)"",
            (char*)"ik",
            (SUBR) aeolus_stop_t::init_,
            (SUBR) aeolus_stop_t::kontrol_,
            0,
        },
        // aeolus_command i_aeolus, S_command
        {
            (char*)"aeolus_note",
            sizeof(aeolus_command_t),
            0,
            1,
            (char*)"",
            (char*)"iS",
            (SUBR) aeolus_command_t::init_,
            0,
            0,
        },
        // aeolus_note i_aeolus, i_midi_channel, i_midi_key, i_midi_velocity
        {
            (char*)"aeolus_note",
            sizeof(aeolus_note_t),
            0,
            1,
            (char*)"",
            (char*)"iiii",
            (SUBR) aeolus_note_t::init_,
            0,
            0,
        },
        // a_out[] aeolus_out i_aeolus
        {
            (char*)"aeolus_out",
            sizeof(aeolus_out_t),
            0,
            3,
            (char*)"a[]",
            (char*)"i",
            (SUBR) aeolus_out_t::init_,
            (SUBR) aeolus_out_t::kontrol_,
            0,
        },
        {
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
        }
    };

    PUBLIC int csoundModuleCreate(CSOUND *csound)
    {
        return 0;
    }

    PUBLIC int csoundModuleInit(CSOUND *csound)
    {
        int status = 0;
        for(OENTRY *oentry = &oentries[0]; oentry->opname; oentry++)
        {
            status |= csound->AppendOpcode(csound, oentry->opname,
                                           oentry->dsblksiz, oentry->flags,
                                           oentry->thread,
                                           oentry->outypes, oentry->intypes,
                                           (int (*)(CSOUND*,void*)) oentry->iopadr,
                                           (int (*)(CSOUND*,void*)) oentry->kopadr,
                                           (int (*)(CSOUND*,void*)) oentry->aopadr);
        }
        return status;
    }

    PUBLIC int csoundModuleDestroy(CSOUND *csound)
    {
        csound->Message(csound, "Aeolus: csoundModuleDestroy...\n");
        for (auto it = instances_for_ids.begin(); it != instances_for_ids.end(); ++it) {
            auto aeolus = it->second;
            aeolus->iface->send_event (EV_EXIT, 1);
            aeolus->thread.join();
        }
        instances_for_ids.clear();
        csound->Message(csound, "Aeolus: csoundModuleDestroy.\n");
        return 0;
    }

}

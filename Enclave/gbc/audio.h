#ifndef AUDIO_H
#define AUDIO_H

#include "types.h"
#ifdef __cplusplus
extern "C" {      
#endif            

static const int AUDIO_SAMPLE_RATE = 44100; /* Hz */
static const int AUDIO_CHANNELS = 2;
static const int AUDIO_SNDBUF_SIZE = 1024; /* Per channel */

int audio_init(struct gb_state *s);
void audio_update(struct gb_state *s);
#ifdef __cplusplus
}
#endif            

#endif

#pragma once
#include <stdbool.h>
#include <stddef.h>

extern audio_event_iface_handle_t audio_pipeline_get_event_iface(audio_pipeline_handle_t pipeline);     
typedef void (*tts_callback)(int status); 

bool tts_init(void);
bool tts_play(const char *text, tts_callback cb);
void tts_cleanup(void);
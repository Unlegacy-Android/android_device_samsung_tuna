/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RIL_INTERFACE_H
#define RIL_INTERFACE_H

#define RIL_CLIENT_LIBPATH "libsecril-client.so"

#define RIL_CLIENT_ERR_SUCCESS      0
#define RIL_CLIENT_ERR_AGAIN        1
#define RIL_CLIENT_ERR_INIT         2 // Client is not initialized
#define RIL_CLIENT_ERR_INVAL        3 // Invalid value
#define RIL_CLIENT_ERR_CONNECT      4 // Connection error
#define RIL_CLIENT_ERR_IO           5 // IO error
#define RIL_CLIENT_ERR_RESOURCE     6 // Resource not available
#define RIL_CLIENT_ERR_UNKNOWN      7

enum ril_sound_type {
    SOUND_TYPE_VOICE,
    SOUND_TYPE_SPEAKER,
    SOUND_TYPE_HEADSET,
    SOUND_TYPE_BTVOICE
};

enum ril_audio_path {
    SOUND_AUDIO_PATH_HANDSET,
    SOUND_AUDIO_PATH_HEADSET,
    SOUND_AUDIO_PATH_SPEAKER,
    SOUND_AUDIO_PATH_BLUETOOTH,
    SOUND_AUDIO_PATH_BLUETOOTH_NO_NR,
    SOUND_AUDIO_PATH_HEADPHONE
};

enum ril_clock_state {
    SOUND_CLOCK_STOP,
    SOUND_CLOCK_START
};

/* Function pointers */
void *(*ril_open_client)(void);
int (*ril_close_client)(void *);
int (*ril_connect)(void *);
int (*ril_is_connected)(void *);
int (*ril_disconnect)(void *);
int (*ril_set_call_volume)(void *, enum ril_sound_type, int);
int (*ril_set_call_audio_path)(void *, enum ril_audio_path);
int (*ril_set_call_clock_sync)(void *, enum ril_clock_state);

/* Function prototypes */
int ril_open(void **ril_handle, void **ril_client);
int ril_close(void *ril_handle, void *ril_client);

#endif

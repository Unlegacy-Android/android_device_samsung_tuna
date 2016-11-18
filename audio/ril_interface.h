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

#include "secril-client.h"

#define RIL_OEM_UNSOL_RESPONSE_BASE 11000 // RIL response base index
#define RIL_UNSOL_WB_AMR_STATE \
    (RIL_OEM_UNSOL_RESPONSE_BASE + 17)    // RIL AMR state index

/* Function prototypes */
int ril_open(void *ril_handle);
int ril_close(void *ril_handle);
int ril_set_call_volume(void *ril_handle, enum _SoundType sound_type,
                        float volume);
int ril_set_call_audio_path(void *ril_handle, enum _AudioPath path);
int ril_set_mic_mute(void *ril_handle, enum _MuteCondition state);
void ril_register_set_wb_amr_callback(void *function, void *data);

#endif

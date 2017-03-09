/*
 * Copyright (C) 2016 The CyanogenMod Project <http://www.cyanogenmod.org>
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
#include <hardware/gps.h>

typedef struct {
    /** set to sizeof(GpsCallbacks_Legacy) */
    size_t      size;
    gps_location_callback location_cb;
    gps_status_callback status_cb;
    gps_sv_status_callback sv_status_cb;
    gps_nmea_callback nmea_cb;
    gps_set_capabilities set_capabilities_cb;
    gps_acquire_wakelock acquire_wakelock_cb;
    gps_release_wakelock release_wakelock_cb;
    gps_create_thread create_thread_cb;
    gps_request_utc_time request_utc_time_cb;
#if 0 //Introduced since N, but causes failure in init.
    gnss_set_system_info set_system_info_cb;
    gnss_sv_status_callback gnss_sv_status_cb;
#endif

} GpsCallbacks_Legacy;

/* CellID for 2G, 3G and LTE, used in AGPS. */
typedef struct {
    AGpsRefLocationType type;
    /** Mobile Country Code. */
    uint16_t mcc;
    /** Mobile Network Code .*/
    uint16_t mnc;
    /** Location Area Code in 2G, 3G and LTE. In 3G lac is discarded. In LTE,
     * lac is populated with tac, to ensure that we don't break old clients that
     * might rely in the old (wrong) behavior.
     */
    uint16_t lac;
#ifdef AGPS_USE_PSC
    uint16_t psc;
#endif
    /** Cell id in 2G. Utran Cell id in 3G. Cell Global Id EUTRA in LTE. */
    uint32_t cid;
#if 0 // introduced in N.
    /** Tracking Area Code in LTE. */
    uint16_t tac;
    /** Physical Cell id in LTE (not used in 2G and 3G) */
    uint16_t pcid;
#endif
} AGpsRefLocationCellIDNoLTE;

/** Represents ref locations */
typedef struct {
    AGpsRefLocationType type;
    union {
        AGpsRefLocationCellIDNoLTE	cellID;
		AGpsRefLocationMac			mac;
    } u;
} AGpsRefLocationNoLTE;

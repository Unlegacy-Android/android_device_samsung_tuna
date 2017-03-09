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
#define LOG_TAG "libgps-shim"

#include <utils/Log.h>
#include <hardware/gps.h>

#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gps.h"
#define REAL_GPS_PATH "/vendor/maguro/lib/hw/gps.omap4.so"

const GpsInterface* (*vendor_get_gps_interface)(struct gps_device_t* dev);
const void* (*vendor_get_extension)(const char* name);
int (*vendor_init)(GpsCallbacks* gpsCallbacks);
void (*vendor_set_ref_location)(const AGpsRefLocationNoLTE *agps_reflocation, size_t sz_struct);

void shim_set_ref_location(AGpsRefLocation *agps_reflocation, size_t sz_struct) {
	AGpsRefLocationNoLTE vendor_ref;
	if (sizeof(AGpsRefLocationNoLTE) > sz_struct) {
		ALOGE("%s: AGpsRefLocation is too small, bailing out!", __func__);
		return;
	}
	ALOGD("%s: shimming AGpsRefLocation", __func__);
	vendor_ref.type = agps_reflocation->type;
	vendor_ref.u.cellID.type = agps_reflocation->u.cellID.type;
	vendor_ref.u.cellID.mcc = agps_reflocation->u.cellID.mcc;
	vendor_ref.u.cellID.mnc = agps_reflocation->u.cellID.mnc;
	vendor_ref.u.cellID.lac = agps_reflocation->u.cellID.lac;
	vendor_ref.u.cellID.cid = agps_reflocation->u.cellID.cid;
	vendor_ref.u.mac = agps_reflocation->u.mac;
	ALOGD("%s: Size of AGpsRefLocation              : %d", __func__, sizeof(AGpsRefLocation));
	ALOGD("%s: Size of vendor's AGpsRefLocationNoLTE: %d", __func__, sizeof(AGpsRefLocationNoLTE));

	vendor_set_ref_location(&vendor_ref, sizeof(AGpsRefLocationNoLTE));
	ALOGD("%s: Executed vendor's set_ref_location with following parameters:", __func__);
	ALOGD("%s: type          : %d => %d", __func__, agps_reflocation->type, vendor_ref.type);
	ALOGD("%s: cellID.u.type : %d => %d", __func__, agps_reflocation->u.cellID.type, vendor_ref.u.cellID.type);
	ALOGD("%s: cellID.u.mcc  : %d => %d", __func__, agps_reflocation->u.cellID.mcc, vendor_ref.u.cellID.mcc);
	ALOGD("%s: cellID.u.mnc  : %d => %d", __func__, agps_reflocation->u.cellID.mnc, vendor_ref.u.cellID.mnc);
	ALOGD("%s: cellID.u.cid  : %d => %d", __func__, agps_reflocation->u.cellID.cid, vendor_ref.u.cellID.cid);
	ALOGD("%s: cellID.u.tac  : %d => NOT SUPPORTED", __func__, agps_reflocation->u.cellID.tac);
	ALOGD("%s: cellID.u.pcid : %d => NOT SUPPORTED", __func__, agps_reflocation->u.cellID.pcid);
	ALOGD("%s: u.mac         : %d => %d", __func__, agps_reflocation->u.mac, vendor_ref.u.mac);

	agps_reflocation->type = vendor_ref.type;
	agps_reflocation->u.cellID.type = vendor_ref.u.cellID.type;
	agps_reflocation->u.cellID.mcc = vendor_ref.u.cellID.mcc;
	agps_reflocation->u.cellID.mnc = vendor_ref.u.cellID.mnc;
	agps_reflocation->u.cellID.lac = vendor_ref.u.cellID.lac;
	agps_reflocation->u.cellID.cid = vendor_ref.u.cellID.cid;
	agps_reflocation->u.mac = vendor_ref.u.mac;
}

const void* shim_get_extension(const char* name) {
	ALOGD("%s(%s)", __func__, name);
	if (strcmp(name, AGPS_RIL_INTERFACE) == 0) {
		// RIL interface
		AGpsRilInterface *ril = (AGpsRilInterface*)vendor_get_extension(name);
		// now we shim the ref_location callback
		ALOGD("%s: shimming RIL ref_location callback", __func__);
		vendor_set_ref_location = ril->set_ref_location;
		ril->set_ref_location = shim_set_ref_location;
		return ril;
	} else {
		return vendor_get_extension(name);
	}
}

int shim_init (GpsCallbacks* gpsCallbacks) {
	ALOGD("%s: shimming GpsCallbacks", __func__);
	GpsCallbacks_Legacy vendor_gpsCallbacks;
	vendor_gpsCallbacks.size = sizeof(GpsCallbacks_Legacy);
	vendor_gpsCallbacks.location_cb = gpsCallbacks->location_cb;
	vendor_gpsCallbacks.status_cb = gpsCallbacks->status_cb;
	vendor_gpsCallbacks.sv_status_cb = gpsCallbacks->sv_status_cb;
	vendor_gpsCallbacks.nmea_cb = gpsCallbacks->nmea_cb;
	vendor_gpsCallbacks.set_capabilities_cb = gpsCallbacks->set_capabilities_cb;
	vendor_gpsCallbacks.acquire_wakelock_cb = gpsCallbacks->acquire_wakelock_cb;
	vendor_gpsCallbacks.release_wakelock_cb = gpsCallbacks->release_wakelock_cb;
	vendor_gpsCallbacks.create_thread_cb = gpsCallbacks->create_thread_cb;
	vendor_gpsCallbacks.request_utc_time_cb = gpsCallbacks->request_utc_time_cb;

	ALOGD("%s: Calling vendor init", __func__);
	return vendor_init(&vendor_gpsCallbacks);
}

const GpsInterface* shim_get_gps_interface(struct gps_device_t* dev) {
	GpsInterface *halInterface = vendor_get_gps_interface(dev);

	ALOGD("%s: shimming vendor get_extension", __func__);
	vendor_get_extension = halInterface->get_extension;
	halInterface->get_extension = &shim_get_extension;

	ALOGD("%s: shimming vendor init", __func__);
	vendor_init = halInterface->init;
	halInterface->init = &shim_init;

	return halInterface;
}

static int open_gps(const struct hw_module_t* module, char const* name,
		struct hw_device_t** device) {
	void *realGpsLib;
	int gpsHalResult;
	struct hw_module_t *realHalSym;

	struct gps_device_t **gps = (struct gps_device_t **)device;

	realGpsLib = dlopen(REAL_GPS_PATH, RTLD_LOCAL);
	if (!realGpsLib) {
		ALOGE("Failed to load real GPS HAL '" REAL_GPS_PATH "': %s", dlerror());
		return -EINVAL;
	}

	realHalSym = (struct hw_module_t*)dlsym(realGpsLib, HAL_MODULE_INFO_SYM_AS_STR);
	if (!realHalSym) {
		ALOGE("Failed to locate the GPS HAL module sym: '" HAL_MODULE_INFO_SYM_AS_STR "': %s", dlerror());
		goto dl_err;
	}

	int result = realHalSym->methods->open(realHalSym, name, device);
	if (result < 0) {
		ALOGE("Real GPS open method failed: %d", result);
		goto dl_err;
	}
	ALOGD("Successfully loaded real GPS hal, shimming get_gps_interface...");
	// now, we shim hw_device_t
	vendor_get_gps_interface = (*gps)->get_gps_interface;
	(*gps)->get_gps_interface = &shim_get_gps_interface;

	return 0;
dl_err:
	dlclose(realGpsLib);
	return -EINVAL;
}

static struct hw_module_methods_t gps_module_methods = {
	.open = open_gps
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
	.tag = HARDWARE_MODULE_TAG,
	.module_api_version = 1,
	.hal_api_version = 0,
	.id = GPS_HARDWARE_MODULE_ID,
	.name = "GSD4t GPS shim",
	.author = "The CyanogenMod Project",
	.methods = &gps_module_methods
};

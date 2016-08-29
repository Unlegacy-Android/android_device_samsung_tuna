#include "secril-shim.h"

/* A copy of the original RIL function table. */
static const RIL_RadioFunctions *origRilFunctions;

/* A copy of the ril environment passed to RIL_Init. */
static const struct RIL_Env *rilEnv;

/* The tuna variant we're running on. */
static int tunaVariant = VARIANT_INIT;


static void onRequestShim(int request, void *data, size_t datalen, RIL_Token t)
{
	switch (request) {
		/* Necessary; RILJ may fake this for us if we reply not supported, but we can just implement it. */
		case RIL_REQUEST_GET_RADIO_CAPABILITY:
			; /* lol C standard */
			int raf = RAF_UNKNOWN;
			if (tunaVariant == VARIANT_MAGURO) {
				raf = RAF_GSM | RAF_GPRS | RAF_EDGE | RAF_HSUPA | RAF_HSDPA | RAF_HSPA | RAF_HSPAP | RAF_UMTS;
			} else if (tunaVariant == VARIANT_TORO || tunaVariant == VARIANT_TOROPLUS) {
				raf = RAF_LTE | RAF_IS95A | RAF_IS95B | RAF_1xRTT | RAF_EVDO_0 | RAF_EVDO_A | RAF_EVDO_B | RAF_EHRPD;
			}
			if (CC_LIKELY(raf != RAF_UNKNOWN)) {
				RIL_RadioCapability rc[1] =
				{
					{ /* rc[0] */
						RIL_RADIO_CAPABILITY_VERSION, /* version */
						0, /* session */
						RC_PHASE_CONFIGURED, /* phase */
						raf, /* rat */
						{ /* logicalModemUuid */
							0,
						},
						RC_STATUS_SUCCESS /* status */
					}
				};
				RLOGW("%s: got request %s: replied with our implementation!\n", __func__, requestToString(request));
				rilEnv->OnRequestComplete(t, RIL_E_SUCCESS, rc, sizeof(rc));
				return;
			}
			/* else fallthrough to RIL_E_REQUEST_NOT_SUPPORTED */

		/* The following requests were introduced post-4.3. */
		case RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC:
		case RIL_REQUEST_SIM_OPEN_CHANNEL: /* !!! */
		case RIL_REQUEST_SIM_CLOSE_CHANNEL:
		case RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL:
		case RIL_REQUEST_NV_READ_ITEM:
		case RIL_REQUEST_NV_WRITE_ITEM:
		case RIL_REQUEST_NV_WRITE_CDMA_PRL:
		case RIL_REQUEST_NV_RESET_CONFIG:
		case RIL_REQUEST_SET_UICC_SUBSCRIPTION:
		case RIL_REQUEST_ALLOW_DATA:
		case RIL_REQUEST_GET_HARDWARE_CONFIG:
		case RIL_REQUEST_SIM_AUTHENTICATION:
		case RIL_REQUEST_GET_DC_RT_INFO:
		case RIL_REQUEST_SET_DC_RT_INFO_RATE:
		case RIL_REQUEST_SET_DATA_PROFILE:
		case RIL_REQUEST_SHUTDOWN: /* TODO: Is there something we can do for RIL_REQUEST_SHUTDOWN ? */
		case RIL_REQUEST_SET_RADIO_CAPABILITY:
		case RIL_REQUEST_START_LCE:
		case RIL_REQUEST_STOP_LCE:
		case RIL_REQUEST_PULL_LCEDATA:
			RLOGW("%s: got request %s: replied with REQUEST_NOT_SUPPPORTED.\n", __func__, requestToString(request));
			rilEnv->OnRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
			return;
	}

	RLOGD("%s: got request %s: forwarded to RIL.\n", __func__, requestToString(request));
	origRilFunctions->onRequest(request, data, datalen, t);
}

static void patchMem(void *libHandle, bool beforeRilInit)
{
	/* hSecOem is a nice symbol to use, it's in all 3 RILs and gives us easy
	 * access to the memory region we're generally most interested in. */
	uint8_t *hSecOem;

	/* MAX_TIMEOUT is used for a call to pthread_cond_timedwait_relative_np.
	 * The issue is bionic has switched to using absolute timeouts instead of
	 * relative timeouts, and a maximum time value can cause an overflow in
	 * the function converting relative to absolute timespecs if unpatched.
	 *
	 * By patching this to 0x01FFFFFF from 0x7FFFFFFF, the timeout should
	 * expire in about a year rather than 68 years, and the RIL should be good
	 * up until the year 2036 or so. */
	uint8_t *MAX_TIMEOUT;

	hSecOem = dlsym(libHandle, "hSecOem");
	if (CC_UNLIKELY(!hSecOem)) {
		RLOGE("%s: hSecOem could not be found!\n", __func__);
		/* If hSecOem is not found we can still try the other patches. */
		goto timeout_patch;
	}

	RLOGD("%s: hSecOem found at %p!\n", __func__, hSecOem);

	/* hSecOem-based patching. */
	switch (tunaVariant) {
		case VARIANT_MAGURO:
			if (!beforeRilInit) {
				/* 'ril features' is (only) used to enable/disable an extension
				 * to LAST_CALL_FAIL_CAUSE. Android had just been happily
				 * ignoring the extra data being sent, until it did introduce a
				 * vendor extension for LAST_CALL_FAIL_CAUSE in Android 6.0;
				 * of course it doesn't like this RIL's extra data now (crashes),
				 * so we need to disable it. rilFeatures is initialized in
				 * RIL_Init, so defer it until afterwards. */
				uint8_t *rilFeatures = hSecOem + 0x1918;

				RLOGD("%s: rilFeatures is currently %" PRIu8 "\n", __func__, *rilFeatures);
				if (CC_LIKELY(*rilFeatures == 1)) {
					*rilFeatures = 0;
					RLOGI("%s: rilFeatures was changed to %" PRIu8 "\n", __func__, *rilFeatures);
				} else {
					RLOGD("%s: rilFeatures was not 1; leaving alone\n", __func__);
				}
			}
			break;
		case VARIANT_TORO:
			break;
		case VARIANT_TOROPLUS:
			break;
	}

timeout_patch:
	/* MAX_TIMEOUT patch, works the same for all RILs. */
	if (beforeRilInit) {
		MAX_TIMEOUT = dlsym(libHandle, "MAX_TIMEOUT");
		if (CC_UNLIKELY(!MAX_TIMEOUT)) {
			RLOGE("%s: MAX_TIMEOUT could not be found!\n", __func__);
			return;
		}
		RLOGD("%s: MAX_TIMEOUT found at %p!\n", __func__, MAX_TIMEOUT);
		/* We need to patch the first byte, since we're little endian
		 * we need to move forward 3 bytes to get that byte. */
		MAX_TIMEOUT += 3;
		RLOGD("%s: MAX_TIMEOUT is currently 0x%" PRIX8 "FFFFFF\n", __func__, *MAX_TIMEOUT);
		if (CC_LIKELY(*MAX_TIMEOUT == 0x7F)) {
			*MAX_TIMEOUT = 0x01;
			RLOGI("%s: MAX_TIMEOUT was changed to 0x%" PRIX8 "FFFFFF\n", __func__, *MAX_TIMEOUT);
		} else {
			RLOGW("%s: MAX_TIMEOUT was not 0x7F; leaving alone\n", __func__);
		}
	}
}

const RIL_RadioFunctions* RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
	RIL_RadioFunctions const* (*origRilInit)(const struct RIL_Env *env, int argc, char **argv);
	static RIL_RadioFunctions shimmedFunctions;
	void *origRil;
	char propBuf[PROPERTY_VALUE_MAX];

	if (CC_LIKELY(tunaVariant == VARIANT_INIT)) {
		property_get("ro.product.subdevice", propBuf, "unknown");
		if (!strcmp(propBuf, "maguro")) {
			tunaVariant = VARIANT_MAGURO;
		} else if (!strcmp(propBuf, "toro")) {
			tunaVariant = VARIANT_TORO;
		} else if (!strcmp(propBuf, "toroplus")) {
			tunaVariant = VARIANT_TOROPLUS;
		} else {
			tunaVariant = VARIANT_UNKNOWN;
		}
		RLOGD("%s: got tuna variant: %i", __func__, tunaVariant);
	}

	rilEnv = env;

	/* Open and Init the original RIL. */

	origRil = dlopen(RIL_LIB_PATH, RTLD_LOCAL);
	if (CC_UNLIKELY(!origRil)) {
		RLOGE("%s: failed to load '" RIL_LIB_PATH  "': %s\n", __func__, dlerror());
		return NULL;
	}

	origRilInit = dlsym(origRil, "RIL_Init");
	if (CC_UNLIKELY(!origRilInit)) {
		RLOGE("%s: couldn't find original RIL_Init!\n", __func__);
		goto fail_after_dlopen;
	}

	/* Fix RIL issues by patching memory: pre-init pass. */
	patchMem(origRil, true);

	origRilFunctions = origRilInit(env, argc, argv);
	if (CC_UNLIKELY(!origRilFunctions)) {
		RLOGE("%s: the original RIL_Init derped.\n", __func__);
		goto fail_after_dlopen;
	}

	/* Fix RIL issues by patching memory: post-init pass. */
	patchMem(origRil, false);

	/* Shim functions as needed. */
	shimmedFunctions = *origRilFunctions;
	shimmedFunctions.onRequest = onRequestShim;

	return &shimmedFunctions;

fail_after_dlopen:
	dlclose(origRil);
	return NULL;
}

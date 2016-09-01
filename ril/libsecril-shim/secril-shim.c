#include "secril-shim.h"

/* A copy of the original RIL function table. */
static const RIL_RadioFunctions *origRilFunctions;

/* A copy of the ril environment passed to RIL_Init. */
static const struct RIL_Env *rilEnv;

/* The tuna variant we're running on. */
static int tunaVariant = VARIANT_INIT;

#if SHIM_UPGRADE_VERSION >= 7
/* For older RILs that do not support new commands RIL_REQUEST_VOICE_RADIO_TECH and
   RIL_UNSOL_VOICE_RADIO_TECH_CHANGED messages, decode the voice radio tech from
   radio state message and store it. Every time there is a change in Radio State
   check to see if voice radio tech changes and notify telephony
 */
static int voiceRadioTech = -1;

/* For older RILs that do not support new commands RIL_REQUEST_GET_CDMA_SUBSCRIPTION_SOURCE
   and RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED messages, decode the subscription
   source from radio state and store it. Every time there is a change in Radio State
   check to see if subscription source changed and notify telephony
 */
static int cdmaSubscriptionSource = -1;

/* For older RILs that do not send RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, decode the
   SIM/RUIM state from radio state and store it. Every time there is a change in Radio State,
   check to see if SIM/RUIM status changed and notify telephony
 */
static int simRuimStatus = -1;
#endif


/* helper functions */
#if SHIM_UPGRADE_VERSION >= 7
static int decodeVoiceRadioTechnology(RIL_RadioState radioState)
{
	switch (radioState) {
		case RADIO_STATE_SIM_NOT_READY:
		case RADIO_STATE_SIM_LOCKED_OR_ABSENT:
		case RADIO_STATE_SIM_READY:
			/* HAX: See GsmCdmaPhone.java --> phoneObjectUpdater(int newVoiceRadioTech)
			 * For v6 RILs with LTE_ON_CDMA mode, they ignore reported voice radio tech
			 * and assume 1xRTT. For toro(plus), we must do the same when upgrading RIL version. */
			if (tunaVariant == VARIANT_TORO || tunaVariant == VARIANT_TOROPLUS) {
				return RADIO_TECH_1xRTT;
			}
			return RADIO_TECH_UMTS;

		case RADIO_STATE_RUIM_NOT_READY:
		case RADIO_STATE_RUIM_READY:
		case RADIO_STATE_RUIM_LOCKED_OR_ABSENT:
		case RADIO_STATE_NV_NOT_READY:
		case RADIO_STATE_NV_READY:
			return RADIO_TECH_1xRTT;

		default:
			RLOGE("%s: invoked with incorrect RadioState!", __func__);
			return -1;
	}
}

static int decodeCdmaSubscriptionSource(RIL_RadioState radioState)
{
	switch (radioState) {
		case RADIO_STATE_SIM_NOT_READY:
		case RADIO_STATE_SIM_LOCKED_OR_ABSENT:
		case RADIO_STATE_SIM_READY:
		case RADIO_STATE_RUIM_NOT_READY:
		case RADIO_STATE_RUIM_READY:
		case RADIO_STATE_RUIM_LOCKED_OR_ABSENT:
			return CDMA_SUBSCRIPTION_SOURCE_RUIM_SIM;

		case RADIO_STATE_NV_NOT_READY:
		case RADIO_STATE_NV_READY:
			return CDMA_SUBSCRIPTION_SOURCE_NV;

		default:
			RLOGE("%s: invoked with incorrect RadioState!", __func__);
			return -1;
    }
}

static int decodeSimStatus(RIL_RadioState radioState)
{
	switch (radioState) {
		case RADIO_STATE_SIM_NOT_READY:
		case RADIO_STATE_RUIM_NOT_READY:
		case RADIO_STATE_NV_NOT_READY:
		case RADIO_STATE_NV_READY:
			return -1;

		case RADIO_STATE_SIM_LOCKED_OR_ABSENT:
		case RADIO_STATE_SIM_READY:
		case RADIO_STATE_RUIM_READY:
		case RADIO_STATE_RUIM_LOCKED_OR_ABSENT:
			return radioState;

		default:
			RLOGE("%s: invoked with incorrect RadioState!", __func__);
			return -1;
	}
}

static bool is3gpp2(int radioTech)
{
	switch (radioTech) {
		case RADIO_TECH_IS95A:
		case RADIO_TECH_IS95B:
		case RADIO_TECH_1xRTT:
		case RADIO_TECH_EVDO_0:
		case RADIO_TECH_EVDO_A:
		case RADIO_TECH_EVDO_B:
		case RADIO_TECH_EHRPD:
			return true;

		default:
			return false;
	}
}
#endif

static void onRequestShim(int request, void *data, size_t datalen, RIL_Token t)
{
	switch (request) {
#if SHIM_UPGRADE_VERSION >= 7
		case RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE:
			RLOGI("%s: got request %s: replying with our implementation!", __func__, requestToString(request));
			RIL_RadioState cdmaSubSourceState = origRilFunctions->onStateRequest();

			if ((RADIO_STATE_UNAVAILABLE == cdmaSubSourceState) || (RADIO_STATE_OFF == cdmaSubSourceState)) {
				rilEnv->OnRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
			}

			cdmaSubscriptionSource = decodeCdmaSubscriptionSource(cdmaSubSourceState);

			if (cdmaSubscriptionSource < 0) {
				rilEnv->OnRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
			} else {
				rilEnv->OnRequestComplete(t, RIL_E_SUCCESS, &cdmaSubscriptionSource, sizeof(int));
			}
			return;
		case RIL_REQUEST_VOICE_RADIO_TECH:
			RLOGI("%s: got request %s: replying with our implementation!", __func__, requestToString(request));
			RIL_RadioState voiceRadioTechState = origRilFunctions->onStateRequest();

			if ((RADIO_STATE_UNAVAILABLE == voiceRadioTechState) || (RADIO_STATE_OFF == voiceRadioTechState)) {
				rilEnv->OnRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
			}

			voiceRadioTech = decodeVoiceRadioTechnology(voiceRadioTechState);

			if (voiceRadioTech < 0) {
				rilEnv->OnRequestComplete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
			} else {
				rilEnv->OnRequestComplete(t, RIL_E_SUCCESS, &voiceRadioTech, sizeof(int));
			}
			return;
#endif
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
				RLOGI("%s: got request %s: replied with our implementation!", __func__, requestToString(request));
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
			RLOGW("%s: got request %s: replied with REQUEST_NOT_SUPPPORTED.", __func__, requestToString(request));
			rilEnv->OnRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
			return;
	}

	RLOGD("%s: got request %s: forwarded to RIL.", __func__, requestToString(request));
	origRilFunctions->onRequest(request, data, datalen, t);
}

#if SHIM_UPGRADE_VERSION >= 7
static RIL_RadioState onStateRequestShim()
{
	RIL_RadioState state = origRilFunctions->onStateRequest();
	if (state > RADIO_STATE_UNAVAILABLE) {
		state = RADIO_STATE_ON;
	}
	return state;
}
#endif

static void onRequestCompleteShim(RIL_Token t, RIL_Errno e, void *response, size_t responselen)
{
	int request;
	RequestInfo *pRI;

	pRI = (RequestInfo *)t;

	/* If pRI is null, this entire function is useless. */
	if (pRI == NULL) {
		goto null_token_exit;
	}

	request = pRI->pCI->requestNumber;

	switch (request) {
		case RIL_REQUEST_GET_SIM_STATUS:
			/* Android 7.0 mishandles RIL_CardStatus_v5.
			 * We can just fake a v6 response instead. */
			if (responselen == sizeof(RIL_CardStatus_v5)) {
				RLOGI("%s: got request %s: upgrading response.", __func__, requestToString(request));

				RIL_CardStatus_v5 *v5response = ((RIL_CardStatus_v5 *) response);

				RIL_CardStatus_v6 *v6response = malloc(sizeof(RIL_CardStatus_v6));

				v6response->card_state = v5response->card_state;
				v6response->universal_pin_state = v5response->universal_pin_state;
				v6response->gsm_umts_subscription_app_index = v5response->gsm_umts_subscription_app_index;
				v6response->cdma_subscription_app_index = v5response->cdma_subscription_app_index;
				v6response->ims_subscription_app_index = -1;
				v6response->num_applications = v5response->num_applications;
				memcpy(v6response->applications, v5response->applications, sizeof(RIL_AppStatus) * 8);

				rilEnv->OnRequestComplete(t, e, v6response, sizeof(RIL_CardStatus_v6));

				free(v6response);
				return;
			}
			/* If this was already a v6 reply, continue as usual. */
			break;
	}

	RLOGD("%s: got request %s: forwarded to libril.", __func__, requestToString(request));
null_token_exit:
	rilEnv->OnRequestComplete(t, e, response, responselen);
}

static void onUnsolicitedResponseShim(int unsolResponse, const void *data, size_t datalen)
{
	switch (unsolResponse) {
#if SHIM_UPGRADE_VERSION >= 7
		case RIL_UNSOL_SIM_REFRESH:
			/* Upgrade SIM_REFRESH to a RIL_SimRefreshResponse_v7 */
			RLOGI("%s: upgrading SIM_REFRESH to RIL_SimRefreshResponse_v7.", __func__);
			int *v6_sim_refresh = ((int *) data);

			RIL_SimRefreshResponse_v7 *v7_sim_refresh = malloc(sizeof(RIL_SimRefreshResponse_v7));

			v7_sim_refresh->result = v6_sim_refresh[0];
			v7_sim_refresh->ef_id  = v6_sim_refresh[1];
			v7_sim_refresh->aid    = NULL;

			rilEnv->OnUnsolicitedResponse(unsolResponse, v7_sim_refresh, sizeof(RIL_SimRefreshResponse_v7));

			free(v7_sim_refresh);
			return;
		case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED:
			/* Uses old radio states to notify changes for voice radio tech, cdma sub src, and sim status. */
			RLOGI("%s: upgrading RESPONSE_RADIO_STATE_CHANGED.", __func__);
			RIL_RadioState newRadioState = origRilFunctions->onStateRequest();
			if ((newRadioState > RADIO_STATE_UNAVAILABLE) && (newRadioState < RADIO_STATE_ON)) {
				int newVoiceRadioTech;
				int newCdmaSubscriptionSource;
				int newSimStatus;

				/* This is old RIL. Decode Subscription source and Voice Radio Technology
				 * from Radio State and send change notifications if there has been a change */
				newVoiceRadioTech = decodeVoiceRadioTechnology(newRadioState);
				if (newVoiceRadioTech != voiceRadioTech) {
					voiceRadioTech = newVoiceRadioTech;
					rilEnv->OnUnsolicitedResponse(RIL_UNSOL_VOICE_RADIO_TECH_CHANGED,
					                              &voiceRadioTech, sizeof(voiceRadioTech));
				}

				if (is3gpp2(newVoiceRadioTech)) {
					newCdmaSubscriptionSource = decodeCdmaSubscriptionSource(newRadioState);
					if (newCdmaSubscriptionSource != cdmaSubscriptionSource) {
						cdmaSubscriptionSource = newCdmaSubscriptionSource;
						rilEnv->OnUnsolicitedResponse(RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED,
						                              &cdmaSubscriptionSource, sizeof(cdmaSubscriptionSource));
					}
				}

				newSimStatus = decodeSimStatus(newRadioState);
				if (newSimStatus != simRuimStatus) {
					simRuimStatus = newSimStatus;
					rilEnv->OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
				}
			}
			/* Still need to forward RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED to libril */
			rilEnv->OnUnsolicitedResponse(unsolResponse, data, datalen);
			return;
#endif
	}

	rilEnv->OnUnsolicitedResponse(unsolResponse, data, datalen);
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
		RLOGE("%s: hSecOem could not be found!", __func__);
		/* If hSecOem is not found we can still try the other patches. */
		goto timeout_patch;
	}

	RLOGD("%s: hSecOem found at %p!", __func__, hSecOem);

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

				RLOGD("%s: rilFeatures is currently %" PRIu8 "", __func__, *rilFeatures);
				if (CC_LIKELY(*rilFeatures == 1)) {
					*rilFeatures = 0;
					RLOGI("%s: rilFeatures was changed to %" PRIu8 "", __func__, *rilFeatures);
				} else {
					RLOGW("%s: rilFeatures was not 1; leaving alone", __func__);
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
			RLOGE("%s: MAX_TIMEOUT could not be found!", __func__);
			return;
		}
		RLOGD("%s: MAX_TIMEOUT found at %p!", __func__, MAX_TIMEOUT);
		/* We need to patch the first byte, since we're little endian
		 * we need to move forward 3 bytes to get that byte. */
		MAX_TIMEOUT += 3;
		RLOGD("%s: MAX_TIMEOUT is currently 0x%" PRIX8 "FFFFFF", __func__, *MAX_TIMEOUT);
		if (CC_LIKELY(*MAX_TIMEOUT == 0x7F)) {
			*MAX_TIMEOUT = 0x01;
			RLOGI("%s: MAX_TIMEOUT was changed to 0x%" PRIX8 "FFFFFF", __func__, *MAX_TIMEOUT);
		} else {
			RLOGW("%s: MAX_TIMEOUT was not 0x7F; leaving alone", __func__);
		}
	}
}

const RIL_RadioFunctions* RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
	RIL_RadioFunctions const* (*origRilInit)(const struct RIL_Env *env, int argc, char **argv);
	static RIL_RadioFunctions shimmedFunctions;
	static struct RIL_Env shimmedEnv;
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

	/* Shim the RIL_Env passed to the real RIL, saving a copy of the original */
	rilEnv = env;
	shimmedEnv = *env;
	shimmedEnv.OnRequestComplete = onRequestCompleteShim;
	shimmedEnv.OnUnsolicitedResponse = onUnsolicitedResponseShim;

	/* Open and Init the original RIL. */

	origRil = dlopen(RIL_LIB_PATH, RTLD_LOCAL);
	if (CC_UNLIKELY(!origRil)) {
		RLOGE("%s: failed to load '" RIL_LIB_PATH  "': %s", __func__, dlerror());
		return NULL;
	}

	origRilInit = dlsym(origRil, "RIL_Init");
	if (CC_UNLIKELY(!origRilInit)) {
		RLOGE("%s: couldn't find original RIL_Init!", __func__);
		goto fail_after_dlopen;
	}

	/* Fix RIL issues by patching memory: pre-init pass. */
	patchMem(origRil, true);

	origRilFunctions = origRilInit(&shimmedEnv, argc, argv);
	if (CC_UNLIKELY(!origRilFunctions)) {
		RLOGE("%s: the original RIL_Init derped.", __func__);
		goto fail_after_dlopen;
	}

	/* Fix RIL issues by patching memory: post-init pass. */
	patchMem(origRil, false);

	/* Shim functions as needed. */
	shimmedFunctions = *origRilFunctions;
	shimmedFunctions.onRequest = onRequestShim;
#if SHIM_UPGRADE_VERSION >= 7
	shimmedFunctions.version = SHIM_UPGRADE_VERSION;
	shimmedFunctions.onStateRequest = onStateRequestShim;
#endif

	return &shimmedFunctions;

fail_after_dlopen:
	dlclose(origRil);
	return NULL;
}

#ifndef __SECRIL_SHIM_H__
#define __SECRIL_SHIM_H__

#define LOG_TAG "secril-shim"
#define RIL_SHLIB

#include <dlfcn.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cutils/compiler.h>
#include <cutils/properties.h>
#include <sys/cdefs.h>
#include <telephony/ril.h>
#include <utils/Log.h>

#define RIL_LIB_PATH "/vendor/lib/libsec-ril.so"

enum variant_type {
	VARIANT_INIT,
	VARIANT_MAGURO,
	VARIANT_TORO,
	VARIANT_TOROPLUS,
	VARIANT_UNKNOWN
};

extern "C" const char * requestToString(int request);

/* TODO: Do we really need to redefine these? They aren't in a header... */
typedef struct {
    int requestNumber;
    void (*dispatchFunction) (void *p, void *pRI);
    int(*responseFunction) (void *p, void *response, size_t responselen);
} CommandInfo;

typedef struct RequestInfo {
    int32_t token;
    CommandInfo *pCI;
    struct RequestInfo *p_next;
    char cancelled;
    char local;
    RIL_SOCKET_ID socket_id;
    int wasAckSent;
} RequestInfo;

#endif /* __SECRIL_SHIM_H__ */

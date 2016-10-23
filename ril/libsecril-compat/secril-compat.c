#define LOG_TAG "secril-compat"
#include <pwd.h>
#include <string.h>
#include <sys/types.h>
#include <utils/Log.h>


/**
 * With the switch to C++11 by default, char16_t became a unique type,
 * rather than basically just a typedef of uint16_t. As a result, the
 * compiler now mangles the symbol for writeString16 differently. Our
 * RIL references the old symbol of course, not the new one.
 */
uintptr_t _ZN7android6Parcel13writeString16EPKDsj(void *instance, void *str, size_t len);
uintptr_t _ZN7android6Parcel13writeString16EPKtj(void *instance, void *str, size_t len)
{
	return _ZN7android6Parcel13writeString16EPKDsj(instance, str, len);
}


/**
 * toroplus's RIL has the ability to take a screenshot. WTF?
 * Supposedly some vendor/testing/factory crap.
 * There's no way in hell that codepath should be hit under
 * any legitimate circumstances. Just define the symbol.
 */
void _ZN7android16ScreenshotClient6updateEv()
{
	ALOGE("%s: CALLED! SHOULD NOT HAPPEN!!", __func__);
}


/**
 * With the media(server) hardening in N, the RIL does not accept
 * sockets from the audio HAL. The HAL runs under UID audioserver,
 * not UID media. This is worked around by changing the getpwuid
 * symbol to this one, cmpt__id, to change audioserver to media.
 */
struct passwd* cmpt__id(uid_t uid)
{
	struct passwd* ret = getpwuid(uid);
	if (!strcmp(ret->pw_name, "audioserver")) {
		strcpy(ret->pw_name, "media");
	}
	return ret;
}

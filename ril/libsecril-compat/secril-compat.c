/**
 * With the switch to C++11 by default, char16_t became a unique type,
 * rather than basically just a typedef of uint16_t. As a result, the
 * compiler now mangles the symbol for writeString16 differently. Our
 * RIL references the old symbol of course, not the new one.
 * Interestingly, nothing actually calls the function that references this!
 * So, just define the symbol; anything more is unnecessary.
 */
void _ZN7android6Parcel13writeString16EPKtj() { }

/**
 * toroplus's RIL has the ability to take a screenshot. WTF?
 * Supposedly some vendor/testing/factory crap.
 * There's no way in hell that codepath should be hit under
 * any legitimate circumstances. Just define the symbol.
 */
void _ZN7android16ScreenshotClient6updateEv() { }

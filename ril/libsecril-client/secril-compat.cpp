/**
 * @file    secril-client.cpp
 *
 * @brief   RIL compatibility for toroplus
 */

#include <utils/Log.h>


    /**
     * toroplus's RIL has the ability to take a screenshot. It's
     * supposedly some vendor/testing/factory stuff. This codepath
     * should never be hit under any legitimate circumstances.
     * Define the symbol, and log any calls to it.
     */
    extern "C"
    void _ZN7android16ScreenshotClient6updateEv() {
        ALOGE("%s: CALLED! SHOULD NOT HAPPEN!!", __func__);
    }

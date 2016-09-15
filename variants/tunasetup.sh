#!/system/bin/sh

# Only run script if we haven't run it before
if [ -e "/system/vendor/build.prop" ] ;then
    exit
fi

# Delete files for a variant due to it being unused.
delete_maguro_files() {
    rm -r /system/vendor/maguro/
    rm /system/etc/wifi/bcmdhd.maguro.cal
    rm /system/etc/permissions/android.hardware.telephony.gsm.xml
}
delete_torocommon_files() {
    rm -r /system/vendor/toro-common/
    rm /system/etc/permissions/android.hardware.telephony.cdma.xml

    # If we're removing the common toro files, we obviously want ALL toro files removed.
    delete_toro_files
    delete_toroplus_files
}
delete_toro_files() {
    rm -r /system/vendor/toro/
    rm /system/etc/wifi/bcmdhd.toro.cal
}
delete_toroplus_files() {
    rm -r /system/vendor/toroplus/
    rm /system/etc/wifi/bcmdhd.toroplus.cal
}

# Move files for variants to their proper locations.
move_maguro_files() {
    mv /system/vendor/maguro/build.prop /system/vendor/build.prop

    mkdir -p /system/vendor/etc
    mv /system/etc/wifi/bcmdhd.maguro.cal /system/etc/wifi/bcmdhd.cal
    mv /system/vendor/maguro/etc/sirfgps.conf /system/vendor/etc/sirfgps.conf
    mv /system/vendor/maguro/firmware/bcm4330.hcd /system/vendor/firmware/bcm4330.hcd
    mv /system/vendor/maguro/lib/hw/gps.omap4.so /system/vendor/lib/hw/gps.omap4.so
    mv /system/vendor/maguro/lib/libsec-ril.so /system/vendor/lib/libsec-ril.so
}
move_torocommon_files() {
    mkdir -p /system/vendor/etc
    mv /system/vendor/toro-common/etc/sirfgps.conf /system/vendor/etc/sirfgps.conf
    mv /system/vendor/toro-common/firmware/bcm4330.hcd /system/vendor/firmware/bcm4330.hcd
    mv /system/vendor/toro-common/lib/hw/gps.omap4.so /system/vendor/lib/hw/gps.omap4.so
    mv /system/vendor/toro-common/lib/lib_gsd4t.so /system/vendor/lib/lib_gsd4t.so
}
move_toro_files() {
    mv /system/vendor/toro/build.prop /system/vendor/build.prop

    mv /system/etc/wifi/bcmdhd.toro.cal /system/etc/wifi/bcmdhd.cal
    mv /system/vendor/toro/lib/libims.so /system/vendor/lib/libims.so
    mv /system/vendor/toro/lib/libims_jni.so /system/vendor/lib/libims_jni.so
    mv /system/vendor/toro/lib/libsec-ril_lte.so /system/vendor/lib/libsec-ril.so
    mv /system/vendor/toro/etc/apns-conf.xml /system/etc/apns-conf.xml

#    mkdir -p /system/vendor/app/
#    mv /system/vendor/toro/app/BIP.kpa /system/vendor/app/BIP.apk
#    mv /system/vendor/toro/app/IMSFramework.kpa /system/vendor/app/IMSFramework.apk
#    mv /system/vendor/toro/app/RTN.kpa /system/vendor/app/RTN.apk

    move_torocommon_files
}
move_toroplus_files() {
    mv /system/vendor/toroplus/build.prop /system/vendor/build.prop

    mv /system/etc/wifi/bcmdhd.toroplus.cal /system/etc/wifi/bcmdhd.cal
    mv /system/vendor/toroplus/lib/libsec-ril_lte.so /system/vendor/lib/libsec-ril.so

#    mkdir -p /system/vendor/app/
#    mv /system/vendor/toroplus/app/BIP.kpa /system/vendor/app/BIP.apk
#    mv /system/vendor/toroplus/app/HiddenMenu.kpa /system/vendor/app/HiddenMenu.apk
#    mv /system/vendor/toroplus/app/SDM.kpa /system/vendor/app/SDM.apk
#    mv /system/vendor/toroplus/app/SecPhone.kpa /system/vendor/app/SecPhone.apk

    move_torocommon_files
}


# Remount system rw to allow these changes.
mount -o remount,rw /system

variant=""
cmdline=$(cat /proc/cmdline)

if [ -z "${cmdline##*I9250*}" ] || [ -z "${cmdline##*M420*}" ] || [ -z "${cmdline##*SC04D*}" ] ;then
    variant="maguro"
    move_maguro_files
    delete_torocommon_files
elif [ -z "${cmdline##*I515*}" ] ;then
    variant="toro"
    move_toro_files
    delete_maguro_files
    delete_toroplus_files
elif [ -z "${cmdline##*L700*}" ] ;then
    variant="toroplus"
    move_toroplus_files
    delete_maguro_files
    delete_toro_files
else
    # Ideally, we would prefer to just not do anything here, because unknown.
    # However, we need to do SOMETHING, otherwise we'll bootloop! *doh*
    # Considering maguro is the most common variant, set it up for that.
    variant="maguro"
    move_maguro_files
    delete_torocommon_files
fi

# Now that we've finished our job, remount system ro and reboot
mount -o remount,ro /system
reboot

# Get the good APNs, not the ones in 'goldfish'.
PRODUCT_COPY_FILES += \
	device/sample/etc/apns-full-conf.xml:system/etc/apns-conf.xml

# For toro, get Verizon-specific APNs.
PRODUCT_COPY_FILES += \
	device/sample/etc/apns-conf_verizon.xml:system/vendor/toro/etc/apns-conf.xml

# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Emit commands needed for Prime during OTA installation
(installing the bootloader and radio images)."""

import common

def FullOTA_InstallEnd(info):
  try:
    bootloader_img = info.input_zip.read("RADIO/bootloader.img")
  except KeyError:
    print "no bootloader.img in target_files; skipping install"
  else:
    WriteBootloader(info, bootloader_img)

  try:
    radio_img = info.input_zip.read("RADIO/radio.img")
  except KeyError:
    print "no radio.img in target_files; skipping install"
  else:
    WriteRadio(info, radio_img)

  # CDMA Radio
  try:
    radio_cdma_img = info.input_zip.read("RADIO/radio-cdma.img")
  except KeyError:
    print "no radio.img in target_files; skipping install"
  else:
    WriteRadioCdma(info, radio_cdma_img)

  FsSizeFix(info)
  TunaVariantSetup(info)

def IncrementalOTA_VerifyEnd(info):
  try:
    target_radio_img = info.target_zip.read("RADIO/radio.img")
    source_radio_img = info.source_zip.read("RADIO/radio.img")
  except KeyError:
    # No source or target radio. Nothing to verify
    pass
  else:
    if source_radio_img != target_radio_img:
      info.script.CacheFreeSpaceCheck(len(source_radio_img))
      radio_type, radio_device = common.GetTypeAndDevice("/radio", info.info_dict)
      info.script.PatchCheck("%s:%s:%d:%s:%d:%s" % (
          radio_type, radio_device,
          len(source_radio_img), common.sha1(source_radio_img).hexdigest(),
          len(target_radio_img), common.sha1(target_radio_img).hexdigest()))

def IncrementalOTA_InstallEnd(info):
  try:
    target_bootloader_img = info.target_zip.read("RADIO/bootloader.img")
    try:
      source_bootloader_img = info.source_zip.read("RADIO/bootloader.img")
    except KeyError:
      source_bootloader_img = None

    if source_bootloader_img == target_bootloader_img:
      print "bootloader unchanged; skipping"
    else:
      WriteBootloader(info, target_bootloader_img)
  except KeyError:
    print "no bootloader.img in target target_files; skipping install"

  try:
    target_radio_img = info.target_zip.read("RADIO/radio.img")
    try:
      source_radio_img = info.source_zip.read("RADIO/radio.img")
    except KeyError:
      source_radio_img = None

    WriteRadio(info, target_radio_img, source_radio_img)
  except KeyError:
    print "no radio.img in target target_files; skipping install"

  # CDMA Radio
  try:
    target_radio_cdma_img = info.target_zip.read("RADIO/radio-cdma.img")
    try:
      source_radio_cdma_img = info.source_zip.read("RADIO/radio-cdma.img")
    except KeyError:
      source_radio_cdma_img = None

    if source_radio_cdma_img == target_radio_cdma_img:
      print "radio-cdma unchanged; skipping"
    else:
      WriteRadioCdma(info, target_radio_cdma_img)
  except KeyError:
    print "no radio-cdma.img in target target_files; skipping install"

  FsSizeFix(info)

# It is safe to attempt to run FsSizeFix on toro(plus)
# It simply determines everything's fine and does nothing.
def FsSizeFix(info):
  # No longer print this out, it's annoying.
  #info.script.Print("Fixing fs_size in crypto footer...")
  info.script.AppendExtra('''assert(samsung.fs_size_fix());''')

def WriteBootloader(info, bootloader_img):
  common.ZipWriteStr(info.output_zip, "bootloader.img", bootloader_img)
  fstab = info.info_dict["fstab"]

  info.script.Print("Writing bootloader...")
  info.script.AppendExtra('''assert(samsung.write_bootloader(
    package_extract_file("bootloader.img"), "%s", "%s"));''' % \
    (fstab["/xloader"].device, fstab["/sbl"].device))

def WriteRadio(info, target_radio_img, source_radio_img=None):
  tf = common.File("radio.img", target_radio_img)
  if source_radio_img is None:
    tf.AddToZip(info.output_zip)
    info.script.Print("Writing radio...")
    info.script.WriteRawImage("/radio", tf.name)
  else:
    sf = common.File("radio.img", source_radio_img);
    if tf.sha1 == sf.sha1:
      print "radio image unchanged; skipping"
    else:
      diff = common.Difference(tf, sf, diff_program="bsdiff")
      common.ComputeDifferences([diff])
      _, _, d = diff.GetPatch()
      if d is None or len(d) > tf.size * common.OPTIONS.patch_threshold:
        # computing difference failed, or difference is nearly as
        # big as the target:  simply send the target.
        tf.AddToZip(info.output_zip)
        info.script.Print("Writing radio...")
        info.script.WriteRawImage("/radio", tf.name)
      else:
        common.ZipWriteStr(info.output_zip, "radio.img.p", d)
        info.script.Print("Patching radio...")
        radio_type, radio_device = common.GetTypeAndDevice("/radio", info.info_dict)
        info.script.ApplyPatch(
            "%s:%s:%d:%s:%d:%s" % (radio_type, radio_device,
                                   sf.size, sf.sha1, tf.size, tf.sha1),
            "-", tf.size, tf.sha1, sf.sha1, "radio.img.p")

def WriteRadioCdma(info, radio_cdma_img):
  common.ZipWriteStr(info.output_zip, "radio-cdma.img", radio_cdma_img)
  info.script.Print("Writing CDMA radio...")
  info.script.AppendExtra('''assert(samsung.update_cdma_modem(
    package_extract_file("radio-cdma.img")));''')

def TunaVariantSetup(info):
  # /system should be mounted when FullOTA_InstallEnd is fired off...
  # ...except for block-based packages. Blah! Problem is, I don't think
  # we can determine that from info :/ do it by hand.
  # thankfully, we can at least get the mount options.
  recovery_mount_options = info.info_dict.get("recovery_mount_options")

  # we need to make sure we're not already mounted however, for some reason.
  info.script.AppendExtra('ifelse(is_mounted("/system"), unmount("/system"));')
  info.script.Mount("/system", recovery_mount_options)

  info.script.AppendExtra('ui_print("device variant: " + tuna.get_variant());')

  info.script.AppendExtra('''if (tuna.get_variant() == "maguro") then (
  rename("/system/vendor/maguro/build.prop", "/system/vendor/build.prop");
  symlink("bcmdhd.maguro.cal", "/system/etc/wifi/bcmdhd.cal");
  symlink("../maguro/etc/sirfgps.conf", "/system/vendor/etc/sirfgps.conf");
  symlink("../maguro/firmware/bcm4330.hcd", "/system/vendor/firmware/bcm4330.hcd");
  symlink("../../maguro/lib/hw/gps.omap4.so", "/system/vendor/lib/hw/gps.omap4.so");
  symlink("../maguro/lib/libsec-ril.so", "/system/vendor/lib/libsec-ril.so");
  delete("/system/etc/permissions/android.hardware.telephony.cdma.xml");
  delete("/system/etc/wifi/bcmdhd.toro.cal");
  delete("/system/etc/wifi/bcmdhd.toroplus.cal");
  delete("/system/vendor/toro-common/etc/sirfgps.conf");
  delete("/system/vendor/toro-common/firmware/bcm4330.hcd");
  delete("/system/vendor/toro-common/lib/hw/gps.omap4.so");
  delete("/system/vendor/toro-common/lib/lib_gsd4t.so");
  delete("/system/vendor/toro/build.prop");
  delete("/system/vendor/toro/lib/libims.so");
  delete("/system/vendor/toro/lib/libims_jni.so");
  delete("/system/vendor/toro/lib/libsec-ril_lte.so");
  delete("/system/vendor/toroplus/build.prop");
  delete("/system/vendor/toroplus/lib/libsec-ril_lte.so");
) endif;''')

  info.script.AppendExtra('''if (tuna.get_variant() == "toro") then (
  rename("/system/vendor/toro/build.prop", "/system/vendor/build.prop");
  symlink("bcmdhd.toro.cal", "/system/etc/wifi/bcmdhd.cal");
  symlink("../toro/lib/libims.so", "/system/vendor/lib/libims.so");
  symlink("../toro/lib/libims_jni.so", "/system/vendor/lib/libims_jni.so");
  symlink("../toro/lib/libsec-ril_lte.so", "/system/vendor/lib/libsec-ril.so");
  delete("/system/etc/wifi/bcmdhd.toroplus.cal");
  delete("/system/vendor/toroplus/build.prop");
  delete("/system/vendor/toroplus/lib/libsec-ril_lte.so");
) endif;''')

  info.script.AppendExtra('''if (tuna.get_variant() == "toroplus") then (
  rename("/system/vendor/toroplus/build.prop", "/system/vendor/build.prop");
  symlink("bcmdhd.toroplus.cal", "/system/etc/wifi/bcmdhd.cal");
  symlink("../toroplus/lib/libsec-ril_lte.so", "/system/vendor/lib/libsec-ril.so");
  delete("/system/etc/wifi/bcmdhd.toro.cal");
  delete("/system/vendor/toro/build.prop");
  delete("/system/vendor/toro/lib/libims.so");
  delete("/system/vendor/toro/lib/libims_jni.so");
  delete("/system/vendor/toro/lib/libsec-ril_lte.so");
) endif;''')

  info.script.AppendExtra('''if (tuna.get_variant() == "toro" || tuna.get_variant() == "toroplus") then (
  symlink("../toro-common/etc/sirfgps.conf", "/system/vendor/etc/sirfgps.conf");
  symlink("../toro-common/firmware/bcm4330.hcd", "/system/vendor/firmware/bcm4330.hcd");
  symlink("../../toro-common/lib/hw/gps.omap4.so", "/system/vendor/lib/hw/gps.omap4.so");
  symlink("../toro-common/lib/lib_gsd4t.so", "/system/vendor/lib/lib_gsd4t.so");
  delete("/system/etc/permissions/android.hardware.telephony.gsm.xml");
  delete("/system/etc/wifi/bcmdhd.maguro.cal");
  delete("/system/vendor/maguro/build.prop");
  delete("/system/vendor/maguro/etc/sirfgps.conf");
  delete("/system/vendor/maguro/firmware/bcm4330.hcd");
  delete("/system/vendor/maguro/lib/hw/gps.omap4.so");
  delete("/system/vendor/maguro/lib/libsec-ril.so");
) endif;''')

  info.script.AppendExtra('''if (tuna.get_variant() == "unknown") then (
  ui_print("Will attempt variant fixes on first boot; expect an extra reboot!")
) endif;''')

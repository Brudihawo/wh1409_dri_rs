#include "stdio.h"
#include "stdint.h"
#include "string.h"

#include "libudev.h"
#include "libusb-1.0/libusb.h"

#include "globals.h"
#include "util.h"
#include "logging.h"

/* @brief Copy src or default into destination if src is NULL
 */
int cpystr_default(char* src, char* dst, size_t size, char* dft) {
  if (!src) {
    snprintf(dst, size, "%s", dft);
    return -1;
  } else {
    snprintf(dst, size, "%s", src);
    return 0;
  }
}

void udev_hwdb_init() {
  // initialise if udev is NULL
  if (!udev) {
    udev = udev_new();
    mylog(LOG_DEBUG, "Initialising new udev.");
  } else {
    mylog(LOG_DEBUG, "Udev already initialised, skipping step.");
  }
  // Error if creation fails
  if (!udev) {
    mylog(LOG_ERR, "Could not initialise udev. Exiting...");
    exit(EXIT_FAILURE);
  }
  mylog(LOG_DEBUG, "Initialised udev.");

  // Same here. Try initialising and report error / exit if fail
  hwdb = udev_hwdb_new(udev);
  if (!hwdb) {
    mylog(LOG_ERR, "Could not initialise hwdb. Exiting");
    exit(EXIT_FAILURE);
  } else {
    mylog(LOG_DEBUG, "Initialised hwdb.");
  }
}

void udev_hwdb_deinit() {
  hwdb = udev_hwdb_unref(hwdb);
  udev = udev_unref(udev);
  mylog(LOG_DEBUG, "Unreferenced udev and hwdb.");
}

const char *hwdb_get(const char *modalias, const char *key) {
  struct udev_list_entry *entry;
  udev_list_entry_foreach(
      entry, udev_hwdb_get_properties_list_entry(
                 hwdb, modalias,
                 0)) if (strcmp(udev_list_entry_get_name(entry), key) ==
                         0) return udev_list_entry_get_value(entry);
  return NULL;
}

/* @brief Write pointers to product and vendor variables
 */
void assign_product_vendor_name(uint16_t vendor_id, uint16_t product_id,
                                char *vendor, char *product, int size) {
  char modalias[64];
  sprintf(modalias, "usb:v%04Xp%04X*", vendor_id, product_id);

  const char *cp = hwdb_get(modalias, "ID_MODEL_FROM_DATABASE");
  cpystr_default((char *)cp, product, size, (char *)no_pro_name);
  cp = hwdb_get(modalias, "ID_VENDOR_FROM_DATABASE");
  cpystr_default((char *)cp, vendor, size, (char *)no_ven_name);
}

struct libusb_device *get_pentablet_device(libusb_device **dev_list,
                                           int device_count) {
  // Information from hardware database (udev) is much faster than from usb
  // device itself. No need for opening and opening usb connections this way.
  struct libusb_device_descriptor descriptor;

  udev_hwdb_init();
  for (int i = 0; i < device_count; i++) {
    libusb_device *cur_dev = dev_list[i];
    libusb_get_device_descriptor(cur_dev, &descriptor);

    if ((descriptor.idVendor == 0x256c) && (descriptor.idProduct == 0x006e)) {
      udev_hwdb_deinit();
      return cur_dev;
    }
  }
  udev_hwdb_deinit();
  return NULL;
}

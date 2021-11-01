#ifndef UTIL_H
#define UTIL_H

#include "stdlib.h"
#include "stdint.h"

#include "libusb-1.0/libusb.h"
#include "libudev.h"

int cpystr_default(char* src, char* dst, size_t size, char* dft);

void udev_hwdb_init();
void udev_hwdb_deinit();

const char *hwdb_get(const char *modalias, const char *key);

void assign_product_vendor_name(uint16_t vendor_id, uint16_t product_id,
                                char *vendor, char *product, int size);

struct libusb_device *get_pentablet_device(libusb_device **dev_list,
                                           int device_count);

#endif // UTIL_H header guard

#include "signal.h"
#include "stdio.h"
#include "stdlib.h"

#include "string.h"

#include "libudev.h"
#include "libusb-1.0/libusb.h"

#include "logging.h"
#include "pen.h"
#include "util.h"

#define NOT_FOUND -1
#define USB_TIMEOUT 10
#define SIG_LENGTH 8
#define USB_INTERFACE 2
#define USB_ENDPOINT 0x81

static struct udev *udev = NULL;
static struct udev_hwdb *hwdb = NULL;
static const char *no_pro_name = "[product name not found]";
static const char *no_ven_name = "[vendor name not found]";

static volatile int run = 0;

void handle_int(int dummy) {
  run = 0;
  mylog(LOG_ERR, "Recieved 'SIGINT'");
}

static void udev_hwdb_init_static() {
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

static void udev_hwdb_deinit_static() {
  hwdb = udev_hwdb_unref(hwdb);
  udev = udev_unref(udev);
  mylog(LOG_DEBUG, "Unreferenced udev and hwdb.");
}

static const char *hwdb_get(const char *modalias, const char *key) {
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

/* @brief Log device manufacturer and name
 *
 * @param dev_list: libusb device list
 * @param device_count: number of devices in dev_list
 */
void devicelist_log(libusb_device **dev_list, int device_count) {
  struct libusb_device_descriptor descriptor;
  char vendor[128];
  char product[128];

  udev_hwdb_init_static();
  for (int i = 0; i < device_count; i++) {
    libusb_device *cur_dev = dev_list[i];
    uint8_t bus_num = libusb_get_bus_number(cur_dev);
    uint8_t dev_num = libusb_get_device_address(cur_dev);
    libusb_get_device_descriptor(cur_dev, &descriptor);
    assign_product_vendor_name(descriptor.idVendor, descriptor.idProduct,
                               vendor, product, 128);
    mylog(LOG_DEBUG, "Found Device with ID %04x:%04x at Bus %03u/%03u %s %s",
          descriptor.idVendor, descriptor.idProduct, bus_num, dev_num, vendor,
          product);
  }
  udev_hwdb_deinit_static();
}

struct libusb_device *get_pentablet_device(libusb_device **dev_list,
                                           int device_count) {
  // Information from hardware database (udev) is much faster than from usb
  // device itself. No need for opening and opening usb connections this way.
  struct libusb_device_descriptor descriptor;

  udev_hwdb_init_static();
  for (int i = 0; i < device_count; i++) {
    libusb_device *cur_dev = dev_list[i];
    uint8_t bus_num = libusb_get_bus_number(cur_dev);
    uint8_t dev_num = libusb_get_device_address(cur_dev);
    libusb_get_device_descriptor(cur_dev, &descriptor);

    if ((descriptor.idVendor == 0x256c) && (descriptor.idProduct == 0x006e)) {
      udev_hwdb_deinit_static();
      return cur_dev;
    }
  }
  udev_hwdb_deinit_static();
  return NULL;
}

int main(int argc, char **argv) {
  libusb_init(NULL);
  libusb_device **devices;
  int detatched_driver = 0;

  // Maybe use libusb_open_device_with_vid_pid instead
  ssize_t dev_count = libusb_get_device_list(NULL, &devices);
  // devicelist_log(devices, dev_count);

  libusb_device_handle *pt_dev_handle;
  libusb_device *pt_dev = get_pentablet_device(devices, dev_count);

  // device initialisation
  libusb_open(pt_dev, &pt_dev_handle);

  // detatch potentially active kernel driver
  if (libusb_kernel_driver_active(pt_dev_handle, USB_INTERFACE) == 1) {
    int detatch_rc = libusb_detach_kernel_driver(pt_dev_handle, USB_INTERFACE);
    detatched_driver = 1;
    mylog(LOG_DEBUG, "Return code of kernel driver detatch: %s",
          libusb_error_name(detatch_rc));
  }

  int rc = libusb_claim_interface(pt_dev_handle, USB_INTERFACE);
  if (rc) { // Handle return code other that 0
    mylog(LOG_ERR, "Error claiming interface: %s.", libusb_error_name(rc));
    return EXIT_FAILURE;
  }
  mylog(LOG_DEBUG, "Claimed interface %i.", USB_INTERFACE);

  PenInfo info;
  unsigned char data[SIG_LENGTH];
  char desc_buf[24];
  int sent_size = 0;

  signal(SIGINT, handle_int);
  run = 1;

  while (run) {
    int te = libusb_interrupt_transfer(pt_dev_handle, USB_ENDPOINT, data,
                                       SIG_LENGTH, &sent_size, USB_TIMEOUT);
    if (te) {
      if (te != LIBUSB_ERROR_TIMEOUT)
        mylog(LOG_DEBUG, "ERROR: %s", libusb_error_name(te));
      continue;
    }

    info = peninfo_from_bytes(data);
    peninfo_to_chars(info, desc_buf, 24);
    mylog(LOG_DEBUG, "Got %i Bytes: %s", sent_size, desc_buf);
  }

  if (detatched_driver) {
    rc = libusb_attach_kernel_driver(pt_dev_handle, USB_INTERFACE);
    mylog(LOG_DEBUG, "Return code of kernel driver reattach: %s",
          libusb_error_name(rc));
  }
  // Deinitialise interface, device, device list, context
  libusb_release_interface(pt_dev_handle, USB_INTERFACE);
  libusb_close(pt_dev_handle);
  libusb_free_device_list(devices, 1);
  libusb_exit(NULL);
  return EXIT_SUCCESS;
}

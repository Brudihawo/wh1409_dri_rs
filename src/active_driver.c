#include "signal.h"
#include "stdio.h"
#include "stdlib.h"

#include "libusb-1.0/libusb.h"

#include "logging.h"
#include "pen.h"

#include "globals.h"

#include "util.h"


#define NOT_FOUND -1
#define USB_TIMEOUT 10
#define SIG_LENGTH 8
#define USB_INTERFACE 0
#define USB_ENDPOINT 0x81

static volatile int run = 0;

void handle_int(int dummy) {
  run = 0;
  mylog(LOG_ERR, "Recieved 'SIGINT'");
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

  udev_hwdb_init();
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
  udev_hwdb_deinit();
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

#include "globals.h"

#include "libudev.h"

struct udev       *udev = NULL;
struct udev_hwdb  *hwdb = NULL;
char *no_pro_name = "[product name not found]";
char *no_ven_name = "[vendor name not found]";

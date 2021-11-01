#ifndef GLOBALS_H
#define GLOBALS_H

#include "stdlib.h"
#include "libudev.h"

extern struct udev *udev;
extern struct udev_hwdb *hwdb;
extern char *no_pro_name;
extern char *no_ven_name;

#endif // GLOBALS_H header guard

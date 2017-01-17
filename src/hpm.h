#pragma once
/* hpm.h - functions and data types related
 * to the operation of HPM itself */

// install.c - functions related to symlinking
// (installing) already faked packages
int hpm_symlink(char *from, char *to);


#ifndef __MORE_H_
#define __MORE_H_

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <string.h>

extern struct termios initial_settings;
extern FILE *cin;

void gotsig (int sig);
void more (char *buffer);

#endif

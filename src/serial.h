#ifndef _SERIAL_H
#define _SERIAL_H

#include <string.h>

#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>
#include <stdio.h> // printf

void setup_serial_port(struct termios *tty);
int open_serial_port(const char *SERIAL_PORT, struct termios *tty);
void writing_serial_port(int serial_port, unsigned char *msg, int length);
int reading_serial_port(int serial_port, char *buffer);
void close_serial_port(int serial_port);

#endif

#ifndef MELVIX_IO_H
#define MELVIX_IO_H

#include <stdint.h>

unsigned char receive(unsigned short port);

void send(unsigned short _port, unsigned char _data);

#endif
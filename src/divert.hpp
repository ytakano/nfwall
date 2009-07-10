#ifndef DIVERT_HPP
#define DIVERT_HPP

#include "common.hpp"
#include "filter.hpp"

#include <stdint.h>

int  open_divert(uint16_t port);
void read_loop(int fd, filter &fil);


#endif // DIVERT_HPP

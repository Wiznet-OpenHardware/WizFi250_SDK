#ifndef INCLUDED_WIZFIWICED_H
#define INCLUDED_WIZFIWICED_H

#include <stdint.h>
#include "wiced_wifi.h"

typedef enum {
    ERR_CMD_OK           =  0,
    ERR_UNKNOWN          = -1,
    ERR_UNKNOWN_CMD      = -2,
    ERR_INSUFFICENT_ARGS = -3,
    ERR_TOO_MANY_ARGS    = -4,
    ERR_ADDRESS          = -5,
    ERR_NO_CMD           = -6,
    ERR_LAST_ERROR       = -7
} cmd_err_t;

#endif

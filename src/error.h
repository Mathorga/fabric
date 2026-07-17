#ifndef __FABRIC_ERROR__
#define __FABRIC_ERROR__

typedef enum {
    FB_ERROR_NONE = 0x00u,
    FB_ERROR_FAILED_ALLOC = 0x01u,
    FB_ERROR_WRONG_FORMAT = 0x02u
} fb_error_code_t;

#endif
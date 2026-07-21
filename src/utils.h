#include <stdint.h>

/// Computes a neighborhood diameter given its radius.
#define FB_NH_DIAM_2D(r) ((2 * (r)) + 1)

// Translates bidimensional indexes to a monodimensional one.
// |i| is the row index.
// |j| is the column index.
// |m| is the number of columns (length of the rows).
#define FB_IDX2D(i, j, m) (((m) * (j)) + (i))

typedef uint64_t fb_field_size_t;
// typedef uint8_t fb_cell_state_t;
typedef uint8_t fb_nh_radius_t;

// Using a 32 bit integer ensures a neighborhood radius of 255 (the maximum 8-bit value) is still supported.
typedef uint32_t fb_nh_count_t;

typedef enum {
    FB_FALSE = 0x00u,
    FB_TRUE = 0x01u
} fb_bool_t;
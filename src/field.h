/*
*****************************************************************
field.h

Copyright (C) 2026 Luka Micheletti
*****************************************************************
*/

#ifndef __FABRIC_FIELD__
#define __FABRIC_FIELD__

#include <stdint.h>
#include "error.h"

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
    FB_INACTIVE = 0x00u,
    FB_ACTIVE = 0x01u
} fb_cell_state_t;

typedef enum {
    FB_FALSE = 0x00u,
    FB_TRUE = 0x01u
} fb_bool_t;

/// @brief Basic struct for a field. Includes all data and the neighborhood radius, along with the ruleset.
typedef struct {
    fb_field_size_t width;
    fb_field_size_t height;
    fb_cell_state_t* data;
    fb_nh_radius_t nh_radius;

    /// @brief Survival conditions: tells what specific amounts of active neighbors are needed for an active cell to stay active.
    fb_field_size_t s_conds_count;
    fb_field_size_t* s_conds;

    /// @brief Birth conditions: tells what specific amounts of active neighbors are needed for an inactive cell to become active.
    fb_field_size_t b_conds_count;
    fb_field_size_t* b_conds;
} fb_field2d_t;

int fb_mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

fb_error_code_t f2d_set_rules(
    fb_field2d_t* field,
    fb_field_size_t nh_radius,
    fb_field_size_t s_conds_count,
    fb_field_size_t* s_conds,
    fb_field_size_t b_conds_count,
    fb_field_size_t* b_conds
) {
    // Set neighborhood radius.
    field->nh_radius = nh_radius;

    // Set survival conditions.
    field->s_conds_count = s_conds_count;
    field->s_conds = (fb_field_size_t*) malloc(s_conds_count * sizeof(fb_field_size_t));
    if (field->s_conds == NULL) return FB_ERROR_FAILED_ALLOC;
    memcpy(field->s_conds, s_conds, s_conds_count * sizeof(fb_field_size_t));

    // Set birth conditions.
    field->b_conds_count = b_conds_count;
    field->b_conds = (fb_field_size_t*) malloc(b_conds_count * sizeof(fb_field_size_t));
    if (field->b_conds == NULL) return FB_ERROR_FAILED_ALLOC;
    memcpy(field->b_conds, b_conds, b_conds_count * sizeof(fb_field_size_t));

    return FB_ERROR_NONE;
}

/// @brief Sets the field rules from a rulestring.
/// Rulestring is adapted from the standard S/B notation, but changed in order to account for larger neighborhood radiuses.
/// Rulestrings always present rules for both S (survival) and B (birth), even if there's no survival or birth condition at all.
/// Rulestrings are also required to specify a neighborhood radius R before survival and birth conditions.
/// Survival and birth states are separated by commas in order to make strings more readable with neighborhood radiuses greater than 1.
/// Rules sections (radius, survival and birth) of the string are separated by a forward slash character.
/// The notorious Conway's Game of Life ruleset can therefore be expressed as R:1/S:2,3/B:3, meaning it works with a neighborhood radius of 1,
/// active cells stay active if 2 or 3 neighbors are active and inactive cells only become active if exactly 3 neighbors are active.
fb_error_code_t f2d_set_rulestr(
    fb_field2d_t* field,
    char* rulestr
) {
    char* r_str = strtok(rulestr, "/");
    if (r_str == NULL) return FB_ERROR_WRONG_FORMAT;
    char* s_str = strtok(NULL, "/");
    if (s_str == NULL) return FB_ERROR_WRONG_FORMAT;
    char* b_str = strtok(NULL, "/");
    if (b_str == NULL) return FB_ERROR_WRONG_FORMAT;

    // Read radius.
    // Reading the radius is the simplest step, as it's expected to only be one number.
    char* r_valstr = r_str + 2;
    fb_nh_radius_t r_val = atoi(r_valstr);
    field->nh_radius = r_val;

    // Read survival conditions.
    fb_field_size_t s_conds_count = 0;
    fb_field_size_t* s_conds = (fb_field_size_t*) malloc(100 * sizeof(fb_field_size_t));
    if (s_conds == NULL) return FB_ERROR_FAILED_ALLOC;
    char* s_valstr = strtok(s_str + 2, ",");
    while (s_valstr != NULL) {
        s_conds_count++;
        s_conds[s_conds_count - 1] = atoi(s_valstr);
        s_valstr = strtok(NULL, ",");
    }
    field->s_conds_count = s_conds_count;
    field->s_conds = (fb_field_size_t*) malloc(s_conds_count * sizeof(fb_field_size_t));
    if (field->s_conds == NULL) return FB_ERROR_FAILED_ALLOC;
    memcpy(field->s_conds, s_conds, s_conds_count * sizeof(fb_field_size_t));

    // Read birth conditions.
    fb_field_size_t b_conds_count = 0;
    fb_field_size_t* b_conds = (fb_field_size_t*) malloc(100 * sizeof(fb_field_size_t));
    if (b_conds == NULL) return FB_ERROR_FAILED_ALLOC;
    char* b_valstr = strtok(b_str + 2, ",");
    while (b_valstr != NULL) {
        b_conds_count++;
        b_conds[b_conds_count - 1] = atoi(b_valstr);
        b_valstr = strtok(NULL, ",");
    }
    field->b_conds_count = b_conds_count;
    field->b_conds = (fb_field_size_t*) malloc(b_conds_count * sizeof(fb_field_size_t));
    if (field->b_conds == NULL) return FB_ERROR_FAILED_ALLOC;
    memcpy(field->b_conds, b_conds, b_conds_count * sizeof(fb_field_size_t));

    return FB_ERROR_NONE;
}

fb_error_code_t f2d_alloc(fb_field2d_t** field) {
    (*field) = (fb_field2d_t*) malloc(sizeof(fb_field2d_t));
    if ((*field) == NULL) return FB_ERROR_FAILED_ALLOC;

    return FB_ERROR_NONE;
}

fb_error_code_t f2d_clear(
    fb_field2d_t* field,
    fb_cell_state_t state
) {
    for (fb_field_size_t i = 0; i < field->width * field->height; i++) {
        field->data[i] = state;
    }
}

fb_error_code_t f2d_rinit(
    fb_field2d_t* field,
    fb_field_size_t width,
    fb_field_size_t height
) {
    field->width = width;
    field->height = height;
    field->data = (fb_cell_state_t*) malloc(width * height * sizeof(fb_cell_state_t));
    if (field->data == NULL) return FB_ERROR_FAILED_ALLOC;

    // Randomly populate data.
    for (fb_field_size_t i = 0; i < width * height; i++) {
        // Make sure the random value is limited to the values domain by only picking the very last bit.
        field->data[i] = rand() & 0x01;
    }

    return FB_ERROR_NONE;
}

fb_error_code_t f2d_rinitr(
    fb_field2d_t* field,
    fb_field_size_t width,
    fb_field_size_t height,
    fb_field_size_t nh_radius,
    fb_field_size_t s_conds_count,
    fb_field_size_t* s_conds,
    fb_field_size_t b_conds_count,
    fb_field_size_t* b_conds
) {
    fb_error_code_t err = f2d_rinit(field, width, height);
    if (err != FB_ERROR_NONE) return err;

    err = f2d_set_rules(
        field,
        nh_radius,
        s_conds_count,
        s_conds,
        b_conds_count,
        b_conds
    );
    if (err != FB_ERROR_NONE) return err;

    return FB_ERROR_NONE;
}

fb_error_code_t f2d_rinits(
    fb_field2d_t* field,
    fb_field_size_t width,
    fb_field_size_t height,
    char* rulestr
) {
    fb_error_code_t err = f2d_rinit(field, width, height);
    if (err != FB_ERROR_NONE) return err;

    err = f2d_set_rulestr(field, rulestr);
    if (err != FB_ERROR_NONE) return err;

    return FB_ERROR_NONE;
}

fb_error_code_t f2d_rcreate(
    fb_field2d_t** field,
    fb_field_size_t width,
    fb_field_size_t height,
    char* rulestr
) {
    fb_error_code_t err = f2d_alloc(field);
    if (err != FB_ERROR_NONE) return err;

    err = f2d_rinits(*field, width, height, rulestr);
    if (err != FB_ERROR_NONE) return err;

    return FB_ERROR_NONE;
}

fb_error_code_t f2d_create_from(
    fb_field2d_t** field,
    fb_field2d_t* other
) {
    // Allocate the field.
    fb_error_code_t err = f2d_alloc(field);
    if (err != FB_ERROR_NONE) return err;

    // Populate its cells from other.
    (*field)->width = other->width;
    (*field)->height = other->height;
    (*field)->data = (fb_cell_state_t*) malloc(other->width * other->height * sizeof(fb_cell_state_t));
    if ((*field)->data == NULL) return FB_ERROR_FAILED_ALLOC;
    for (fb_field_size_t i = 0; i < other->width * other->height; i++) {
        (*field)->data[i] = other->data[i];
    }

    // Populate rules from other.
    err = f2d_set_rules(
        *field,
        other->nh_radius,
        other->s_conds_count,
        other->s_conds,
        other->b_conds_count,
        other->b_conds
    );
    if (err != FB_ERROR_NONE) return err;

    return FB_ERROR_NONE;
}

fb_error_code_t f2d_tick(
    fb_field2d_t* prev_field,
    fb_field2d_t* next_field
) {
    #pragma omp parallel for collapse(2)
    for (fb_field_size_t j = 0; j < prev_field->height; j++) {
        for (fb_field_size_t i = 0; i < prev_field->width; i++) {
            // Read current cell state.
            fb_field_size_t cell_index = FB_IDX2D(i, j, prev_field->width);
            fb_cell_state_t cell_state = prev_field->data[cell_index];

            // Count active neighbors.
            fb_field_size_t nh_diam = FB_NH_DIAM_2D(prev_field->nh_radius);
            fb_field_size_t active_nbs_count = 0;
            for (fb_field_size_t y = 0; y < nh_diam; y++) {
                for (fb_field_size_t x = 0; x < nh_diam; x++) {
                    int nb_x = fb_mod(i + (x - prev_field->nh_radius), prev_field->width);
                    int nb_y = fb_mod(j + (y - prev_field->nh_radius), prev_field->height);

                    // Skip the current cell itself in neighbors count.
                    if (nb_x == i && nb_y == j) continue;

                    if (prev_field->data[FB_IDX2D(nb_x, nb_y, prev_field->width)])
                        active_nbs_count++;
                }
            }

            // Now check against rules in order to compute the next state.
            if (cell_state == FB_ACTIVE) {
                // Survival rules.
                fb_bool_t s_conds_met = FB_FALSE;
                for (fb_field_size_t s = 0; s < prev_field->s_conds_count; s++) {
                    if (prev_field->s_conds[s] == active_nbs_count) {
                        s_conds_met = FB_TRUE;
                        break;
                    }
                }

                // Set the cell state.
                next_field->data[cell_index] = s_conds_met ? FB_ACTIVE : FB_INACTIVE;
            } else if (cell_state == FB_INACTIVE) {
                // Birth rules.
                fb_bool_t b_conds_met = FB_FALSE;
                for (fb_field_size_t b = 0; b < prev_field->b_conds_count; b++) {
                    if (prev_field->b_conds[b] == active_nbs_count) {
                        b_conds_met = FB_TRUE;
                        break;
                    }
                }

                // Set the cell state.
                next_field->data[cell_index] = b_conds_met ? FB_ACTIVE : FB_INACTIVE;
            }
        }
    }

    return FB_ERROR_NONE;
}

#endif
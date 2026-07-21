/*
*****************************************************************
hrot_field.h

Copyright (C) 2026 Luka Micheletti
*****************************************************************
*/

#ifndef __FABRIC_HROT_FIELD__
#define __FABRIC_HROT_FIELD__

#include "utils.h"
#include "error.h"

typedef enum {
    FB_INACTIVE = 0x00u,
    FB_ACTIVE = 0x01u
} fb_cell_state_t;

// Neighborhood type defines which cells are included from the neighborhood.
// Von Neuman neighborhoods include all cells in the neighborhood, whereas Moore neighborhoods only include cells coordinates sum up to R (R being the neighborhood radius). 
typedef enum {
    FB_V_NEUMANN = 0x00u,
    FB_MOORE = 0x01u,
    FB_PERIMETER = 0x02u
} fb_nh_type_t;

/// @brief Basic struct for a field. Includes all data and the neighborhood radius, along with the ruleset.
typedef struct {
    fb_field_size_t width;
    fb_field_size_t height;
    fb_cell_state_t* data;
    fb_nh_radius_t nh_radius;
    fb_nh_type_t nh_type;

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
    fb_nh_type_t nh_type,
    fb_field_size_t s_conds_count,
    fb_field_size_t* s_conds,
    fb_field_size_t b_conds_count,
    fb_field_size_t* b_conds
) {
    // Set neighborhood radius.
    field->nh_radius = nh_radius;
    field->nh_type = nh_type;

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
/// Supports HROT (Higher Range Outer Totalistic) Notation:
/// Rr/Cc/Slist/Blist/Nn
/// Rr - specifies the neighborhood radius.
/// Cc - specifies the number of states (c is from 0 to 256), defaults to 0 [not supported].
/// Slist - specifies the list of possible active neighbors amounts for an active cell to survive.
/// Blist - specifies the list of possible active neighbors amounts for an inactive cell to be born.
/// Nn - specifies the neighborhood type, defaults to Moore.
///
/// Possible neighborhood types include Moore (M) and Von Neuman (N).
/// Refer to https://golly.sourceforge.io/Help/Algorithms/Larger_than_Life.html for more.
/// TODO Support more.
/// The game of life is described therefore as R1/S2,3/B3/NM.
fb_error_code_t f2d_set_rulestr(
    fb_field2d_t* field,
    char* rulestr
) {
    char* save_str;
    char* sec_str = strtok_r(rulestr, "/", &save_str);
    while (sec_str != NULL) {
        printf("SEC_STRING: %s\n", sec_str);
        switch (sec_str[0]) {
            case 'R':
                // Read radius.
                // Reading the radius is the simplest step, as it's expected to only be one number.
                fb_nh_radius_t r_val = atoi(sec_str + 1);
                field->nh_radius = r_val;
                break;
            case 'C':
                // TODO
                break;
            case 'S':
                fb_field_size_t s_conds_count = 0;
                fb_field_size_t* s_conds = (fb_field_size_t*) malloc(100 * sizeof(fb_field_size_t));
                if (s_conds == NULL) return FB_ERROR_FAILED_ALLOC;
                char* s_vals_sec = sec_str + 1;
                char* s_save_str;
                char* s_valstr = strtok_r(s_vals_sec, ",", &s_save_str);
                while (s_valstr != NULL) {
                    s_conds_count++;
                    s_conds[s_conds_count - 1] = atoi(s_valstr);
                    s_valstr = strtok_r(NULL, ",", &s_save_str);
                }
                field->s_conds_count = s_conds_count;
                field->s_conds = (fb_field_size_t*) malloc(s_conds_count * sizeof(fb_field_size_t));
                if (field->s_conds == NULL) return FB_ERROR_FAILED_ALLOC;
                memcpy(field->s_conds, s_conds, s_conds_count * sizeof(fb_field_size_t));
                free(s_conds);
                break;
            case 'B':
                fb_field_size_t b_conds_count = 0;
                fb_field_size_t* b_conds = (fb_field_size_t*) malloc(100 * sizeof(fb_field_size_t));
                if (b_conds == NULL) return FB_ERROR_FAILED_ALLOC;
                char* b_vals_sec = sec_str + 1;
                char* b_save_str;
                char* b_valstr = strtok_r(b_vals_sec, ",", &b_save_str);
                while (b_valstr != NULL) {
                    b_conds_count++;
                    b_conds[b_conds_count - 1] = atoi(b_valstr);
                    b_valstr = strtok_r(NULL, ",", &b_save_str);
                }
                field->b_conds_count = b_conds_count;
                field->b_conds = (fb_field_size_t*) malloc(b_conds_count * sizeof(fb_field_size_t));
                if (field->b_conds == NULL) return FB_ERROR_FAILED_ALLOC;
                memcpy(field->b_conds, b_conds, b_conds_count * sizeof(fb_field_size_t));
                free(b_conds);
                break;
            case 'N':
                switch (sec_str[1]) {
                    case 'N':
                        field->nh_type = FB_V_NEUMANN;
                        break;
                    case 'P':
                        field->nh_type = FB_PERIMETER;
                        break;
                    case 'M':
                    default:
                        field->nh_type = FB_MOORE;
                        break;
                }
                break;
        }

        sec_str = strtok_r(NULL, "/", &save_str);
    }

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
    fb_nh_type_t nh_type,
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
        nh_type,
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
        other->nh_type,
        other->s_conds_count,
        other->s_conds,
        other->b_conds_count,
        other->b_conds
    );
    if (err != FB_ERROR_NONE) return err;

    return FB_ERROR_NONE;
}

/// @brief Counts active neighbors for the cell at coordinates (x_coord,y_coord).
/// @param field The field to read neighbors from.
/// @param x_coord The x coordinate for the requested cell.
/// @param y_coord The y coordinate for the requested cell.
/// @param counter The counter to increment on active neighbor found.
/// @return Error
fb_error_code_t fb_count_active_nbs(
    fb_field2d_t* field,
    fb_field_size_t x_coord,
    fb_field_size_t y_coord,
    fb_field_size_t* counter
) {
    fb_field_size_t nh_diam = FB_NH_DIAM_2D(field->nh_radius);
    switch (field->nh_type) {
        case FB_MOORE:
        case FB_V_NEUMANN:
            for (fb_field_size_t j = 0; j < nh_diam; j++) {
                for (fb_field_size_t i = 0; i < nh_diam; i++) {
                    int dx = abs(i - field->nh_radius);
                    int dy = abs(j - field->nh_radius);
                    if (field->nh_type == FB_V_NEUMANN && dx + dy > field->nh_radius) continue;

                    int nb_x = fb_mod(x_coord + (i - field->nh_radius), field->width);
                    int nb_y = fb_mod(y_coord + (j - field->nh_radius), field->height);

                    // Skip the current cell itself in neighbors count.
                    if (nb_x == x_coord && nb_y == y_coord) continue;

                    if (field->data[FB_IDX2D(nb_x, nb_y, field->width)]) (*counter)++;
                }
            }
            break;
        case FB_PERIMETER:
            for (fb_field_size_t j = 0; j < nh_diam; j++) {
                for (fb_field_size_t i = 0; i < nh_diam; i++) {
                    if (i > 0 && i < nh_diam - 1 && j > 0 && j < nh_diam - 1) continue;

                    int nb_x = fb_mod(x_coord + (i - field->nh_radius), field->width);
                    int nb_y = fb_mod(y_coord + (j - field->nh_radius), field->height);

                    // Skip the current cell itself in neighbors count.
                    if (nb_x == x_coord && nb_y == y_coord) continue;

                    if (field->data[FB_IDX2D(nb_x, nb_y, field->width)]) (*counter)++;
                }
            }
        default:
            break;
    }
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
            fb_field_size_t active_nbs_count = 0;
            fb_error_code_t error = fb_count_active_nbs(prev_field, i, j, &active_nbs_count);

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
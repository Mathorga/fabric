#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// #include <raylib.h>

/// Computes a neighborhood diameter given its radius.
#define FB_NH_DIAM_2D(r) (2 * (r) + 1)

typedef uint64_t fb_field_size_t;
typedef uint8_t fb_cell_state_t;
typedef uint8_t fb_nh_radius_t;

// Using a 32 bit integer ensures a neighborhood radius of 255 (the maximum 8-bit value) is still supported.
typedef uint32_t fb_nh_count_t;

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

/// @brief Sets the field rules from a rulestring.
/// Rulestring is adapted from the standard S/B notation, but changed in order to account for larger neighborhood radiuses.
/// Rulestrings always present rules for both S (survival) and B (birth), even if there's no survival or birth condition at all.
/// Rulestrings are also required to specify a neighborhood radius R before survival and birth conditions.
/// Survival and birth states are separated by commas in order to make strings more readable with neighborhood radiuses greater than 1.
/// Rules sections (radius, survival and birth) of the string are separated by a forward slash character.
/// The notorious Conway's Game of Life ruleset can therefore be expressed as R:1/S:2,3/B:3, meaning it works with a neighborhood radius of 1,
/// active cells stay active if 2 or 3 neighbors are active and inactive cells only become active if exactly 3 neighbors are active.
int f2d_set_rules(
    fb_field2d_t* field,
    char* rulestr
) {
    char* r_str = strtok(rulestr, "/");
    if (r_str == NULL) return 1;
    char* s_str = strtok(NULL, "/");
    if (s_str == NULL) return 1;
    char* b_str = strtok(NULL, "/");
    if (b_str == NULL) return 1;

    // Read radius.
    char* r_valstr = r_str + 2;
    fb_nh_radius_t r_val = atoi(r_valstr);

    // Read survival conditions.
    fb_field_size_t s_conds_count = 0;
    fb_field_size_t* s_conds = (fb_field_size_t*) malloc(100 * sizeof(fb_field_size_t));
    char* s_valstr = strtok(s_str + 2, ",");
    while (s_valstr != NULL) {
        s_conds_count++;
        s_conds[s_conds_count - 1] = atoi(s_valstr);
        s_valstr = strtok(NULL, ",");
    }

    field->s_conds_count = s_conds_count;
    field->s_conds = (fb_field_size_t*) malloc(s_conds_count * sizeof(fb_field_size_t));
    memcpy(field->s_conds, s_conds, s_conds_count * sizeof(fb_field_size_t));

    // Read birth conditions.
    fb_field_size_t b_conds_count = 0;
    fb_field_size_t* b_conds = (fb_field_size_t*) malloc(100 * sizeof(fb_field_size_t));
    char* b_valstr = strtok(b_str + 2, ",");
    while (b_valstr != NULL) {
        b_conds_count++;
        b_conds[b_conds_count - 1] = atoi(b_valstr);
        b_valstr = strtok(NULL, ",");
    }

    field->b_conds_count = b_conds_count;
    field->b_conds = (fb_field_size_t*) malloc(b_conds_count * sizeof(fb_field_size_t));
    memcpy(field->b_conds, b_conds, b_conds_count * sizeof(fb_field_size_t));
}

void main() {
    fb_field2d_t* field = (fb_field2d_t*) malloc(sizeof(fb_field2d_t));
    field->width = 100;
    field->height = 100;
    field->data = (fb_cell_state_t*) malloc(field->width * field->height * sizeof(fb_cell_state_t));
    char rulestr[] = "R:1/S:2,3/B:3";
    f2d_set_rules(field, rulestr);
}
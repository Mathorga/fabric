/// Compile with:
/// mkdir -p ./bin && gcc-15 -g -I./src -I./libs/raylib/raylib-6.0_macos/include ./libs/raylib/raylib-6.0_macos/lib/libraylib.a -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -fopenmp ./src/test.c -o ./bin/test
///
/// Run with:
/// ./bin/test
///
/// Interesting rulestrings:
/// R1/S2,3/B3/NM (Conway's Game of Life)
/// R1/S2,3,6/B3/NM
/// R1/S5,6,7,8/B3,5,6,7,8/NM (Diamoeba)
/// R1/S2,4,6,7,8/B3,5,7,8/NM (Geology)
/// R1/S3,5,6,7,8/B4,6,7,8/NM (Anneal)
/// R2/S3,5,7/B6,7,9/NM
/// R2/S3,5,8/B6,7,9/NM
/// R2/S10,11,13/B6,7,9/NM
/// R2/S9,10,13/B5,8,9/NM
/// R3/S20,21/B10,11,12,14,15,23,30/NM
/// R3/S20,21/B10,11,12,14,15,23,31,38,42/NM
/// R4/S12,20/B11,14,15,23,31,38,45/NM
/// R1/S2/B2,3/NN
/// R1/S2/B2,3,4/NN
/// R1/S2,3/B2,4/NN "Mondrian"
/// R1/S1,4/B2,3/NN "Burn it all"
/// R2/S2/B3/NN
/// R4/S8,9,10,11,15/B9,10,13,19/NN

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <raylib.h>

#include "field.h"

#define FIELD_WIDTH 500u
#define FIELD_HEIGHT 500u
#define WINDOW_WIDTH 500u
#define WINDOW_HEIGHT 500u
#define ACTIVE_COLOR RAYWHITE
#define INACTIVE_COLOR BLACK

fb_error_code_t draw_field(
    fb_field2d_t* field,
    int window_width,
    int window_height
) {
    const int cell_width = window_width / field->width;
    const int cell_height = window_height / field->height;

    // Draw cells.
    for (fb_field_size_t i = 0; i < field->width * field->height; i++) {
        // Read current cell state.
        fb_cell_state_t cell_state = field->data[i];

        fb_field_size_t cell_location_x = i % field->width;
        fb_field_size_t cell_location_y = i / field->height;

        DrawRectangle(
            cell_location_x * cell_width,
            cell_location_y * cell_height,
            cell_width,
            cell_height,
            cell_state == FB_ACTIVE ? ACTIVE_COLOR : INACTIVE_COLOR
        );
    }

    return FB_ERROR_NONE;
}

void main(int argc, char* argv[]) {
    srand(time(NULL));

    fb_field2d_t* even_field;
    char rulestr[] = "R1/S2,3/B3/NM";
    fb_error_code_t err = f2d_rcreate(&even_field, FIELD_WIDTH, FIELD_HEIGHT, argc > 1 ? argv[1] : rulestr);
    if (err != FB_ERROR_NONE) {
        printf("ERROR creating field: %u\n", err);
        return;
    }

    fb_field2d_t* odd_field;
    err = f2d_create_from(&odd_field, even_field);
    if (err != FB_ERROR_NONE) {
        printf("ERROR cloning field: %u\n", err);
        return;
    }
    InitWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "Fabric test"
    );

    const int canvas_width = even_field->width;
    const int canvas_height = even_field->height;
    RenderTexture2D canvas = LoadRenderTexture(
        canvas_width,
        canvas_height
    );
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);

    SetTargetFPS(0);

    for (uint64_t i = 0; !WindowShouldClose(); i++) {
        fb_field2d_t* prev_field = i % 2 ? odd_field : even_field;
        fb_field2d_t* next_field = i % 2 ? even_field : odd_field;

        fb_error_code_t err = f2d_tick(prev_field, next_field);

        BeginTextureMode(canvas);
            ClearBackground(BLACK);
            
            draw_field(
                prev_field,
                canvas_width,
                canvas_height
            );
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);

            // Source rectangle: The entire canvas texture. 
            // *CRITICAL*: The negative height flips the texture upright.
            Rectangle source_rec = { 
                0.0f, 
                0.0f, 
                (float) canvas.texture.width, 
                -(float) canvas.texture.height 
            };

            // Destination rectangle: Where on the screen to draw it and how big.
            // We set it to the screen width and height to scale it.
            Rectangle dest_rec = { 
                0.0f, 
                0.0f, 
                (float) WINDOW_WIDTH, 
                -(float) WINDOW_HEIGHT
            };

            // Origin is the rotation pivot point (top-left is 0,0).
            Vector2 origin = {0.0f, 0.0f};

            // Draw the texture scaled to destRec.
            DrawTexturePro(
                canvas.texture,
                source_rec,
                dest_rec,
                origin,
                0.0f,
                WHITE
            );

            DrawFPS(10, 10);
            
        EndDrawing();
    }

    CloseWindow();
}
#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "tutorial_stb-image_library_examples/stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tutorial_stb-image_library_examples/stb_image/stb_image_write.h"

//#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Function to calculate Roberts Cross gradient
void robert_cross(unsigned char* input, unsigned char* output, int width, int height, int channels) {
    for (int y = 0; y < height - 1; y++) {
        for (int x = 0; x < width - 1; x++) {
            int idx = y * width + x;

            // Get neighboring pixel intensities
            unsigned char P1 = input[idx * channels];
            unsigned char P2 = input[(idx + 1) * channels];
            unsigned char P3 = input[(idx + width) * channels];
            unsigned char P4 = input[(idx + width + 1) * channels];

        /*
            // Calculate Roberts Cross gradients + MAX assigner
            int Gx = abs(I1 - I4);
            int Gy = abs(I2 - I3);

            // Combine gradients
            output[idx] = (unsigned char)(MAX(Gx, Gy));
        */

            // Calculate Roberts Cross gradients as merged kernel
            output[idx] = abs(P1 - P4) + abs(P2 - P3);
        }
    }
}

int main() {
    // Load the input image
    int width, height, channels;
    unsigned char *input_image = stbi_load("bike.jpg", &width, &height, &channels, 0);
    if (input_image == NULL) {
        printf("Error: Could not load the image.\n");
        return 1;
    }

    // Convert image to grayscale if it's not already
    if (channels != 1) {
        printf("Converting image to grayscale...\n");
        unsigned char *gray_image = (unsigned char *)malloc(width * height);
        for (int i = 0; i < width * height; i++) {
            int r = input_image[i * channels];
            int g = input_image[i * channels + 1];
            int b = input_image[i * channels + 2];
            gray_image[i] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
        }
        free(input_image);
        input_image = gray_image;
        channels = 1;

        
        stbi_write_png("grayscale.jpg", width, height, 1, input_image, width);
    }
    // Allocate memory for the output image
    unsigned char *filtered_image = (unsigned char *)malloc(width * height);
    if (filtered_image == NULL) {
        printf("Error: Could not allocate memory for the filtered image.\n");
        free(input_image);
        return 1;
    }

    // Apply Roberts Cross edge detection
    robert_cross(input_image, filtered_image, width, height, channels);
    stbi_write_png("output_image.jpg", width, height, 1, filtered_image, width);

    // Free resources
    free(input_image);
    free(filtered_image);

    return 0;
}

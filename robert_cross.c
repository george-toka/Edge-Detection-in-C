#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "tutorial_stb-image_library_examples/stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tutorial_stb-image_library_examples/stb_image/stb_image_write.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Function to calculate Roberts Cross gradient
void roberts_cross(unsigned char* input, unsigned char* output, int width, int height, int channels) {
    for (int y = 0; y < height - 1; y++) {
        for (int x = 0; x < width - 1; x++) {
            int idx = y * width + x;

            // Get neighboring pixel intensities
            unsigned char I1 = input[idx * channels];
            unsigned char I2 = input[(idx + 1) * channels];
            unsigned char I3 = input[(idx + width) * channels];
            unsigned char I4 = input[(idx + width + 1) * channels];

            // Calculate Roberts Cross gradients
            int Gx = abs(I1 - I4);
            int Gy = abs(I2 - I3);

            // Combine gradients
            unsigned char magnitude = (unsigned char)(MAX(Gx, Gy));
            
            // Write the output (grayscale, so only one channel)
            output[idx] = magnitude;
        }
    }
}

int main() {
    // Load the input image
    int width, height, channels;
    unsigned char *input_image = stbi_load("one.jpg", &width, &height, &channels, 0);
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

    unsigned char *output_image = (unsigned char *)malloc(width * height);

    // Apply Roberts Cross edge detection
    roberts_cross(input_image, output_image, width, height, channels);
    stbi_write_png("output_image.jpg", width, height, 1, output_image, width);

    // Free resources
    free(output_image);
    stbi_image_free(input_image);

    return 0;
}
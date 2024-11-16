#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "tutorial_stb-image_library_examples/stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tutorial_stb-image_library_examples/stb_image/stb_image_write.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// Function to calculate Prewitt operator
void applyPrewitt(unsigned char *input, unsigned char *output, int width, int height, int threshold);

// Function to calculate Roberts Cross operator
void apply_robert_cross(unsigned char* input, unsigned char* output, int width, int height, int channels);

int main() {
    // Load the input image
    int width, height, channels;
    unsigned char *input_image = stbi_load("WHO'S THAT POKEMON.jpg", &width, &height, &channels, 0);
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
    // Apply Prewitt edge detection
    int threshold = 100;

    // Choose between one of the two 
    //applyPrewitt(input_image, filtered_image, width, height, threshold);
    apply_robert_cross(input_image, filtered_image, width, height, channels);

    // Save the output image
    stbi_write_png("output_image.jpg", width, height, 1, filtered_image, width);
    
    // Free memory
    free(input_image);
    free(filtered_image);

    printf("Edge detection completed. Output saved as 'output_image.jpg'.\n");

    return 0;
}


void applyPrewitt(unsigned char *input, unsigned char *output, int width, int height, int threshold) {
    // Prewitt Operator Masks
    int Mx[3][3] = {
        {-1, 0, 1},
        {-1, 0, 1},
        {-1, 0, 1}
    };
    int My[3][3] = {
        {-1, -1, -1},
        { 0,  0,  0},
        { 1,  1,  1}
    };
    
    // Iterate over each pixel (excluding the borders)
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int Gx = 0;
            int Gy = 0;

            // Apply the masks to calculate Gx and Gy
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int pixel = input[(y + ky) * width + (x + kx)];
                    Gx += pixel * Mx[ky + 1][kx + 1];
                    Gy += pixel * My[ky + 1][kx + 1];
                }
            }

            // Calculate the magnitude of the gradient
            int magnitude = (int)sqrt(Gx * Gx + Gy * Gy);
            
            // Thresholding to enhance edges
            output[y * width + x] = (magnitude >= threshold) ? 255 : 0;
        }
    }
}

void apply_robert_cross(unsigned char* input, unsigned char* output, int width, int height, int channels) {
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
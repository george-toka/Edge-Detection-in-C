#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Function prototypes */
unsigned char *readPPM(const char *filename, int *width, int *height, int *channels);
void writePPM(const char *filename, unsigned char *data, int width, int height, int channels);
void convertToGrayscale(unsigned char *input, unsigned char *output, int width, int height, int channels);
void applyPrewitt(unsigned char *input, unsigned char *output, int width, int height, int threshold);
void applyRobertsCross(unsigned char *input, unsigned char *output, int width, int height);

/* Main function */
int main() {
    int width, height, channels;
    int threshold;
    unsigned char *input_image;
    unsigned char *gray_image;
    unsigned char *filtered_image;

    /* Read the input image */
    input_image = readPPM("input.ppm", &width, &height, &channels);
    if (input_image == NULL) {
        printf("Error: Could not load the image.\n");
        return 1;
    }

    /* Convert to grayscale if not already */
    gray_image = (unsigned char *)malloc(width * height);
    if (channels != 1) {
        printf("Converting image to grayscale...\n");
        convertToGrayscale(input_image, gray_image, width, height, channels);
        free(input_image);
        input_image = gray_image;
        channels = 1;

        /* Save the grayscale image for debugging */
        writePPM("grayscale.ppm", input_image, width, height, channels);
    }

    /* Allocate memory for the filtered image */
    filtered_image = (unsigned char *)malloc(width * height);
    if (filtered_image == NULL) {
        printf("Error: Could not allocate memory for the filtered image.\n");
        free(input_image);
        return 1;
    }

    /* Apply edge detection */
    threshold = 100;
    printf("Applying Prewitt edge detection...\n");
    applyPrewitt(input_image, filtered_image, width, height, threshold);

    /* Save the Prewitt output image */
    writePPM("prewitt_output.ppm", filtered_image, width, height, channels);

    printf("Applying Roberts Cross edge detection...\n");
    applyRobertsCross(input_image, filtered_image, width, height);

    /* Save the Roberts Cross output image */
    writePPM("roberts_output.ppm", filtered_image, width, height, channels);

    /* Free memory */
    free(input_image);
    free(filtered_image);

    printf("Edge detection completed. Outputs saved as 'prewitt_output.ppm' and 'roberts_output.ppm'.\n");
    return 0;
}

/* Function to read a PPM file */
unsigned char *readPPM(const char *filename, int *width, int *height, int *channels) {
    FILE *file;
    char format[3];
    int max_value;
    int pixel_count;
    unsigned char *data;

    file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    /* Read PPM header */
    if (fscanf(file, "%2s", format) != 1 || (format[0] != 'P' || format[1] != '6')) {
        printf("Error: Unsupported file format.\n");
        fclose(file);
        return NULL;
    }

    fscanf(file, "%d %d %d", width, height, &max_value);
    if (max_value != 255) {
        printf("Error: Unsupported max value in PPM file.\n");
        fclose(file);
        return NULL;
    }

    fgetc(file); /* Consume the newline after the header */

    /* Allocate memory for pixel data */
    pixel_count = (*width) * (*height);
    *channels = 3; /* PPM is RGB */
    data = (unsigned char *)malloc(pixel_count * (*channels));
    if (data == NULL) {
        printf("Error: Could not allocate memory.\n");
        fclose(file);
        return NULL;
    }

    /* Read pixel data */
    fread(data, 1, pixel_count * (*channels), file);
    fclose(file);
    return data;
}

/* Function to write a PPM file */
void writePPM(const char *filename, unsigned char *data, int width, int height, int channels) {
    FILE *file;

    file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file");
        return;
    }

    /* Write the PPM header */
    fprintf(file, "P6\n%d %d\n255\n", width, height);

    /* Write pixel data */
    fwrite(data, 1, width * height * channels, file);
    fclose(file);
}

/* Function to convert an image to grayscale */
void convertToGrayscale(unsigned char *input, unsigned char *output, int width, int height, int channels) {
    int i;
    for (i = 0; i < width * height; i++) {
        int r = input[i * channels];
        int g = input[i * channels + 1];
        int b = input[i * channels + 2];
        output[i] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
    }
}

/* Function to apply Prewitt operator */
void applyPrewitt(unsigned char *input, unsigned char *output, int width, int height, int threshold) {
    int Mx[3][3] = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
    int My[3][3] = {{-1, -1, -1}, {0, 0, 0}, {1, 1, 1}};
    int x, y;

    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            int Gx = 0, Gy = 0, ky, kx, pixel, magnitude;

            for (ky = -1; ky <= 1; ky++) {
                for (kx = -1; kx <= 1; kx++) {
                    pixel = input[(y + ky) * width + (x + kx)];
                    Gx += pixel * Mx[ky + 1][kx + 1];
                    Gy += pixel * My[ky + 1][kx + 1];
                }
            }

            magnitude = (int)sqrt(Gx * Gx + Gy * Gy);
            if (magnitude > 255) magnitude = 255;
            output[y * width + x] = (magnitude >= threshold) ? 255 : 0;
        }
    }
}

/* Function to apply Roberts Cross operator */
void applyRobertsCross(unsigned char *input, unsigned char *output, int width, int height) {
    int x, y;

    for (y = 0; y < height - 1; y++) {
        for (x = 0; x < width - 1; x++) {
            int idx = y * width + x;

            int I1 = input[idx];
            int I2 = input[idx + 1];
            int I3 = input[idx + width];
            int I4 = input[idx + width + 1];

            int Gx = abs(I1 - I4);
            int Gy = abs(I2 - I3);

            int mag = (int)sqrt(Gx * Gx + Gy * Gy);
            if (mag > 255) mag = 255;
            output[idx] = mag;
        }
    }
}

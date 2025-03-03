#include <stdio.h>
#include <stdlib.h>

#define width 640
#define height 480
#define im_size (width * height)

// we define the prototypes of the functions we are gonna use 

void readPPM(const char *filename);
void applyPrewitt(void);
void applyRobertsCross(void);

#pragma arm section rwdata="rw_ram"
unsigned char input_image[im_size], output_image[im_size];
int outim[im_size];

int i, max_value;
int channels = 1;
int P1, P2, P3, P4;
int iic, oic;

#pragma arm section 

int main() {
	 FILE *prewittFile;
	 FILE *robertsFile; 
    
    // read the file, check if we cant read it
    readPPM("C:/SOC_lab1/lab3/grayscale.ppm");
    if (input_image == NULL) {
        printf("Error: Could not load the image.\n");
        return 1;
    }

    // we apply prewitt
    printf("Applying Prewitt edge detection...\n");
    applyPrewitt();

    // write the prewitt output file 
    prewittFile = fopen("prewitt_output.ppm", "wb");
    if (!prewittFile) {
        perror("Error opening file for Prewitt output");
        return 1;
    }
    fprintf(prewittFile, "P5\n%d %d\n255\n", width, height);  // write header
    fwrite(output_image, 1, im_size, prewittFile);           // write pixel data
    fclose(prewittFile);									// close the file

    // apply roberts cross 
    printf("Applying Roberts Cross edge detection...\n");
    applyRobertsCross();

    // 
    robertsFile = fopen("roberts_output.ppm", "wb");
    if (!robertsFile) {
        perror("Error opening file for Roberts output");
        return 1;
    }
    fprintf(robertsFile, "P5\n%d %d\n255\n", width, height);  // Write header
    fwrite(output_image, 1, im_size, robertsFile);           // Write pixel data
    fclose(robertsFile);

    printf("Edge detection completed. Outputs saved as 'prewitt_output.ppm' and 'roberts_output.ppm'.\n");
    return 0;
}

//  func to read a PPM file 
void readPPM(const char *filename) {
    FILE *file;
    char format[2];

    file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
    }

    // read the PPM header 
    if (fscanf(file, "%2c", format) != 1 || format[0] != 'P' || format[1] != '5' ) {
        printf("Error: Unsupported file format.\n");
        fclose(file);
    }

    // get the max value
    fscanf(file, "%*d %*d %d", &max_value);
    if (max_value != 255) {
        printf("Error: Unsupported max value in PPM file.\n");
        fclose(file);
    }

    // read the pixel data 
    fread(input_image, 1,im_size, file);
    fclose(file);
}


// func to apply Prewitt operator
void applyPrewitt() {
    int idx = width + 1;
    int f_id = im_size - idx - 1;
    max_value = 1;
    iic = oic = 0;
    do{ 
        int I1 = input_image[idx - width - 1];
        int I2 = input_image[idx - width];
        int I3 = input_image[idx - width + 1];
        int I4 = input_image[idx - 1];
        int I5 = input_image[idx];
        int I6 = input_image[idx + 1];
        int I7 = input_image[idx + width - 1];
        int I8 = input_image[idx + width];
        int I9 = input_image[idx + width + 1];
        int I10 = input_image[idx - width + 2];
        int I11 = input_image[idx + 2];
        int I12 = input_image[idx + width + 2];

		iic += 12;

        P1 = I1 + I2 + I3 - I7 - I8 - I9;
        P2 = I1 + I4 + I7 - I3 - I6 - I9;
        P3 = I2 + I3 + I10 - I8 - I9 - I12;
        P4 = I2 + I5 + I8 - I10 - I11 - I12;
        
        outim[idx] = abs(P1) + abs(P2);
        outim[idx + 1] = abs(P3) + abs(P4);
        
        oic += 2; // because of the ifs
        
        if (outim[idx] > max_value) 
		{	
			max_value = outim[idx];
			oic += 1;
		}
        if (outim[idx + 1] > max_value) 
		{	
			max_value = outim[idx + 1];
			oic += 1;
		}
		
		
        idx += 2;       
    } while(idx < f_id );
    
    // now we normalize
 
    // each pixel to the range [0, 255]
    for (i = 0; i < im_size; i++) {
        output_image[i] = (unsigned char)((outim[i] * 255) / max_value);
        oic += 1;
    }
    
    printf("In Roberts Function we access the Input Image matrix: %d times\n", iic);
    printf("In Roberts Function we access the Output Image matrix: %d times\n", oic);
    
}


// func to  apply Roberts Cross operator 
void applyRobertsCross() { 
    int idx = 0;
    int f_id = im_size - width - 4;
    iic = oic = 0;
    max_value = 1;
    do{
        int I1 = input_image[idx];
        int I2 = input_image[idx + 1];
        int I3 = input_image[idx + width];
        int I4 = input_image[idx + width + 1];
        int I5 = input_image[idx + 2];
        int I6 = input_image[idx + width + 2];
        int I7 = input_image[idx + 3];
        int I8 = input_image[idx + width + 3];
        int I9 = input_image[idx + 4];
        int I10 = input_image[idx + width + 4];

       	iic += 10;
        
        outim[idx] = (abs(I1-I4) + abs(I2-I3));
        outim[idx + 1] = (abs(I2-I6) + abs(I5-I4));
        outim[idx + 2] = (abs(I5-I8) + abs(I7-I6));
        outim[idx + 3] = (abs(I7-I10) + abs(I9-I8));
        
        oic += 4;
        
        if (outim[idx] > max_value) 
        {
        	max_value = outim[idx];
        	oic += 1;
        }
        if (outim[idx + 1] > max_value) 
        {
        	max_value = outim[idx + 1];
        	oic += 1;
        }
        if (outim[idx + 2] > max_value) 
        {
        	max_value = outim[idx + 2];
        	oic += 1;
        }
        if (outim[idx + 3] > max_value) 
        {
        	max_value = outim[idx + 3];
        	oic += 1;
        }
        
        idx+=4;
        
    } while(idx < f_id);
    
    // each pixel to the range [0, 255]
    for (i = 0; i < im_size; i++) {
        output_image[i] = (unsigned char)((outim[i] * 255) / max_value);
        oic += 1;
    }
    
    printf("In Roberts Function we access the Input Image matrix: %d times\n", iic);
    printf("In Roberts Function we access the Output Image matrix: %d times\n", oic);
  
}



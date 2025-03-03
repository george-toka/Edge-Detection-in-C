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

int channels = 1;

#pragma arm section 

#pragma arm section zidata = "cache"
int buff[12];
int P1,P2,P3,P4;
int i, max_value, idx;
int iic, oic, bc;
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
    int f_id = im_size - idx - 1;
    max_value = 1;
    idx = width + 1;
	iic = oic = bc = 0;
    do{ 
        buff[0] = input_image[idx - width - 1]; 	// I1
        buff[1] = input_image[idx - width]; 		// I2
        buff[2] = input_image[idx - width + 1];	// I3
        buff[3] = input_image[idx - 1];			// I4
        buff[4] = input_image[idx];				// I5
        buff[5] = input_image[idx + 1];			// I6
        buff[6] = input_image[idx + width - 1];	// I7
        buff[7] = input_image[idx + width];		// I8
        buff[8] = input_image[idx + width + 1];	// I9
        buff[9] = input_image[idx - width + 2];	// I10
        buff[10] = input_image[idx + 2];			// I11
        buff[11] = input_image[idx + width + 2];	// I12
        
        iic += 12;

        P1 = buff[0] + buff[1] + buff[2] - buff[6] - buff[7] - buff[8];
        P2 = buff[0] + buff[3] + buff[6] - buff[2] - buff[5] - buff[8];
        P3 = buff[1] + buff[2] + buff[9] - buff[7] - buff[8] - buff[11];
        P4 = buff[1] + buff[4] + buff[7] - buff[9] - buff[10] - buff[11];

        bc += 24;
       
        
        outim[idx] = abs(P1) + abs(P2);
        outim[idx + 1] = abs(P3) + abs(P4);
        
        oic += 2;
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
    
    printf("In Prewitt Function we access the Input Image matrix: %d times\n", iic);
    printf("In Prewitt Function we access the Output Image matrix: %d times\n", oic);
   	printf("In Prewitt Function we access the Output Image matrix: %d times\n", bc);
   
}


// func to  apply Roberts Cross operator 
void applyRobertsCross() { 
    int f_id = im_size - width - 4;
    max_value = 1;
    idx = 0;
    iic = oic = bc = 0;
    do{
      
        buff[0] = input_image[idx]; 			// I1
        buff[1] = input_image[idx + 1];			// I2
        buff[2] = input_image[idx + width];		// I3
        buff[3] = input_image[idx + width + 1];	// I4
        buff[4] = input_image[idx + 2];			// I5
        buff[5] = input_image[idx + width + 2];	// I6
        buff[6] = input_image[idx + 3];			// I7
        buff[7] = input_image[idx + width + 3];	// I8
        buff[8] = input_image[idx + 4];			// I9
        buff[9] = input_image[idx + width + 4];	// I10
        
        outim[idx] = (abs(buff[0]-buff[3]) + abs(buff[1]-buff[2]));
        outim[idx + 1] = (abs(buff[1]-buff[5]) + abs(buff[4]-buff[3]));
        outim[idx + 2] = (abs(buff[4]-buff[7]) + abs(buff[6]-buff[5]));
        outim[idx + 3] = (abs(buff[6]-buff[9]) + abs(buff[8]-buff[7]));
        
        iic += 10;
        bc += 16;
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
    
    printf("In Prewitt Function we access the Input Image matrix: %d times\n", iic);
    printf("In Prewitt Function we access the Output Image matrix: %d times\n", oic);
  	printf("In Prewitt Function we access the Cache buffer matrix: %d times\n", bc);
  
  
}



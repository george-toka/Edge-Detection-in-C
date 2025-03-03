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

#pragma arm section zidata = "L1Cache"
unsigned char small_buff[12];
int P1,P2,P3,P4;
int i, max_value;
int idx;
int f_id;
int bigbuffsize;
int cache_load_rep;
int bbi;
int t;
int temp1, temp2;
#pragma arm section

#pragma arm section zidata = "L2Cache"
unsigned char big_buff[3*width+1];
int out_buff[width-1];
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
    idx = width + 1;
    f_id = im_size - idx - 1;
    bigbuffsize = 3 * width;
    cache_load_rep = 3;
    bbi = width + 1;
    t = 0;
    max_value = 1;
    
    // load the first three lines
    for (i = 0; i < bigbuffsize; i++) {
                big_buff[i] = input_image[i];
    }

    do{ 

        temp1 = bbi + width;
        temp2 = bbi - width;

        small_buff[0] = big_buff[temp2 - 1]; 	// I1
        small_buff[1] = big_buff[temp2]; 		// I2
        small_buff[2] = big_buff[temp2 + 1];	// I3
        small_buff[3] = big_buff[bbi - 1];			// I4
        small_buff[4] = big_buff[bbi];				// I5
        small_buff[5] = big_buff[bbi + 1];			// I6
        small_buff[6] = big_buff[temp1 - 1];	// I7
        small_buff[7] = big_buff[temp1];		// I8
        small_buff[8] = big_buff[temp1 + 1];	// I9
        small_buff[9] = big_buff[temp2 + 2];	// I10
        small_buff[10] = big_buff[bbi + 2];			// I11
        small_buff[11] = big_buff[temp1 + 2];	// I12
        bbi += 2;

        P1 = small_buff[0] + small_buff[1] + small_buff[2] - small_buff[6] - small_buff[7] - small_buff[8];
        P2 = small_buff[0] + small_buff[3] + small_buff[6] - small_buff[2] - small_buff[5] - small_buff[8];
        P3 = small_buff[1] + small_buff[2] + small_buff[9] - small_buff[7] - small_buff[8] - small_buff[11];
        P4 = small_buff[1] + small_buff[4] + small_buff[7] - small_buff[9] - small_buff[10] - small_buff[11];

        out_buff[temp2-1] = abs(P1) + abs(P2);
        out_buff[temp2] = abs(P3) + abs(P4);
        
        if (out_buff[temp2-1] > max_value) max_value = out_buff[temp2-1];
        if (out_buff[temp2] > max_value) max_value = out_buff[temp2];
        idx += 2;    

        //push last two lines in the first two lines and bring last line from ram
        if(idx % width == 1) {
            t = cache_load_rep * width;
            for (i = 0; i < bigbuffsize-width; i++) {
                big_buff[i] = big_buff[i + width];
            }
            for (i = bigbuffsize-width; i < bigbuffsize; i++) {
            	big_buff[i] = input_image[t + i];
            }

            for (i = 0; i < width-2; i++){
                outim[width * (cache_load_rep - 2) + i + 1] = out_buff[i];
            }
            
            cache_load_rep += 1;
            bbi = width + 1;
        }
        
    } while(idx < f_id );
    
    // now we normalize
 
    // each pixel to the range [0, 255]
    for (i = 0; i < im_size; i++) {
        output_image[i] = (unsigned char)((outim[i] * 255) / max_value);
    }
    
    
}


// func to  apply Roberts Cross operator 
void applyRobertsCross() { 
    idx = 0;
    f_id = im_size - width - 4;
    bigbuffsize = 2 * width;
    cache_load_rep = 2;
    bbi = 0;
    t = 0;
    
    max_value = 1;
    
    // load the first three lines
    for (i = 0; i < bigbuffsize; i++) {
                big_buff[i] = input_image[i];
    }

    do{ 
		

        temp1 = bbi + width;
        small_buff[0] = big_buff[bbi]; 			// I1
        small_buff[1] = big_buff[bbi + 1];			// I2
        small_buff[2] = big_buff[temp1];		// I3
        small_buff[3] = big_buff[temp1 + 1];	// I4
        small_buff[4] = big_buff[bbi + 2];			// I5
        small_buff[5] = big_buff[temp1 + 2];	// I6
        small_buff[6] = big_buff[bbi + 3];			// I7
        small_buff[7] = big_buff[temp1 + 3];	// I8
        small_buff[8] = big_buff[bbi + 4];			// I9
        small_buff[9] = big_buff[temp1 + 4];	// I10
        
        

        out_buff[bbi] = (abs(small_buff[0]-small_buff[3]) + abs(small_buff[1]-small_buff[2]));
        out_buff[bbi + 1] = (abs(small_buff[1]-small_buff[5]) + abs(small_buff[4]-small_buff[3]));
        out_buff[bbi + 2] = (abs(small_buff[4]-small_buff[7]) + abs(small_buff[6]-small_buff[5]));
        out_buff[bbi + 3] = (abs(small_buff[6]-small_buff[9]) + abs(small_buff[8]-small_buff[7]));
        
        if (out_buff[bbi] > max_value) max_value = out_buff[bbi];
        if (out_buff[bbi + 1] > max_value) max_value = out_buff[bbi + 1];
        if (out_buff[bbi + 2] > max_value) max_value = out_buff[bbi + 2];
        if (out_buff[bbi + 3] > max_value) max_value = out_buff[bbi + 3];
        
        bbi += 4;
        idx+=4;

        //push last two lines in the first two lines and bring last line from ram
        if(idx % width == 0) {
            t = cache_load_rep * width;
            for (i = 0; i < bigbuffsize-width; i++) {
                big_buff[i] = big_buff[i + width];          
            }
            for (i = bigbuffsize-width; i < bigbuffsize; i++) {
            	big_buff[i] = input_image[t + i];
                
            }
            for (i = 0; i < width-1; i++){
                outim[width * (cache_load_rep - 2) + i] = out_buff[i];
            }

            cache_load_rep += 1;
            bbi = 0;
        }

    } while(idx < f_id);
    
    // each pixel to the range [0, 255]
    for (i = 0; i < im_size; i++) {
        output_image[i] = (unsigned char)((outim[i] * 255) / max_value);
    }
}



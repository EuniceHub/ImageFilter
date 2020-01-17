#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*Author: Eunice Huang  studentnumber: 400092172    macid: huangb16*/
//readppm, writeppm and read_filter refer to the 2017 2s03 assignment 4 solution


typedef struct {
    int r, g, b;
}RGB;

void writePPM(const char* file, int width, int height, int max, const RGB* image){

    int i;

    // open file for writing
    FILE* fd;
    fd = fopen(file, "w");

    // output the header
    fprintf(fd, "P3\n");
    fprintf(fd, "%d %d\n%d\n", width, height, max);

    for (i = 0; i < height * width; i++) {
        const RGB* p = image + i;
        fprintf(fd, "%d %d %d ", p->r, p->g, p->b);
    }
    fclose(fd);
}

int* read_filter(const char* file, int* n, int* scale){
    FILE* fd;
    int count;
    int content;
    int i, j;

    fd = fopen(file, "r");
    count = fscanf(fd, "%d", n); //read value of n
    count = fscanf(fd, "%d", scale); //read value of scale
    
    int size = (*n) * (*n);
    int* filter = (int*)malloc(size * sizeof(int));
    assert(filter);
    for (i = 0; i < size; i++) {
        count = fscanf(fd, "%d", &content);
        assert(count == 1);
        filter[i] = content;
    }
    /*if(scale != 0){
        for(j = 0; j < size; j++){
            filter[j] = filter[j]/(*scale);
            printf("%d\n", *scale);
        }
    }*/
    fclose(fd);
    return filter;
    
}

RGB* readPPM(const char* file, int* width, int* height, int* max)
{
    /* Read a PPM P3 image from a file into a buffer.  Return the
     buffer, width, height, and the max value in the image. */

    FILE* fd;
    char c;
    int i, n;
    char b[100];
    int red, green, blue;

    // check if P3 ppm format
    fd = fopen(file, "r");
    n = fscanf(fd, "%[^\n] ", b);
    if (b[0] != 'P' || b[1] != '3') {
        printf("%s is not a PPM file!\n", file);
        exit(0);
    }
    //  printf("%s is a PPM file\n", file);
    n = fscanf(fd, "%c", &c);
    while (c == '#') {
        n = fscanf(fd, "%[^\n] ", b);
        printf("%s\n", b);
        n = fscanf(fd, "%c", &c);
    }
    ungetc(c, fd);
    n = fscanf(fd, "%d %d %d", width, height, max);
    assert(n == 3);

    //  printf("%d x %d image, max value= %d\n", *width, *height, *max);

    // size of image
    int size = *width * (*height);

    RGB* image = (RGB*)malloc(size * sizeof(RGB));
    assert(image);

    for (i = 0; i < size; i++) {
        n = fscanf(fd, "%d %d %d", &red, &green, &blue);
        assert(n == 3);
        image[i].r = red;
        image[i].g = green;
        image[i].b = blue;
    }

    fclose(fd);
    return image;
}


//size is the size of the filter, i and j is the location of the reference mid point, width and height is the width and height of the input image
RGB *generate_basematrix(RGB *pic, int size, int i, int j, int width, int height){
    RGB *base = malloc(size*size*sizeof(RGB));
    int upperbound = i - size/2;
    int lowerbound = i + size/2;
    int leftbound = j - size/2;
    int rightbound = j + size/2; //finding range of index for the basematrix
    int counter = 0;
    
    int row, col;
    //check if the newly generated base according to the given mid point is within the valid boundary.
    for(row = upperbound; row <= lowerbound; row++){
        for(col = leftbound; col <= rightbound; col++){
            if(row < 0 || row > height - 1 || col < 0 || col > width - 1){
                base[counter].r = 0;
                base[counter].g = 0;
                base[counter].b = 0;
                counter++;
            }
            else{
                base[counter].r = pic[row * width + col].r;
                base[counter].g = pic[row * width + col].g;
                base[counter].b = pic[row * width + col].b;
                counter++;
            }
        }
    }
    return base;
}

//size-wise, base and filter is the same, which is the n
//multiply the base reversely with the filter
RGB *multiplication(RGB *base, int *filter, int n){
    RGB *new_node = malloc(sizeof(RGB));
    int i, length;
    length = n*n;
    new_node->r = 0;
    new_node->g = 0;
    new_node->b = 0;
    for(i = 0; i < n*n; i++){
        new_node->r = new_node->r + (base[length - 1 - i].r * filter[i]);
        new_node->g = new_node->g + (base[length - 1 - i].g * filter[i]);
        new_node->b = new_node->b + (base[length - 1 - i].b * filter[i]);
    }
    return new_node; //the result after being convoluted.
}


//width of the image, height of the image. n if the row or column of the filter
RGB *process_image(int width, int height, RGB *image, int *filter, int n, int scale){
    int size = width * height;
    int index = 0;
    RGB *new_image = malloc(size*sizeof(RGB));
    RGB *base, *tmp;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            base = generate_basematrix(image, n, i, j, width, height);
            tmp = multiplication(base, filter, n);
            new_image[index].r = tmp->r;
            new_image[index].b = tmp->b;
            new_image[index].g = tmp->g;
            
            new_image[index].r /= scale;
            new_image[index].b /= scale;
            new_image[index].g /= scale;
            index++;
        }
    }
    free(base);
    free(tmp);
    return new_image;
}

int main(int argc, const char* argv[]) {
    const char *input, *kernel, *output;
    input = argv[1];
    kernel = argv[2];
    output = argv[3];
    
    int w, h, max, n, scale;
    int *filter = read_filter(kernel, &n, &scale);
    
    RGB* input_file = readPPM(input, &w, &h, &max);
    RGB* output_file = process_image(w, h, input_file, filter, n, scale);
    writePPM(output, w, h, max, output_file);
    
    free(filter);
    free(input_file);
    free(output_file);
    
    return 0;
}


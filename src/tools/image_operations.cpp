#define PNG_DEBUG (3)
#include <png.h>
#include <iostream>
#include <cstdlib>

typedef struct
{
    png_structp png_ptr;
    png_infop info_ptr;
    int width, height, number_of_passes;
    unsigned char color_type, bit_depth;
    png_bytep* row_pointers;
} Image;

bool read_png_file(char* file, Image* image)
{
    bool value = false;

    //Keep space for the file header.
    png_byte header[8];

    //Try to open the file.
    FILE* fp = fopen(file, "rb");
    if(fp != NULL)
    {
        //Read the first 8 bytes of the file.
        fread(header, 1, 8, fp);

        //Check the header.
        if(!png_sig_cmp(header, 0, 8))
        {
            image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if(image->png_ptr != NULL)
            {
                image->info_ptr = png_create_info_struct(image->png_ptr);
                if(image->info_ptr != NULL)
                {
                    if(setjmp(png_jmpbuf(image->png_ptr)))
                    {
                        std::cerr << "Error during read initialization." << std::endl;
                    }
                    else
                    {
                        //Now let's begin to read the image.
                        png_init_io(image->png_ptr, fp);
                        png_set_sig_bytes(image->png_ptr, 8);
                        png_read_info(image->png_ptr, image->info_ptr);
                        
                        image->width = png_get_image_width(image->png_ptr, image->info_ptr);
                        image->height = png_get_image_height(image->png_ptr, image->info_ptr);
                        image->color_type = png_get_color_type(image->png_ptr, image->info_ptr);
                        image->bit_depth = png_get_bit_depth(image->png_ptr, image->info_ptr);

                        image->number_of_passes = png_set_interlace_handling(image->png_ptr);
                        png_read_update_info(image->png_ptr, image->info_ptr);

                        if(setjmp(png_jmpbuf(image->png_ptr)))
                        {
                            std::cerr << "Error during read." << std::endl;
                        }
                        else
                        {
                            //Allocate space for image.
                            image->row_pointers = new png_bytep[image->height];
                            for(int y = 0; y < image->height; ++y)
                            {
                                (image->row_pointers)[y] = (png_byte*)malloc(png_get_rowbytes(image->png_ptr, image->info_ptr));
                            }

                            png_read_image(image->png_ptr, image->row_pointers);
                            value = true;
                            fclose(fp);
                        }
                    }
                }
                else
                { 
                    std::cerr << "Info struct creation failed." << std::endl;
                }
            }
            else
            {
                std::cerr << "Creation of read struct failed." << std::endl;
            }
        }
        else
        {
            std::cerr << "The file (" << file << ") does not appear to be png." << std::endl;
        }
    }
    else
    {
       std::cerr << "The file (" << file << ") could not be opened." << std::endl;
    }

    return value;
}

void deleteImage(Image* image)
{
    for(int y = 0; y < image->height; ++y)
    {
        free((image->row_pointers)[y]);
    }
    delete[] image->row_pointers;
}

void compare_images(Image* im1, Image* im2)
{
    //Check the width and height.
    if( (im1->height == im2->height) && (im1->width == im2->width) )
    {
        //Keep a counter for the number of different pixels.
        int differing = 0;

        //Now, check each of the values in the image for equality.
        for(int row = 0; row < im1->height; ++row)
        {
            png_byte* im1r = (im1->row_pointers)[row];
            png_byte* im2r = (im2->row_pointers)[row];
            for(int column = 0; column < im1->width; ++column)
            {
                //Determine if the pixels are the same by comparing each of the RGB values.
                if( (im1r[0] != im2r[0]) || (im1r[1] != im2r[1])
                    || (im1r[2] != im2r[2]) )
                {
                    //We have encountered a difference here...
                    std::cout << "ERROR: Pixel (" << row << "," << column << ") is different.";
                    std::cout << " (R,G,B) values: 1.) (" << (int)im1r[0] << "," << (int)im1r[1] << "," << (int)im1r[2] << "); ";
                    std::cout << "2.) (" << (int)im2r[0] << "," << (int)im2r[1] << "," << (int)im2r[2] << ")" << std::endl;

                    //Mark it as a difference.
                    differing++;
                }

                //Update the pointers to the next pixel.
                im1r += 3; im2r += 3;
            }
        }

        //Print the summary.
        std::cout << std::endl << std::endl;
        std::cout << "Number of different pixels: " << differing << std::endl;
        std::cout << "Percent of image: " << (100.0 * differing / (im1->height * im1->width)) << "%" << std::endl;
    }
    else
    {
        std::cout << "ERROR: Images have different dimensions" << std::endl;
    }
}

int main(int argc, char* argv[])
{
    //Make sure the inputs are provided.
    if(argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " input1.png input2.png" << std::endl;
        return 1;
    }

    Image inputImage1, inputImage2;
    bool read1 = read_png_file(argv[1], &inputImage1);
    bool read2 = read_png_file(argv[2], &inputImage2);

    //Compare the images.
    compare_images(&inputImage1, &inputImage2);

    if(read1) deleteImage(&inputImage1);
    if(read2) deleteImage(&inputImage2);
    return 0;
}

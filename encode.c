#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"


/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    // return secret file size
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    rewind(fptr);
    return size;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

 Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // step 1 -> check if source image argv[2] has .bmp extension
    if (!has_extension(argv[2], ".bmp"))
    {
        printf("--------------------------------------------------\n");
        printf("ERROR: Invalid source image file '%s'.\n", argv[2]);
        printf("The source image file must have a '.bmp' extension.\n");
        printf("--------------------------------------------------\n");
        return e_failure;
    }
    encInfo->src_image_fname = argv[2];

    // step 2 -> check if source image file exists
    FILE *fp_src = fopen(argv[2], "r");
    if (!fp_src)
    {
        printf("--------------------------------------------------\n");
        printf("ERROR: Source image file '%s' does not exist.\n", argv[2]);
        printf("Please provide a valid source BMP file.\n");
        printf("--------------------------------------------------\n");
        return e_failure;
    }
    fclose(fp_src);

    // step 3 -> check if secret file argv[3] exists
    FILE *fp_secret = fopen(argv[3], "r");
    if (!fp_secret)
    {
        printf("--------------------------------------------------\n");
        printf("ERROR: Secret file '%s' does not exist.\n", argv[3]);
        printf("Please provide a valid secret file.\n");
        printf("--------------------------------------------------\n");
        return e_failure;
    }
    fclose(fp_secret);
    encInfo->secret_fname = argv[3];

    // step 4 -> check if optional output stego image argv[4] is provided
    if (argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        // default output filename if not provided
        encInfo->stego_image_fname = "stego.bmp";
    }

    return e_success;
}
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    // magic             =2*8=16
    //extension size     =4*8=32
    //extension          =4*8=32
    //secret size        =4*8=32
    //secret data        =9*8=72
    //step 1 -> encInfo -> image_capacity = call get_image_size_for_bmp(encoInfo -> fptr_src_image)
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    
    //step 2 -> encInfo -> size_secret_file = call get_file_size(encInfo -> fptr_secret)
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    //step 3 -> check encInfo -> image_capacity > 16 + 32 + (strlen(encInfo -> extn_secret_file) * 8) + 32 + (encInfo -> size_secret_file * 8)
    long total_required_bits = 16 + 32 + (strlen(encInfo->extn_secret_file) * 8) + 32 + (encInfo->size_secret_file * 8);

    if (encInfo->image_capacity > total_required_bits)
    {
        return e_success;       //t -> return e_success;
    }
    return e_failure;       //f -> return e_failure
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];
    rewind(fptr_src_image);   // step 1 -> rewind source file pointer

    // step 2 -> read 54 bytes from source file
    if (fread(header, 1, 54, fptr_src_image) != 54)   
    {
        return e_failure;
    }

    // step 3 -> write 54 bytes to dest file
    if (fwrite(header, 1, 54, fptr_dest_image) != 54)
    {
        return e_failure;
    }

    return e_success; // return e_success
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // char buffer[8];

    //step 1 -> read 8 bytes from source file

    //step 2 -> call encode_byte_to_lsb(magic_string[i], buffer)

    //step 3 -> write the buffer into dest file

    // repeat this for strlen(magic_string) times from step 1

    char image_buffer[8];
    int size=strlen(magic_string);
    for(int i=0;i<size;i++)
    {
        fread(image_buffer,8,1,encInfo ->fptr_src_image);  //Read 8 bytes
        encode_byte_to_lsb(magic_string[i],image_buffer);
        fwrite(image_buffer,8,1,encInfo->fptr_stego_image);  //write modified bytes
    }
    if(ftell(encInfo ->fptr_src_image) == ftell(encInfo->fptr_stego_image)) //Check if both pointers match
    {
        return e_success;
    }
    return e_failure;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];

    // step 1 -> read 32 bytes from source file
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    // step 2 -> call encode_size_to_lsb(size, buffer)
    encode_size_to_lsb(size, buffer);

    // step 3 -> write the buffer into dest file
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];
    int size = strlen(file_extn);

    // repeat this for strlen(file_extn) times
    for (int i = 0; i < size; i++)
    {
        // step 1 -> read 8 bytes from source file
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        // step 2 -> call encode_byte_to_lsb(file_extn[i], buffer)
        encode_byte_to_lsb(file_extn[i], buffer);

        // step 3 -> write the buffer into dest file
        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
    }

    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];

    // step 1 -> read 32 bytes from source file
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    // step 2 -> call encode_size_to_lsb(file_size, buffer)
    encode_size_to_lsb((int)file_size, buffer);

    // step 3 -> write the buffer into dest file
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char buffer[8];
    char ch;

    // Reset secret file pointer to the beginning
    rewind(encInfo->fptr_secret);

    // Loop through each byte of secret file
    for (int i = 0; i < encInfo->size_secret_file; i++)
    {
        // step 1 -> read secret data from file
        if (fread(&ch, 1, 1, encInfo->fptr_secret) != 1)
        {
            return e_failure;
        }

        // step 2 -> read 8 bytes from source file
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        // step 3 -> call encode_byte_to_lsb(ch, buffer)
        encode_byte_to_lsb(ch, buffer);

        // step 4 -> write the buffer into dest file
        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
    }

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    // write logic for copy remaining data
    char ch;

    while(fread(&ch,1,1, fptr_src)==1)  //Read remaining bytes
    {
        fwrite(&ch,1,1, fptr_dest);     //write  to destination
    }

    if(ftell(fptr_src) == ftell(fptr_dest))   //Check if both pointers match 
    {
        return e_success;          // return e_success
    }
    return e_failure;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // Repeat for 8 bits 
    for (int i = 0; i < 8; i++)
    {
        // Extract LSB from data / Shift data right
        char bit = (data >> (7 - i)) & 1; 

        // Clear LSB of image byte
        image_buffer[i] = image_buffer[i] & 0xFE; 

        // Put data bit into image LSB
        image_buffer[i] = image_buffer[i] | bit; 
    }
    //Shift data right 
    return e_success;
}

Status encode_size_to_lsb(int size, char *image_buffer)
{
    // write logic to encode the int
    // Repeat for 32 bits
    for (int i = 0; i < 32; i++)
    {
        // Extract bit from size (from bit 31 down to bit 0)
        int bit = (size >> (31 - i)) & 1;

        // Clear LSB of image byte
        image_buffer[i] = image_buffer[i] & 0xFE;

        // Put size bit into image LSB
        image_buffer[i] = image_buffer[i] | bit;
    }

    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    // Step 1: Open required files
    if (open_files(encInfo) == e_success)
    {
        printf("INFO: Open files successful\n");
    }
    else
    {
        printf("ERROR: Open files failed\n");
        return e_failure;
    }

    // Step 2: Check capacity
    if (check_capacity(encInfo) == e_success)
    {
        printf("INFO: Capacity check successful\n");
    }
    else
    {
        printf("ERROR: Image capacity is insufficient\n");
        return e_failure;
    }

    // Step 3: Copy BMP Header
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO: BMP Header copied successfully\n");
    }
    else
    {
        printf("ERROR: Failed to copy BMP header\n");
        return e_failure;
    }

    // Step 4: Encode Magic String
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    {
        printf("INFO: Magic string encoded successfully\n");
    }
    else
    {
        printf("ERROR: Failed to encode magic string\n");
        return e_failure;
    }

    // Step 5: Encode Secret File Extension Size
    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_success)
    {
        printf("INFO: Secret file extension size encoded successfully\n");
    }
    else
    {
        printf("ERROR: Failed to encode extension size\n");
        return e_failure;
    }

    // Step 6: Encode Secret File Extension
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
    {
        printf("INFO: Secret file extension encoded successfully\n");
    }
    else
    {
        printf("ERROR: Failed to encode extension\n");
        return e_failure;
    }

    // Step 7: Encode Secret File Size
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
    {
        printf("INFO: Secret file size encoded successfully\n");
    }
    else
    {
        printf("ERROR: Failed to encode secret file size\n");
        return e_failure;
    }

    // Step 8: Encode Secret File Data
    if (encode_secret_file_data(encInfo) == e_success)
    {
        printf("INFO: Secret file data encoded successfully\n");
    }
    else
    {
        printf("ERROR: Failed to encode secret file data\n");
        return e_failure;
    }

    // Step 9: Copy Remaining Image Data
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO: Remaining image data copied successfully\n");
    }
    else
    {
        printf("ERROR: Failed to copy remaining image data\n");
        return e_failure;
    }

    return e_success;  
}
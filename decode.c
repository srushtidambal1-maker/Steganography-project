#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "encode.h"
#include "types.h"
#include "common.h"

/* Read and validate command line arguments for decoding */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // step 1 -> check if stego image argv[2] has .bmp extension
    if (!has_extension(argv[2], ".bmp"))
    {
        printf("--------------------------------------------------\n");
        printf("ERROR: Invalid stego image file '%s'.\n", argv[2]);
        printf("The stego image file must have a '.bmp' extension.\n");
        printf("--------------------------------------------------\n");
        return e_failure;
    }

    // step 2 -> check if stego image file exists
    FILE *fp_stego = fopen(argv[2], "r");
    if (!fp_stego)
    {
        printf("--------------------------------------------------\n");
        printf("ERROR: Stego image file '%s' does not exist.\n", argv[2]);
        printf("Please provide a valid stego BMP file to decode.\n");
        printf("--------------------------------------------------\n");
        return e_failure;
    }
    fclose(fp_stego);
    decInfo->stego_image_fname = argv[2];

    // step 3 -> check if optional output filename argv[3] is provided
    if (argv[3] != NULL)
    {
        decInfo->output_fname = argv[3];
    }
    else
    {
        // default output base name if not provided
        decInfo->output_fname = "output";
    }

    return e_success;
}

Status open_decode_files(DecodeInfo *decInfo)
{
    // Open stego image file in binary read mode
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

/* Decode a byte from 8 bytes of image buffer (LSB) */
char decode_byte_from_lsb(const char *image_buffer)
{
    char byte = 0;
    for (int i = 0; i < 8; i++)
    {
        byte = (byte << 1) | (image_buffer[i] & 1);
    }
    return byte;
}

/* Decode a 32-bit integer size from 32 bytes of image buffer (LSB) */
int decode_size_from_lsb(const char *image_buffer)
{
    int val = 0;
    for (int i = 0; i < 32; i++)
    {
        val = (val << 1) | (image_buffer[i] & 1);
    }
    return val;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    // Skip 54 bytes of BMP header
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    char magic_str[3] = {0}; // Buffer to hold 2 characters + '\0'
    char buffer[8];

    for (int i = 0; i < strlen(MAGIC_STRING); i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }

        magic_str[i] = decode_byte_from_lsb(buffer);
    }
    magic_str[strlen(MAGIC_STRING)] = '\0';

    // Verify if magic string matches "#*"
    if (strcmp(magic_str, MAGIC_STRING) == 0)
    {
        printf("INFO: Magic String matching successful: %s\n", magic_str);
        return e_success;
    }
    else
    {
        printf("ERROR: Magic String signature mismatch! Not a valid stego image.\n");
        return e_failure;
    }
}

/* Decode secret file extension size */
Status decode_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];

    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    decInfo->size_extn = decode_size_from_lsb(buffer);
    return e_success;
}

/* Decode secret file extension and create output file */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];

    for (int i = 0; i < decInfo->size_extn; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
        decInfo->extn_secret_file[i] = decode_byte_from_lsb(buffer);
    }
    decInfo->extn_secret_file[decInfo->size_extn] = '\0';

    // Construct full output filename (e.g., "output.txt" or "output_fname.txt")
    char full_output_fname[100];
    strcpy(full_output_fname, decInfo->output_fname);
    
    // Append extension if user didn't already include it
    if (strrchr(decInfo->output_fname, '.') == NULL)
    {
        strcat(full_output_fname, decInfo->extn_secret_file);
    }

    // Open output file now that we know the full filename and extension
    decInfo->fptr_output = fopen(full_output_fname, "wb");
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open output file %s\n", full_output_fname);
        return e_failure;
    }

    printf("INFO: Output file created successfully: %s\n", full_output_fname);
    return e_success;
}

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];

    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    decInfo->size_secret_file = decode_size_from_lsb(buffer);
    return e_success;
}

/* Decode secret file data and write to output file */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }

        ch = decode_byte_from_lsb(buffer);
        fputc(ch, decInfo->fptr_output);
    }

    return e_success;
}

/* Main driver function for decoding workflow */
Status do_decoding(DecodeInfo *decInfo)
{
    printf("INFO: Decoding procedure started...\n");

    if (open_decode_files(decInfo) == e_failure)
    {
        printf("ERROR: Open files failed during decoding.\n");
        return e_failure;
    }
    printf("INFO: Open files successful\n");

    if (decode_magic_string(decInfo) == e_failure)
    {
        printf("ERROR: Magic string verification failed.\n");
        return e_failure;
    }

    if (decode_file_extn_size(decInfo) == e_failure)
    {
        printf("ERROR: Secret file extension size decoding failed.\n");
        return e_failure;
    }
    printf("INFO: Secret file extension size decoded successfully\n");

    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        printf("ERROR: Secret file extension decoding failed.\n");
        return e_failure;
    }
    printf("INFO: Secret file extension decoded successfully\n");

    if (decode_secret_file_size(decInfo) == e_failure)
    {
        printf("ERROR: Secret file size decoding failed.\n");
        return e_failure;
    }
    printf("INFO: Secret file size decoded successfully\n");

    if (decode_secret_file_data(decInfo) == e_failure)
    {
        printf("ERROR: Secret file data decoding failed.\n");
        return e_failure;
    }
    printf("INFO: Secret file data decoded successfully\n");

    // Close files
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    return e_success;
}
#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

#define FILE_EXTN_SIZE 10

typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output Secret File Info */
    char *output_fname;
    FILE *fptr_output;

    /* Extracted Metadata */
    char extn_secret_file[FILE_EXTN_SIZE];
    long size_secret_file;
    int size_extn;

} DecodeInfo;

/* Function Prototypes */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);
Status open_decode_files(DecodeInfo *decInfo);
Status decode_magic_string(DecodeInfo *decInfo);
Status decode_file_extn_size(DecodeInfo *decInfo);
Status decode_secret_file_extn(DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Helper Functions */
char decode_byte_from_lsb(const char *image_buffer);
int decode_size_from_lsb(const char *image_buffer);
#endif
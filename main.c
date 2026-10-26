#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *);

// Helper function to check file extensions
int has_extension(const char *filename, const char *ext)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0;
    return (strcmp(dot, ext) == 0);
}

int main(int argc, char *argv[])  // ./a.out -e <source_file> <secret_data_file> <optional>
{
    // step 1 -> check argument count and check_operation_type
    if (argc < 2 || check_operation_type(argv[1]) == e_unsupported)
    {
        printf("Usage:\n");
        printf("  Encoding: %s -e <src.bmp> <secret_file> [stego.bmp]\n", argv[0]);
        printf("  Decoding: %s -d <stego.bmp> [output_file]\n", argv[0]);
        return 1;
    }

    // step 2 -> check check_operation_type(argv[1]) is returning e_encode or not
    if (check_operation_type(argv[1]) == e_encode)
    {
        // check minimum required arguments for encoding (-e <src.bmp> <secret_file>)
        if (argc < 4)
        {
            printf("ERROR: Missing arguments for encoding.\n");
            printf("Usage: %s -e <src.bmp> <secret_file> [stego.bmp]\n", argv[0]);
            return 1;
        }

        // check output file extension if optional 4th argument is provided
        if (argc >= 5 && !has_extension(argv[4], ".bmp"))
        {
            printf("--------------------------------------------------\n");
            printf("ERROR: Invalid output file extension '%s'.\n", argv[4]);
            printf("The output image file must have a '.bmp' extension.\n");
            printf("--------------------------------------------------\n");
            return 1;
        }

        EncodeInfo encInfo; 
        // check read_and_validate_encode_args(argv, &encInfo) is returning e_success or e_failure
        if (read_and_validate_encode_args(argv, &encInfo) == e_success) 
        { 
            printf("Read and validate encode arguments is successful\n");

            // check do_encoding(&encInfo) is returning e_success or e_failure
            if (do_encoding(&encInfo) == e_success) 
            { 
                printf("Encoding is successful\n"); 
            } 
            else 
            { 
                printf("Encoding is unsuccessful\n"); 
                return 1;
            } 
        } 
        else 
        { 
            printf("Read and validate encode arguments is unsuccessful\n"); 
            return 1;
        } 
    } 

    // Handle Decoding (-d)
    else if (check_operation_type(argv[1]) == e_decode)
    {
        // check minimum required arguments for decoding (-d <stego.bmp>)
        if (argc < 3)
        {
            printf("ERROR: Missing arguments for decoding.\n");
            printf("Usage: %s -d <stego.bmp> [output_file]\n", argv[0]);
            return 1;
        }

        DecodeInfo decInfo;
        // check read_and_validate_decode_args(argv, &decInfo) is returning e_success or e_failure
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("Read and validate decode arguments is successful\n");

            // check do_decoding(&decInfo) is returning e_success or e_failure
            if (do_decoding(&decInfo) == e_success)
            {
                printf("Decoding is successful\n");
            }
            else
            {
                printf("Decoding is unsuccessful\n");
                return 1;
            }
        }
        else
        {
            printf("Read and validate decode arguments is unsuccessful\n");
            return 1;
        }
    } 

    else 
    { 
        printf("Invalid operation\n"); 
    } 
    return 0; 
}

OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)         // step 1 -> check symbol is -e or not 
    {
        return e_encode;                   // yes -> return e_encode
    }
    else if (strcmp(symbol, "-d") == 0)    // step 2 -> check symbol is -d or not
    {
        return e_decode;                   // yes -> return e_decode
    }
    else
    {
        return e_unsupported;              // return e_unsupported
    }                                       
}
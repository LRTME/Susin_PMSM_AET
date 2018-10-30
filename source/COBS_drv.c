/*************************************************************
* FILE:         COBS_drv.c
* DESCRIPTION:  COBS(Consistent Overhead Byte Stuffing) encoder in decoder
* AUTHOR:       Mitja Nemec
*
*************************************************************/
#include    "COBS_drv.h"

/**************************************************************
* Function which decodes sting of data, encoded by COBS algorithm
* It also checks CRC of decoded string
* returns:  number bytes decoded
* arg1:     pointer to data for decoding
* arg2:     number of bytes to decode
* arg3:     pointer to decoded data
**************************************************************/
int COBS_decode(int* src, int length, int* dst, bool* data_correct)
{
    int input_str_index = 0;
    int output_str_index = 0;
    int input_current_byte;
    int code;
    int counter;

    // to include CRC
    int crc;

    CRC_init();

    // until all bytes were read
    while (input_str_index < length)
    {
        // get the length of current seqence
        input_current_byte = __byte(src,input_str_index);
        input_str_index = input_str_index + 1;

        code = input_current_byte;
        // copy apropriate number of bytes
        for (counter = 1; counter < code; counter++)
        {
            // read byte
            input_current_byte = __byte(src,input_str_index);
            input_str_index = input_str_index + 1;

            // write byte
            __byte(dst,output_str_index) = input_current_byte;
            output_str_index = output_str_index + 1;

            // get CRC for this byte
            CRC_add_byte(input_current_byte);
        }

        // when at the end of current sequence, write 0
        if (code < 0xFF)
        {
            __byte(dst,output_str_index) = 0x00;
            output_str_index = output_str_index + 1;

            // get CRC for this byte
            CRC_add_byte(0x0000);
        }
    }

    // read final CRC
    crc = CRC_get();

    // as the CRC was addae to the CRC caluclation, final CRC should be 0x0000
    if (crc == 0x0000)
    {
        *data_correct = TRUE;
    }
    else
    {
        *data_correct = FALSE;
    }

    // return number of bytes decoded
    return(output_str_index-1-2);
}

/**************************************************************
* Function which encodes data string with COBS algorithm
* It also adds CRC do encoded string
* returns:  number bytes encoded
* arg1:     pointer to data for encoding
* arg2:     number of bytes to encode
* arg3:     pointer to encoded data
**************************************************************/
int COBS_encode(int* src, int length, int* dst)
{
    int input_str_index = 0;
    int output_str_index = 1;
    int input_current_byte;
    int code_str_index = 0;

    // to store number of bytes in sequence
    int code = 1;

    // for crc clculation and storage
    int crc, crc_lsb, crc_msb;

    // initilize CRC calculation
    CRC_init();

    // walk through the input string until done
    while(input_str_index < length)
    {
        // read one byte
        input_current_byte = __byte(src,input_str_index);

        // get CRC for this byte
        CRC_add_byte(input_current_byte);

        // if byte is zero
        if (input_current_byte == 0)
        {
            // write number of bytes in current sequence
            __byte(dst,code_str_index) = code;

            // reset pointer to curent sequence lenght
            code_str_index = output_str_index;

            // move to next byte
            output_str_index = output_str_index + 1;

            // reset sequence length
            code = 1;
        }
        else
        {
            // write current byte
            __byte(dst,output_str_index) = input_current_byte;
            output_str_index = output_str_index + 1;

            // increase length of current sequence
            code = code + 1;

            // if curent sequence is 255 bytes long
            // start new sequence
            if (code == 0x00FF)
            {
                // write number of bytes in current sequence
                __byte(dst,code_str_index) = code;

                // reset pointer to curent sequence lenght
                code_str_index = output_str_index;

                // move to next byte
                output_str_index = output_str_index + 1;

                // reset sequence length
                code = 1;
            }
        }
        // move to next byte
        input_str_index = input_str_index + 1;
    }

    // encode the CRC
    crc = CRC_get();
    crc_lsb = crc & 0x00FF;
    crc_msb = (crc >> 8) & 0x00FF;

    if (crc_lsb == 0)
    {
        // write number of bytes in current sequence
        __byte(dst,code_str_index) = code;

        // reset pointer to curent sequence lenght
        code_str_index = output_str_index;

        // move to next byte
        output_str_index = output_str_index + 1;

        // reset sequence length
        code = 1;
    }
    else
    {
        // write current byte
        __byte(dst,output_str_index) = crc_lsb;
        output_str_index = output_str_index + 1;

        // increase length of current sequence
        code = code + 1;

        // if curent sequence is 255 bytes long
        // start new sequence
        if (code == 0x00FF)
        {
            // write number of bytes in current sequence
            __byte(dst,code_str_index) = code;

            // reset pointer to curent sequence lenght
            code_str_index = output_str_index;

            // move to next byte
            output_str_index = output_str_index + 1;

            // reset sequence length
            code = 1;
        }
    }

    if (crc_msb == 0)
    {
        // write number of bytes in current sequence
        __byte(dst,code_str_index) = code;

        // reset pointer to curent sequence lenght
        code_str_index = output_str_index;

        // move to next byte
        output_str_index = output_str_index + 1;

        // reset sequence length
        code = 1;
    }
    else
    {
        // write current byte
        __byte(dst,output_str_index) = crc_msb;
        output_str_index = output_str_index + 1;

        // increase length of current sequence
        code = code + 1;

        // if curent sequence is 255 bytes long
        // start new sequence
        if (code == 0x00FF)
        {
            // write number of bytes in current sequence
            __byte(dst,code_str_index) = code;

            // reset pointer to curent sequence lenght
            code_str_index = output_str_index;

            // move to next byte
            output_str_index = output_str_index + 1;

            // reset sequence length
            code = 1;
        }
    }

    // write number of bytes in current sequence
    __byte(dst,code_str_index) = code;

    // terminate the string with 0x00 for synchronization purposes
    __byte(dst,output_str_index) = 0x0000;
    output_str_index = output_str_index + 1;

    // return length of encoded string
    return(output_str_index);
}


/****************************************************************
* FILENAME:     LRTME_stack.c
* DESCRIPTION:  LRTME stack code
* AUTHOR:       Mitja Nemec
*
****************************************************************/

#include "LRTME_stack.h"

// packet counters
long    LRTME_crc_error_count = 0;
long	LRTME_packets_received = 0;
long	LRTME_packets_sent = 0;
long    LRTME_bytes_sent = 0;

// go silent if nobody is listening
bool    quiet_mode = FALSE;
long    quiet_timeout = 5000;     // go quiet after x mili seconds
long    quiet_timeout_counter = 5000;
extern  bool pulse_10ms;

// transmision buffers and variables
int     LRTME_raw_send[LRTME_TX_BUFF_SIZE];
int     LRTME_raw_send_length;
int     LRTME_encoded_send[LRTME_TX_BUFF_SIZE];
int     LRTME_encoded_send_length;

int     LRTME_raw_send_code;
int     *LRTME_raw_send_data;

void    (*LRTME_raw_tx_callback)(void);

struct  LRTME_tx_struct
{
    int     code;
    int     *data;
    int     length;
    void    (*LRTME_tx_callback)(void);
};

struct  LRTME_tx_struct LRTME_tx_send_queue[LRTME_TX_QUEUE_SIZE];
int     LRTME_tx_Q_first = 0;
int     LRTME_tx_Q_last = 0;
int     LRTME_tx_Q_number = 0;

// reception buffers and variables
int     LRTME_raw_received[LRTME_RX_BUFF_SIZE];
int     LRTME_raw_received_length;
int     LRTME_decoded_received[LRTME_RX_BUFF_SIZE];
int     LRTME_decoded_received_length;

int     LRTME_received_code;
void    (*LRTME_rx_handler)(int *);

struct  LRTME_rx_struct
{
    int code;
    void (*function)(int *);
};

// receive callback registration buffer
struct  LRTME_rx_struct LRTME_rx_handler_list[LRTME_RX_NR_HANDLERS];
int     LRTME_rx_list_elements = 0;

// declaration of function prototypes
void LRTME_receive(void);
void LRTME_receive_register(int code, void (*function)(int *));
int LRTME_tx_queue_poll(void);
void LRTME_transmit(void);
void LRTME_tx_queue_put(struct  LRTME_tx_struct *to_send);
bool LRTME_tx_queue_get(struct  LRTME_tx_struct *to_send);

/**************************************************************
* LRTME stack main function
**************************************************************/
void LRTME_stack(void)
{
    // quiet_timeout
    if (quiet_mode == FALSE)
    {
        if (pulse_10ms == TRUE)
        {
            quiet_timeout_counter = quiet_timeout_counter + 10;
        }
        if (quiet_timeout_counter >= quiet_timeout)
        {
            quiet_mode = TRUE;
        }
    }
    // if any packets arive, then reset the timer and start transmiting
    if (SCI_chk_packet_ready() == TRUE)
    {
        quiet_mode = FALSE;
        quiet_timeout_counter = 0;
    }

    // handle reception
    LRTME_receive();
    // handle transmision
    LRTME_transmit();
}

/**************************************************************
* LRTME are we in quiet mode?
**************************************************************/
bool LRTME_quiet(void)
{
    return(quiet_mode);
}

/**************************************************************
* LRTME transmision of packets
**************************************************************/
#pragma CODE_SECTION(LRTME_transmit, "ramfuncs");
void LRTME_transmit(void)
{
    struct  LRTME_tx_struct za_poslat;
    int i;

    // check if packets cen be sent at all
    if (SCI_data_sent() == TRUE)
    {
        // if there is anything to sent at all
        if (LRTME_tx_queue_poll() != 0)
        {
            // get the packet
            LRTME_tx_queue_get(&za_poslat);
            LRTME_raw_send_code = za_poslat.code;
            LRTME_raw_send_data = za_poslat.data;
            LRTME_raw_send_length = za_poslat.length;
            LRTME_raw_tx_callback = za_poslat.LRTME_tx_callback;

            // copy in local buffer
            LRTME_raw_send[0] = LRTME_raw_send_code;
            for (i = 1; i <= LRTME_raw_send_length/2; i = i + 1)
            {
                LRTME_raw_send[i] = *LRTME_raw_send_data;
                LRTME_raw_send_data = LRTME_raw_send_data + 1;
            }

            // encode packet
            LRTME_encoded_send_length = COBS_encode(LRTME_raw_send, LRTME_raw_send_length + 2, LRTME_encoded_send);

            // once encoded issue a callback if necessary
            if (LRTME_raw_tx_callback != NULL)
            {
                (*LRTME_raw_tx_callback)();
            }

            // increase the counters
            LRTME_packets_sent = LRTME_packets_sent + 1;
            LRTME_bytes_sent = LRTME_bytes_sent + LRTME_encoded_send_length;
            // and send the packet to SCI port
            SCI_send_word(LRTME_encoded_send, LRTME_encoded_send_length);
        }
    }
}

/**************************************************************
* put a new packet on send queue
**************************************************************/
#pragma CODE_SECTION(LRTME_send, "ramfuncs");
void LRTME_send(int code, int *data, int length, void    (*tx_callback)(void))
{
    struct  LRTME_tx_struct za_poslat;

    if (quiet_mode == FALSE)
    {
        za_poslat.code = code;
        za_poslat.data = data;
        za_poslat.length = length;
        za_poslat.LRTME_tx_callback = tx_callback;

        LRTME_tx_queue_put(&za_poslat);
    }
}

/**************************************************************
* return the number of elements on a send queue
**************************************************************/
int LRTME_tx_queue_poll(void)
{
    return (LRTME_tx_Q_number);
}

/**************************************************************
* transit queue initialization
* returns:
**************************************************************/
void LRTME_tx_queue_init(void)
{
    int i = 0;

    LRTME_tx_Q_first = 0;
    LRTME_tx_Q_last = 0;
    LRTME_tx_Q_number = 0;

    for (i = 0; i < SCI_RX_BUFFER_SIZE; i++)
    {
        LRTME_tx_send_queue[i].code = 0;
        LRTME_tx_send_queue[i].data = 0;
        LRTME_tx_send_queue[i].length = 0;
        LRTME_tx_send_queue[i].LRTME_tx_callback = 0;
    }
}

/**************************************************************
* queue put function
* returns:
* arg1:     element to put to the queue
**************************************************************/
#pragma CODE_SECTION(LRTME_tx_queue_put, "ramfuncs");
void LRTME_tx_queue_put(struct  LRTME_tx_struct *to_send)
{
    // place in the queue
    LRTME_tx_send_queue[LRTME_tx_Q_last].code = to_send->code;
    LRTME_tx_send_queue[LRTME_tx_Q_last].data = to_send->data;
    LRTME_tx_send_queue[LRTME_tx_Q_last].length = to_send->length;
    LRTME_tx_send_queue[LRTME_tx_Q_last].LRTME_tx_callback = to_send->LRTME_tx_callback;

    // increase element counter and pointer to last element
    LRTME_tx_Q_last = LRTME_tx_Q_last + 1;
    LRTME_tx_Q_number = LRTME_tx_Q_number + 1;

    // wraparound
    if (LRTME_tx_Q_last >= LRTME_TX_QUEUE_SIZE)
    {
        LRTME_tx_Q_last = 0;
    }

    // throw the oldest element out if queue is full
    if (LRTME_tx_Q_number >= LRTME_TX_QUEUE_SIZE)
    {
        LRTME_tx_Q_first = LRTME_tx_Q_first + 1;
    }
    if (LRTME_tx_Q_first >= LRTME_TX_QUEUE_SIZE)
    {
        LRTME_tx_Q_first = 0;
    }
}

/**************************************************************
* queue get function
* returns:  TRUE if an element was put
* arg1:     pointer to the element to get
**************************************************************/
#pragma CODE_SECTION(LRTME_tx_queue_get, "ramfuncs");
bool LRTME_tx_queue_get(struct  LRTME_tx_struct *to_send)
{
    // it there is enything to get
    if (LRTME_tx_Q_number > 0)
    {
        // get the element
        to_send->code = LRTME_tx_send_queue[LRTME_tx_Q_first].code;
        to_send->data = LRTME_tx_send_queue[LRTME_tx_Q_first].data;
        to_send->length = LRTME_tx_send_queue[LRTME_tx_Q_first].length;
        to_send->LRTME_tx_callback = LRTME_tx_send_queue[LRTME_tx_Q_first].LRTME_tx_callback;

        // increase element counter and pointer to first element
        LRTME_tx_Q_first = LRTME_tx_Q_first + 1;
        LRTME_tx_Q_number = LRTME_tx_Q_number - 1;

        // wraparound
        if (LRTME_tx_Q_first >= LRTME_TX_QUEUE_SIZE)
        {
            LRTME_tx_Q_first = 0;
        }

        return (TRUE);
    }
    // otherwise signal there was no element to get
    else
    {
        return (FALSE);
    }
}

/**************************************************************
* LRTME reception of packets
**************************************************************/
#pragma CODE_SECTION(LRTME_receive, "ramfuncs");
void LRTME_receive(void)
{
    int i;

    bool data_correct;

    // check if there are any new packets waiting
    if (SCI_chk_packet_ready() == TRUE)
    {
        // get the packet
        LRTME_raw_received_length = SCI_get_packet(LRTME_raw_received,LRTME_RX_BUFF_SIZE);

        // decode the packet
        LRTME_decoded_received_length = COBS_decode(LRTME_raw_received, LRTME_raw_received_length, LRTME_decoded_received, &data_correct);

        // if crc correct
        if (data_correct == TRUE)
        {
        	// increase the packet counter
        	LRTME_packets_received = LRTME_packets_received + 1;

            // get the code
            LRTME_received_code = LRTME_decoded_received[0];

            // got through registered handlers
            for (i = 0; i < LRTME_rx_list_elements; i = i + 1)
            {
                // if there is
                if (LRTME_rx_handler_list[i].code == LRTME_received_code)
                {
                    LRTME_rx_handler = LRTME_rx_handler_list[i].function;

                    // if packet has data execute callback with pointer to data
                    if (LRTME_decoded_received_length > 2)
                    {
                        (*LRTME_rx_handler)(&LRTME_decoded_received[1]);
                    }
                    // otherwise execute callback with null pointer
                    else
                    {
                        (*LRTME_rx_handler)(0);
                    }
                }
            }
        }
        // incase of CRC error increase the counter
        else
        {
            LRTME_crc_error_count = LRTME_crc_error_count + 1;
        }
    }
}

/**************************************************************
* function which registeres receive callback function
**************************************************************/
void LRTME_receive_register(int code, void (*function)(int *))
{
    LRTME_rx_handler_list[LRTME_rx_list_elements].code = code;
    LRTME_rx_handler_list[LRTME_rx_list_elements].function = function;
    LRTME_rx_list_elements = LRTME_rx_list_elements + 1;

    // check if there is room available
    if (LRTME_rx_list_elements >= LRTME_RX_NR_HANDLERS)
    {
        // you should increase LRTME_RX_NR_HANDLERS buffer size
        asm(" ESTOP0");
    }
}

/**************************************************************
* LRTME stack initialization
**************************************************************/
void LRTME_init(void)
{
    size_t i;
    LRTME_tx_queue_init();
    for (i = 0; i < sizeof(LRTME_raw_send); i = i + 1)
    {
        LRTME_raw_send[i] = 0;
    }
    for (i = 0; i < sizeof(LRTME_encoded_send); i = i + 1)
    {
        LRTME_encoded_send[i] = 0;
    }

    for (i = 0; i < sizeof(LRTME_raw_received); i = i + 1)
    {
        LRTME_raw_received[i] = 0;
    }
    for (i = 0; i < sizeof(LRTME_decoded_received); i = i + 1)
    {
        LRTME_decoded_received[i] = 0;
    }
}


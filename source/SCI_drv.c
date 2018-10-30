/*************************************************************
* FILE:         SCI_drv.c
* DESCRIPTION:  SCI port driver with FIFO and interrupts
* AUTHOR:       Mitja Nemec
*
*************************************************************/
#include "SCI_drv.h"

// wheather I'm sending bytes or words
int SCI_tx_send_type = 0;

// receive overflow count
long SCI_rx_overflow_count = 0;

// number of bytes/words sent
volatile int SCI_tx_sent = 0;
volatile int *SCI_tx_ptr = 0;
volatile int SCI_tx_nr_bytes = 0;

// number of bytes/words received
volatile int SCI_rx_received = 0;
volatile int *SCI_rx_ptr = 0;
volatile int SCI_rx_nr_bytes = 0;
volatile int SCI_rx_packets = 0;

// rx queue
unsigned int SCI_Q_first_byte;
unsigned int SCI_Q_last_byte;
unsigned int SCI_Q_nr_bytes;
int SCI_Q_buffer[SCI_RX_BUFFER_SIZE];

// interupt routines prototypes
interrupt void SCI_rx_interrupt(void);
interrupt void SCI_tx_interrupt(void);

// timeout function prototypes
void SCI_timer_init(void);
void SCI_timer_start(long timeout);
int  SCI_timer_check_timeout(void);

// rx queue function prototypes
void SCI_queue_init(void);
bool SCI_queue_put(int element);
int  SCI_queue_get(void);
int SCI_get_packet(int *dst, int buff_size);


/**************************************************************
* receive interrupt function
* returns:
**************************************************************/
#pragma CODE_SECTION(SCI_rx_interrupt, "ramfuncs");
interrupt void SCI_rx_interrupt(void)
{
    int data = 0;
    bool data_put;

    // enable all higher priority interrupts
    EINT;

    // transfer all the data from FIFO to the queue
    while(SCI_MODUL.SCIFFRX.bit.RXFFST != 0)
    {
        // read byte
        data = SCI_MODUL.SCIRXBUF.bit.SAR;

        // and put byte in queue
        data_put = SCI_queue_put(data);

        // if received 0x0000 signal new packed has arrived
        if (data == 0x0000)
        {
            SCI_rx_packets = SCI_rx_packets + 1;
        }

        // if queue is full or an overflow occured
        if (   (data_put == FALSE)
            || (SCI_MODUL.SCIFFRX.bit.RXFFOVF == 1))
        {
            // increase event count
            SCI_rx_overflow_count = SCI_rx_overflow_count + 1;
            // clear overflow flag
            SCI_MODUL.SCIFFRX.bit.RXFFOVRCLR = 1;
            // flush the queue
            SCI_queue_init();
            // flush FIFO
            while(SCI_MODUL.SCIFFRX.bit.RXFFST != 0)
            {
                data = SCI_MODUL.SCIRXBUF.bit.SAR;
            }
            // clear packet count
            SCI_rx_packets = 0;
            // end with receiving
            break;
        }

    }

    // acknowledge the interrupt
    SCI_MODUL.SCIFFRX.bit.RXFFINTCLR = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
    
    // Disable all higher priority interrupts
    // to prevent RB corruption
    DINT;
}

/**************************************************************
* read all data from FIFO
* returns:
**************************************************************/
void SCI_flush_fifo(void)
{
    int data;
    bool data_put;

    SCI_disable_rx_interrupts();
    while(SCI_MODUL.SCIFFRX.bit.RXFFST != 0)
    {
        // read byte
        data = SCI_MODUL.SCIRXBUF.bit.SAR;
        //and put in queue
        data_put = SCI_queue_put(data);

        // if received 0x0000 signal new packed has arrived
        if (data == 0x0000)
        {
            SCI_rx_packets = SCI_rx_packets + 1;
        }

        // if queue is full or an overflow occured
        if (data_put == FALSE)
        {
            // increase event count
            SCI_rx_overflow_count = SCI_rx_overflow_count + 1;
            // flush the queue
            SCI_queue_init();
            // flush FIFO
            while(SCI_MODUL.SCIFFRX.bit.RXFFST != 0)
            {
                data = SCI_MODUL.SCIRXBUF.bit.SAR;
            }
            // clear packet count
            SCI_rx_packets = 0;
            // end with receiving
            break;
         }
    }
    SCI_enable_rx_interrupts();
}

/**************************************************************
* read n-bytes from SCI port
* data is writen to low bytes of integer only
* returns:  number of characteres received
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout in us, to wait for all the data
**************************************************************/
int SCI_receive_byte(void *data, int nr_bytes, long timeout)
{
    SCI_rx_received = 0;
    SCI_rx_ptr = data;
    SCI_rx_nr_bytes = nr_bytes;

    // start timeout - only if required
    if (timeout != 0)
    {
        SCI_timer_start(timeout);
    }

    // transfer data from queue until you get all the data
    // or timeout occurs
    while (   (SCI_timer_check_timeout() == FALSE)
            &&(SCI_rx_received < SCI_rx_nr_bytes) )
    {
        // if data available get it out
        if (SCI_Q_nr_bytes != 0)
        {
            *SCI_rx_ptr = SCI_queue_get();
            SCI_rx_received = SCI_rx_received + 1;
            SCI_rx_ptr = SCI_rx_ptr + 1;
        }
        // if waiting for less then FIFO interrupt level, then
        // reduce FIFO interrupt level
        if (   (SCI_rx_received < SCI_rx_nr_bytes)
                &&((SCI_rx_nr_bytes - SCI_rx_received) < SCI_FIFO_DEPTH))
        {
                SCI_MODUL.SCIFFRX.bit.RXFFIL = SCI_rx_nr_bytes - SCI_rx_received;
        }
    }

    // whene received all the data reset FIFO interrupt level
    SCI_MODUL.SCIFFRX.bit.RXFFIL = SCI_FIFO_DEPTH;

    return(SCI_rx_received);
}

/**************************************************************
* read n-bytes from SCI port
* data is writen to high and low bytes of integer
* returns:  number of characteres received
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout in us, to wait for all the data
**************************************************************/
int SCI_receive_word(void *data, int nr_bytes, long timeout)
{
    SCI_rx_received = 0;
    SCI_rx_ptr = data;
    SCI_rx_nr_bytes = nr_bytes;

    // start timeout - only if required
    if (timeout != 0)
    {
        SCI_timer_start(timeout);
    }

    // transfer data from queue until you get all the data
    // or timeout occurs
    while (   (SCI_timer_check_timeout() == FALSE)
            &&(SCI_rx_received < SCI_rx_nr_bytes) )
    {
        // if data available get it out
        if (SCI_Q_nr_bytes != 0)
        {
            *SCI_rx_ptr = SCI_queue_get();
            SCI_rx_received = SCI_rx_received + 1;
            SCI_rx_ptr = SCI_rx_ptr + 1;
        }
        // if waiting for less then FIFO interrupt level, then
        // reduce FIFO interrupt level
        if (   (SCI_rx_received < SCI_rx_nr_bytes)
                &&(((SCI_rx_nr_bytes - SCI_rx_received)*2) < SCI_FIFO_DEPTH))
        {
                SCI_MODUL.SCIFFRX.bit.RXFFIL = SCI_rx_nr_bytes - SCI_rx_received;
        }
    }

    // whene received all the data reset FIFO interrupt level
    SCI_MODUL.SCIFFRX.bit.RXFFIL = SCI_FIFO_DEPTH;

    return(SCI_rx_received);
}

/**************************************************************
* transmitt interrupt function
* returns:
**************************************************************/
#pragma CODE_SECTION(SCI_tx_interrupt, "ramfuncs");
interrupt void SCI_tx_interrupt(void)
{
    // enable all higher priority interrupts
    EINT;

    // if sending low bytes only
    if (SCI_tx_send_type == 0)
    {
        // fill the fifo, while there is data to send and place in fifo
        while(   (SCI_MODUL.SCIFFTX.bit.TXFFST < SCI_FIFO_DEPTH)
               &&(SCI_tx_sent < SCI_tx_nr_bytes) )
        {
            SCI_MODUL.SCITXBUF.bit.TXDT = *SCI_tx_ptr;
            SCI_tx_ptr = SCI_tx_ptr + 1;
            SCI_tx_sent = SCI_tx_sent + 1;
        }

        // if all data was send, disable TX FIFO interrupt
        if (SCI_tx_sent >= SCI_tx_nr_bytes)
        {
            SCI_MODUL.SCIFFTX.bit.TXFFIENA = 0;
        }
    }
    // if sending complete words
    else
    {
        // fill the fifo, while there is data to send and place in fifo
        while(   (SCI_MODUL.SCIFFTX.bit.TXFFST < (SCI_FIFO_DEPTH - 1))
               &&(SCI_tx_sent < SCI_tx_nr_bytes) )
        {
            // first send MSB
            #pragma diag_push
            #pragma diag_suppress 169
            SCI_MODUL.SCITXBUF.bit.TXDT = __byte(SCI_tx_ptr, SCI_tx_sent);
            #pragma diag_pop
            SCI_tx_sent = SCI_tx_sent + 1;

            // if send all, then break
            if (SCI_tx_sent >= SCI_tx_nr_bytes)
            {
                break;
            }

            // then send LSB
            #pragma diag_push
            #pragma diag_suppress 169
            SCI_MODUL.SCITXBUF.bit.TXDT = __byte(SCI_tx_ptr, SCI_tx_sent);
            #pragma diag_pop
            SCI_tx_sent = SCI_tx_sent + 1;
        }

        // if all data was send, disable TX FIFO interrupt
        if (SCI_tx_sent >= SCI_tx_nr_bytes)
        {
            SCI_MODUL.SCIFFTX.bit.TXFFIENA = 0;
        }
    }

    // acknowledge interrupt
    SCI_MODUL.SCIFFTX.bit.TXFFINTCLR = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
    
    // Disable all higher priority interrupts
    // to prevent RB corruption
    DINT;
}

/**************************************************************
* send n-bytes from SCI port (blocking)
* only low bytes of integer are sent
* returns:  number of bytes to sent
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout (in us) for data to send
**************************************************************/
int SCI_send_byte(void *data, int nr_bytes, long timeout)
{
    SCI_tx_send_type = 0;
    SCI_tx_sent = 0;
    SCI_tx_ptr = data;
    SCI_tx_nr_bytes = nr_bytes;

    // fill the fifo, while there is data to send and place in fifo
    while(   (SCI_MODUL.SCIFFTX.bit.TXFFST < SCI_FIFO_DEPTH)
           &&(SCI_tx_sent < SCI_tx_nr_bytes) )
    {
        SCI_MODUL.SCITXBUF.bit.TXDT = *SCI_tx_ptr;
        SCI_tx_ptr = SCI_tx_ptr + 1;
        SCI_tx_sent = SCI_tx_sent + 1;
        // reset TX fifo flag, to prevent triggering spurious interrupts
        SCI_MODUL.SCIFFTX.bit.TXFFINTCLR = 1;
    }

    // if there is more data to send enable interrupts
    if (SCI_tx_sent < SCI_tx_nr_bytes)
    {
        SCI_MODUL.SCIFFTX.bit.TXFFIENA = 1;
    }

    // start timeout


    // either yield (if using RTOS) or wait until sent, or timeout runs out
    while (SCI_tx_sent != SCI_tx_nr_bytes)
    {
        /* DO NOTHING */
    }

    // if ended due to timeout, cancel transmission in progress

    // and signal how many bytes/words were sent
    return(SCI_tx_sent);
}

/**************************************************************
* send n-bytes from SCI port (non-blocking)
* sends high and low bytes of integer
* returns:  number of bytes sent
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout (in us) for data to send
**************************************************************/
int SCI_send_word(void *data, int nr_bytes)
{
    // preper for begining of the send sequence
    SCI_tx_send_type = 1;
    SCI_tx_sent = 0;

    SCI_tx_ptr = data;
    SCI_tx_nr_bytes = nr_bytes;

    // fill the fifo, while there is data to send and place in fifo
    while(   (SCI_MODUL.SCIFFTX.bit.TXFFST < (SCI_FIFO_DEPTH - 1))
           &&(SCI_tx_sent < SCI_tx_nr_bytes) )
    {
        // first send MSB
        #pragma diag_push
        #pragma diag_suppress 169
        SCI_MODUL.SCITXBUF.bit.TXDT = __byte(SCI_tx_ptr, SCI_tx_sent);
        #pragma diag_pop
        SCI_tx_sent = SCI_tx_sent + 1;
        // ce je slucajno konec potem grem ven iz zanke
        if (SCI_tx_sent >= SCI_tx_nr_bytes)
        {
            break;
        }
		// then send LSB
        #pragma diag_push
        #pragma diag_suppress 169
        SCI_MODUL.SCITXBUF.bit.TXDT = __byte(SCI_tx_ptr, SCI_tx_sent);
        #pragma diag_pop
        SCI_tx_sent = SCI_tx_sent + 1;

        // clear int flag
        SCI_MODUL.SCIFFTX.bit.TXFFINTCLR = 1;
    }

    // if there is more data to send enable interrupts
    if (SCI_tx_sent < SCI_tx_nr_bytes)
    {
        SCI_MODUL.SCIFFTX.bit.TXFFIENA = 1;
    }
    // and signal how many bytes/words were sent
    return(SCI_tx_sent);
}

/**************************************************************
* send n-bytes from SCI port (blocking)
* sends high and low bytes of integer
* returns:  number of bytes sent
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout (in us) for data to send
**************************************************************/
int SCI_send_word_blocking(void *data, int nr_bytes, long timeout)
{
    SCI_send_word(data, nr_bytes);
    // wait until sent
    while (SCI_tx_sent < SCI_tx_nr_bytes)
    {
        /* DO NOTHING */
    }
    return(SCI_tx_sent);
}

/**************************************************************
* checks if all data was sent
* returns:  TRUE or FALSE
**************************************************************/
bool SCI_data_sent(void)
{
    if (SCI_tx_sent < SCI_tx_nr_bytes)
    {
        return (FALSE);
    }
    else
    {
        return( TRUE);
    }
}

/**************************************************************
* SCI modul initialization
* returns:
* arg1:     baud rate
**************************************************************/
void SCI_init(long baud_rate)
{
    // local variables for baud rate calculation
    float baud_f;
    int baud;
    int baud_H;
    int baud_L;
    long sci_freq;

    // configure inputs/ouputs
    EALLOW;
    GPIO_SetupPinMux(42, GPIO_MUX_CPU1, 15);
    GPIO_SetupPinMux(43, GPIO_MUX_CPU1, 15);
    EDIS;

    // calculate baudrate
    if(ClkCfgRegs.LOSPCP.bit.LSPCLKDIV == 0)
    {
        sci_freq = CPU_FREQ;
    }
    else
    {
        sci_freq = CPU_FREQ / (2 * ClkCfgRegs.LOSPCP.bit.LSPCLKDIV);
    }

    baud_f = (sci_freq / ((float)(baud_rate * 8L))) - 1L;
    baud = baud_f;
    baud_H = (int)(baud >> 8) ;
    baud_L = (int)(baud & 0x00FF);

    SCI_MODUL.SCIHBAUD.bit.BAUD = baud_H;
    SCI_MODUL.SCILBAUD.bit.BAUD = baud_L;

    // configure SCI peripherals to use FIFO
    SCI_MODUL.SCICTL1.bit.SWRESET = 0;

    SCI_MODUL.SCICCR.bit.STOPBITS = 1;
    SCI_MODUL.SCICCR.bit.PARITY = 0;
    SCI_MODUL.SCICCR.bit.PARITYENA = 0;
    SCI_MODUL.SCICCR.bit.LOOPBKENA = 0;
    SCI_MODUL.SCICCR.bit.ADDRIDLE_MODE = 0;
    SCI_MODUL.SCICCR.bit.SCICHAR = 7;

    SCI_MODUL.SCICTL1.bit.RXERRINTENA = 0;
    SCI_MODUL.SCICTL1.bit.TXWAKE = 0;
    SCI_MODUL.SCICTL1.bit.SLEEP = 0;

    SCI_MODUL.SCICTL2.bit.TXINTENA = 0;
    SCI_MODUL.SCICTL1.bit.RXERRINTENA = 0;

    #if SCI_DEBUG_MODE == 0
    SCI_MODUL.SCIPRI.bit.FREESOFT = 0;
    #endif
    #if SCI_DEBUG_MODE == 2
    SCI_MODUL.SCIPRI.bit.FREESOFT = 3;
    #endif

    // setup FIFO registers
    SCI_MODUL.SCIFFTX.bit.SCIFFENA = 1;

    SCI_MODUL.SCIFFCT.bit.ABDCLR = 0;
    SCI_MODUL.SCIFFCT.bit.CDC = 0;
    SCI_MODUL.SCIFFCT.bit.FFTXDLY = 4;

    SCI_MODUL.SCIFFTX.bit.SCIFFENA = 1;
    SCI_MODUL.SCIFFTX.bit.TXFIFORESET = 1;
    SCI_MODUL.SCIFFTX.bit.TXFFINT = 0;
    SCI_MODUL.SCIFFTX.bit.TXFFIL = 0;
    SCI_MODUL.SCIFFTX.bit.TXFFINTCLR = 1;
    SCI_MODUL.SCIFFTX.bit.TXFFIENA = 0;

    SCI_MODUL.SCIFFRX.bit.RXFIFORESET = 0;
    SCI_MODUL.SCIFFRX.bit.RXFFOVRCLR = 1;
    SCI_MODUL.SCIFFRX.bit.RXFFINTCLR = 1;
    SCI_MODUL.SCIFFRX.bit.RXFFIL = SCI_FIFO_DEPTH;
    SCI_MODUL.SCIFFRX.bit.RXFFIENA = 1;
    SCI_MODUL.SCIFFRX.bit.RXFIFORESET = 1;

    SCI_MODUL.SCIFFCT.bit.FFTXDLY = 3;

    // enable receiver and transmiter
    SCI_MODUL.SCICTL1.bit.RXENA = 1;
    SCI_MODUL.SCICTL1.bit.TXENA = 1;

    SCI_MODUL.SCICTL1.bit.SWRESET = 1;

    // register interrupt routines
    EALLOW;
    PieVectTable.SCIA_RX_INT = &SCI_rx_interrupt;
    PieVectTable.SCIA_TX_INT = &SCI_tx_interrupt;
    EDIS;
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
    PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
    IER |= M_INT9;

    // enable SCI operation in real time mode
    SetDBGIER(M_INT9);

    // initialize timer
    SCI_timer_init();

    //initialize queue
    SCI_queue_init();
}

/**************************************************************
* disables SCI module interrupt
* returns:
**************************************************************/
#pragma CODE_SECTION(SCI_disable_rx_interrupts, "ramfuncs");
inline void SCI_disable_rx_interrupts(void)
{
    SCI_MODUL.SCIFFRX.bit.RXFFIENA = 0;
}

/**************************************************************
* enables SCI module interrupt
* returns:
**************************************************************/
#pragma CODE_SECTION(SCI_enable_rx_interrupts, "ramfuncs");
inline void SCI_enable_rx_interrupts(void)
{
    SCI_MODUL.SCIFFRX.bit.RXFFIENA = 1;
}

/**************************************************************
* Initialize receive queue
* returns:
**************************************************************/
void SCI_queue_init(void)
{
    // rx queue
    int i = 0;

    SCI_Q_first_byte = 0;
    SCI_Q_last_byte = 0;
    SCI_Q_nr_bytes = 0;

    // clear queue
    for (i = 0; i < SCI_RX_BUFFER_SIZE; i++)
    {
        SCI_Q_buffer[i] = 0;
    }
}

/**************************************************************
* Queue put function
* returns:  TRUE, if element stored succesfully
* arg1:     element to put to the queue
**************************************************************/
bool SCI_queue_put(int element)
{
    bool ret_val = TRUE;

    // if full, throw the oldest out
    if ((SCI_Q_nr_bytes >> 1) >= SCI_RX_BUFFER_SIZE)
    {
        SCI_Q_nr_bytes = SCI_Q_nr_bytes - 1;
        SCI_Q_first_byte = SCI_Q_first_byte + 1;
        ret_val = FALSE;
        if ((SCI_Q_first_byte >> 1) >= SCI_RX_BUFFER_SIZE)
        {
            SCI_Q_first_byte = 0;
        }
    }
    // put the element in
    __byte(SCI_Q_buffer,SCI_Q_last_byte) = element;

    // increase element count
    SCI_Q_last_byte = SCI_Q_last_byte + 1;
    SCI_Q_nr_bytes = SCI_Q_nr_bytes + 1;

    // wrap around
    if ((SCI_Q_last_byte >> 1) >= SCI_RX_BUFFER_SIZE)
    {
        SCI_Q_last_byte = 0;
    }

    return(ret_val);
}

/**************************************************************
* Queue get function
* returns:  element got from the queue
**************************************************************/
int SCI_queue_get(void)
{
    int element;
    // get only if there is something to get
    if (SCI_Q_nr_bytes > 0)
    {
        // read the element
        element = __byte(SCI_Q_buffer,SCI_Q_first_byte);
        // decrease number of elements in queue and adjust index
        SCI_Q_first_byte = SCI_Q_first_byte + 1;
        SCI_Q_nr_bytes = SCI_Q_nr_bytes - 1;

        // wrap around
        if ((SCI_Q_first_byte >> 1) >= SCI_RX_BUFFER_SIZE)
        {
            SCI_Q_first_byte = 0;
        }
    }
    else
    {
        element = 0xFF00;
    }
    return (element);
}

/**************************************************************
* read the packet from queue
* returns:  length of packet
* arg1: *dst pointer where to store packet
* arg2: buff_size buffer size in order to prevent buffer overflow
**************************************************************/
int SCI_get_packet(int *dst, int buff_size)
{
    unsigned int index = 0;
    int byte;

    // read only if packet available
    if (SCI_rx_packets != 0)
    {
        // pobiram, dokler ne naletim na konec paketa
        do
        {
            SCI_disable_rx_interrupts();
            // get one byte
            byte = SCI_queue_get();

            // write the byte
            __byte(dst,index) = byte;
            index = index + 1;

            // if packet to big, then stop the CPU
            if (index >= buff_size)
            {
                asm(" ESTOP0");
                //break;
            }
            SCI_enable_rx_interrupts();
        } while (byte != 0);

        // decrease packet counter
        SCI_disable_rx_interrupts();
        SCI_rx_packets  = SCI_rx_packets - 1;
        SCI_enable_rx_interrupts();
    }
    return (index - 1);
}

/**************************************************************
* Check if packet is available
* returns:  TRUE or FALSE
**************************************************************/
bool SCI_chk_packet_ready(void)
{
    // Flush the FIFO
    SCI_flush_fifo();

    if (SCI_rx_packets != 0)
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

/**************************************************************
* Timeout timer initialization
* returns:
**************************************************************/
void SCI_timer_init(void)
{
    SCI_TIMER.PRD.all  = 0xFFFFFFFF;
    SCI_TIMER.TPR.all  = 0;
    SCI_TIMER.TPRH.all = 0;
    SCI_TIMER.TCR.bit.TSS = 1;
    SCI_TIMER.TCR.bit.TRB = 1;
    SCI_TIMER.TCR.bit.FREE = 0;
    SCI_TIMER.TCR.bit.SOFT = 1;
}

/**************************************************************
* Start timeout timer
* returns:
* arg1:     Timeout time in us
**************************************************************/
void SCI_timer_start(long timeout)
{
    long timer;

    timer = timeout * (CPU_FREQ / 1000000L);
    SCI_TIMER.PRD.all  = timer;
    SCI_TIMER.TCR.bit.TRB = 1;
    SCI_TIMER.TCR.bit.TIF = 0;
    SCI_TIMER.TCR.bit.TSS = 0;

}

/**************************************************************
* check for timeout
* returns:  TURE or FALSE regardless if timer has counted the time
**************************************************************/
int  SCI_timer_check_timeout(void)
{
    int return_value = FALSE;
    if (SCI_TIMER.TCR.bit.TIF == 1)
    {
        return_value = TRUE;
        SCI_TIMER.TCR.bit.TSS = 1;
    }
    return (return_value);
}



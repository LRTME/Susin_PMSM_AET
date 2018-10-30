/*************************************************************
* FILE:         SCI_drv.h
* DESCRIPTION:  SCI port driver with FIFO and interrupts
* AUTHOR:       Mitja Nemec
*
*************************************************************/
#ifndef     __SCI_DRV_H__
#define     __SCI_DRV_H__
        
#include    "F28x_Project.h"
#include    "define.h"

// which peripheral is in use
#define     SCI_MODUL       SciaRegs

// which timer to use for timeout - NOT USED
#define     SCI_TIMER       CpuTimer2Regs

// SCI FIFO buffer depth
#define     SCI_FIFO_DEPTH  12

// SCI RX buffer length
#define     SCI_RX_BUFFER_SIZE  40

// SCI peripheral response on debug event
// 0 - stop immediatelly, 2 - run free
#define     SCI_DEBUG_MODE      2

/**************************************************************
* read n-bytes from SCI port
* data is writen to low bytes of integer only
* returns:  number of characteres received
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout in us, to wait for all the data
**************************************************************/
extern int SCI_receive_byte(void *data, int nr_bytes, long timeout);

/**************************************************************
* read n-bytes from SCI port
* data is writen to high and low bytes of integer
* returns:  number of characteres received
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout in us, to wait for all the data
**************************************************************/
extern int SCI_receive_word(void *data, int nr_bytes, long timeout);

/**************************************************************
* send n-bytes from SCI port (blocking)
* only low bytes of integer are sent
* returns:  number of bytes to sent
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout (in us) for data to send
**************************************************************/
extern int SCI_send_byte(void *data, int nr_bytes, long timeout);

/**************************************************************
* send n-bytes from SCI port (non-blocking)
* sends high and low bytes of integer
* returns:  number of bytes sent
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout (in us) for data to send
**************************************************************/
extern int SCI_send_word(void *data, int nr_bytes);

/**************************************************************
* send n-bytes from SCI port (blocking)
* sends high and low bytes of integer
* returns:  number of bytes sent
* arg1:     pointer to data string
* arg2:     number of bytes to send
* arg3:     timeout (in us) for data to send
**************************************************************/
extern int SCI_send_word_blocking(void *data, int nr_bytes, long timeout);

/**************************************************************
* checks if all data was sent
* returns:  TRUE or FALSE
**************************************************************/
extern bool SCI_data_sent(void);

/**************************************************************
* read the packet from queue
* returns:  length of packet
* arg1: *dst pointer where to store packet
* arg2: buff_size buffer size in order to prevent buffer overflow
**************************************************************/
extern int SCI_get_packet(int *dst, int buff_size);

/**************************************************************
* Check if packet is available
* returns:  TRUE or FALSE
**************************************************************/
extern bool SCI_chk_packet_ready(void);

/**************************************************************
* SCI modul initialization
* returns:
* arg1:     baud rate
**************************************************************/
extern void SCI_init(long baud_rate);

/**************************************************************
* disables SCI module interrupt
* returns:
**************************************************************/
extern inline void SCI_disable_rx_interrupts(void);

/**************************************************************
* enables SCI module interrupt
* returns:
**************************************************************/
extern inline void SCI_enable_rx_interrupts(void);

#endif      // __SCI_DRV_H__

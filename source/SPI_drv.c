/**************************************************************
* FILE:         SPI_drv.c
* DESCRIPTION:  basic SPI driver
* AUTHOR:       Mitja Nemec
**************************************************************/
#include    "SPI_drv.h"
#include 	"F28x_Project.h"
#include    "define.h"


/**************************************************************
* Funkcija, ki inicializira komunikacijo
**************************************************************/
void SPI_init(int bits, long clock, int clk_phase, int clk_polarity)
{
    long SpiaRegs_clock = 1;
    long spi_brr = 1;
    int  spi_psc = 0;

    // za pravilno nastavitev Baud registra, da dosežemo pravo uro
    if (ClkCfgRegs.LOSPCP.bit.LSPCLKDIV == 0)
    {
        spi_psc = 1;
    }
    else
    {
        spi_psc = ClkCfgRegs.LOSPCP.bit.LSPCLKDIV << 1;
    }

    // izraèunam vrednost za baud rate register
    SpiaRegs_clock = ((long)CPU_FREQ) / spi_psc;

    spi_brr = (SpiaRegs_clock / clock) - 1;

    if (spi_brr < 0)
    {
        spi_brr = 0;
    }

    // lahko imam problem, èe je spi_brr veèji od 127
    if (spi_brr > 127)
    {
        spi_brr = 127;
    }

    // najprej nastavimo GPIO pine
    GPIO_SetupPinMux(24, GPIO_MUX_CPU1, 6); 			// GPIO multiplexing (GPIO_NUM, CPU_SEL, GPIO_index) - GPIO24 is SPISIMOB
    GPIO_SetupPinOptions(24, GPIO_INPUT, GPIO_INPUT);  	// GPIO direction    (GPIO_NUM, GPIO_DIR, GPIO_flag)

    GPIO_SetupPinMux(25, GPIO_MUX_CPU1, 6); 			// GPIO multiplexing (GPIO_NUM, CPU_SEL, GPIO_index) - GPIO25 is SPISOMIB
    GPIO_SetupPinOptions(25, GPIO_INPUT, GPIO_INPUT);  	// GPIO direction    (GPIO_NUM, GPIO_DIR, GPIO_flag)

    GPIO_SetupPinMux(26, GPIO_MUX_CPU1, 6); 			// GPIO multiplexing (GPIO_NUM, CPU_SEL, GPIO_index) - GPIO26 is SPICLKB
    GPIO_SetupPinOptions(26, GPIO_INPUT, GPIO_INPUT);  	// GPIO direction    (GPIO_NUM, GPIO_DIR, GPIO_flag)

    GPIO_SetupPinMux(27, GPIO_MUX_CPU1, 6); 			// GPIO multiplexing (GPIO_NUM, CPU_SEL, GPIO_index) - GPIO27 is SPISTEB
    GPIO_SetupPinOptions(27, GPIO_INPUT, GPIO_INPUT);  	// GPIO direction    (GPIO_NUM, GPIO_DIR, GPIO_flag)

    // zdaj pa inicializiramo SPI vmesnik za FIFO delovanje
    SPI_MODUL.SPICCR.bit.SPISWRESET = 0;     // reset SPI

    // clock polarity
    SPI_MODUL.SPICCR.bit.CLKPOLARITY = clk_polarity;    // clock polarity
    SPI_MODUL.SPICCR.bit.SPILBK = 0;         // loopback disabled
    SPI_MODUL.SPICCR.bit.SPICHAR = bits-1;   // bit lenght
    SPI_MODUL.SPICTL.bit.OVERRUNINTENA = 0;  // no overrun interrupta

    // clock phase
    SPI_MODUL.SPICTL.bit.CLK_PHASE = clk_phase;      // clock phase
    SPI_MODUL.SPICTL.bit.MASTER_SLAVE = 1;   // SPI is master
    SPI_MODUL.SPICTL.bit.TALK = 1;           // enable transmit
    SPI_MODUL.SPICTL.bit.SPIINTENA = 0;      // no interrupts
    SPI_MODUL.SPISTS.bit.BUFFULL_FLAG = 1;   // clear flags
    SPI_MODUL.SPISTS.bit.INT_FLAG = 1;       // clear flags
    SPI_MODUL.SPISTS.bit.OVERRUN_FLAG = 1;   // clear flags

    SPI_MODUL.SPIBRR.bit.SPI_BIT_RATE = spi_brr;          // SPICLK =   Mhz

    SPI_MODUL.SPIFFTX.bit.SPIRST = 0;        // SPI FIFO reset
    SPI_MODUL.SPIFFTX.bit.SPIFFENA = 0;      // enable FIFO
    SPI_MODUL.SPIFFTX.bit.TXFIFO = 0;        // reset FIFO cnt
    SPI_MODUL.SPIFFTX.bit.TXFIFO = 0;        // enable FIFO cnt
    SPI_MODUL.SPIFFTX.bit.TXFFIENA = 0;      // no FIFO interrupts
    SPI_MODUL.SPIFFCT.bit.TXDLY = 0;         // 0 delay bits
    SPI_MODUL.SPIFFTX.bit.TXFFINTCLR = 1;    // clear flags
    SPI_MODUL.SPIFFRX.bit.RXFIFORESET = 0;   // put out of reset
    SPI_MODUL.SPIFFRX.bit.RXFFIENA = 0;      // no RX interrupts
    SPI_MODUL.SPIFFRX.bit.RXFFINTCLR = 1;    // clear flags
    SPI_MODUL.SPIFFRX.bit.RXFFOVFCLR = 1;    // clear flags
    SPI_MODUL.SPIFFRX.bit.RXFIFORESET = 1;   // put out of reset
    SPI_MODUL.SPIFFTX.bit.SPIRST = 1;        // SPI FIFO out of reset
    SPI_MODUL.SPICCR.bit.SPISWRESET = 1;     // put out ofreset SPI
}

/**************************************************************
* Funkcija, ki komunicira z SPI dajalnikom položaja
* returns:  koda, ki jo vrne dajalnik
**************************************************************/
#pragma CODE_SECTION(SPI_getset_blocking, "ramfuncs");
unsigned int SPI_getset_blocking(unsigned int data_out)
{
    unsigned int data_in;

    // pocakam, da lahko posljem nov podatek
    while(SPI_MODUL.SPISTS.bit.BUFFULL_FLAG == 1)
    {
        // DO NOTHING
    }

    // posljem nov paket, da znova dobim podatek
    SPI_MODUL.SPITXBUF = data_out;

    // pocakam, da prejmem nov podatek
    while(SPI_MODUL.SPISTS.bit.INT_FLAG != 1)
    {
        // DO NOTHING
    }

    // preberem podatek
    data_in = SPI_MODUL.SPIRXBUF;

    return(data_in);
}

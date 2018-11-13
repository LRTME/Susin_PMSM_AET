/****************************************************************
* FILENAME:     PER_int.c
* DESCRIPTION:  periodic interrupt code
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include    "PER_int.h"
#include    "TIC_toc.h"

// variables required for voltage measurement
float   voltage = 0.0;

// CPU load estimation
float   cpu_load  = 0.0;
long    interrupt_cycles = 0;

// CPU temperature
float	cpu_temp = 0.0;

// counter of too long interrupt function executions
int     interrupt_overflow_counter = 0;

// overcurrent trip variable
bool 	trip_oc_flag = FALSE;


// mechanical variables
bool 	incremental_encoder_connected = FALSE;
int		kot_raw = 0;
float	kot_meh = 0.0;
float	kot_el = 0.0;

// current variables
bool 	current_offset_calibrated = FALSE;
long 	current_offset_counter = 0;

long   	tok_i1_raw_accu = 0;
long   	tok_i2_raw_accu = 0;
long   	tok_i3_raw_accu = 0;

long	tok_i1_raw_offset = 0;
long	tok_i2_raw_offset = 0;
long	tok_i3_raw_offset = 0;

float   tok_i_gain = (48.0/0.625) * (7.5/5.6) * (1.0/4096.0);// (7.5/5.6) * (48.0/0.625) * (3.0/4096.0) * (1.0/3.0);

float	tok_i1 = 0.0;
float	tok_i2 = 0.0;
float	tok_i3 = 0.0;

// voltage variables
float	nap_v1_offset = 0.0;
float	nap_v2_offset = 0.0;
float	nap_v3_offset = 0.0;
float	nap_dc_offset = 0.0;

float   nap_v_gain = 0.982 * ((620.0 + 10000.0)/620.0) * (3.0/4096.0);
float	nap_dc_gain = 0.971 * ((620.0 + 10000.0)/620.0) * (3.0/4096.0);

float	nap_v1 = 0.0;
float	nap_v2 = 0.0;
float	nap_v3 = 0.0;
float	nap_dc = 0.0;

// other electrical variables
float	pot_rel = 0.0;

/* control algorithm variables */
// general variables
bool	set_null_position_flag = FALSE;
bool	reset_null_position_procedure = FALSE;
bool	control_enable_flag = FALSE;
float	duty_DC = 0.0;
float 	duty_six_step = 0.0;
int 	sector_six_step = 1;
float	V_alpha = 0.0;
float	V_beta = 0.0;
float	amp_rel = 0.0;
float	freq = 0.0;
float	freq_meh = 0.0;

volatile enum	MODULATION modulation = SVM;
volatile enum	CONTROL control = OPEN_LOOP;

IPARK_float		voltage_open_loop = IPARK_FLOAT_DEFAULTS;

// current variables
CLARKE_float 	clarke_tok = CLARKE_FLOAT_DEFAULTS;
PARK_float 		park_tok = PARK_FLOAT_DEFAULTS;
float	tok_d = 0.0;
float	tok_q = 0.0;

// temporary variables
float 	temp1 = 0.0;
float 	temp2 = 0.0;
float 	temp3 = 0.0;

// extern variables
extern bool sw1_state;
extern bool b1_state;
extern bool b1_press_int;
extern bool b2_press_int;
extern bool b3_press_int;
extern bool b4_press_int;
extern enum SVM_STATE svm_status;

// functions
void 	get_mechanical(void);
void 	get_meh_speed(void);
void 	get_meh_accel(void);
void 	get_electrical(void);
void 	set_null_position(bool reset_procedure);
void	control_algorithm(void);
void	open_loop_control(void);
void	current_loop_control(void);
void	speed_loop_control(void);
void	position_loop_control(void);

/**************************************************************
* interrupt funcion
**************************************************************/
#pragma CODE_SECTION(PER_int, "ramfuncs");
void interrupt PER_int(void)
{
    /// local variables
    
    // acknowledge interrupt within PWM module
    EPwm1Regs.ETCLR.bit.INT = 1;
    // acknowledge interrupt within PIE module
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
    
    // start CPU load stopwatch
    interrupt_cycles = TIC_time;
    TIC_start();

    // get previoust CPU load estimate
    cpu_load = (float)interrupt_cycles / (CPU_FREQ/SAMPLE_FREQ);

    // increase and wrap around interrupt counter every 1 second
    interrupt_cnt = interrupt_cnt + 1;
    if (interrupt_cnt >= SAMPLE_FREQ)
    {
        interrupt_cnt = 0;
        interrupt_cnt_s = interrupt_cnt_s + 1;
    }

    // number of seconds passed
    if (interrupt_cnt_s == 60)
    {
    	interrupt_cnt_s = 0;
    	interrupt_cnt_min = interrupt_cnt_min + 1;
    }


    // reference value generator
    REF_GEN_update();

    // wait for the ADC to finish with conversion
    ADC_A_wait();
    ADC_B_wait();
    ADC_C_wait();

    // calculate CPU temperature
    cpu_temp = GetTemperatureC(ADC_TEMP);

    // read mechanical (rotor angle) and electrical signals (currents, voltages, pots, ...)
    get_mechanical();
    get_electrical();




    /* 3 phase inverter control alghorithm */

    // b2 changes modulation mode
    if(b2_press_int == TRUE)
    {
    	modulation = modulation + 1;
    }
    if(modulation == 3)
    {
    	modulation = 0;
    }

    // b3 changes control mode
    if(b3_press_int == TRUE)
    {
    	control = control + 1;
    }
    if(control == 4)
    {
    	control = 0;
    }

    // b4 toggles LED4
    if(b4_press_int == TRUE)
    {
    	PCB_LED4_toggle();
    }

    if(modulation != SVM)
    {
    	control = OPEN_LOOP;
    }

	// switch 1 means on/off
    if(sw1_state == FALSE)
    {
    	SVM_disable();
    	PCB_LED2_off();
    	set_null_position_flag = FALSE;
    	control_enable_flag = FALSE;
    	reset_null_position_procedure = TRUE;
    	incremental_encoder_connected = FALSE;
    }
    else
    {
    	if(set_null_position_flag == FALSE)
    	{
    		set_null_position(reset_null_position_procedure);
    		reset_null_position_procedure = FALSE;
    	}
    	else
    	{
    		if(b1_press_int == TRUE && control_enable_flag == FALSE && pot_rel < 0.1)
    		{
    			control_enable_flag = TRUE;
    			SVM_enable();
    			SVM_update(0.0, 0.0);
    			PCB_LED2_on();
    		}
    		else if(b1_press_int == TRUE && control_enable_flag == TRUE)
    		{
    			control_enable_flag = FALSE;
    			SVM_disable();
    			PCB_LED2_off();
    		}
    	} // end of null position
    } // end of enable switch

    if(control_enable_flag == TRUE)
    {
    	control_algorithm();
    }

    /* End of 3 phase inverter control alghorithm */




    // store values for display within CCS or GUI
    DLOG_GEN_update();
    



    /* If overcurrent trip event has occured, shut down power stage
     * and signalise with red LED.
     */
    if(PCB_TRIP_OC_read() == TRUE || SVM_MODUL1.TZSEL.bit.OSHT1)
    {
    	trip_oc_flag = TRUE;
    }

    if(trip_oc_flag == TRUE)
    {
    	SVM_trip();
    }

    /* Test if new interrupt is already waiting.
     * If so, then something is seriously wrong.
     */
    if (EPwm1Regs.ETFLG.bit.INT == TRUE)
    {
        // count number of interrupt overflow events
        interrupt_overflow_counter = interrupt_overflow_counter + 1;

        /* if interrupt overflow event happened more than 10 times
         * stop the CPU
         *
         * Better solution would be to properly handle this event
         * (shut down the power stage, ...)
         */
        if (interrupt_overflow_counter >= 10)
        {
        	SVM_trip();
        	PCB_LED1_on();
        	PCB_LED2_off();
        	PCB_LED3_off();
        	PCB_LED4_off();
            asm(" ESTOP0");
        }
    }
    
    // signalize trip state with red LED
    if(svm_status == TRIP)
    {
    	PCB_LED1_on();
    	PCB_LED2_off();
    	PCB_LED3_off();
    	PCB_LED4_off();
    }
    else
    {
    	PCB_LED1_off();
    }

    // stop the CPU load stopwatch
    TIC_stop();

    // clear buttons, if pressed
    b1_press_int = FALSE;
    b2_press_int = FALSE;
    b3_press_int = FALSE;
    b4_press_int = FALSE;

}   // end of PWM_int




/**************************************************************
* Function, where mechanical measurements is handled:
* - mechanical angle [0.0 1.0] (1.0 means one full mechanical revolution)
* - electrical angle [0.0 1.0] (1.0 means one quarter of revolution, if pole pair is 4)
* - calls function for mechanical speed calculation
* - calls function for mechanical acceleration calculation
**************************************************************/
#pragma CODE_SECTION(get_mechanical, "ramfuncs");
void get_mechanical(void)
{
	// lokalne spremenljivke
    int i;


    // signal iz inkrementalnega dajalnika - mehanski kot
    // preberem kot rotorja iz QEP modula
    kot_raw = QEP_cnt();

    // izracunam kot rotorja [1]
    kot_meh = QEP_mehKot();

    // omejim mehanski kot od 0.0 do 1.0
    if (kot_meh < 0.0)
    {
    	kot_meh = kot_meh + 1.0;
    }
    if (kot_meh >= 1.0)
    {
    	kot_meh = kot_meh - 1.0;
    }


    // elektri�ni kot

    // iz mehanskega kota izra�unam �e elektri�nega
    kot_el = POLE_PAIRS*kot_meh;


    // omejim elektri�ni kot od 0.0 do 1.0
    if (kot_el < 0.0)
    {
    	kot_el = kot_el + 1.0;
    }
    // od�tejem zaradi polovih parov P, da dobim kot_el med 0.0 in 1.0
    for (i = POLE_PAIRS - 1; i > 0; i = i - 1)
    {
    	if (kot_el >= i*1.0)
    	{
    		kot_el = kot_el - i*1.0;
    		break;
    	}

    }

    // pokli�em funkcijo, ki vrne mehansko kro�no frekvenco
//    get_meh_speed();

    // pokli�em funkcijo, ki vrne mehanski kotni pospe�ek
//    get_meh_accel();

} // end of function



/**************************************************************
* Function, where mechanical speed is calculated (out of mechanical angle)
**************************************************************/
#pragma CODE_SECTION(get_meh_speed, "ramfuncs");
void get_meh_speed(void)
{

}




/**************************************************************
* Function, where mechanical acceleration is calculated (out of mechanical angle)
**************************************************************/
#pragma CODE_SECTION(get_meh_accel, "ramfuncs");
void get_meh_accel(void)
{

}




/**************************************************************
* Function, where electrical measurements is handled
**************************************************************/
#pragma CODE_SECTION(get_electrical, "ramfuncs");
void get_electrical(void)
{
    // lokalne spremenljivke

    /* preberem vrednosti iz AD pretvornika */

    // tokovi 1,2,3
    // kalibracija preostalega toka
    if (current_offset_calibrated == FALSE)
    {
        // akumuliram offset
        tok_i1_raw_accu = tok_i1_raw_accu + ADC_CURRENT_1;
        tok_i2_raw_accu = tok_i2_raw_accu + ADC_CURRENT_2;
        tok_i3_raw_accu = tok_i3_raw_accu + ADC_CURRENT_3;

        // ko potece dovolj casa, sporocim da lahko grem naprej
        // in izracunam povprecni offset
        current_offset_counter = current_offset_counter + 1;
        if (current_offset_counter == SAMPLE_FREQ)
        {
            current_offset_calibrated = TRUE;
            tok_i1_raw_offset = tok_i1_raw_accu / SAMPLE_FREQ;
            tok_i2_raw_offset = tok_i2_raw_accu / SAMPLE_FREQ;
            tok_i3_raw_offset = tok_i3_raw_accu / SAMPLE_FREQ;
        }
        tok_i1 = 0.0;
        tok_i2 = 0.0;
        tok_i3 = 0.0;
    }
    else
    {
        tok_i1 = tok_i_gain * (ADC_CURRENT_1 - tok_i1_raw_offset);
        tok_i2 = tok_i_gain * (ADC_CURRENT_2 - tok_i2_raw_offset);
        tok_i3 = tok_i_gain * (ADC_CURRENT_3 - tok_i3_raw_offset);
    }

    // napetosti 1,2,3
    nap_v1 = nap_v_gain * (ADC_VOLTAGE_1 - nap_v1_offset);
    nap_v2 = nap_v_gain * (ADC_VOLTAGE_2 - nap_v2_offset);
    nap_v3 = nap_v_gain * (ADC_VOLTAGE_3 - nap_v3_offset);

    // napetost DC linka
    nap_dc = nap_dc_gain * (ADC_VOLTAGE_DC - nap_dc_offset);

    // pot_rel
    pot_rel = ADC_POT * (1/4096.0);
    // okoli nicle vrzem zeleno vrednost na cisto niclo
    if ((pot_rel > 0.0) && (pot_rel < +0.05))
    {
        pot_rel = 0.0;
    }
/*    if (pot_rel > +0.05)
    {
        pot_rel = pot_rel - 0.05;
    }
    if ((pot_rel < 0.0) && (pot_rel > -0.05))
    {
        pot_rel = 0.0;
    }
    if (pot_rel < -0.05)
    {
        pot_rel = pot_rel + 0.05;
    }
*/

    // Clarke-ina transformacija tokov
    clarke_tok.As = tok_i1;
    clarke_tok.Bs = tok_i2;
    CLARKE_FLOAT_CALC(clarke_tok);

    // Park-ova transformacija tokov
    park_tok.Alpha = clarke_tok.Alpha;
    park_tok.Beta = clarke_tok.Beta;
    park_tok.Angle = 2 * PI * kot_el;
    PARK_FLOAT_CALC(park_tok);
    tok_d = park_tok.Ds;
    tok_q = park_tok.Qs;

/*
    // izra�un dejanskega navora
    navor_elektromagnetni = 3.0/2.0*POLE_PAIRS*PSI_ROT*tok_q;
    navor_reluktancni = 3.0/2.0*POLE_PAIRS*(Ld - Lq)*tok_d*tok_q;

    navor = navor_elektromagnetni + navor_reluktancni;

    // izra�un dinami�nega navora
    navor_dinamicni = 2*PI* J * alfa_meh;
*/
} // end of function




/**************************************************************
* Function, which alignes d axis with phase 1 axis
**************************************************************/
#pragma CODE_SECTION(set_null_position, "ramfuncs");
void set_null_position(bool reset_procedure)
{
	/* Vnaprej vem, da moram v kratkem stiku na posamezno
	 * fazo PMSM-ja pritisniti 2V, da se bo zavrtel v pravo lego.
	 */
    float 	duty_cycle = 2.0/nap_dc;
    int 	sector;

    static int	i = 0;
    static long interrupts_passed = 0;

    static unsigned int kot_raw = 0.0;
    static unsigned int kot_raw_old = 0.0;

	SVM_enable();
	PCB_LED2_on();

	if(reset_procedure == TRUE)
	{
		i = 0;
		interrupts_passed = 0;
		kot_raw = 0;
	}

    if (i == 0)    // pol sekunde pritiskam napetostni vektor v smeri faze 3
    {
        sector = 5;
        SVM_update_six(duty_cycle, sector);
        interrupts_passed = interrupts_passed + 1;

        if(interrupts_passed > SAMPLE_FREQ/2)
        {
        	i = 1;
        	interrupts_passed = 0;

        	// simple check if incremental encoder is connected
        	kot_raw = QEP_cnt();
        	if(kot_raw - kot_raw_old != 0)
        	{
        		kot_raw_old = kot_raw;
        	}
        }
    } // end of i == 0

    if (i == 1)    // pol sekunde pritiskam napetostni vektor v smeri faze 1
    {
        sector = 1;
        SVM_update_six(duty_cycle, sector);
        interrupts_passed = interrupts_passed + 1;

        if(interrupts_passed > SAMPLE_FREQ/2)
        {
        	i = 2;
        	interrupts_passed = 0;

        	// simple check if incremental encoder is connected
        	kot_raw = QEP_cnt();
        	if(kot_raw - kot_raw_old != 0)
        	{
        		incremental_encoder_connected = TRUE;
        	}
        }
    } // end of i == 1

    if (i == 2)     // �ez nekaj �asa re�em to je pozicija 0, in postavim zastavico
    {
    	interrupts_passed = interrupts_passed + 1;

    	if(interrupts_passed >= SAMPLE_FREQ/2)
    	{
    		set_null_position_flag = TRUE;
    		interrupts_passed = 0;
    		i = 0;

    		// resetiram pozicijo rotorja
    		QEP_reset();

    		// onemogo�im mosti�
    		SVM_disable();
    		PCB_LED2_off();
    	}
    } // end of i == 2

} // end of function





/**************************************************************
* Function, which covers control of 3 phase PMSM
**************************************************************/
#pragma CODE_SECTION(control_algorithm, "ramfuncs");
void control_algorithm(void)
{
	switch(control)
	{
	case OPEN_LOOP:
		open_loop_control();
		break;
	case CURRENT_CONTROL:
		current_loop_control();
		break;
	case SPEED_CONTROL:
		speed_loop_control();
		break;
	case POSITION_CONTROL:
		position_loop_control();
		break;
	default:
		SVM_disable();
		PCB_LED2_off();
		break;
	}

	switch(modulation)
	{
	case SVM:
		SVM_update(V_alpha, V_beta);
		break;
	case SIX_STEP:
		SVM_update_six(duty_six_step, sector_six_step);
		break;
	case SINGLE_PHASE_DC:
		SVM_update_DC(duty_DC);
		break;
	default:
		SVM_disable();
		PCB_LED2_off();
		break;
	}
}




/**************************************************************
* Function for open loop control
**************************************************************/
#pragma CODE_SECTION(open_loop_control, "ramfuncs");
void open_loop_control(void)
{
	if(modulation == SVM)
	{
		if(incremental_encoder_connected == FALSE)
		{
			// if incremental encoder is NOT connnected
			freq = POLE_PAIRS * freq_meh;
			amp_rel = pot_rel;

			V_alpha = amp_rel*cos(2*PI*freq*interrupt_cnt/SAMPLE_FREQ);
			V_beta =  amp_rel*sin(2*PI*freq*interrupt_cnt/SAMPLE_FREQ);
		}
		else
		{
			// if incremental encoder is connnected
			voltage_open_loop.Ds = 0.0;
			voltage_open_loop.Qs = pot_rel*0.577;
			voltage_open_loop.Angle = kot_el;

			IPARK_FLOAT_CALC(voltage_open_loop);

			V_alpha = voltage_open_loop.Alpha;
			V_beta = voltage_open_loop.Beta;
		}
	}
	else if(modulation == SIX_STEP)
	{
		// duty_six_step = 0.1;
		// sector_six_step = 1;

		if(pot_rel > 0.0)
		{
			sector_six_step = 1;
		}
		if(pot_rel > 0.15)
		{
			sector_six_step = 2;
		}
		if(pot_rel > 0.30)
		{
			sector_six_step = 3;
		}
		if(pot_rel > 0.45)
		{
			sector_six_step = 4;
		}
		if(pot_rel > 0.60)
		{
			sector_six_step = 5;
		}
		if(pot_rel > 0.75)
		{
			sector_six_step = 6;
		}



	}
	else if(modulation == SINGLE_PHASE_DC)
	{
		duty_DC = pot_rel;
	}
}




/**************************************************************
* Function for current loop control
**************************************************************/
#pragma CODE_SECTION(current_loop_control, "ramfuncs");
void current_loop_control(void)
{

}




/**************************************************************
* Function for speed loop control
**************************************************************/
#pragma CODE_SECTION(speed_loop_control, "ramfuncs");
void speed_loop_control(void)
{

}




/**************************************************************
* Function for position loop control
**************************************************************/
#pragma CODE_SECTION(position_loop_control, "ramfuncs");
void position_loop_control(void)
{

}




/**************************************************************
* Function which initializes all required for execution of
* interrupt function
**************************************************************/
void PER_int_setup(void)
{
    // initialize data logger
    dlog.mode = Normal;
    dlog.auto_time = 1;
    dlog.holdoff_time = 1;

    dlog.downsample_ratio = 1;

    dlog.slope = Negative;
    dlog.trig = &ref_gen.angle;
    dlog.trig_level = 0.5;

    dlog.iptr1 = &ref_gen.angle;
    dlog.iptr2 = &cpu_temp;
    dlog.iptr3 = &V_alpha;
    dlog.iptr4 = &V_beta;

    // initialize reference generator
    ref_gen.type = REF_Step;
    ref_gen.amp = 1.0;
    ref_gen.offset = 0.0;
    ref_gen.freq = 1.0;
    ref_gen.duty = 0.5;
    ref_gen.slew = 100;
    ref_gen.samp_period = SAMPLE_TIME;
    
    // initialize stopwatch
    TIC_init();

    // setup interrupt trigger
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_ZERO;
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;
    EPwm1Regs.ETCLR.bit.INT = 1;
    EPwm1Regs.ETSEL.bit.INTEN = 1;

    // register the interrupt function
    EALLOW;
    PieVectTable.EPWM1_INT = &PER_int;
    EDIS;

    // acknowledge any spurious interrupts
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

    // enable interrupt within PIE
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;

    // enable interrupt within CPU
    IER |= M_INT3;

    // enable interrupt in real time mode
    SetDBGIER(M_INT3);
}


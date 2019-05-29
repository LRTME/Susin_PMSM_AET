/****************************************************************
* FILENAME:     PER_int.c
* DESCRIPTION:  periodic interrupt code
* AUTHOR:       Mitja Nemec, Denis Susin
*
****************************************************************/
#include    "PER_int.h"
#include    "TIC_toc.h"
#include    "TIC_toc_1.h"

/**************************************************************************
 * User interface - variables that can be changed during operation by user
**************************************************************************/
volatile enum	MODULATION modulation = SVM;
volatile enum	CONTROL control = OPEN_LOOP;

volatile enum	{NONE, RES, RES_multiple, REP, DCT} advanced_current_reg_type = NONE;

bool			control_enable = FALSE;

bool			enable_advanced_current_reg = FALSE;

bool			auto_calc_of_advanced_reg_params = FALSE;

/**************************************************************************
 * End of user interface
**************************************************************************/

// CPU load estimation
float   cpu_load  = 0.0;
long    interrupt_cycles = 0;

// CPU temperature
float	cpu_temp = 0.0;

// counter of too long interrupt function executions
int     interrupt_overflow_counter = 0;


// mechanical variables
int		kot_raw = 0;
float	kot_meh_ref = 0.0;
float	kot_meh = 0.0;
float	kot_el = 0.0;
float	speed_meh_ref = 0.0;
float	speed_meh_ABF = 0.0;
float 	period_CAP;
float 	speed_meh_CAP;
int 	direction_QEP;
float	accel_meh_Hz_per_s = 0.0;

ABF_omega	abf_speed_meh_ABF = ABF_OMEGA_DEFAULTS;
ABF_omega	abf_accel_meh = ABF_OMEGA_DEFAULTS;


// current variables
long 	current_offset_counter = 0;

long   	tok_i1_raw_accu = 0;
long   	tok_i2_raw_accu = 0;
long   	tok_i3_raw_accu = 0;

long	tok_i1_raw_offset = 0;
long	tok_i2_raw_offset = 0;
long	tok_i3_raw_offset = 0;

float   tok_i_gain = 1.10*(48.0/0.625) * (7.5/5.6) * (1.0/4096.0);// (7.5/5.6) * (48.0/0.625) * (3.0/4096.0) * (1.0/3.0);

float	tok_i1 = 0.0;
float	tok_i2 = 0.0;
float	tok_i3 = 0.0;

float	tok_d_ref = 0.0;
float	tok_q_ref = 0.0;
float	tok_d = 0.0;
float	tok_q = 0.0;

CLARKE_float 	clarke_tok = CLARKE_FLOAT_DEFAULTS;
PARK_float 		park_tok = PARK_FLOAT_DEFAULTS;


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
float	nap_v12 = 0.0;
float	nap_v23 = 0.0;
float	nap_v31 = 0.0;
float	nap_dc = 0.0;

float	nap_alpha_ref = 0.0;
float	nap_beta_ref = 0.0;
float	nap_d_ref = 0.0;
float	nap_q_ref = 0.0;
float	nap_dq_ref = 0.0;
bool	saturation = FALSE;


// other electrical variables
float	pot_rel = 0.0;
float	pot_rel_discrete = 0.0;
float	pot_rel_discrete_old = 0.0;


/* control algorithm variables */
// general variables
float	duty_DC = 0.0;
float 	duty_six_step = 0.0;
int 	sector_six_step = 1;
float	amp_rel = 0.0;
float	freq = 0.0;
float	freq_meh = 0.0;

int		direction = 1;
long	tic_direction = 0;
long	delta_tic_direction = 0;

IPARK_float		ipark_nap = IPARK_FLOAT_DEFAULTS;


// controllers variables
PI_ctrl			id_PI_reg = PI_CTRL_DEFAULTS;
PI_ctrl			iq_PI_reg = PI_CTRL_DEFAULTS;
PI_ctrl			speed_PI_reg = PI_CTRL_DEFAULTS;
PID_ctrl		position_PID_reg = PI_CTRL_DEFAULTS;
float			advanced_id_reg_out = 0.0;
float			advanced_iq_reg_out = 0.0;

// current PI controller
float   Kp_id_PI_reg = 0.015;    	  		// Vdc = 12V: 0.03            	Vdc = 24V: 0.015
float   Ki_id_PI_reg = 13.0/SAMPLE_FREQ;	// Vdc = 12V: 26.0/SAMPLE_FREQ 	Vdc = 24V: 13.0/SAMPLE_FREQ
float   Kp_iq_PI_reg = 0.015;    			// Vdc = 12V: 0.03            	Vdc = 24V: 0.015
float   Ki_iq_PI_reg = 13.0/SAMPLE_FREQ;   	// Vdc = 12V: 26.0/SAMPLE_FREQ 	Vdc = 24V: 13.0/SAMPLE_FREQ

// advanced current multiple resonant (RES) controllers
RES_REG_float	id_RES_reg_1 = RES_REG_FLOAT_DEFAULTS;
RES_REG_float	iq_RES_reg_1 = RES_REG_FLOAT_DEFAULTS;
RES_REG_float	id_RES_reg_2 = RES_REG_FLOAT_DEFAULTS;
RES_REG_float	iq_RES_reg_2 = RES_REG_FLOAT_DEFAULTS;
RES_REG_float	id_RES_reg_3 = RES_REG_FLOAT_DEFAULTS;
RES_REG_float	iq_RES_reg_3 = RES_REG_FLOAT_DEFAULTS;
RES_REG_float	id_RES_reg_4 = RES_REG_FLOAT_DEFAULTS;
RES_REG_float	iq_RES_reg_4 = RES_REG_FLOAT_DEFAULTS;
RES_REG_float	id_RES_reg_5 = RES_REG_FLOAT_DEFAULTS;
RES_REG_float	iq_RES_reg_5 = RES_REG_FLOAT_DEFAULTS;
RES_REG_float	id_RES_reg_6 = RES_REG_FLOAT_DEFAULTS;
RES_REG_float	iq_RES_reg_6 = RES_REG_FLOAT_DEFAULTS;
float 			id_multiple_RES_reg_out = 0.0;
float 			iq_multiple_RES_reg_out = 0.0;
float			cas_izracuna_RES_reg = 0.0;
float			cas_izracuna_RES_multiple_reg = 0.0;

// automatic calculation of RES current controller parameters
float			freq_critical = 0.0;
float			factor_res_reg_gain = 0.0;
float			factor_res_reg_gain_additional = 1.0;

// advanced current repetitive (REP) controllers
REP_REG_float	id_REP_reg = REP_REG_FLOAT_DEFAULTS;
REP_REG_float	iq_REP_reg = REP_REG_FLOAT_DEFAULTS;
int 			clear_REP_buffer_index = 0;
float			cas_izracuna_REP_reg = 0.0;

// advanced current discrete cosinus transform (DCT) controller
//DCT_REG_float	id_DCT_reg = DCT_REG_FLOAT_DEFAULTS;
//DCT_REG_float	iq_DCT_reg = DCT_REG_FLOAT_DEFAULTS;
int 			clear_DCT_buffer_index = 0;
float			cas_izracuna_DCT_reg = 0.0;

// speed PI controller
float   Kp_speed_PI_reg = 3.0;  			// velja èe merimo napetost z ABF: Kp = 3.0
float   Ki_speed_PI_reg = 5e-4;  			// velja èe merimo napetost z ABF: Ki = 5e-4 (agresivno delovanje)

// position PID controller
float   Kp_position_PID_reg = 200.0;  		// velja, èe ni hitrostne zanke Kp = 200.0
float   Ki_position_PID_reg = 0.0;  		// velja, èe ni hitrostne zanke Ki = 0.0
float   Kd_position_PID_reg = 10.0;  		// velja, èe ni hitrostne zanke Kd = 10.0


// software limits
float	nap_dc_max = 50.0; 						// V
float	nap_dc_min = 0.0;						// V
float	nap_v_max = 50.0;						// V
float	nap_v_min = -50.0;						// V
float	tok_i_max = 45.0;						// A

float   nap_d_ref_max = 0.577350269189626; 		// per unit
float   nap_d_ref_min = -0.577350269189626;   	// per unit
float   nap_q_ref_max = 0.577350269189626;    	// per unit
float   nap_q_ref_min = -0.577350269189626;   	// per unit
float   tok_d_ref_max = 5.0;   					// A
float   tok_d_ref_min = -5.0;  					// A
float   tok_q_ref_max = 40.0;   				// A
float   tok_q_ref_min = -40.0;  				// A

float   navor_ref_max = 5.89;    				// Nm
float   navor_ref_min = -5.89;   				// Nm

float   speed_ref_max = 40.0;  					// Hz
float   speed_ref_min = -40.0; 					// Hz


// flags
bool 	current_offset_calibrated_flag = FALSE;

bool	direction_change_flag = FALSE;

bool	set_null_position_flag = FALSE;
bool	reset_null_position_procedure_flag = FALSE;

bool 	incremental_encoder_connected_flag = FALSE;

bool 	hardware_trip_oc_flag = FALSE;
bool	software_trip_flag = FALSE;
bool	trip_reset_flag = FALSE;

bool	nap_dc_overvoltage_flag = FALSE;
bool	nap_dc_undervoltage_flag = FALSE;
bool	nap_v1_overvoltage_flag = FALSE;
bool	nap_v1_undervoltage_flag = FALSE;
bool	nap_v2_overvoltage_flag = FALSE;
bool	nap_v2_undervoltage_flag = FALSE;
bool	nap_v3_overvoltage_flag = FALSE;
bool	nap_v3_undervoltage_flag = FALSE;
bool	tok_i1_overcurrent_flag = FALSE;
bool	tok_i2_overcurrent_flag = FALSE;
bool	tok_i3_overcurrent_flag = FALSE;


// temporary variables
float 	temp1 = 0.0;
float 	temp2 = 0.0;
float 	temp3 = 0.0;


// extern variables
extern bool 	sw1_state;
extern bool 	b1_state;
extern bool 	b1_press_int;
extern bool 	b2_press_int;
extern bool 	b3_press_int;
extern bool 	b4_press_int;
extern enum 	SVM_STATE svm_status;


// functions
void 	get_mechanical(void);
void 	get_meh_speed(void);
void 	get_meh_accel(void);
void 	get_electrical(void);
void 	set_null_position(bool reset_procedure);
void 	software_protection(void);
void	control_algorithm(void);
void	open_loop_control(void);
void	current_loop_control(void);
void	advanced_current_loop_control(void);
void	speed_loop_control(void);
void	position_loop_control(void);
void	trip_reset(void);
void 	clear_controllers(void);
void 	clear_advanced_controllers(void);
float	phase_lag_comp_calc(float phase_lag_freq);

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




    // buttons
    if(control_enable == FALSE)
    {
    	// button 2 changes direction of rotation
    	if(b2_press_int == TRUE)
    	{
    		direction = -direction;
    		if(direction >= 0)
    		{
    			direction = 1;
    		}
    		else
    		{
    			direction = -1;
    		}

    		direction_change_flag = TRUE;
        	tic_direction = interrupt_cnt;
    	}
    	// button 3 changes modulation mode
    	if(b3_press_int == TRUE)
    	{
    		modulation = modulation + 1;
    	}
    	if(modulation == 3)
    	{
    		modulation = 0;
    	}

    	// button 4 changes control mode
    	if(b4_press_int == TRUE)
    	{
    		control = control + 1;
    	}
    	if(control == 4)
    	{
    		control = 0;
    	}

    	if(modulation != SVM)
    	{
    		control = OPEN_LOOP;
    	}
    }


    // LEDs

    switch(control)
    {
    case OPEN_LOOP:
    	PCB_LED3_off();
    	PCB_LED4_off();
    	break;
    case CURRENT_CONTROL:
    	PCB_LED3_off();
    	PCB_LED4_on();
    	break;
    case SPEED_CONTROL:
    	PCB_LED3_on();
    	PCB_LED4_off();
    	break;
    case POSITION_CONTROL:
    	PCB_LED3_on();
    	PCB_LED4_on();
    	break;
    default:
    	PCB_LED3_off();
    	PCB_LED4_off();
    }

	// signalize direction change with LED3 and LED4
    if(control_enable == FALSE)
    {
    	if(direction_change_flag == TRUE)
    	{
    		// najprej ugasni obe LED
    		PCB_LED3_off();
    		PCB_LED4_off();

    		delta_tic_direction = interrupt_cnt - tic_direction;
    		if(interrupt_cnt < tic_direction)
    		{
    			delta_tic_direction = delta_tic_direction + SAMPLE_FREQ;
    		}

    		if(direction >= 0)
    		{
    			// za pozitivno smer najprej prižgi LED4
    			if(delta_tic_direction >= (long)(1*SAMPLE_FREQ/10))
    			{

    				PCB_LED3_off();
    				PCB_LED4_on();
    			}

    			// preklopi LED4
    			if(delta_tic_direction >= (long)(2*SAMPLE_FREQ/10))
    			{
    				PCB_LED4_toggle();
    			}

    			// preklopi LED4
    			if(delta_tic_direction >= (long)(3*SAMPLE_FREQ/10))
    			{
    				PCB_LED4_toggle();
    			}

    			// preklopi LED4
    			if(delta_tic_direction >= (long)(4*SAMPLE_FREQ/10))
    			{
    				PCB_LED4_toggle();
    			}

    			// preklopi LED4
    			if(delta_tic_direction >= (long)(5*SAMPLE_FREQ/10))
    			{
    				PCB_LED4_toggle();
    			}

    			// preklopi obe LED
    			if(delta_tic_direction >= (long)(6*SAMPLE_FREQ/10))
    			{
    				PCB_LED3_off();
    				PCB_LED4_off();
    			}
    		}
    		else
    		{
    			// za negativno smer najprej prižgi LED3
    			if(delta_tic_direction >= (long)(1*SAMPLE_FREQ/10))
    			{

    				PCB_LED3_on();
    				PCB_LED4_off();
    			}

    			// preklopi LED3
    			if(delta_tic_direction >= (long)(2*SAMPLE_FREQ/10))
    			{
    				PCB_LED3_toggle();
    			}

    			// preklopi LED3
    			if(delta_tic_direction >= (long)(3*SAMPLE_FREQ/10))
    			{
    				PCB_LED3_toggle();
    			}

    			// preklopi LED3
    			if(delta_tic_direction >= (long)(4*SAMPLE_FREQ/10))
    			{
    				PCB_LED3_toggle();
    			}

    			// preklopi LED3
    			if(delta_tic_direction >= (long)(5*SAMPLE_FREQ/10))
    			{
    				PCB_LED3_toggle();
    			}

    			// preklopi obe LED
    			if(delta_tic_direction >= (long)(6*SAMPLE_FREQ/10))
    			{
    				PCB_LED3_off();
    				PCB_LED4_off();
    			}
    		}

    		if(delta_tic_direction >= (long)(9.9*SAMPLE_FREQ/10))
    		{
    			// ne vplivaj veè na LED
    			direction_change_flag = FALSE;
    			delta_tic_direction = 0;
    		}
    	}
    }
    else
    {
    	// èe je algoritem vodenja že aktiven, ne rabiš veè signalizirati smeri vrtenja

		// ne vplivaj veè na LED
		direction_change_flag = FALSE;
		delta_tic_direction = 0;
    }


    // main conditions for control

    // wait for current offset calibration procedure
    if(current_offset_calibrated_flag == TRUE)
    {
    	// switch 1 means on/off
    	if(sw1_state == FALSE)
    	{
    		SVM_disable();
    		PCB_LED2_off();
    		set_null_position_flag = FALSE;
    		control_enable = FALSE;
    		reset_null_position_procedure_flag = TRUE;
    		incremental_encoder_connected_flag = FALSE;

    		if(hardware_trip_oc_flag == TRUE || software_trip_flag == TRUE)
    		{
    			trip_reset_flag = TRUE;
    		}
    	}
    	else
    	{
    		// switch 1 starts "set null position" procedure
    		if(set_null_position_flag == FALSE)
    		{
    			set_null_position(reset_null_position_procedure_flag);
    			reset_null_position_procedure_flag = FALSE;
    			modulation = SVM;
    		}
    		else
    		{
    			// button 1 enables control alghorithm
    			if(b1_press_int == TRUE && control_enable == FALSE && pot_rel <= 0.5)
    			{
    				control_enable = TRUE;
    				SVM_enable();
    				SVM_update(0.0, 0.0);
    				PCB_LED2_on();
    			}
    			else if(b1_press_int == TRUE && control_enable == TRUE)
    			{
    				SVM_disable();
    				control_enable = FALSE;
    				PCB_LED2_off();
    			} // end of button 1
    		} // end of null position
    	} // end of switch 1
    } // end of current_offset_calibrated


    // if all the conditions are met, control alghorithm becomes active
    if(control_enable == TRUE)
    {
    	control_algorithm();
    }
    else
    {
    	// Clear integral parts and outputs of all controllers
    	clear_controllers();

    	// clear all reference values
    	nap_alpha_ref = 0.0;
    	nap_beta_ref = 0.0;
    	nap_d_ref = 0.0;
    	nap_q_ref = 0.0;
    	tok_d_ref = 0.0;
    	tok_q_ref = 0.0;
    	speed_meh_ref = 0.0;
    	kot_meh_ref = 0.0;

    	// clear all open loop values
    	amp_rel = 0.0;
    	freq_meh = 0.0;
    	duty_six_step = 0.0;
    	duty_DC = 0.0;
    }


    if(trip_reset_flag == TRUE)
    {
    	trip_reset();
    	trip_reset_flag = FALSE;
    }


//    temp1 = cos(2*PI*((float)interrupt_cnt/SAMPLE_FREQ));
//    temp2 = sin(2*PI*((float)interrupt_cnt/SAMPLE_FREQ));

    /* End of 3 phase inverter control alghorithm */




    // store values for display within CCS or GUI
    DLOG_GEN_update();
    
    // check limits
    software_protection();

    /* If overcurrent trip event has occured, shut down power stage
     * and signalise with red LED.
     */
    if(PCB_TRIP_OC_read() == TRUE || SVM_MODUL1.TZSEL.bit.OSHT1)
    {
    	hardware_trip_oc_flag = TRUE;
    }

    if(hardware_trip_oc_flag == TRUE)
    {
    	SVM_trip();
    	PCB_LED1_on();
    	PCB_LED2_off();
    	PCB_LED3_off();
    	PCB_LED4_off();
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

}   // end of PER_int




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


    // elektrièni kot

    // iz mehanskega kota izraèunam še elektriènega
    kot_el = POLE_PAIRS*kot_meh;


    // omejim elektrièni kot od 0.0 do 1.0
    if (kot_el < 0.0)
    {
    	kot_el = kot_el + 1.0;
    }
    // odštejem zaradi polovih parov P, da dobim kot_el med 0.0 in 1.0
    for (i = POLE_PAIRS - 1; i > 0; i = i - 1)
    {
    	if (kot_el >= i*1.0)
    	{
    		kot_el = kot_el - i*1.0;
    		break;
    	}

    }

    // poklièem funkcijo, ki vrne mehansko krožno frekvenco
    get_meh_speed();

    // poklièem funkcijo, ki vrne mehanski kotni pospešek
    // get_meh_accel();

} // end of function



/**************************************************************
* Function, where mechanical speed is calculated (out of mechanical angle)
**************************************************************/
#pragma CODE_SECTION(get_meh_speed, "ramfuncs");
void get_meh_speed(void)
{
	/* Alfa Beta tracker za izraèun hitrosti */
	float dusenje_ABF;
	float mejna_frekvenca_ABF;

	// dusenje clena 2. reda [1]
	dusenje_ABF = SQRT2/2.0;
	// mejna frekvenca clena 2. reda [Hz]
	mejna_frekvenca_ABF = 100.0;

	abf_speed_meh_ABF.Alpha = ( 2.0*PI*mejna_frekvenca_ABF/ABF_OMEGA_SAMPLING_FREQ )*( 2.0*dusenje_ABF - 2.0*PI*mejna_frekvenca_ABF/(2.0*ABF_OMEGA_SAMPLING_FREQ) );
	abf_speed_meh_ABF.Beta = ( 2.0*PI*mejna_frekvenca_ABF/ABF_OMEGA_SAMPLING_FREQ )*( 2.0*PI*mejna_frekvenca_ABF/ABF_OMEGA_SAMPLING_FREQ );
	abf_speed_meh_ABF.KotIn = kot_meh;

	ABF_OMEGA_CALC(abf_speed_meh_ABF);

	speed_meh_ABF = abf_speed_meh_ABF.Omega;




	/* CAP modul za izraèun hitrosti */

	// doloèim predznak vrtenja
	direction_QEP = QEP_dir();

	// prescalar CAP enote je zapisan v globalni spremenljivki CAP_PRESCALER

	// merjenje periode s CAP modulom med dvema frontama [s]
	period_CAP = CAP_period();

	// izraèun mehanske frekvence s CAP modulom [Hz]
	speed_meh_CAP = direction_QEP * 1.0/((float)QEP_LINES*period_CAP);

	// hitrost ne more biti neskonèno
	if (period_CAP == 0.0)
	{
		speed_meh_CAP = 0.0;
	}

}




/**************************************************************
* Function, where mechanical acceleration is calculated (out of mechanical angle)
**************************************************************/
#pragma CODE_SECTION(get_meh_accel, "ramfuncs");
void get_meh_accel(void)
{
/*
	float dusenje_ABF;
	float mejna_frekvenca_ABF;

	// dusenje clena 2. reda [1]
	dusenje_ABF = SQRT2/2.0;
	// mejna frekvenca clena 2. reda [Hz]
	mejna_frekvenca_ABF = 500.0;

	abf_accel_meh.Alpha = ( 2.0*PI*mejna_frekvenca_ABF/ABF_OMEGA_SAMPLING_FREQ )*( 2.0*dusenje_ABF - 2.0*PI*mejna_frekvenca_ABF/(2.0*ABF_OMEGA_SAMPLING_FREQ) );
	abf_accel_meh.Beta = ( 2.0*PI*mejna_frekvenca_ABF/ABF_OMEGA_SAMPLING_FREQ )*( 2.0*PI*mejna_frekvenca_ABF/ABF_OMEGA_SAMPLING_FREQ );
	abf_accel_meh.KotIn = speed_meh_ABF;

	ABF_OMEGA_CALC(abf_accel_meh);

	accel_meh_Hz_per_s = abf_accel_meh.Omega;
*/
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
    if (current_offset_calibrated_flag == FALSE)
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
            current_offset_calibrated_flag = TRUE;
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

    // medfazne napetosti v12, v23, v31
    nap_v12 = nap_v1 - nap_v2;
    nap_v23 = nap_v2 - nap_v3;
    nap_v31 = nap_v3 - nap_v1;

    // napetost DC linka
    nap_dc = nap_dc_gain * (ADC_VOLTAGE_DC - nap_dc_offset);

    // pot_rel
    pot_rel = ADC_POT * (1/4096.0);
    // okoli nicle vrzem zeleno vrednost na cisto niclo
    if ((pot_rel > 0.0) && (pot_rel < +0.01))
    {
        pot_rel = 0.0;
    }

    // na potenciometer dodaj se histerezo - manj suma
    // najprej zaokrozim na +-0.01
    pot_rel_discrete = (long)(pot_rel * 100.0);

    // dodaj histerezo
    if (fabs(pot_rel_discrete_old - pot_rel_discrete) < 1)
    {
    	pot_rel_discrete = pot_rel_discrete_old;
    }
    // zgodovina
    // pot_rel_discrete_old = 2*(long)(pot_rel_discrete / 2.0);
    // spravi dol na obmoèje [0,1]
    pot_rel_discrete = pot_rel_discrete / 100.0;
    // zgodovina
    pot_rel_discrete_old = pot_rel_discrete;


    // Clarke-ina transformacija tokov
    clarke_tok.As = tok_i1;
    clarke_tok.Bs = tok_i2;
    CLARKE_FLOAT_CALC(clarke_tok);

    // Park-ova transformacija tokov
    park_tok.Alpha = clarke_tok.Alpha;
    park_tok.Beta = clarke_tok.Beta;
    park_tok.Angle = kot_el;
    PARK_FLOAT_CALC(park_tok);
    tok_d = park_tok.Ds;
    tok_q = park_tok.Qs;

    // izracun nasièenja
    nap_dq_ref = sqrt(nap_d_ref*nap_d_ref + nap_q_ref*nap_q_ref);

    if(nap_dq_ref < 1.0/SQRT3 && interrupt_cnt == 0)
    {
    	saturation = FALSE;
    }
    if(nap_dq_ref > 1.0/SQRT3)
    {
    	saturation = TRUE;
    }


/*
    // izraèun dejanskega navora
    navor_elektromagnetni = 3.0/2.0*POLE_PAIRS*PSI_ROT*tok_q;
    navor_reluktancni = 3.0/2.0*POLE_PAIRS*(Ld - Lq)*tok_d*tok_q;

    navor = navor_elektromagnetni + navor_reluktancni;

    // izraèun dinamiènega navora
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
    float 	duty_cycle = 0.1 * 24.0/nap_dc; // z napetostjo DC linka se spreminja
    int 	sector;

    static int	i = 0;
    static long interrupts_passed = 0;

    static unsigned int kot_raw_temp = 0;
    static unsigned int kot_raw_temp_old = 0;

	SVM_enable();
	PCB_LED2_on();

	if(reset_procedure == TRUE)
	{
		i = 0;
		interrupts_passed = 0;
		kot_raw_temp = 0;
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
        	kot_raw_temp = QEP_cnt();
        	if(kot_raw_temp - kot_raw_temp_old != 0)
        	{
        		kot_raw_temp_old = kot_raw_temp;
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
        	kot_raw_temp = QEP_cnt();
        	if(kot_raw_temp - kot_raw_temp_old != 0)
        	{
        		incremental_encoder_connected_flag = TRUE;
        	}
        }
    } // end of i == 1

    if (i == 2)     // èez nekaj èasa reèem to je pozicija 0, in postavim zastavico
    {
    	interrupts_passed = interrupts_passed + 1;

    	if(interrupts_passed >= SAMPLE_FREQ/2)
    	{
    		set_null_position_flag = TRUE;
    		interrupts_passed = 0;
    		i = 0;

    		// resetiram pozicijo rotorja
    		QEP_reset();

    		// onemogoèim mostiè
    		SVM_disable();
    		PCB_LED2_off();
    	}
    } // end of i == 2

} // end of function




/**************************************************************
* Function, which checks, if voltages and currents are respecting limits
**************************************************************/
#pragma CODE_SECTION(software_protection, "ramfuncs");
void software_protection(void)
{
	// DC link voltage protection
	if(nap_dc > nap_dc_max)
	{
		nap_dc_overvoltage_flag = TRUE;
		software_trip_flag = TRUE;
	}
	else if((nap_dc < nap_dc_min))
	{
		nap_dc_undervoltage_flag = TRUE;
		software_trip_flag = TRUE;
	}

	// phase voltage protection
	if(nap_v1 > nap_v_max)
	{
		nap_v1_overvoltage_flag = TRUE;
		software_trip_flag = TRUE;
	}
	else if((nap_v1 < nap_v_min))
	{
		nap_v1_undervoltage_flag = TRUE;
		software_trip_flag = TRUE;
	}
	if(nap_v2 > nap_v_max)
	{
		nap_v2_overvoltage_flag = TRUE;
		software_trip_flag = TRUE;
	}
	else if((nap_v2 < nap_v_min))
	{
		nap_v2_undervoltage_flag = TRUE;
		software_trip_flag = TRUE;
	}
	if(nap_v3 > nap_v_max)
	{
		nap_v3_overvoltage_flag = TRUE;
		software_trip_flag = TRUE;
	}
	else if((nap_v3 < nap_v_min))
	{
		nap_v3_undervoltage_flag = TRUE;
		software_trip_flag = TRUE;
	}

	// phase curent protection
	if(fabs(tok_i1) > tok_i_max)
	{
		tok_i1_overcurrent_flag = TRUE;
		software_trip_flag = TRUE;
	}
	if(fabs(tok_i2) > tok_i_max)
	{
		tok_i2_overcurrent_flag = TRUE;
		software_trip_flag = TRUE;
	}
	if(fabs(tok_i3) > tok_i_max)
	{
		tok_i3_overcurrent_flag = TRUE;
		software_trip_flag = TRUE;
	}

	// trip execution
	if(software_trip_flag == TRUE)
	{
		SVM_trip();
		PCB_LED1_on();
		PCB_LED2_off();
		PCB_LED3_off();
		PCB_LED4_off();
	}

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
		control_enable = FALSE;
		PCB_LED2_off();
		break;
	}

	switch(modulation)
	{
	case SVM:
		SVM_update(nap_alpha_ref, nap_beta_ref);
		break;
	case SIX_STEP:
		SVM_update_six(duty_six_step, sector_six_step);
		break;
	case SINGLE_PHASE_DC:
		SVM_update_DC(duty_DC);
		break;
	default:
		SVM_disable();
		control_enable = FALSE;
		PCB_LED2_off();
		break;
	}
} // end of function




/**************************************************************
* Function for open loop control
**************************************************************/
#pragma CODE_SECTION(open_loop_control, "ramfuncs");
void open_loop_control(void)
{
	if(modulation == SVM)
	{
		if(incremental_encoder_connected_flag == FALSE)
		{
			// if incremental encoder is NOT connnected

			// potenciometer is changing duty cycle
			amp_rel = pot_rel_discrete;

			// button 3 and 4 are changing mechanical freqency
			if(b3_press_int == TRUE)
			{
				freq_meh = freq_meh - 0.5;
			}
			else if(b4_press_int == TRUE)
			{
				freq_meh = freq_meh + 0.5;
			}

			// omejitev na [0,speed_ref_max]
			if(freq_meh > speed_ref_max)
			{
				freq_meh = speed_ref_max;
			}
			else if(freq_meh < 0.0)
			{
				freq_meh = 0.0;
			}

			freq = POLE_PAIRS * freq_meh;

			nap_alpha_ref = amp_rel*cos(2*PI*freq*interrupt_cnt/SAMPLE_FREQ);
			nap_beta_ref =  direction*amp_rel*sin(2*PI*freq*interrupt_cnt/SAMPLE_FREQ);
		}
		else
		{
			// if incremental encoder is connnected
			amp_rel = direction*pot_rel_discrete*0.577;

			nap_d_ref = 0.0;
			nap_q_ref = amp_rel;

			ipark_nap.Ds = nap_d_ref;
			ipark_nap.Qs = nap_q_ref;
			ipark_nap.Angle = kot_el;

			IPARK_FLOAT_CALC(ipark_nap);

			nap_alpha_ref = ipark_nap.Alpha;
			nap_beta_ref = ipark_nap.Beta;
		}
	}
	else if(modulation == SIX_STEP)
	{
		// button 3 and 4 are changing duty cycle
		if(b3_press_int == TRUE)
		{
			duty_six_step = duty_six_step - 0.01;
		}
		else if(b4_press_int == TRUE)
		{
			duty_six_step = duty_six_step + 0.01;
		}

		// omejitev na [0,1]
		if(duty_six_step > 1.0)
		{
			duty_six_step = 1.0;
		}
		else if(duty_six_step < 0.0)
		{
			duty_six_step = 0.0;
		}

		// doloèanje smeri s potenciometrom
		if(direction >= 0)
		{
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
	}
	else if(modulation == SINGLE_PHASE_DC)
	{
		duty_DC = direction*pot_rel_discrete;
	}
}




/**************************************************************
* Function for current loop control
**************************************************************/
#pragma CODE_SECTION(current_loop_control, "ramfuncs");
void current_loop_control(void)
{
	if(modulation == SVM && incremental_encoder_connected_flag == TRUE)
	{
		// omejim želene tokove za vsak sluèaj, èe še prej nisem


		// pazim na I del regulatorjev, ko med delovanjem izklopim mostiè
		if (svm_status != ENABLE)
		{
			// Clear integral parts and outputs of all controllers
			clear_controllers();
		}

		// samo, èe je izbran režim tokovne regulacije, definiraj referenco q toka
		if(control == CURRENT_CONTROL)
		{
			tok_d_ref = 0.0;
			tok_q_ref = direction*pot_rel_discrete*tok_q_ref_max;
		}




		// tokovna PI regulacija - d os
		id_PI_reg.Ref = tok_d_ref;
		id_PI_reg.Fdb = tok_d;
		id_PI_reg.Kp = Kp_id_PI_reg * 24.0/nap_dc; // z napetostjo DC linka se spreminja
		id_PI_reg.Ki = Ki_id_PI_reg * 24.0/nap_dc; // z napetostjo DC linka se spreminja
		id_PI_reg.OutMax = nap_d_ref_max;
		id_PI_reg.OutMin = nap_d_ref_min;
		PI_ctrl_calc(&id_PI_reg);


		// tokovna PI regulacija - q os
		iq_PI_reg.Ref = tok_q_ref;
		iq_PI_reg.Fdb = tok_q;
		iq_PI_reg.Kp = Kp_iq_PI_reg * 24.0/nap_dc; // z napetostjo DC linka se spreminja
		iq_PI_reg.Ki = Ki_iq_PI_reg * 24.0/nap_dc; // z napetostjo DC linka se spreminja
		iq_PI_reg.OutMax = nap_q_ref_max;
		iq_PI_reg.OutMin = nap_q_ref_min;
		PI_ctrl_calc(&iq_PI_reg);




		// dodatna napredna tokovna regulacija (uporaba RES, REP ali DCT regulatorjev)
		if(advanced_current_reg_type != NONE)
		{
			advanced_current_loop_control();
		}
		else
		{
			advanced_id_reg_out = 0.0;
			advanced_iq_reg_out = 0.0;
			clear_advanced_controllers();
		}


		// upostevanje morebitnih dodatnih naprednih tokovnih regulatorjev
		nap_d_ref = id_PI_reg.Out + advanced_id_reg_out;
		nap_q_ref = iq_PI_reg.Out + advanced_iq_reg_out;




		// izracun napetosti za SVM z inverzno Parkovo transformacijo
		ipark_nap.Ds = nap_d_ref;
		ipark_nap.Qs = nap_q_ref;
		ipark_nap.Angle = kot_el;

		IPARK_FLOAT_CALC(ipark_nap);

		nap_alpha_ref = ipark_nap.Alpha;
		nap_beta_ref = ipark_nap.Beta;
	} // end of if(modulation == SVM)
	else
	{
		SVM_disable();
		control_enable = FALSE;
		PCB_LED2_off();
	} // end of else

} // end of function




/**************************************************************
* Function for current loop control
**************************************************************/
#pragma CODE_SECTION(current_loop_control, "ramfuncs");
void advanced_current_loop_control(void)
{
	// tokovna napredna (RES, RES_multiple, REP ali DCT) regulacija - d os

	if(advanced_current_reg_type == RES || advanced_current_reg_type == RES_multiple)
	{
		// avtomatski izraèun parametrov RES reg.: ojaèanj in kompenzacije zamika
		if(auto_calc_of_advanced_reg_params == TRUE)
		{
			// avtomatski izraèun ojaèanj
			freq_critical = 1.0/(2.0*PI) * SAMPLE_FREQ * 1.0/(2.0*1.5*sqrt(1 + (1.5/SAMPLE_FREQ)*(1.5/SAMPLE_FREQ))); // izraèun velja samo, èe je PI reg. parametriran po optimumu iznosa
			freq_critical = 1.0/(2.0*PI) * nap_dc/Rs * id_PI_reg.Ki * SAMPLE_FREQ * 1.0/(sqrt(1 + (1.5/SAMPLE_FREQ)*(1.5/SAMPLE_FREQ)));

			// avtomatski izracun ojacanj za RES reg. 1
			factor_res_reg_gain = 1 - (id_RES_reg_1.Harmonic*POLE_PAIRS*fabs(speed_meh_CAP)/freq_critical)*(POLE_PAIRS*id_RES_reg_1.Harmonic*fabs(speed_meh_CAP)/freq_critical);
			if(factor_res_reg_gain > 0.0)
			{
				id_RES_reg_1.Kres = factor_res_reg_gain_additional*factor_res_reg_gain*id_PI_reg.Ki;
			}
			else
			{
				id_RES_reg_1.Kres = 0.0;
			}
			// avtomatski izracun ojacanj za RES reg. 2
			factor_res_reg_gain = 1 - (id_RES_reg_2.Harmonic*POLE_PAIRS*fabs(speed_meh_CAP)/freq_critical)*(POLE_PAIRS*id_RES_reg_2.Harmonic*fabs(speed_meh_CAP)/freq_critical);
			if(factor_res_reg_gain > 0.0)
			{
				id_RES_reg_2.Kres = factor_res_reg_gain_additional*factor_res_reg_gain*id_PI_reg.Ki;
			}
			else
			{
				id_RES_reg_2.Kres = 0.0;
			}
			// avtomatski izracun ojacanj za RES reg. 3
			factor_res_reg_gain = 1 - (id_RES_reg_3.Harmonic*POLE_PAIRS*fabs(speed_meh_CAP)/freq_critical)*(POLE_PAIRS*id_RES_reg_3.Harmonic*fabs(speed_meh_CAP)/freq_critical);
			if(factor_res_reg_gain > 0.0)
			{
				id_RES_reg_3.Kres = factor_res_reg_gain_additional*factor_res_reg_gain*id_PI_reg.Ki;
			}
			else
			{
				id_RES_reg_3.Kres = 0.0;
			}
			// avtomatski izracun ojacanj za RES reg. 4
			factor_res_reg_gain = 1 - (id_RES_reg_4.Harmonic*POLE_PAIRS*fabs(speed_meh_CAP)/freq_critical)*(POLE_PAIRS*id_RES_reg_4.Harmonic*fabs(speed_meh_CAP)/freq_critical);
			if(factor_res_reg_gain > 0.0)
			{
				id_RES_reg_4.Kres = factor_res_reg_gain_additional*factor_res_reg_gain*id_PI_reg.Ki;
			}
			else
			{
				id_RES_reg_4.Kres = 0.0;
			}
			// avtomatski izracun ojacanj za RES reg. 5
			factor_res_reg_gain = 1 - (id_RES_reg_5.Harmonic*POLE_PAIRS*fabs(speed_meh_CAP)/freq_critical)*(POLE_PAIRS*id_RES_reg_5.Harmonic*fabs(speed_meh_CAP)/freq_critical);
			if(factor_res_reg_gain > 0.0)
			{
				id_RES_reg_5.Kres = factor_res_reg_gain_additional*factor_res_reg_gain*id_PI_reg.Ki;
			}
			else
			{
				id_RES_reg_5.Kres = 0.0;
			}
			// avtomatski izracun ojacanj za RES reg.
			factor_res_reg_gain = 1 - (id_RES_reg_6.Harmonic*POLE_PAIRS*fabs(speed_meh_CAP)/freq_critical)*(POLE_PAIRS*id_RES_reg_6.Harmonic*fabs(speed_meh_CAP)/freq_critical);
			if(factor_res_reg_gain > 0.0)
			{
				id_RES_reg_6.Kres = factor_res_reg_gain_additional*factor_res_reg_gain*id_PI_reg.Ki;
			}
			else
			{
				id_RES_reg_6.Kres = 0.0;
			}

			// avtomatski izraèun kompenzacije zamika za vse RES reg.
			// v praksi ne deluje
			// id_RES_reg_1.PhaseCompDeg = phase_lag_comp_calc((id_RES_reg_1.Harmonic-1)*POLE_PAIRS*speed_meh_CAP);
			// id_RES_reg_2.PhaseCompDeg = phase_lag_comp_calc((id_RES_reg_2.Harmonic-1)*POLE_PAIRS*speed_meh_CAP);
			// id_RES_reg_3.PhaseCompDeg = phase_lag_comp_calc((id_RES_reg_3.Harmonic-1)*POLE_PAIRS*speed_meh_CAP);
			// id_RES_reg_4.PhaseCompDeg = phase_lag_comp_calc((id_RES_reg_4.Harmonic-1)*POLE_PAIRS*speed_meh_CAP);
			// id_RES_reg_5.PhaseCompDeg = phase_lag_comp_calc((id_RES_reg_5.Harmonic-1)*POLE_PAIRS*speed_meh_CAP);
			// id_RES_reg_6.PhaseCompDeg = phase_lag_comp_calc((id_RES_reg_6.Harmonic-1)*POLE_PAIRS*speed_meh_CAP);

			// v praksi deluje
			id_RES_reg_1.PhaseCompDeg = atan2(2*PI*(id_RES_reg_1.Harmonic-1)*speed_meh_ABF*Ld,Rs)*180/PI;
			id_RES_reg_2.PhaseCompDeg = atan2(2*PI*(id_RES_reg_2.Harmonic-1)*speed_meh_ABF*Ld,Rs)*180/PI;
			id_RES_reg_3.PhaseCompDeg = atan2(2*PI*(id_RES_reg_3.Harmonic-1)*speed_meh_ABF*Ld,Rs)*180/PI;
			id_RES_reg_4.PhaseCompDeg = atan2(2*PI*(id_RES_reg_4.Harmonic-1)*speed_meh_ABF*Ld,Rs)*180/PI;
			id_RES_reg_5.PhaseCompDeg = atan2(2*PI*(id_RES_reg_5.Harmonic-1)*speed_meh_ABF*Ld,Rs)*180/PI;
			id_RES_reg_6.PhaseCompDeg = atan2(2*PI*(id_RES_reg_6.Harmonic-1)*speed_meh_ABF*Ld,Rs)*180/PI;

		} // end of if(auto_calc_of_advanced_reg_params == TRUE)




		// izracun RES reg. - d os
		TIC_start_1();

		id_RES_reg_1.Ref = tok_d_ref;
		id_RES_reg_1.Fdb = tok_d;
		id_RES_reg_1.Angle = kot_el;
		RES_REG_CALC(id_RES_reg_1);

		id_RES_reg_2.Ref = tok_d_ref;
		id_RES_reg_2.Fdb = tok_d;
		id_RES_reg_2.Angle = kot_el;
		RES_REG_CALC(id_RES_reg_2);

		id_RES_reg_3.Ref = tok_d_ref;
		id_RES_reg_3.Fdb = tok_d;
		id_RES_reg_3.Angle = kot_el;
		RES_REG_CALC(id_RES_reg_3);

		id_RES_reg_4.Ref = tok_d_ref;
		id_RES_reg_4.Fdb = tok_d;
		id_RES_reg_4.Angle = kot_el;
		RES_REG_CALC(id_RES_reg_4);

		id_RES_reg_5.Ref = tok_d_ref;
		id_RES_reg_5.Fdb = tok_d;
		id_RES_reg_5.Angle = kot_el;
		RES_REG_CALC(id_RES_reg_5);

		id_RES_reg_6.Ref = tok_d_ref;
		id_RES_reg_6.Fdb = tok_d;
		id_RES_reg_6.Angle = kot_el;
		RES_REG_CALC(id_RES_reg_6);


		id_multiple_RES_reg_out = id_RES_reg_1.Out + id_RES_reg_2.Out + id_RES_reg_3.Out + \
						 id_RES_reg_4.Out + id_RES_reg_5.Out + id_RES_reg_6.Out;

		TIC_stop_1();
		cas_izracuna_RES_multiple_reg = (float) TIC_time_1 * 1.0/CPU_FREQ;
		cas_izracuna_RES_reg = cas_izracuna_RES_multiple_reg/6.0;

	} // end of if(advanced_current_reg_type == RES)
	else if(advanced_current_reg_type == REP)
	{
		if(auto_calc_of_advanced_reg_params == TRUE)
		{
			// v praksi ne deluje
			// id_REP_reg.k = phase_lag_comp_calc(POLE_PAIRS*speed_meh_CAP)/180.0 * id_REP_reg.BufferHistoryLength;

			// v praksi deluje
			id_REP_reg.k = atan2(2*PI*speed_meh_CAP*Ld,Rs)/(2*PI)*id_REP_reg.BufferHistoryLength;
		}

		// izracun REP reg. - d os
		id_REP_reg.Ref = tok_d_ref;
		id_REP_reg.Fdb = tok_d;
		id_REP_reg.SamplingSignal = kot_el;

		TIC_start_1();

		REP_REG_CALC(&id_REP_reg);

		TIC_stop_1();
		cas_izracuna_REP_reg = (float) TIC_time_1 * 1.0/CPU_FREQ;

	}// end of else if(advanced_current_reg_type == REP)
	else if(advanced_current_reg_type == DCT)
	{

	}// end of else if(advanced_current_reg_type == DCT)








	// tokovna napredna (RES, RES_multiple, REP ali DCT) regulacija - q os

	if(advanced_current_reg_type == RES || advanced_current_reg_type == RES_multiple)
	{
		// avtomatski izraèun parametrov RES reg: ojaèanj in kompenzacije zamika
		if(auto_calc_of_advanced_reg_params == TRUE)
		{
			// avtomatski izraèun ojaèanj
			freq_critical = 1.0/(2.0*PI) * SAMPLE_FREQ * 1.0/(2.0*1.5*sqrt(1 + (1.5/SAMPLE_FREQ)*(1.5/SAMPLE_FREQ))); // izraèun velja samo, èe je PI reg. parametriran po optimumu iznosa
			freq_critical = 1.0/(2.0*PI) * nap_dc/Rs * iq_PI_reg.Ki * SAMPLE_FREQ * 1.0/(sqrt(1 + (1.5/SAMPLE_FREQ)*(1.5/SAMPLE_FREQ)));

			factor_res_reg_gain = 1 - (iq_RES_reg_1.Harmonic*POLE_PAIRS*fabs(speed_meh_CAP)/freq_critical)*(POLE_PAIRS*iq_RES_reg_1.Harmonic*fabs(speed_meh_CAP)/freq_critical);
			if(factor_res_reg_gain > 0.0)
			{
				iq_RES_reg_1.Kres = factor_res_reg_gain_additional*factor_res_reg_gain*iq_PI_reg.Ki;
			}
			else
			{
				iq_RES_reg_1.Kres = 0.0;
			}

			factor_res_reg_gain = 1 - (iq_RES_reg_2.Harmonic*POLE_PAIRS*fabs(speed_meh_CAP)/freq_critical)*(POLE_PAIRS*iq_RES_reg_2.Harmonic*fabs(speed_meh_CAP)/freq_critical);
			if(factor_res_reg_gain > 0.0)
			{
				iq_RES_reg_2.Kres = factor_res_reg_gain_additional*factor_res_reg_gain*iq_PI_reg.Ki;
			}
			else
			{
				iq_RES_reg_2.Kres = 0.0;
			}

			factor_res_reg_gain = 1 - (iq_RES_reg_3.Harmonic*POLE_PAIRS*fabs(speed_meh_CAP)/freq_critical)*(POLE_PAIRS*iq_RES_reg_3.Harmonic*fabs(speed_meh_CAP)/freq_critical);
			if(factor_res_reg_gain > 0.0)
			{
				iq_RES_reg_3.Kres = factor_res_reg_gain_additional*factor_res_reg_gain*iq_PI_reg.Ki;
			}
			else
			{
				iq_RES_reg_3.Kres = 0.0;
			}

			factor_res_reg_gain = 1 - (iq_RES_reg_4.Harmonic*POLE_PAIRS*fabs(speed_meh_CAP)/freq_critical)*(POLE_PAIRS*iq_RES_reg_4.Harmonic*fabs(speed_meh_CAP)/freq_critical);
			if(factor_res_reg_gain > 0.0)
			{
				iq_RES_reg_4.Kres = factor_res_reg_gain_additional*factor_res_reg_gain*iq_PI_reg.Ki;
			}
			else
			{
				iq_RES_reg_4.Kres = 0.0;
			}

			factor_res_reg_gain = 1 - (iq_RES_reg_5.Harmonic*POLE_PAIRS*fabs(speed_meh_CAP)/freq_critical)*(POLE_PAIRS*iq_RES_reg_5.Harmonic*fabs(speed_meh_CAP)/freq_critical);
			if(factor_res_reg_gain > 0.0)
			{
				iq_RES_reg_5.Kres = factor_res_reg_gain_additional*factor_res_reg_gain*iq_PI_reg.Ki;
			}
			else
			{
				iq_RES_reg_5.Kres = 0.0;
			}

			factor_res_reg_gain = 1 - (iq_RES_reg_6.Harmonic*POLE_PAIRS*fabs(speed_meh_CAP)/freq_critical)*(POLE_PAIRS*iq_RES_reg_6.Harmonic*fabs(speed_meh_CAP)/freq_critical);
			if(factor_res_reg_gain > 0.0)
			{
				iq_RES_reg_6.Kres = factor_res_reg_gain_additional*factor_res_reg_gain*iq_PI_reg.Ki;
			}
			else
			{
				iq_RES_reg_6.Kres = 0.0;
			}

			// avtomatski izraèun kompenzacije zamika

			// v praksi ne deluje
			// iq_RES_reg_1.PhaseCompDeg = phase_lag_comp_calc((iq_RES_reg_1.Harmonic-1)*POLE_PAIRS*speed_meh_CAP);
			// iq_RES_reg_2.PhaseCompDeg = phase_lag_comp_calc((iq_RES_reg_2.Harmonic-1)*POLE_PAIRS*speed_meh_CAP);
			// iq_RES_reg_3.PhaseCompDeg = phase_lag_comp_calc((iq_RES_reg_3.Harmonic-1)*POLE_PAIRS*speed_meh_CAP);
			// iq_RES_reg_4.PhaseCompDeg = phase_lag_comp_calc((iq_RES_reg_4.Harmonic-1)*POLE_PAIRS*speed_meh_CAP);
			// iq_RES_reg_5.PhaseCompDeg = phase_lag_comp_calc((iq_RES_reg_5.Harmonic-1)*POLE_PAIRS*speed_meh_CAP);
			// iq_RES_reg_6.PhaseCompDeg = phase_lag_comp_calc((iq_RES_reg_6.Harmonic-1)*POLE_PAIRS*speed_meh_CAP);

			// v praksi deluje
			iq_RES_reg_1.PhaseCompDeg = atan2(2*PI*(iq_RES_reg_1.Harmonic-1)*speed_meh_ABF*Lq,Rs)*180/PI;
			iq_RES_reg_2.PhaseCompDeg = atan2(2*PI*(iq_RES_reg_2.Harmonic-1)*speed_meh_ABF*Lq,Rs)*180/PI;
			iq_RES_reg_3.PhaseCompDeg = atan2(2*PI*(iq_RES_reg_3.Harmonic-1)*speed_meh_ABF*Lq,Rs)*180/PI;
			iq_RES_reg_4.PhaseCompDeg = atan2(2*PI*(iq_RES_reg_4.Harmonic-1)*speed_meh_ABF*Lq,Rs)*180/PI;
			iq_RES_reg_5.PhaseCompDeg = atan2(2*PI*(iq_RES_reg_5.Harmonic-1)*speed_meh_ABF*Lq,Rs)*180/PI;
			iq_RES_reg_6.PhaseCompDeg = atan2(2*PI*(iq_RES_reg_6.Harmonic-1)*speed_meh_ABF*Lq,Rs)*180/PI;
		} // end of if(auto_calc_of_advanced_reg_params == TRUE)




		// izraèun RES reg. - q os
		TIC_start_1();

		iq_RES_reg_1.Ref = tok_q_ref;
		iq_RES_reg_1.Fdb = tok_q;
		iq_RES_reg_1.Angle = kot_el;
		RES_REG_CALC(iq_RES_reg_1);

		iq_RES_reg_2.Ref = tok_q_ref;
		iq_RES_reg_2.Fdb = tok_q;
		iq_RES_reg_2.Angle = kot_el;
		RES_REG_CALC(iq_RES_reg_2);

		iq_RES_reg_3.Ref = tok_q_ref;
		iq_RES_reg_3.Fdb = tok_q;
		iq_RES_reg_3.Angle = kot_el;
		RES_REG_CALC(iq_RES_reg_3);

		iq_RES_reg_4.Ref = tok_q_ref;
		iq_RES_reg_4.Fdb = tok_q;
		iq_RES_reg_4.Angle = kot_el;
		RES_REG_CALC(iq_RES_reg_4);

		iq_RES_reg_5.Ref = tok_q_ref;
		iq_RES_reg_5.Fdb = tok_q;
		iq_RES_reg_5.Angle = kot_el;
		RES_REG_CALC(iq_RES_reg_5);

		iq_RES_reg_6.Ref = tok_q_ref;
		iq_RES_reg_6.Fdb = tok_q;
		iq_RES_reg_6.Angle = kot_el;
		RES_REG_CALC(iq_RES_reg_6);


		iq_multiple_RES_reg_out = iq_RES_reg_1.Out + iq_RES_reg_2.Out + iq_RES_reg_3.Out + \
						 iq_RES_reg_4.Out + iq_RES_reg_5.Out + iq_RES_reg_6.Out;

		TIC_stop_1();
		cas_izracuna_RES_multiple_reg = (float) TIC_time_1 * 1.0/CPU_FREQ;
		cas_izracuna_RES_reg = cas_izracuna_RES_multiple_reg/6.0;

	} // end of if(advanced_current_reg_type == RES)

	else if(advanced_current_reg_type == REP)
	{
		if(auto_calc_of_advanced_reg_params == TRUE)
		{
			// v praksi ne deluje
			// iq_REP_reg.k = phase_lag_comp_calc(POLE_PAIRS*speed_meh_CAP)/180.0 * iq_REP_reg.BufferHistoryLength; // ne deluje v praksi

			// v praksi deluje
			iq_REP_reg.k = atan2(2*PI*speed_meh_CAP*Lq,Rs)/(2*PI)*iq_REP_reg.BufferHistoryLength;
		}

		// izracun REP reg. - q os
		iq_REP_reg.Ref = tok_q_ref;
		iq_REP_reg.Fdb = tok_q;
		iq_REP_reg.SamplingSignal = kot_el;

		TIC_start_1();

		REP_REG_CALC(&iq_REP_reg);

		TIC_stop_1();
		cas_izracuna_REP_reg = (float) TIC_time_1 * 1.0/CPU_FREQ;

	}// end of else if(advanced_current_reg_type == REP)

	else if(advanced_current_reg_type == DCT)
	{

	}// end of else if(advanced_current_reg_type == DCT)






	if(enable_advanced_current_reg == TRUE)
	{
		switch(advanced_current_reg_type)
		{
		case NONE:
			advanced_id_reg_out = 0.0;
			advanced_iq_reg_out = 0.0;
			clear_advanced_controllers();
			break;
		case RES:
			advanced_id_reg_out = id_RES_reg_1.Out;
			advanced_iq_reg_out = iq_RES_reg_1.Out;
			break;
		case RES_multiple:
			advanced_id_reg_out = id_multiple_RES_reg_out;
			advanced_iq_reg_out = iq_multiple_RES_reg_out;
			break;
		case REP:
			advanced_id_reg_out = id_REP_reg.Out;
			advanced_iq_reg_out = iq_REP_reg.Out;
			break;
		case DCT:
//			advanced_id_reg_out = id_DCT_reg.Out;
//			advanced_iq_reg_out = iq_DCT_reg.Out;
			advanced_id_reg_out = 0.0;
			advanced_iq_reg_out = 0.0;
			break;
		default:
			advanced_id_reg_out = 0.0;
			advanced_iq_reg_out = 0.0;
			clear_advanced_controllers();
		} // end of switch(advanced_current_reg_type)

	} // if(enable_advanced_current_reg != FALSE)
	else
	{
		advanced_id_reg_out = 0.0;
		advanced_iq_reg_out = 0.0;
	}
} // end of void advanced_current_loop_control(void)




/**************************************************************
}
* Function for speed loop control
**************************************************************/
#pragma CODE_SECTION(speed_loop_control, "ramfuncs");
void speed_loop_control(void)
{
	if(modulation == SVM && incremental_encoder_connected_flag == TRUE)
	{
		// omejim želeno hitrost za vsak sluèaj, èe še prej nisem


		// pazim na I del regulatorja, ko med delovanjem izklopim mostiè
		if (svm_status != ENABLE)
		{
			// Clear integral parts and outputs of all controllers
			clear_controllers();
		}

		// samo, èe je izbran režim hitrostne regulacije, definiraj referenco hitrosti
		if (control == SPEED_CONTROL)
		{
			speed_meh_ref = direction*pot_rel_discrete*speed_ref_max;
		}
			// hitrostna PI regulacija
			speed_PI_reg.Ref = speed_meh_ref;
			speed_PI_reg.Fdb = speed_meh_CAP;
			speed_PI_reg.Kp = Kp_speed_PI_reg;
			speed_PI_reg.Ki = Ki_speed_PI_reg;
			speed_PI_reg.OutMax = tok_q_ref_max;
			speed_PI_reg.OutMin = tok_q_ref_min;
			PI_ctrl_calc(&speed_PI_reg);

			tok_q_ref = speed_PI_reg.Out;

			// tokovna PI regulacija
			current_loop_control();

	} // end of modulation: SVM
	else
	{
		SVM_disable();
		PCB_LED2_off();
	}

} // end of speed_loop_control




/**************************************************************
* Function for position loop control
**************************************************************/
#pragma CODE_SECTION(position_loop_control, "ramfuncs");
void position_loop_control(void)
{
	if(modulation == SVM && incremental_encoder_connected_flag == TRUE)
	{
		// omejim želene tokove za vsak sluèaj, èe še prej nisem


		// pazim na I del regulatorja, ko med delovanjem izklopim mostiè
		if (svm_status != ENABLE)
		{
			// Clear integral parts and outputs of all controllers
			clear_controllers();
		}

		// samo, èe je izbran režim pozicijske regulacije, definiraj referenco kota
		if (control == POSITION_CONTROL)
		{
			kot_meh_ref = pot_rel_discrete;
		}

		// pozicijska PID regulacija
		position_PID_reg.Fdc = 1000.0;							// differential filter cuttof frequency
		position_PID_reg.Kff = 0.0;								// Parameter: Feedforward gain
		position_PID_reg.Sampling_period = 1.0/SAMPLE_FREQ;     // sampling period

		// korigiraj mejni primer, ko gre kot nad 1.0 in pod 0.0
		if(kot_meh_ref - abf_speed_meh_ABF.KotOut > 0.5)
		{
			kot_meh_ref = kot_meh_ref - 1.0;
		}
		else if(kot_meh_ref - abf_speed_meh_ABF.KotOut < -0.5)
		{
			kot_meh_ref = kot_meh_ref + 1.0;
		}

		position_PID_reg.Ref = kot_meh_ref;
		position_PID_reg.Fdb = abf_speed_meh_ABF.KotOut;
		position_PID_reg.Kp = Kp_position_PID_reg;
		position_PID_reg.Ki = Ki_position_PID_reg;
		position_PID_reg.Kd = Kd_position_PID_reg;
		position_PID_reg.OutMax = tok_q_ref_max;
		position_PID_reg.OutMin = tok_q_ref_min;
		PID_CTRL_CALC(position_PID_reg);

 		tok_q_ref = position_PID_reg.Out;

		current_loop_control();

	} // end of modulation: SVM
	else
	{
		SVM_disable();
		PCB_LED2_off();
	}

} // end of position_loop_control




/**************************************************************
* Function, which resets control alghorithm after trip
**************************************************************/
void trip_reset(void)
{
	// clear hardware trip flags
	TRIP_OC_reset();
	// disable PWM
	SVM_disable();
	

	// user interface defaults
	modulation = SVM;
	control = OPEN_LOOP;
	advanced_current_reg_type = NONE;

	control_enable = FALSE;
	enable_advanced_current_reg = FALSE;
	auto_calc_of_advanced_reg_params = FALSE;

	// clear all flags except next two
	current_offset_calibrated_flag = TRUE;
	reset_null_position_procedure_flag = TRUE;
	
	set_null_position_flag = FALSE;
	incremental_encoder_connected_flag = FALSE;
	direction_change_flag = FALSE;
	
	hardware_trip_oc_flag = FALSE;
	software_trip_flag = FALSE;
	
	nap_dc_overvoltage_flag = FALSE;
	nap_dc_undervoltage_flag = FALSE;
	nap_v1_overvoltage_flag = FALSE;
	nap_v1_undervoltage_flag = FALSE;
	nap_v2_overvoltage_flag = FALSE;
	nap_v2_undervoltage_flag = FALSE;
	nap_v3_overvoltage_flag = FALSE;
	nap_v3_undervoltage_flag = FALSE;
	tok_i1_overcurrent_flag = FALSE;
	tok_i2_overcurrent_flag = FALSE;
	tok_i3_overcurrent_flag = FALSE;

	// shut off all LEDs
	PCB_LED1_off();
	PCB_LED2_off();
	PCB_LED3_off();
	PCB_LED4_off();
	
	// Clear integral parts and outputs of all controllers
	clear_controllers();

	// clear all reference values
	nap_alpha_ref = 0.0;
	nap_beta_ref = 0.0;
	nap_d_ref = 0.0;
	nap_q_ref = 0.0;
	tok_d_ref = 0.0;
	tok_q_ref = 0.0;
	speed_meh_ref = 0.0;
	kot_meh_ref = 0.0;

	// clear all open loop values
	amp_rel = 0.0;
	freq_meh = 0.0;
	duty_six_step = 0.0;
	sector_six_step = 1;
	duty_DC = 0.0;
	
	// set variables to initial state
	direction = 1;
	tic_direction = 0;
	delta_tic_direction = 0;
	
	kot_raw = 0;
	kot_meh = 0.0;
}




/**************************************************************
* Function, which clears integral parts and outputs of controllers
**************************************************************/
void clear_controllers(void)
{
	// clear all integral parts of controllers
	id_PI_reg.Ui = 0.0;
	iq_PI_reg.Ui = 0.0;
	speed_PI_reg.Ui = 0.0;
	position_PID_reg.Ui = 0.0;

	// clear all outputs of PI controllers
	id_PI_reg.Out = 0.0;
	iq_PI_reg.Out = 0.0;
	speed_PI_reg.Out = 0.0;
	position_PID_reg.Out = 0.0;

	clear_advanced_controllers();
}




/**************************************************************
* Function, which clears integral parts and outputs of
* advaced current controllers
**************************************************************/
void clear_advanced_controllers(void)
{
	// clear all integral parts of resonant controllers
	id_RES_reg_1.Ui1 = 0.0;
	id_RES_reg_1.Ui2 = 0.0;
	iq_RES_reg_1.Ui1 = 0.0;
	iq_RES_reg_1.Ui2 = 0.0;
	id_RES_reg_2.Ui1 = 0.0;
	id_RES_reg_2.Ui2 = 0.0;
	iq_RES_reg_2.Ui1 = 0.0;
	iq_RES_reg_2.Ui2 = 0.0;
	id_RES_reg_3.Ui1 = 0.0;
	id_RES_reg_3.Ui2 = 0.0;
	iq_RES_reg_3.Ui1 = 0.0;
	iq_RES_reg_3.Ui2 = 0.0;
	id_RES_reg_4.Ui1 = 0.0;
	id_RES_reg_4.Ui2 = 0.0;
	iq_RES_reg_4.Ui1 = 0.0;
	iq_RES_reg_4.Ui2 = 0.0;
	id_RES_reg_4.Ui1 = 0.0;
	id_RES_reg_5.Ui2 = 0.0;
	iq_RES_reg_5.Ui1 = 0.0;
	iq_RES_reg_5.Ui2 = 0.0;
	id_RES_reg_6.Ui1 = 0.0;
	id_RES_reg_6.Ui2 = 0.0;
	iq_RES_reg_6.Ui1 = 0.0;
	iq_RES_reg_6.Ui2 = 0.0;

	// clear all outputs of resonant controllers
	id_RES_reg_1.Out = 0.0;
	iq_RES_reg_1.Out = 0.0;
	id_RES_reg_2.Out = 0.0;
	iq_RES_reg_2.Out = 0.0;
	id_RES_reg_3.Out = 0.0;
	iq_RES_reg_3.Out = 0.0;
	id_RES_reg_4.Out = 0.0;
	iq_RES_reg_4.Out = 0.0;
	id_RES_reg_5.Out = 0.0;
	iq_RES_reg_5.Out = 0.0;
	id_RES_reg_6.Out = 0.0;
	iq_RES_reg_6.Out = 0.0;


	

	// clear all integral parts of repetitive controller
	// (be careful: the fact is that some time must be spend to clear the whole buffer, which is typical les than 1 s)
	id_REP_reg.ErrSumHistory[clear_REP_buffer_index] = 0.0;
	iq_REP_reg.ErrSumHistory[clear_REP_buffer_index] = 0.0;
	id_REP_reg.ErrSum = 0.0;
	iq_REP_reg.ErrSum = 0.0;
	clear_REP_buffer_index = clear_REP_buffer_index + 1;
	if(clear_REP_buffer_index >= id_REP_reg.BufferHistoryLength - 1)
	{
		clear_REP_buffer_index = 0;
	}
	id_REP_reg.i = 0;
	id_REP_reg.i_prev = -1;
	iq_REP_reg.i = 0;
	iq_REP_reg.i_prev = -1;

	// clear all outputs of repetitive controllers
	id_REP_reg.Out = 0.0;
	iq_REP_reg.Out = 0.0;




	// clear all integral parts of DCT controller
	// (be careful: the fact is that some time must be spend to clear the whole buffer, which is typical les than 1 s)
}




/**************************************************************
* Function, which calculates systems (L filter) phase delay,
* obtained from sweep test and aproximated with a polynomial
* and returns controllers phase delay compensation (units [°])
**************************************************************/
float	phase_lag_comp_calc(float phase_lag_freq)
{
	float phase_lag_deg = 0.0;
	float phase_lag_comp_deg = 0.0;

	phase_lag_deg = 0.000000000328741*phase_lag_freq*phase_lag_freq*phase_lag_freq*phase_lag_freq - 	\
			  	    0.000000838238599*phase_lag_freq*phase_lag_freq*phase_lag_freq + 					\
			        0.000747034545213*phase_lag_freq*phase_lag_freq - 									\
			        0.322363338196571*phase_lag_freq - 													\
			        2.157682293314558;

	if((phase_lag_deg >= -90.0) && (phase_lag_deg <= 0.0))
	{
		phase_lag_comp_deg = -phase_lag_deg;
	}
	else
	{
		phase_lag_comp_deg = 90.0;
	}

	return phase_lag_comp_deg;
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

    dlog.downsample_ratio = 10;

    dlog.slope = Positive;
    dlog.trig = &kot_meh;
    dlog.trig_level = 0.01;

    dlog.iptr1 = &tok_d;
    dlog.iptr2 = &tok_q;
    dlog.iptr3 = &speed_meh_CAP;
    dlog.iptr4 = &tok_i1;


    // initialize reference generator
    ref_gen.type = REF_Step;
    ref_gen.amp = 1.0;
    ref_gen.offset = 0.0;
    ref_gen.freq = 1.0;
    ref_gen.duty = 0.5;
    ref_gen.slew = 100;
    ref_gen.samp_period = SAMPLE_TIME;
    

    // initialize current resonant controllers
	id_RES_reg_1.Harmonic = 6;
	id_RES_reg_1.Kres = 0.9*Ki_id_PI_reg;
	id_RES_reg_1.PhaseCompDeg = 0.0;
    id_RES_reg_1.OutMax = 0.2;
    id_RES_reg_1.OutMin = -0.2;
    id_RES_reg_1.Out = 0.0;

    iq_RES_reg_1.Harmonic = 6;
    iq_RES_reg_1.Kres = 0.9*Ki_iq_PI_reg;
    iq_RES_reg_1.PhaseCompDeg = 0.0;
    iq_RES_reg_1.OutMax = 0.2;
    iq_RES_reg_1.OutMin = -0.2;
    iq_RES_reg_1.Out = 0.0;

    id_RES_reg_2.Harmonic = 12;
    id_RES_reg_2.Kres = 0.0*Ki_id_PI_reg;
    id_RES_reg_2.PhaseCompDeg = 0.0;
    id_RES_reg_2.OutMax = 0.1;
    id_RES_reg_2.OutMin = -0.1;
    id_RES_reg_2.Out = 0.0;

    iq_RES_reg_2.Harmonic = 12;
    iq_RES_reg_2.Kres = 0.0*Ki_iq_PI_reg;
    iq_RES_reg_2.PhaseCompDeg = 0.0;
    iq_RES_reg_2.OutMax = 0.1;
    iq_RES_reg_2.OutMin = -0.1;
    iq_RES_reg_2.Out = 0.0;

    id_RES_reg_3.Harmonic = 18;
    id_RES_reg_3.Kres = 0.0*Ki_id_PI_reg;
    id_RES_reg_3.PhaseCompDeg = 60.0;
    id_RES_reg_3.OutMax = 0.02;
    id_RES_reg_3.OutMin = -0.02;
    id_RES_reg_3.Out = 0.0;

    iq_RES_reg_3.Harmonic = 18;
    iq_RES_reg_3.Kres = 0.0*Ki_iq_PI_reg;
    iq_RES_reg_3.PhaseCompDeg = 60.0;
    iq_RES_reg_3.OutMax = 0.02;
    iq_RES_reg_3.OutMin = -0.02;
    iq_RES_reg_3.Out = 0.0;

    id_RES_reg_4.Harmonic = 24;
    id_RES_reg_4.Kres = 0.0*Ki_id_PI_reg;
    id_RES_reg_4.PhaseCompDeg = 90.0;
    id_RES_reg_4.OutMax = 0.02;
    id_RES_reg_4.OutMin = -0.02;
    id_RES_reg_4.Out = 0.0;

    iq_RES_reg_4.Harmonic = 24;
    iq_RES_reg_4.Kres = 0.0*Ki_iq_PI_reg;
    iq_RES_reg_4.PhaseCompDeg = 90.0;
    iq_RES_reg_4.OutMax = 0.02;
    iq_RES_reg_4.OutMin = -0.02;
    iq_RES_reg_4.Out = 0.0;

    id_RES_reg_5.Harmonic = 30;
    id_RES_reg_5.Kres = 0.0*Ki_id_PI_reg;
    id_RES_reg_5.PhaseCompDeg = 90.0;
    id_RES_reg_5.OutMax = 0.02;
    id_RES_reg_5.OutMin = -0.02;
    id_RES_reg_5.Out = 0.0;

    iq_RES_reg_5.Harmonic = 30;
    iq_RES_reg_5.Kres = 0.0*Ki_iq_PI_reg;
    iq_RES_reg_5.PhaseCompDeg = 90.0;
    iq_RES_reg_5.OutMax = 0.02;
    iq_RES_reg_5.OutMin = -0.02;
    iq_RES_reg_5.Out = 0.0;

    id_RES_reg_6.Harmonic = 36;
    id_RES_reg_6.Kres = 0.0*Ki_id_PI_reg;
    id_RES_reg_6.PhaseCompDeg = 90.0;
    id_RES_reg_6.OutMax = 0.02;
    id_RES_reg_6.OutMin = -0.02;
    id_RES_reg_6.Out = 0.0;

    iq_RES_reg_6.Harmonic = 36;
    iq_RES_reg_6.Kres = 0.0*Ki_iq_PI_reg;
    iq_RES_reg_6.PhaseCompDeg = 90.0;
    iq_RES_reg_6.OutMax = 0.02;
    iq_RES_reg_6.OutMin = -0.02;
    iq_RES_reg_6.Out = 0.0;


    // initialize current repetitive controller
    REP_REG_INIT_MACRO(id_REP_reg);
    id_REP_reg.BufferHistoryLength = 1000; // 400 = 20kHz/50 Hz
    id_REP_reg.Krep = 0.01; // 0.01
    id_REP_reg.k = 6; // 6
    id_REP_reg.w0 = 0.403; // 0.2 ali 0.403
    id_REP_reg.w1 = 0.250; // 0.2 ali 0.250
    id_REP_reg.w2 = 0.049; // 0.2 ali 0.049
    id_REP_reg.ErrSumMax = 0.6;
    id_REP_reg.ErrSumMin = -0.6;
    id_REP_reg.OutMax = 0.1;
    id_REP_reg.OutMin = -0.1;

    REP_REG_INIT_MACRO(iq_REP_reg);
    iq_REP_reg.BufferHistoryLength = id_REP_reg.BufferHistoryLength; // 400 = 20kHz/50 Hz
    iq_REP_reg.Krep = id_REP_reg.Krep; // 0.02
    iq_REP_reg.k = id_REP_reg.k; // 6
    iq_REP_reg.w0 = id_REP_reg.w0; // 0.2
    iq_REP_reg.w1 = id_REP_reg.w1; // 0.2
    iq_REP_reg.w2 = id_REP_reg.w2; // 0.2
    iq_REP_reg.ErrSumMax = id_REP_reg.ErrSumMax;
    iq_REP_reg.ErrSumMin = id_REP_reg.ErrSumMin;
    iq_REP_reg.OutMax = id_REP_reg.OutMax;
    iq_REP_reg.OutMin = id_REP_reg.OutMin;


	// clear integral parts and outputs of all controllers
	clear_controllers();

	// disable advanced current controllers
	enable_advanced_current_reg = FALSE;
	auto_calc_of_advanced_reg_params = FALSE;

    // initialize stopwatch
	TIC_init();
	TIC_init_1();


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


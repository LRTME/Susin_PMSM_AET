/****************************************************************
* FILENAME:     DCT_REG.c
* DESCRIPTION:  DCT controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Su�in
* START DATE:   29.8.2017
* VERSION:      1.0
*
* CHANGES :
* VERSION   DATE        WHO             DETAIL
*
****************************************************************/

#include "DCT_REG.h"

// deklaracija funkcij
int circular_buffer_transformation2(int IndexLinearBuffer, int BufferSize);

// globalne spremenljivke

// spremenljivke za FIR filter iz FPU modula

// Create an Instance of FIRFILT_GEN module and place the object in "firfilt" section
#pragma DATA_SECTION(firFP, "firfilt")
FIR_FP  firFP = FIR_FP_DEFAULTS;

// Define the Delay buffer for the FIR filter with specifed length
float dbuffer[FIR_FILTER_NUMBER_OF_COEFF];
// Define the Delay buffer for the FIR filter and place it in "firldb" section
#pragma DATA_SECTION(dbuffer, "firldb")
// Align the delay buffer for max 2048 words (1024 float variables)
#pragma DATA_ALIGN (dbuffer,0x800)

// Define the coeff buffer for the FIR filter with specifed length
float coeff[FIR_FILTER_NUMBER_OF_COEFF];
// Define coefficient array and place it in "coefffilter" section
#pragma DATA_SECTION(coeff, "coefffilt");
// Align the coefficent buffer for max 2048 words (1024 float coeff)
#pragma DATA_ALIGN (coeff,0x800)



/****************************************************************************************************
* Funkcija, ki izvede algoritem DCT regulatorja.
* Zelo za�eleno je, da je razmerje med vzor�no frekvenco in osnovno frekvenco reguliranega signala
* enako velikosti pomnilnika "BufferHistoryLength" (in ve�je od 20), saj je regulator na to ob�utljiv,
* kar lahko privede do nezanemarljivega pogre�ka v stacionarnem stanju.
****************************************************************************************************/
#pragma CODE_SECTION(DCT_REG_CALC, "ramfuncs");
void DCT_REG_CALC (DCT_REG_float *v)
{
	// lokalne spremenljivke




	// program

	// omejitev dol�ine circular bufferja
	if (v->BufferHistoryLength > FIR_FILTER_NUMBER_OF_COEFF)
	{
		v->BufferHistoryLength = FIR_FILTER_NUMBER_OF_COEFF;
	}
	else if (v->BufferHistoryLength < 0)
	{
		v->BufferHistoryLength = 0;
	}

	// omejitev kompenzacije zakasnitve, ki ne sme presegati dol�ine bufferja
	if (v->k > v->BufferHistoryLength)
	{
		v->k = v->BufferHistoryLength;
	}
	else if (v->k < -v->BufferHistoryLength)
	{
		v->k = -v->BufferHistoryLength;
	}

	// omejitev vzor�nega signala med 0.0 in 0.9999
	//(SamplingSignal ne sme biti enak ena, ker mora biti indeks i omejen od 0 do BufferHistoryLength-1)
	v->SamplingSignal = (v->SamplingSignal > 0.99999)? 0.99999: v->SamplingSignal;
	v->SamplingSignal = (v->SamplingSignal < 0.0)? 0.0: v->SamplingSignal;




	v->BufferHistoryLength = FIR_FILTER_NUMBER_OF_COEFF;
    v->k = LAG_COMPENSATION;

    // izra�un trenutnega indeksa bufferja
	v->i = (int)(v->SamplingSignal*v->BufferHistoryLength);




	// �e se indeks spremeni, potem gre algoritem dalje:
	//   1. �e je "SamplingSignal" zunaj te funkcije natan�no sinhroniziran z vzor�no frekvenco ("i_delta" = 1)
	//      se algoritem izvajanja repetitivnega regulatorja izvede vsako vzor�no periodo/interval in se
	//      izkoristi celotna velikost pomnilnika, kar je optimalno
	//   2. �e je "SamplingSignal" prepo�asen ("i_delta" < 1) oz. osnovna frekvenca reguliranega signala prenizka,
	//      ni nujno, da se algoritem izvajanja repetitivnega regulatorja izvede vsako vzor�no periodo/interval,
	//      kar pomeni, da ta algoritem lahko deluje s frekvenco ni�jo od vzor�ne frekvence
	//   3. �e je "SamplingSignal" prehiter ("i_delta" > 1), oz. osnovna frekvenca reguliranega signala previsoka,
	//      se algoritem izvajanja repetitivnega regulatorja izvede vsako vzor�no periodo/interval,
	//      a se velikost pomnilnika umetno zmanj�a za faktor "i_delta", saj zapisujemo in beremo le vsak "i_delta"-ti vzorec,
	//      kar pomeni, kot da bi bila velikost pomnilnika manj�a za faktor "i_delta"
	//      OPOMBA: To zadnjo funkcionalnost je mo�no izklopiti z odkomentiranjem vrstice "v->i_delta = 1;"!
	if ((v->i != v->i_prev))
	{
		// izra�un razlike med trenutnim indeksom "i" in prej�njim indeksom "i_prev"
		// (�e je "SamplingSignal" prehiter, lahko velikost pomnilnika
		// umetno zmanj�amo za faktor "i_delta", saj zapisujemo in beremo le vsak "i_delta"-ti vzorec,
		// ki pa ne sme presegati polovico velikosti pomnilnika)
		v->i_delta = v->i - v->i_prev;

		// manipuliranje z indeksi - zaradi circular bufferja; �e indeks nara��a - inkrementiranje
		if ( (v->i < v->i_prev) && (v->i_delta < -(v->BufferHistoryLength >> 1)) )
		{
			v->i_delta = v->BufferHistoryLength - v->i_delta;
		}
		// manipuliranje z indeksi - zaradi circular bufferja; �e indeks pada - dekrementiranje
		else if ( (v->i > v->i_prev) && (v->i_delta > (v->BufferHistoryLength >> 1)) )
		{
			v->i_delta = -(v->BufferHistoryLength - v->i_delta);
		}


		// �e funkcionalnost umetnega zmanj�evanja velikosti pomnilnika ni za�elena ali ni potrebna (opis pod to�ko 3.),
		// odkomentiraj naslednjo vrstico
		v->i_delta = 1;



		// manipuliranje z indeksi - zaradi circular bufferja
		v->index = circular_buffer_transformation2(v->i + v->k*v->i_delta,v->BufferHistoryLength);




		/***************************************************/
		/* koda DCT regulatorja */
		/***************************************************/

		// izra�unam trenutni error
		v->Err = v->Ref - v->Fdb;

		// izra�unam novi akumuliran error
		v->ErrSum = v->Kdct * v->Err +						\
					v->CorrectionHistory[v->index];

		// omejim trenutni error, da ne gre v nasi�enje
		v->ErrSum = (v->ErrSum > v->ErrSumMax)? v->ErrSumMax: v->ErrSum;
		v->ErrSum = (v->ErrSum < v->ErrSumMin)? v->ErrSumMin: v->ErrSum;




		/* DCT filter - FIR filter */
		firFP.input = v->ErrSum;
		firFP.calc(&firFP);
		v->Correction = firFP.output;

		// zapi�em trenutno vrednost korekcijskega signala v buffer na trenutno mesto
		v->CorrectionHistory[v->i] = v->Correction;

		// izra�unam izhod
		v->Out = v->Correction;

	    // shranim vrednost indeksa i, ki bo v naslednjem ciklu prej�nji i
	    v->i_prev = v->i;




    } // end of if (i != i_prev)




    // omejim izhod
    v->Out = (v->Out > v->OutMax)? v->OutMax: v->Out;
    v->Out = (v->Out < v->OutMin)? v->OutMin: v->Out;




} // konec funkcije








/****************************************************************************************************
 * Realizacija funkcije kro�nega pomnilnika (angl. circular buffer), s katero "index",
 * ki je lahko ve�ji od BufferSize-1 oz. manj�i od 0 reducira na obmo�je [0,BufferSize-1].
 * OPOMBA: Na omejeno obmo�je [0,BufferSize-1] lahko funkcija transformira le �tevila,
 * ki so absolutno manj�a od 10-kratnika velikosti pomnilnika "BufferSize" (glej for zanko)!
 * For zanka (namesto while) je implementirana zato, da omeji najve�je �tevilo iteracij zanke.
****************************************************************************************************/
int circular_buffer_transformation2(IndexLinearBuffer,BufferSize)
{
	int IndexCircularBuffer = IndexLinearBuffer;
	static int i;

	// omejim stevilo iteracij na 10
	for(i = 0; i < 10; i++)
	{
		if(IndexCircularBuffer > BufferSize - 1)
		{
			IndexCircularBuffer = IndexCircularBuffer - BufferSize;
		}
		else if(IndexCircularBuffer < 0)
		{
			IndexCircularBuffer = IndexCircularBuffer + BufferSize;
		}
		else
		{
			break;
		}
	}

	return(IndexCircularBuffer);
}

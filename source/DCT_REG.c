/****************************************************************
* FILENAME:     DCT_REG.c
* DESCRIPTION:  DCT controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Sušin
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
* Zelo zaželeno je, da je razmerje med vzorèno frekvenco in osnovno frekvenco reguliranega signala
* enako velikosti pomnilnika "BufferHistoryLength" (in veèje od 20), saj je regulator na to obèutljiv,
* kar lahko privede do nezanemarljivega pogreška v stacionarnem stanju.
****************************************************************************************************/
#pragma CODE_SECTION(DCT_REG_CALC, "ramfuncs");
void DCT_REG_CALC (DCT_REG_float *v)
{
	// lokalne spremenljivke




	// program

	// omejitev dolžine circular bufferja
	if (v->BufferHistoryLength > FIR_FILTER_NUMBER_OF_COEFF)
	{
		v->BufferHistoryLength = FIR_FILTER_NUMBER_OF_COEFF;
	}
	else if (v->BufferHistoryLength < 0)
	{
		v->BufferHistoryLength = 0;
	}

	// omejitev kompenzacije zakasnitve, ki ne sme presegati dolžine bufferja
	if (v->k > v->BufferHistoryLength)
	{
		v->k = v->BufferHistoryLength;
	}
	else if (v->k < -v->BufferHistoryLength)
	{
		v->k = -v->BufferHistoryLength;
	}

	// omejitev vzorènega signala med 0.0 in 0.9999
	//(SamplingSignal ne sme biti enak ena, ker mora biti indeks i omejen od 0 do BufferHistoryLength-1)
	v->SamplingSignal = (v->SamplingSignal > 0.99999)? 0.99999: v->SamplingSignal;
	v->SamplingSignal = (v->SamplingSignal < 0.0)? 0.0: v->SamplingSignal;




	v->BufferHistoryLength = FIR_FILTER_NUMBER_OF_COEFF;
    v->k = LAG_COMPENSATION;

    // izraèun trenutnega indeksa bufferja
	v->i = (int)(v->SamplingSignal*v->BufferHistoryLength);




	// èe se indeks spremeni, potem gre algoritem dalje:
	//   1. èe je "SamplingSignal" zunaj te funkcije natanèno sinhroniziran z vzorèno frekvenco ("i_delta" = 1)
	//      se algoritem izvajanja repetitivnega regulatorja izvede vsako vzorèno periodo/interval in se
	//      izkoristi celotna velikost pomnilnika, kar je optimalno
	//   2. èe je "SamplingSignal" prepoèasen ("i_delta" < 1) oz. osnovna frekvenca reguliranega signala prenizka,
	//      ni nujno, da se algoritem izvajanja repetitivnega regulatorja izvede vsako vzorèno periodo/interval,
	//      kar pomeni, da ta algoritem lahko deluje s frekvenco nižjo od vzorène frekvence
	//   3. èe je "SamplingSignal" prehiter ("i_delta" > 1), oz. osnovna frekvenca reguliranega signala previsoka,
	//      se algoritem izvajanja repetitivnega regulatorja izvede vsako vzorèno periodo/interval,
	//      a se velikost pomnilnika umetno zmanjša za faktor "i_delta", saj zapisujemo in beremo le vsak "i_delta"-ti vzorec,
	//      kar pomeni, kot da bi bila velikost pomnilnika manjša za faktor "i_delta"
	//      OPOMBA: To zadnjo funkcionalnost je možno izklopiti z odkomentiranjem vrstice "v->i_delta = 1;"!
	if ((v->i != v->i_prev))
	{
		// izraèun razlike med trenutnim indeksom "i" in prejšnjim indeksom "i_prev"
		// (èe je "SamplingSignal" prehiter, lahko velikost pomnilnika
		// umetno zmanjšamo za faktor "i_delta", saj zapisujemo in beremo le vsak "i_delta"-ti vzorec,
		// ki pa ne sme presegati polovico velikosti pomnilnika)
		v->i_delta = v->i - v->i_prev;

		// manipuliranje z indeksi - zaradi circular bufferja; èe indeks narašèa - inkrementiranje
		if ( (v->i < v->i_prev) && (v->i_delta < -(v->BufferHistoryLength >> 1)) )
		{
			v->i_delta = v->BufferHistoryLength - v->i_delta;
		}
		// manipuliranje z indeksi - zaradi circular bufferja; èe indeks pada - dekrementiranje
		else if ( (v->i > v->i_prev) && (v->i_delta > (v->BufferHistoryLength >> 1)) )
		{
			v->i_delta = -(v->BufferHistoryLength - v->i_delta);
		}


		// èe funkcionalnost umetnega zmanjševanja velikosti pomnilnika ni zaželena ali ni potrebna (opis pod toèko 3.),
		// odkomentiraj naslednjo vrstico
		v->i_delta = 1;



		// manipuliranje z indeksi - zaradi circular bufferja
		v->index = circular_buffer_transformation2(v->i + v->k*v->i_delta,v->BufferHistoryLength);




		/***************************************************/
		/* koda DCT regulatorja */
		/***************************************************/

		// izraèunam trenutni error
		v->Err = v->Ref - v->Fdb;

		// izraèunam novi akumuliran error
		v->ErrSum = v->Kdct * v->Err +						\
					v->CorrectionHistory[v->index];

		// omejim trenutni error, da ne gre v nasièenje
		v->ErrSum = (v->ErrSum > v->ErrSumMax)? v->ErrSumMax: v->ErrSum;
		v->ErrSum = (v->ErrSum < v->ErrSumMin)? v->ErrSumMin: v->ErrSum;




		/* DCT filter - FIR filter */
		firFP.input = v->ErrSum;
		firFP.calc(&firFP);
		v->Correction = firFP.output;

		// zapišem trenutno vrednost korekcijskega signala v buffer na trenutno mesto
		v->CorrectionHistory[v->i] = v->Correction;

		// izraèunam izhod
		v->Out = v->Correction;

	    // shranim vrednost indeksa i, ki bo v naslednjem ciklu prejšnji i
	    v->i_prev = v->i;




    } // end of if (i != i_prev)




    // omejim izhod
    v->Out = (v->Out > v->OutMax)? v->OutMax: v->Out;
    v->Out = (v->Out < v->OutMin)? v->OutMin: v->Out;




} // konec funkcije








/****************************************************************************************************
 * Realizacija funkcije krožnega pomnilnika (angl. circular buffer), s katero "index",
 * ki je lahko veèji od BufferSize-1 oz. manjši od 0 reducira na obmoèje [0,BufferSize-1].
 * OPOMBA: Na omejeno obmoèje [0,BufferSize-1] lahko funkcija transformira le števila,
 * ki so absolutno manjša od 10-kratnika velikosti pomnilnika "BufferSize" (glej for zanko)!
 * For zanka (namesto while) je implementirana zato, da omeji najveèje število iteracij zanke.
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

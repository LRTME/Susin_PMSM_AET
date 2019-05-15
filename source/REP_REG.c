/****************************************************************
* FILENAME:     REP_REG.c
* DESCRIPTION:  Repetitive controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Su�in
* START DATE:   6.4.2016
* VERSION:      1.0
*
* CHANGES :
* VERSION   DATE        WHO             DETAIL
* 1.0       6.4.2016   Denis Su�in      Initial version
* 1.1		21.8.2017  Denis Su�in		Corrections of comments and names of variables
* 2.0		15.5.2019  Denis Su�in		Circular buffer compacted into function
****************************************************************/

#include "REP_REG.h"

// deklaracija funkcij
int circular_buffer_transformation(int IndexLinearBuffer, int BufferSize);

// globalne spremenljivke








/****************************************************************************************************
* Funkcija, ki izvede algoritem repetitivnega regulatorja
****************************************************************************************************/
#pragma CODE_SECTION(REP_REG_CALC, "ramfuncs");
void REP_REG_CALC (REP_REG_float *v)
{
    // lokalne spremenljivke




    // program

    // omejitev dol�ine circular bufferja
    if (v->BufferHistoryLength > MAX_LENGTH_REP_REG_BUFFER)
    {
        v->BufferHistoryLength = MAX_LENGTH_REP_REG_BUFFER;
    }
    else if (v->BufferHistoryLength < 1)
    {
        v->BufferHistoryLength = 1;
    }
	
	// omejitev kompenzacije zakasnitve, ki ne sme presegati dol�ine bufferja
    if (v->k > v->BufferHistoryLength)
    {
        v->k = v->BufferHistoryLength;
    }
    else if (v->k < 0)
    {
        v->k = 0;
    }

    // omejitev vzor�nega signala med 0.0 in 0.9999 (SamplingSignal ne sme biti enak ena, ker mora biti indeks i omejen od 0 do BufferHistoryLength-1)
    v->SamplingSignal = (v->SamplingSignal > 0.99999)? 0.99999: v->SamplingSignal;
    v->SamplingSignal = (v->SamplingSignal < 0.0)? 0.0: v->SamplingSignal;

	
	

    // izra�un trenutnega indeksa bufferja
    v->i = (int)(v->SamplingSignal*v->BufferHistoryLength);




    // �e se indeks spremeni, potem gre algoritem dalje
    // (�e je "SamplingSignal" prepo�asen, ni nujno da se algoritem izvajanja repetitivnega regulatorja
    // izvede vsako vzor�no periodo/interval, kar pomeni, da ta algoritem lahko deluje s frekvenco
    // ni�jo od vzor�nega intervala)
    if ((v->i != v->i_prev))
    {
        // manipuliranje z indeksi - zaradi circular bufferja; �e indeks nara��a - inkrementiranje
        if ( (v->i > v->i_prev) || (v->i - v->i_prev < 0) )
        {
            // manipuliranje z indeksi - zaradi circular bufferja
            v->index = circular_buffer_transformation(v->i + v->k,v->BufferHistoryLength);
            v->i_plus_one = circular_buffer_transformation(v->i + 1,v->BufferHistoryLength);
            v->i_minus_one = circular_buffer_transformation(v->i - 1,v->BufferHistoryLength);
            v->i_plus_two = circular_buffer_transformation(v->i + 2,v->BufferHistoryLength);
            v->i_minus_two = circular_buffer_transformation(v->i -2,v->BufferHistoryLength);

        } // end of if (v->i > v->i_prev)

        // manipuliranje z indeksi - zaradi circular bufferja; �e indeks pada - dekrementiranje
        else if ( (v->i < v->i_prev) || (v->i - v->i_prev == (v->BufferHistoryLength - 1)) )
        {
            // manipuliranje z indeksi - zaradi circular bufferja
            v->index = circular_buffer_transformation(v->i - v->k,v->BufferHistoryLength);
            v->i_plus_one = circular_buffer_transformation(v->i - 1,v->BufferHistoryLength);
            v->i_minus_one = circular_buffer_transformation(v->i + 1,v->BufferHistoryLength);
            v->i_plus_two = circular_buffer_transformation(v->i - 2,v->BufferHistoryLength);
            v->i_minus_two = circular_buffer_transformation(v->i + 2,v->BufferHistoryLength);
        } // end of else if (v->i < v->i_prev)




		/***************************************************
        * koda repetitivnega regulatorja
		***************************************************/
		
        // izra�unam trenutni error
        v->Err = v->Ref - v->Fdb;

        // izra�unam novi akumuliran error
        // (pri "ErrSumHistory" na mestu z indeksom i,
        // je zaenkrat �e "ErrSumHistory" iz trenutka i - N
        // ker �e ni prepisan)
        v->ErrSum = v->Krep * v->Err +
                    v->w0 * v->ErrSumHistory[v->i] +
                    v->w1 * v->ErrSumHistory[v->i_plus_one] +
                    v->w1 * v->ErrSumHistory[v->i_minus_one] +
                    v->w2 * v->ErrSumHistory[v->i_plus_two] +
                    v->w2 * v->ErrSumHistory[v->i_minus_two];

        // omejim trenutni error, da ne gre v nasi�enje
        v->ErrSum = (v->ErrSum > v->ErrSumMax)? v->ErrSumMax: v->ErrSum;
        v->ErrSum = (v->ErrSum < v->ErrSumMin)? v->ErrSumMin: v->ErrSum;

        // zapi�em trenutni akumuliran error v buffer na trenutno mesto (v naslednjem ciklu bo to error v prej�njem ciklu)
        v->ErrSumHistory[v->i] = v->ErrSum;

        // izra�unam korekcijo s pomo�jo zgodovine (upo�tevana kompenzacija zakasnitve)
        v->Correction = v->ErrSumHistory[v->index];

        // izra�unam izhod
        v->Out = v->Correction;

        // shranim vrednost indeksa i, ki bo v naslednjem ciklu prej�nji i
        v->i_prev = v->i;




    } // end of if (i != i_prev)




    // omejim izhod
    v->Out = (v->Out > v->OutMax)? v->OutMax: v->Out;
    v->Out = (v->Out < v->OutMin)? v->OutMin: v->Out;




} // konec funkcije void REP_REG_CALC (REP_REG_float *v)








/****************************************************************************************************
 * Realizacija funkcije kro�nega pomnilnika (angl. circular buffer), s katero indeks "index",
 * ki je lahko ve�ji od BufferSize-1 oz. manj�i od 0 reducira na obmo�je [0,BufferSize-1].
 * OPOMBA: Na omejeno obmo�je [0,BufferSize-1] lahko funkcija transformira le �tevila,
 * ki so absolutno manj�a od 10-kratnika velikosti pomnilnika "BufferSize" (glej for zanko)!
 * For zanka (namesto while) je implementirana zato, da omeji najve�je �tevilo iteracij zanke.
****************************************************************************************************/
int circular_buffer_transformation(IndexLinearBuffer,BufferSize)
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
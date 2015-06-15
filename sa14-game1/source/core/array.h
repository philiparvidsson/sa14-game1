/*------------------------------------------------------------------------------
 * File: array.h
 * Created: June 8, 2015
 * Last changed: June 13, 2015
 *
 * Author(s): Philip Arvidsson (philip@philiparvidsson.com)
 *
 * Description:
 *   Erbjuder en dynamisk array-typ.
 *
 *----------------------------------------------------------------------------*/

#ifndef array_h_
#define array_h_

/*------------------------------------------------
 * INCLUDES
 *----------------------------------------------*/

#include "core/common.h"

#include <stdlib.h>

/*------------------------------------------------
 * TYPES
 *----------------------------------------------*/

/*--------------------------------------
 * Type: arrayT
 *
 * Description:
 *   Representerar en array.
 *------------------------------------*/
typedef struct {
    const size_t elem_size; /* Storleken p� elementen i arrayen, i bytes. */
    const int    num_elems; /* Antal element i arrayen. */
} arrayT;

/*------------------------------------------------
 * FUNCTIONS
 *----------------------------------------------*/

/*--------------------------------------
 * Function: newArray()
 * Parameters:
 *   elem_size  Storleken p� elementen i arrayen.
 *
 * Returns:
 *   En pekare till arrayen.
 *
 * Description:
 *   Skapar en ny, dynamisk array.
 *------------------------------------*/
arrayT *newArray(size_t elem_size);

/*--------------------------------------
 * Function: freeArray()
 * Parameters:
 *   a  Arrayen som ska deallokeras.
 *
 * Description:
 *   Deallokerar en array.
 *------------------------------------*/
void freeArray(arrayT *a);

/*--------------------------------------
 * Function: arrayAdd()
 * Parameters:
 *   a     Arrayen till vilken ett element ska l�ggas.
 *   elem  En pekare till elementdatan.
 *
 * Returns:
 *   En pekare till minnesplatsen dit elementet kopierades.
 *
 * Description:
 *   L�gger till ett element i den specificerade arrayen.
 *------------------------------------*/
void *arrayAdd(arrayT *a, const void *elem);

/*--------------------------------------
 * Function: arrayGet()
 * Parameters:
 *   a     Arrayen fr�n vilket ett element ska l�sas ut.
 *   i     Index till det element som ska l�sas ut.
 *   elem  Pekare till datablock d�r elementat ska lagras.
 *
 * Description:
 *   L�ser ut ett element fr�n arrayen.
 *------------------------------------*/
void arrayGet(const arrayT *a, int i, void *elem);

#endif /* array_h_ */
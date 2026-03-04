/***************************************************************************
 * Copyright (c) 2024 Microsoft Corporation
 * Copyright (c) 2026-present Eclipse ThreadX contributors
 *
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 *
 * SPDX-License-Identifier: MIT
 **************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** FileX Component                                                       */
/**                                                                       */
/**   Trace                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef TX_SOURCE_CODE
#define TX_SOURCE_CODE
#endif

#ifndef FX_SOURCE_CODE
#define FX_SOURCE_CODE
#endif

#include "fx_api.h"

#ifdef TX_ENABLE_EVENT_TRACE


/* Include necessary system files.  */

#include "tx_trace.h"



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_trace_object_unregister                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function unregisters a FileX object in the trace registry.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    object_ptr                            Address of system object      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_trace_object_unregister           Actual unregister function    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    FileX delete functions                                              */
/*                                                                        */
/**************************************************************************/
VOID  _fx_trace_object_unregister(VOID *object_ptr)
{

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* Call actual object unregister function.  */
    _tx_trace_object_unregister(object_ptr);

    /* Restore interrupts.  */
    TX_RESTORE
}
#endif


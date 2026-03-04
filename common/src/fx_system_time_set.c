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
/**   System                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_system_time_set                                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function set the system time to the value specified by the     */
/*    caller.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hour                                  Hour (0-23)                   */
/*    minute                                Minute (0-59)                 */
/*    second                                Second (0-59)                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    FX_SUCCESS                                                          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_system_time_set(UINT hour, UINT minute, UINT second)
{


    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_SYSTEM_TIME_SET, hour, minute, second, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Set the new system time.  */
    _fx_system_time  =  (hour << FX_HOUR_SHIFT) |
        (minute << FX_MINUTE_SHIFT) | (second / 2);

    /* Return successful status.  */
    return(FX_SUCCESS);
}


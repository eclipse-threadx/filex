/***************************************************************************
 * Copyright (c) 2024 Microsoft Corporation
 * Copyright (C) 2026-present Eclipse ThreadX contributors
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
/*    _fxe_system_date_get                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the get system date call.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    year                                  Pointer to year               */
/*    month                                 Pointer to month              */
/*    day                                   Pointer to day                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    FX_PTR_ERROR                          Invalid destination pointer   */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_system_date_get                   Actual get system date call   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/**************************************************************************/
UINT  _fxe_system_date_get(UINT *year, UINT *month, UINT *day)
{

UINT status;


    /* Check for an invalid pointer.  */
    if ((year == FX_NULL) || (month == FX_NULL) || (day == FX_NULL))
    {
        return(FX_PTR_ERROR);
    }

    /* Call actual service to get the system date.  */
    status =  _fx_system_date_get(year, month, day);

    /* Return status.  */
    return(status);
}


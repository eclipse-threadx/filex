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
/**   Unicode                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_unicode.h"


/* DEPRECATION NOTICE
 * fx_unicode_length_get() is deprecated. Do not use it in new code.
 *
 * WHY: Does not accept a buffer length; scans up to 256 bytes regardless of the actual buffer size, risking an overread.
 *
 * WHAT TO DO: replace calls with fx_unicode_length_get_extended(), passing the actual
 * destination buffer size as an additional argument.
 */
#pragma message("fx_unicode_length_get() is deprecated. " \
                "Use fx_unicode_length_get_extended() and pass the actual buffer size.")

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_unicode_length_get                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the length of the supplied unicode name.      */
/*                                                                        */
/*    DEPRECATED. Use fx_unicode_length_get_extended() instead, passing the actual
    destination buffer length. Does not accept a buffer length; scans up to 256 bytes regardless of the actual buffer size, risking an overread.
/*    should be used. The maximum buffer size of unicode_name is 256.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    unicode_name                          Pointer to unicode name       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    length                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_unicode_length_get_extended       Actual unicode length get     */
/*                                            service                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/**************************************************************************/
ULONG  _fx_unicode_length_get(UCHAR *unicode_name)
{

    /* Call the extended version with 256 bytes maximum buffer length.  */
    return(_fx_unicode_length_get_extended(unicode_name, 256));
}


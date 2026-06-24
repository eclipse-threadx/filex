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
 * fx_unicode_name_get() is deprecated. Do not use it in new code.
 *
 * WHY: Does not accept a destination buffer length; writes up to FX_MAX_LONG_NAME_LEN*2 bytes regardless of the caller's buffer size, risking a buffer overrun.
 *
 * WHAT TO DO: replace calls with fx_unicode_name_get_extended(), passing the actual
 * destination buffer size as an additional argument.
 */
#pragma message("fx_unicode_name_get() is deprecated. " \
                "Use fx_unicode_name_get_extended() and pass the actual buffer size.")

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_unicode_name_get                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds the unicode name associated with the supplied   */
/*    short 8.3 name.                                                     */
/*                                                                        */
/*    DEPRECATED. Use fx_unicode_name_get_extended() instead, passing the actual
    destination buffer length. Does not accept a destination buffer length; writes up to FX_MAX_LONG_NAME_LEN*2 bytes regardless of the caller's buffer size, risking a buffer overrun.
/*    be used. The maximum written size to destination_unicode_name could */
/*    be FX_MAX_LONG_NAME_LEN * 2.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media              */
/*    source_short_name                     Pointer to short file name    */
/*    destination_unicode_name              Pointer to destination name   */
/*    destination_unicode_length            Destination for name length   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_unicode_name_get_extended         Actual unicode name get       */
/*                                            service                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_unicode_name_get(FX_MEDIA *media_ptr, CHAR *source_short_name,
                           UCHAR *destination_unicode_name, ULONG *destination_unicode_length)
{

    /* Call the extended version with 13 bytes short name length.  */
    return(_fx_unicode_name_get_extended(media_ptr, source_short_name, destination_unicode_name, destination_unicode_length, FX_MAX_LONG_NAME_LEN * 2));
}


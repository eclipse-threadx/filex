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
/*    Note, this API is deprecated as fx_unicode_name_get_extended should */
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


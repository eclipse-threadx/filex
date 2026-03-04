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
/*    _fx_unicode_short_name_get                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds the short name associated with the supplied     */
/*    unicode name.                                                       */
/*                                                                        */
/*    Note, this API is deprecated as fx_unicode_short_name_get_extended  */
/*    should be used. The maximum written size to destination_short_name  */
/*    could be 13.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media              */
/*    source_unicode_name                   Pointer to unicode name       */
/*    source_unicode_length                 Length of unicode name        */
/*    destination_short_name                Pointer to destination name   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_unicode_short_name_get_extended   Actual unicode short name get */
/*                                            service                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_unicode_short_name_get(FX_MEDIA *media_ptr, UCHAR *source_unicode_name, ULONG source_unicode_length,
                                 CHAR *destination_short_name)
{

    /* Call the extended version with 13 bytes short name length.  */
    return(_fx_unicode_short_name_get_extended(media_ptr, source_unicode_name, source_unicode_length, destination_short_name, 13));
}


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
/**   Media                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"
#include "fx_directory.h"
#include "fx_media.h"
#include "fx_utility.h"


/* DEPRECATION NOTICE
 * fx_media_volume_get() is deprecated. Do not use it in new code.
 *
 * WHY: Does not accept a volume-name buffer length; writes up to 12 bytes regardless of the caller's buffer size, risking a buffer overrun.
 *
 * WHAT TO DO: replace calls with fx_media_volume_get_extended(), passing the actual
 * destination buffer size as an additional argument.
 */
#pragma message("fx_media_volume_get() is deprecated. " \
                "Use fx_media_volume_get_extended() and pass the actual buffer size.")

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_media_volume_get                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads the volume name stored in the media's boot      */
/*    record or root directory.                                           */
/*                                                                        */
/*    DEPRECATED. Use fx_media_volume_get_extended() instead, passing the actual
    destination buffer length. Does not accept a volume-name buffer length; writes up to 12 bytes regardless of the caller's buffer size, risking a buffer overrun.
/*    be used. The maximum written size to volume_name could be 12.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    volume_name                           Pointer to destination for    */
/*                                            the volume name (maximum    */
/*                                            11 characters + NULL)       */
/*    volume_source                         Source of volume              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_media_volume_get_extended         Actual media volume get       */
/*                                            service                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_media_volume_get(FX_MEDIA *media_ptr, CHAR *volume_name, UINT volume_source)
{

    /* Call the extended version with 12 bytes volume name length.  */
    return(_fx_media_volume_get_extended(media_ptr, volume_name, 12, volume_source));
}


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
/**   Utility                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_FAT_sector_get                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the sector of supplied FAT entry.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    cluster                               Cluster entry number          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return sector of FAT entry                                          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    FileX System Functions                                              */
/*                                                                        */
/**************************************************************************/
ULONG  _fx_utility_FAT_sector_get(FX_MEDIA *media_ptr, ULONG cluster)
{

ULONG FAT_sector;
ULONG byte_offset;

    /* Determine which type of FAT is present.  */
    if (media_ptr -> fx_media_12_bit_FAT)
    {

        /* 12-bit FAT is present.  */

        /* Calculate the byte offset to the cluster entry.  */
        byte_offset =  (((ULONG)cluster << 1) + cluster) >> 1;

    }
    else if (!media_ptr -> fx_media_32_bit_FAT)
    {

        /* 16-bit FAT is present.  */

        /* Calculate the byte offset to the cluster entry.  */
        byte_offset =  (((ULONG)cluster) << 1);
    }
    else
    {

        /* 32-bit FAT is present.  */

        /* Calculate the byte offset to the cluster entry.  */
        byte_offset =  (((ULONG)cluster) * 4);
    }

    /* Calculate the FAT sector the requested FAT entry resides in.  */
    FAT_sector =  (byte_offset / media_ptr -> fx_media_bytes_per_sector) +
        (ULONG)media_ptr -> fx_media_reserved_sectors;

    /* Return successful status.  */
    return(FAT_sector);
}


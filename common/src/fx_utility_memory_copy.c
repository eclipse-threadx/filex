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
#include "fx_system.h"
#include "fx_utility.h"
#include "string.h"


/* Remove any previous remapping for memory copy when compiling this
   module.  */

#undef _fx_utility_memory_copy


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_memory_copy                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function copies the specified number of bytes from the source  */
/*    to the destination.                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    source_ptr                            Source memory pointer         */
/*    dest_ptr                              Destination memory pointer    */
/*    size                                  number of bytes to copy       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    FileX System Functions                                              */
/*                                                                        */
/**************************************************************************/
VOID  _fx_utility_memory_copy(UCHAR *source_ptr, UCHAR *dest_ptr, ULONG size)
{

    /* Copy the memory.  */
    memcpy(dest_ptr, source_ptr, size); /* Use case of memcpy is verified. */
}


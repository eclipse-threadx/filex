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
/**   Directory                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"
#include "fx_file.h"
#include "fx_utility.h"
#include "fx_directory.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fxe_directory_local_path_set                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the local path set call.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    local_path_ptr                        Local path control block ptr  */
/*    new_path_name                         New path to set current local */
/*                                            working directory to        */
/*    local_path_control_block_size         Size of the FX_LOCAL_PATH     */
/*                                            structure                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_local_path_set          Actual core service call      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/**************************************************************************/
UINT  _fxe_directory_local_path_set(FX_MEDIA *media_ptr, FX_LOCAL_PATH *local_path_ptr, CHAR *new_path_name, UINT local_path_control_block_size)
{

UINT status;


    /* Check for a null media pointer or null local path.  */
    if ((media_ptr == FX_NULL) || (local_path_ptr == FX_NULL) || (local_path_control_block_size != sizeof(FX_LOCAL_PATH)))
    {
        return(FX_PTR_ERROR);
    }

    /* Call actual local path set service.  */
    status =  _fx_directory_local_path_set(media_ptr, local_path_ptr, new_path_name);

    /* Return status.  */
    return(status);
}


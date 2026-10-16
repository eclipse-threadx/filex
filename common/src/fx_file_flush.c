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
/**   File                                                                */
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
/*    _fx_file_flush                                      PORTABLE C      */
/*                                                           6.5.x        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Eclipse ThreadX contributors                                        */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function flushes the specified file's directory entry (with    */
/*    the new size and time/date stamp) and the media's cached FAT and    */
/*    logical sector information out to the media, without closing the   */
/*    file. This allows the application to checkpoint a single open      */
/*    file's data to the media between write operations.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    file_ptr                              File control block pointer    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_write             Write the directory entry     */
/*    _fx_utility_FAT_flush                 Flush cached FAT entries      */
/*    _fx_utility_FAT_map_flush             Flush primary FAT changes to  */
/*                                            secondary FAT(s)            */
/*    _fx_utility_logical_sector_flush      Flush logical sector cache    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_file_flush(FX_FILE *file_ptr)
{

UINT      status;
FX_MEDIA *media_ptr;
FX_INT_SAVE_AREA


    /* First, determine if the file is still open.  */
    if (file_ptr -> fx_file_id != FX_FILE_ID)
    {

        /* Return the file not open error status.  */
        return(FX_NOT_OPEN);
    }

    /* Setup a pointer to the associated media.  */
    media_ptr =  file_ptr -> fx_file_media_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_FILE_FLUSH, file_ptr, file_ptr -> fx_file_current_file_size, 0, 0, FX_TRACE_FILE_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Check for write protect at the media level (set by driver).  */
    if (media_ptr -> fx_media_driver_write_protect)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return write protect error.  */
        return(FX_WRITE_PROTECT);
    }

    /* Determine if this file needs to have its directory entry written
       back to the media.  */
    if ((file_ptr -> fx_file_open_mode == FX_OPEN_FOR_WRITE) &&
        (file_ptr -> fx_file_modified))
    {

        /* Lockout interrupts for time/date access.  */
        FX_DISABLE_INTS

        /* Set the new time and date.  */
        file_ptr -> fx_file_dir_entry.fx_dir_entry_time =  _fx_system_time;
        file_ptr -> fx_file_dir_entry.fx_dir_entry_date =  _fx_system_date;

        /* Restore interrupts.  */
        FX_RESTORE_INTS

        /* Copy the new file size into the directory entry.  */
        file_ptr -> fx_file_dir_entry.fx_dir_entry_file_size =
            file_ptr -> fx_file_current_file_size;

        /* Write the directory entry to the media.  */
        status = _fx_directory_entry_write(media_ptr, &(file_ptr -> fx_file_dir_entry));

        /* Check for a good status.  */
        if (status != FX_SUCCESS)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Error writing the directory.  */
            return(status);
        }

        /* Clear the file modified flag.  */
        file_ptr -> fx_file_modified =  FX_FALSE;

        /* Flush the cached individual FAT entries. The file's cluster chain
           lives in the media-wide FAT cache, so this cannot be scoped to a
           single file.  */
        _fx_utility_FAT_flush(media_ptr);

        /* Flush changed sector(s) in the primary FAT to secondary FATs.  */
        _fx_utility_FAT_map_flush(media_ptr);

        /* Flush the internal logical sector cache. As with the FAT cache, the
           sector cache is shared across the media, so all dirty sectors are
           written out rather than only the ones touched by this file.  */
        status =  _fx_utility_logical_sector_flush(media_ptr, ((ULONG64) 1), (ULONG64) (media_ptr -> fx_media_total_sectors), FX_FALSE);

        /* Check for a good status.  */
        if (status != FX_SUCCESS)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* Error flushing the logical sector cache.  */
            return(status);
        }
    }

    /* Release media protection.  */
    FX_UNPROTECT

    /* Return status to the caller.  */
    return(FX_SUCCESS);
}

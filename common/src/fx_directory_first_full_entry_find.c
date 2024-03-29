/***************************************************************************
 * Copyright (c) 2024 Microsoft Corporation 
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
#include "fx_directory.h"
#include "fx_utility.h"

#ifndef FX_NO_LOCAL_PATH
FX_LOCAL_PATH_SETUP
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_first_full_entry_find                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the first directory entry of the current      */
/*    working directory and selected information about the entry.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    directory_name                        Destination for directory     */
/*                                            name                        */
/*    attributes                            Destination for attributes    */
/*    size                                  Destination for size          */
/*    year                                  Destination for year          */
/*    month                                 Destination for month         */
/*    day                                   Destination for day           */
/*    hour                                  Destination for hour          */
/*    minute                                Destination for minute        */
/*    second                                Destination for second        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_next_full_entry_find    Find next full directory entry*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    FileX System Functions                                              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_directory_first_full_entry_find(FX_MEDIA *media_ptr,
                                          CHAR *directory_name, UINT *attributes, ULONG *size,
                                          UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second)
{

UINT status;


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_directory_first_full_entry_finds++;
#endif

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_DIRECTORY_FIRST_FULL_ENTRY_FIND, media_ptr, directory_name, 0, 0, FX_TRACE_DIRECTORY_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Determine if a local path is in effect at this point.  */
#ifndef FX_NO_LOCAL_PATH
    if (_tx_thread_current_ptr -> tx_thread_filex_ptr)
    {

        /* Yes, there is a local path.  Set the current entry to zero.  */
        ((FX_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr) -> fx_path_current_entry =  0;
    }
    else
    {

        /* Use global default directory.  Set the current entry to 0 in
           order to pickup the first entry.  */
        media_ptr -> fx_media_default_path.fx_path_current_entry =  0;
    }
#else

    /* Set the current entry to 0 in order to pickup the first entry.  */
    media_ptr -> fx_media_default_path.fx_path_current_entry =  0;
#endif

    /* Release media protection.  */
    FX_UNPROTECT

    /* Call the next directory full entry service to pickup the first entry.  */
    status =  _fx_directory_next_full_entry_find(media_ptr, directory_name, attributes,
                                                 size, year, month, day, hour, minute, second);

    /* Return status to the caller.  */
    return(status);
}


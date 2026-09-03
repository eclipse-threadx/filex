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
#include "fx_directory.h"
#include "fx_file.h"
#include "fx_utility.h"
#ifdef FX_ENABLE_FAULT_TOLERANT
#include "fx_fault_tolerant.h"
#endif /* FX_ENABLE_FAULT_TOLERANT */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_rename                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function first attempts to find the specified directory.       */
/*    If found, the rename request is valid and the directory will be     */
/*    changed to the new name.  Otherwise, if the directory is not found, */
/*    the appropriate error code is returned to the caller.               */
/*    If the directory is moved under a different parent, the ".."        */
/*    entry inside the directory is updated to name the new parent.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    old_directory_name                    Old file directory pointer    */
/*    new_directory_name                    New file directory pointer    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_directory_entry_read              Read entries from directory   */
/*    _fx_directory_entry_write             Write the new directory entry */
/*    _fx_directory_free_search             Search for a free directory   */
/*                                            entry                       */
/*    _fx_directory_name_extract            Extract directory name        */
/*    _fx_directory_search                  Search for the file name in   */
/*                                          the directory structure       */
/*    _fx_fault_tolerant_transaction_start  Start fault tolerant          */
/*                                            transaction                 */
/*    _fx_fault_tolerant_transaction_end    End fault tolerant transaction*/
/*    _fx_fault_tolerant_recover            Recover FAT chain             */
/*    _fx_fault_tolerant_reset_log_file     Reset the log file            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/**************************************************************************/
UINT  _fx_directory_rename(FX_MEDIA *media_ptr, CHAR *old_directory_name, CHAR *new_directory_name)
{

UINT         status;
FX_DIR_ENTRY old_dir_entry;
FX_DIR_ENTRY new_dir_entry;
FX_DIR_ENTRY search_directory;
CHAR        *new_name_ptr;
ULONG        i;
ULONG        parent_cluster;
CHAR        *work_ptr;
CHAR         alpha, beta;
#ifdef FX_RENAME_PATH_INHERIT
UINT         j;
#endif


#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of times this service has been called.  */
    media_ptr -> fx_media_directory_renames++;
#endif

    /* Setup pointers to media name buffers.  */
    old_dir_entry.fx_dir_entry_name =     media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;
    new_dir_entry.fx_dir_entry_name =     media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN * 2;
    search_directory.fx_dir_entry_name =  media_ptr -> fx_media_name_buffer + FX_MAX_LONG_NAME_LEN * 3;

    /* Clear the short name strings.  */
    old_dir_entry.fx_dir_entry_short_name[0] =     0;
    new_dir_entry.fx_dir_entry_short_name[0] =     0;
    search_directory.fx_dir_entry_short_name[0] =  0;

    /* Determine if the supplied name is less than the maximum supported name size. The
       maximum name (FX_MAX_LONG_NAME_LEN) is defined in fx_api.h.  */
    i =  0;
    work_ptr =  (CHAR *)new_directory_name;
    while (*work_ptr && (i < FX_MAX_LONG_NAME_LEN))
    {

        /* Determine if the character designates a new path.  */
        if ((*work_ptr == '\\') || (*work_ptr == '/'))
        {
            /* Yes, reset the name size.  */
            i =  0;
        }
        /* Check for leading spaces.  */
        else if ((*work_ptr != ' ') || (i != 0))
        {

            /* No leading spaces, increment the name size.  */
            i++;
        }

        /* Move to the next character.  */
        work_ptr++;
    }

    /* Determine if the supplied name is valid.  */
    if ((i == 0) || (i >= FX_MAX_LONG_NAME_LEN))
    {

        /* Return an invalid name value.  */
        return(FX_INVALID_NAME);
    }

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_DIRECTORY_RENAME, media_ptr, old_directory_name, new_directory_name, 0, FX_TRACE_DIRECTORY_EVENTS, 0, 0)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Start transaction. */
    _fx_fault_tolerant_transaction_start(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Check for write protect at the media level (set by driver).  */
    if (media_ptr -> fx_media_driver_write_protect)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return write protect error.  */
        return(FX_WRITE_PROTECT);
    }

    /* Search the system for the supplied directory name.  */
    status =  _fx_directory_search(media_ptr, old_directory_name, &old_dir_entry, &search_directory, FX_NULL);

    /* Determine if the search was successful.  */
    if (status != FX_SUCCESS)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

    /* Check to make sure the found entry is a directory.  */
    if (!(old_dir_entry.fx_dir_entry_attributes & (UCHAR)(FX_DIRECTORY)))
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the not a directory error code.  */
        return(FX_NOT_DIRECTORY);
    }

#ifdef FX_RENAME_PATH_INHERIT

    /* Determine if the source directory name has a path and the target directory name does not.  */
    if (((old_directory_name[0] == '/') || (old_directory_name[0] == '\\')) && (new_directory_name[0] != '/') && (new_directory_name[0] != '\\'))
    {

        /* In this case, we need to prepend the path of the old directory name to that of the new directory name.  */

        /* Setup pointer to the rename buffer.  */
        work_ptr =  (CHAR *)media_ptr -> fx_media_rename_buffer;

        /* First, copy the path of the old directory name.  */
        i =  0;
        j =  0;
        while ((old_directory_name[i]) && (i < FX_MAXIMUM_PATH))
        {

            /* Copy a character into the rename buffer.  */
            *work_ptr++ =  old_directory_name[i];

            /* Determine if this character is directory separator.  */
            if ((old_directory_name[i] == '/') || (old_directory_name[i] == '\\'))
            {

                /* Yes, directory separator has been found - remember the index.  */
                j =  i;
            }

            /* Move to next position in the old directory name.  */
            i++;
        }

        /* At this point, we have the path stored in the rename buffer.  */

        /* Position past the last slash or backslash.  */
        j++;

        /* Reset the working pointer to the position after the last directory separator.  */
        work_ptr =  (CHAR *)&(media_ptr -> fx_media_rename_buffer[j]);

        /* Now copy the new directory name into the rename buffer.  */
        i =  0;
        while ((new_directory_name[i]) && (j < FX_MAXIMUM_PATH))
        {

            /* Copy a character into the rename buffer.  */
            *work_ptr++ =  new_directory_name[i];

            /* Move to next character.  */
            i++;
            j++;
        }

        /* Determine if the path was successfully prepended.  */
        if (new_directory_name[i])
        {

            /* No, there was not enough room in the destination buffer.  */

#ifdef FX_ENABLE_FAULT_TOLERANT
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the invalid path error code.  */
            return(FX_INVALID_PATH);
        }

        /* Place a NULL at the end of the string.  */
        *work_ptr =  (CHAR)FX_NULL;

        /* At this point, we have successfully prepended the path in the new directory name, override
           the new directory name so it is used from now on.  */
        new_directory_name =  (CHAR *)media_ptr -> fx_media_rename_buffer;
    }
#endif

    /* Search the media for the new directory name - including any supplied path.  */
    status = _fx_directory_search(media_ptr, new_directory_name, &new_dir_entry, &search_directory, &new_name_ptr);

    /* Determine if the search found anything.  */
    if (status == FX_SUCCESS)
    {

        /* Determine if the new name simply has an ASCII case change. If so, simply let the processing
           continue.  */
        i =  0;
        do
        {

            /* Pickup an old name and new name character and convert to upper case if necessary.  */
            alpha =  old_directory_name[i];
            if ((alpha >= 'a') && (alpha <= 'z'))
            {

                /* Lower case, convert to upper case!  */
                alpha =  (CHAR)((INT)alpha - 0x20);
            }
            beta =   new_directory_name[i];
            if ((beta >= 'a') && (beta <= 'z'))
            {

                /* Lower case, convert to upper case!  */
                beta =  (CHAR)((INT)beta - 0x20);
            }

            /* Now compare the characters.  */
            if ((alpha != beta) || (alpha == 0))
            {

                /* Get out of this loop!  */
                break;
            }

            /* Move to next character.  */
            i++;
        } while (i < (FX_MAXIMUM_PATH-1));

        /* Now determine if the names match.  */
        if (alpha != beta)
        {

            /* Yes, the directory name already exists in the target location.  */

#ifdef FX_ENABLE_FAULT_TOLERANT
            FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

            /* Release media protection.  */
            FX_UNPROTECT

            /* Return the error code.  */
            return(FX_ALREADY_CREATED);
        }
    }

    /* Make sure the name is valid.  */
    if (_fx_directory_name_extract(new_name_ptr, &new_dir_entry.fx_dir_entry_name[0]))
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection */
        FX_UNPROTECT

        /* Return the error code */
        return(FX_INVALID_NAME);
    }

    /* Look for a free slot in the target directory.  */
    status =  _fx_directory_free_search(media_ptr, &search_directory, &new_dir_entry);

    /* Was a free slot found?  */
    if (status != FX_SUCCESS)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* No, release protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

    /* Extract the new directory name.  */
    _fx_directory_name_extract(new_name_ptr, &new_dir_entry.fx_dir_entry_name[0]);

    /* Determine if the name was a long directory name.   */
    if (new_dir_entry.fx_dir_entry_long_name_present)
    {

        /* Yes, clear the short directory name to force a new one.  */
        new_dir_entry.fx_dir_entry_short_name[0] =  0;
    }

    /* Setup new attributes for the new directory entry.  */
    new_dir_entry.fx_dir_entry_attributes = old_dir_entry.fx_dir_entry_attributes;
    new_dir_entry.fx_dir_entry_cluster    = old_dir_entry.fx_dir_entry_cluster;
    new_dir_entry.fx_dir_entry_file_size  = old_dir_entry.fx_dir_entry_file_size;

    /* Save the reserved field.  */
    new_dir_entry.fx_dir_entry_reserved =            old_dir_entry.fx_dir_entry_reserved;

    /* Set time and date stamps.  */
    new_dir_entry.fx_dir_entry_created_time_ms =     old_dir_entry.fx_dir_entry_created_time_ms;
    new_dir_entry.fx_dir_entry_created_time =        old_dir_entry.fx_dir_entry_created_time;
    new_dir_entry.fx_dir_entry_created_date =        old_dir_entry.fx_dir_entry_created_date;
    new_dir_entry.fx_dir_entry_last_accessed_date =  old_dir_entry.fx_dir_entry_last_accessed_date;
    new_dir_entry.fx_dir_entry_time =                old_dir_entry.fx_dir_entry_time;
    new_dir_entry.fx_dir_entry_date =                old_dir_entry.fx_dir_entry_date;


    /* Is there a leading dot?  */
    if (new_dir_entry.fx_dir_entry_name[0] == '.')
    {

        /* Yes, toggle the hidden attribute bit.  */
        new_dir_entry.fx_dir_entry_attributes |=  FX_HIDDEN;
    }

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE

    /* Invalidate the directory cache.  */
    media_ptr -> fx_media_last_found_name[0] =  FX_NULL;
#endif

    /* Now write out the directory entry.  */
    status =  _fx_directory_entry_write(media_ptr, &new_dir_entry);

    /* Determine if the write was successful.  */
    if (status != FX_SUCCESS)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

    /* Set the old directory entry to free.  */
    old_dir_entry.fx_dir_entry_name[0] =        (CHAR)FX_DIR_ENTRY_FREE;
    old_dir_entry.fx_dir_entry_short_name[0] =  (CHAR)FX_DIR_ENTRY_FREE;

    /* Now wipe out the old directory entry.  */
    status =  _fx_directory_entry_write(media_ptr, &old_dir_entry);

    /* Determine if the write was successful.  */
    if (status != FX_SUCCESS)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

    /* The renamed directory itself contains a ".." entry, which stores
       the starting cluster of its parent directory. If the rename moved
       the directory under a different parent, that stored cluster is
       stale now: ".." inside the renamed directory would still lead to
       the old parent and, once the old parent's cluster is reused, to an
       unrelated place. Update it in place.  */

    /* Determine the starting cluster of the new parent directory.
       An empty name in the search directory marks the root
       directory - the name is the only root indication, the other
       fields keep earlier values in that case. A ".." entry stores
       its parent's starting cluster; when the parent is the root
       directory, it stores cluster 0.  */
    if (search_directory.fx_dir_entry_name[0])
    {

        /* New parent is a sub-directory.  */
        parent_cluster =  search_directory.fx_dir_entry_cluster;
    }
    else
    {

        /* New parent is the root directory.  */
        parent_cluster =  0;
    }

    /* old_dir_entry is no longer needed and still holds the starting
       cluster of the renamed directory - reuse it to address that
       directory as the one to read from. Ensure the cluster chain is
       walked from the start.  */
    old_dir_entry.fx_dir_entry_last_search_cluster =  0;

    /* Read the second directory entry of the renamed directory - its
       ".." entry, created together with the directory. new_dir_entry
       is no longer needed either and serves as the destination.  */
    i =  1;
    status =  _fx_directory_entry_read(media_ptr, &old_dir_entry, &i, &new_dir_entry);

    /* Update the entry only if it is the ".." entry the FAT format
       places at this position - on a non-conformant media the rename
       result is left as it is - and only if the stored cluster does
       not match the new parent already. A rename inside one parent
       ends here without an additional write.  */
    if ((status == FX_SUCCESS) &&
        (new_dir_entry.fx_dir_entry_name[0] == '.') &&
        (new_dir_entry.fx_dir_entry_name[1] == '.') &&
        (new_dir_entry.fx_dir_entry_name[2] == 0) &&
        (new_dir_entry.fx_dir_entry_cluster != parent_cluster))
    {

        /* Update the stored parent cluster. Everything else of the
           entry - its position included - stays untouched: the first
           two entries of a directory are a format invariant other
           services rely on.  */
        new_dir_entry.fx_dir_entry_cluster =  parent_cluster;

        /* Write the updated ".." entry back.  */
        status =  _fx_directory_entry_write(media_ptr, &new_dir_entry);
    }

    /* Determine if the ".." update was successful.  */
    if (status != FX_SUCCESS)
    {

#ifdef FX_ENABLE_FAULT_TOLERANT
        FX_FAULT_TOLERANT_TRANSACTION_FAIL(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the error code.  */
        return(status);
    }

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* End transaction. */
    status = _fx_fault_tolerant_transaction_end(media_ptr);
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Release media protection.  */
    FX_UNPROTECT

    /* Directory rename is complete, return status.  */
    return(status);
}


/***************************************************************************/
/* Copyright (c) 2024 Microsoft Corporation                                */
/* Copyright (c) 2026 Eclipse ThreadX contributors                         */
/*                                                                         */
/* This program and the accompanying materials are made available under    */
/* the terms of the MIT License which is available at                      */
/* https://opensource.org/licenses/MIT.                                    */
/*                                                                         */
/* SPDX-License-Identifier: MIT                                            */
/***************************************************************************/

/* This FileX test concentrates on the directory rename operations.  */

#ifndef FX_STANDALONE_ENABLE
#include   "tx_api.h"
#endif
#include   "fx_api.h"
#include   "fx_directory.h"
#include   "fx_ram_driver_test.h"
#include   "fx_fault_tolerant.h"
#include   "fx_utility.h"
#include   <stdio.h>

#define     DEMO_STACK_SIZE         8192
#define     CACHE_SIZE              4096
#ifdef FX_ENABLE_FAULT_TOLERANT
#define     FAULT_TOLERANT_SIZE     FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#else
#define     FAULT_TOLERANT_SIZE     0
#endif


/* Define the ThreadX and FileX object control blocks...  */

#ifndef FX_STANDALONE_ENABLE
static TX_THREAD                ftest_0;
#endif
static FX_MEDIA                 ram_disk;
static FX_FILE                  probe_file;
static CHAR                     max_name[FX_MAX_LONG_NAME_LEN + 2];
static CHAR                     max_newname[FX_MAX_LONG_NAME_LEN + 1];


/* Define the counters used in the test application...  */

#ifndef FX_STANDALONE_ENABLE
static UCHAR                    *ram_disk_memory;
static UCHAR                    *cache_buffer;
static UCHAR                    *fault_tolerant_buffer;
#else
static UCHAR                    cache_buffer[CACHE_SIZE];
static UCHAR                    fault_tolerant_buffer[FAULT_TOLERANT_SIZE];
#endif


/* Define thread prototypes.  */

void    filex_directory_rename_application_define(void *first_unused_memory);
static void    ftest_0_entry(ULONG thread_input);
static UINT    dot_dot_probe(CHAR *marker_path);
static UINT    directory_cluster_read(CHAR *directory_path, ULONG *cluster_ptr);
static UINT    dot_dot_cluster_read(CHAR *directory_path, ULONG *cluster_ptr);

VOID  _fx_ram_driver(FX_MEDIA *media_ptr);
void  test_control_return(UINT status);



/* Define what the initial system looks like.  */

#ifdef CTEST
void test_application_define(void *first_unused_memory)
#else
void    filex_directory_rename_application_define(void *first_unused_memory)
#endif
{

#ifndef FX_STANDALONE_ENABLE
    UCHAR    *pointer;


    /* Setup the working pointer.  */
    pointer =  (UCHAR *) first_unused_memory;

    /* Create the main thread.  */
    tx_thread_create(&ftest_0, "thread 0", ftest_0_entry, 0,
            pointer, DEMO_STACK_SIZE,
            4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Setup memory for the RAM disk and the sector cache.  */
    cache_buffer =  pointer;
    pointer =  pointer + CACHE_SIZE;
    fault_tolerant_buffer = pointer;
    pointer += FAULT_TOLERANT_SIZE;
    ram_disk_memory =  pointer;

#endif

    /* Initialize the FileX system.  */
    fx_system_initialize();
#ifdef FX_STANDALONE_ENABLE
    ftest_0_entry(0);
#endif
}



/* Open and close the file at marker_path - a path that contains a ".."
   component - to report where ".." currently leads.  */
static UINT    dot_dot_probe(CHAR *marker_path)
{

UINT status;

    status =  fx_file_open(&ram_disk, &probe_file, marker_path, FX_OPEN_FOR_READ);
    if (status == FX_SUCCESS)
    {
        fx_file_close(&probe_file);
    }

    return(status);
}


/* Return the starting cluster of the directory at directory_path.  */
static UINT    directory_cluster_read(CHAR *directory_path, ULONG *cluster_ptr)
{

UINT         status;
FX_DIR_ENTRY dir_entry;

    /* The directory's own entry carries its starting cluster.  */
    dir_entry.fx_dir_entry_name =  ram_disk.fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;
    dir_entry.fx_dir_entry_short_name[0] =  0;
    status =  _fx_directory_search(&ram_disk, directory_path, &dir_entry, FX_NULL, FX_NULL);

    /* Return the cluster only on success.  */
    if (status == FX_SUCCESS)
    {
        *cluster_ptr =  dir_entry.fx_dir_entry_cluster;
    }

    return(status);
}


/* Read the ".." entry - entry 1 - of the directory at directory_path and
   return the parent starting cluster stored in it. Verifies that the
   entry actually reads back as "..".  */
static UINT    dot_dot_cluster_read(CHAR *directory_path, ULONG *cluster_ptr)
{

UINT         status;
ULONG        entry_index;
FX_DIR_ENTRY dir_entry;
FX_DIR_ENTRY dot_dot_entry;

    /* Find the directory itself first.  */
    dir_entry.fx_dir_entry_name =  ram_disk.fx_media_name_buffer + FX_MAX_LONG_NAME_LEN;
    dir_entry.fx_dir_entry_short_name[0] =  0;
    status =  _fx_directory_search(&ram_disk, directory_path, &dir_entry, FX_NULL, FX_NULL);
    if (status != FX_SUCCESS)
    {
        return(status);
    }

    /* Read its second directory entry - the ".." entry.  */
    dir_entry.fx_dir_entry_last_search_cluster =  0;
    dot_dot_entry.fx_dir_entry_name =  ram_disk.fx_media_name_buffer + FX_MAX_LONG_NAME_LEN * 2;
    dot_dot_entry.fx_dir_entry_short_name[0] =  0;
    entry_index =  1;
    status =  _fx_directory_entry_read(&ram_disk, &dir_entry, &entry_index, &dot_dot_entry);
    if (status != FX_SUCCESS)
    {
        return(status);
    }

    /* Anything other than ".." at this position is a failure.  */
    if ((dot_dot_entry.fx_dir_entry_name[0] != '.') ||
        (dot_dot_entry.fx_dir_entry_name[1] != '.') ||
        (dot_dot_entry.fx_dir_entry_name[2] != 0))
    {
        return(FX_INVALID_STATE);
    }

    *cluster_ptr =  dot_dot_entry.fx_dir_entry_cluster;

    return(FX_SUCCESS);
}


/* Define the test threads.  */

static void    ftest_0_entry(ULONG thread_input)
{

    UINT        status;
    UINT        i;
    ULONG       parent_cluster;
    ULONG       dot_dot_cluster;
#ifndef FX_MEDIA_STATISTICS_DISABLE
    ULONG       entry_writes_before;
#endif

    FX_PARAMETER_NOT_USED(thread_input);

    /* Print out some test information banners.  */
    printf("FileX Test:   Directory rename test..................................");

    /* Format the media.  This needs to be done before opening it!  */
    status =  fx_media_format(&ram_disk,
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            128,                    // Directory Entries
                            0,                      // Hidden sectors

/* We need a larger disk to test fault tolerant feature. */
#ifdef FX_ENABLE_FAULT_TOLERANT
                            4096 * 8,               // Total sectors
                            256,                    // Sector size
                            8,                      // Sectors per cluster
#else
                            4096,                   // Total sectors
                            128,                    // Sector size
                            1,                      // Sectors per cluster
#endif

                            1,                      // Heads
                            1);                     // Sectors per track
    return_if_fail( status == FX_SUCCESS);

    /* Test a directory name before the media is opened to generate an error */
    status = fx_directory_name_test(&ram_disk, "/A0");
    return_if_fail( status == FX_MEDIA_NOT_OPEN);

    /* Try to rename a directory to an invalid name */
    status = fx_directory_rename(&ram_disk, "/A0", "");
    return_if_fail( status == FX_INVALID_NAME);

    /* Open the ram_disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

#ifdef FX_ENABLE_FAULT_TOLERANT
    /* Enable fault tolerant if FX_ENABLE_FAULT_TOLERANT is defined. */
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);
#endif

    /* Attempt to rename a directory while the media is write protected */
    ram_disk.fx_media_driver_write_protect = FX_TRUE;
    status = fx_directory_rename(&ram_disk, "/A0", "/A1");
    ram_disk.fx_media_driver_write_protect = FX_FALSE;
    return_if_fail( status == FX_WRITE_PROTECT);

    /* Attempt to rename a directory that does not exist */
    status = fx_directory_rename(&ram_disk, "/DOES_NOT_EXIST", "/VALID");
    return_if_fail( status != FX_SUCCESS);

    /* Attempt to rename something that is not a directory */
    status = fx_file_create(&ram_disk, "NOT_A_DIR");
    status += fx_directory_rename(&ram_disk, "NOT_A_DIR", "/VALID");
    status += fx_file_delete(&ram_disk, "NOT_A_DIR");
    return_if_fail( status == FX_NOT_DIRECTORY);

/* Only run this if error checking is enabled */
#ifndef FX_DISABLE_ERROR_CHECKING

    /* send null pointer to generate an error */
    status = fx_directory_rename(FX_NULL, "MYTEST", "OURTEST");
    return_if_fail( status == FX_PTR_ERROR);

#endif /* FX_DISABLE_ERROR_CHECKING */

    /* Simple 8.3 rename in the root directory.  */
    status =  fx_directory_create(&ram_disk, "MYTEST");
    status += fx_directory_rename(&ram_disk, "MYTEST", "OURTEST");
    return_if_fail( status == FX_SUCCESS);

    /* Now create a sub-directory in the "OURTEST" directory.  */
    status =  fx_directory_create(&ram_disk, "OURTEST/newdir");
    status += fx_directory_rename(&ram_disk, "OURTEST/newdir", "newnewdir");
    status += fx_directory_rename(&ram_disk, "newnewdir", "OURTEST/newdir");
    status += fx_directory_create(&ram_disk, "/OURTEST/newdir/finaltest");
    return_if_fail( status == FX_SUCCESS);

    /* Attempt to rename a directory to a name that is taken */
    status = fx_directory_rename(&ram_disk, "/OURTEST/newdir/finaltest", "/OURTEST/newdir");
    return_if_fail( status == FX_ALREADY_CREATED);

    /* Attempt to rename a directory to a subdirectory with an invalid name */
    status = fx_directory_rename(&ram_disk, "OURTEST/newdir/finaltest", "OURTEST/newdir/does_not_exist/this_wont_work");
    return_if_fail( status == FX_INVALID_NAME);

    /* Rename a directory and make it hidden */
    status = fx_directory_rename(&ram_disk, "OURTEST/newdir/finaltest", "/.hidden");
    return_if_fail( status == FX_SUCCESS);

    /* Create a root directory so we can rename it.  */
    status =  fx_directory_create(&ram_disk, "newroot");

    /* Now try to rename the root directory, but with leading and trailing spaces.  */
    status += fx_directory_rename(&ram_disk, "newroot", "     newnewroot    ");
    return_if_fail( status == FX_SUCCESS);

    /* Now test the maximum size of the new directory name.  */
    for (i = 0; i < FX_MAX_LONG_NAME_LEN; i++)
    {
        max_name[i] =     'a';
    }
    max_name[i] =  0;

    /* Now try to test the maximum new name length.  */
    status =  fx_directory_rename(&ram_disk, "newnewroot", max_name);
    return_if_fail( status == FX_INVALID_NAME);

    max_name[i] = 'a';
    max_name[i + 1] = 'a';

    /* Now try to test the maximum new name length.  */
    status = fx_directory_rename(&ram_disk, "newnewroot", max_name);
    return_if_fail(status == FX_INVALID_NAME);

    /* Now make the max name valid.  */
    max_name[FX_MAX_LONG_NAME_LEN-1] =  0;
    max_name[FX_MAX_LONG_NAME_LEN-2] =  '~';

    /* Rename the directory to a valid maximum name.  */
    status =  fx_directory_rename(&ram_disk, "newnewroot", max_name);
    return_if_fail( status == FX_SUCCESS);

    /* Now build a new long file name with only one cap character different.  */
    for (i = 0; i <  FX_MAX_LONG_NAME_LEN+1; i++)
    {
        max_newname[i] =  max_name[i];
    }
    /* Change just the first character to upper case.  */
    max_newname[0] =  'A';

    /* Rename the directory to a valid maximum name with just an upper case change.  */
    status =  fx_directory_rename(&ram_disk, max_name, max_newname);

/* If we rename a file with longest allowed long file, the file size will overflow FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE, which is defined as 2048 in test enviroment. */
#if defined(FX_ENABLE_FAULT_TOLERANT) && (FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE < 3072)
    return_if_fail( status == FX_NO_MORE_SPACE);
#else
    return_if_fail( status == FX_SUCCESS);
#endif

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Open the ram_disk - but with only one sector cache.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, 256);
    return_if_fail( status == FX_SUCCESS);

    /* Now try to get an I/O error on the directory free search call.  */
#ifndef FX_ENABLE_FAULT_TOLERANT
    _fx_utility_logical_sector_read_error_request =  74;
#else
    _fx_utility_logical_sector_read_error_request =  22;
#endif
    status =  fx_directory_rename(&ram_disk, max_newname, "newroot");
    _fx_utility_logical_sector_read_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Now try to get an I/O error on the directory write call.  */
    _fx_utility_logical_sector_write_error_request =  1;
    status =  fx_directory_rename(&ram_disk, max_newname, "newroot");
    _fx_utility_logical_sector_write_error_request =  0;
    return_if_fail( status == FX_IO_ERROR);

    /* Now rename the direction with a root directory path.  */
    status =  fx_directory_rename(&ram_disk, max_newname, "\\newroot");
    return_if_fail( status == FX_SUCCESS);

    /* Fail to update old directory entry. */
    _fx_directory_entry_write_error_request = 2;
    status =  fx_directory_rename(&ram_disk, "\\newroot", "aaa");
    return_if_fail( status == FX_IO_ERROR);

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    /* Format the media again: the checks below build their own fixture
       and must not disturb - or depend on - the directory layout the
       error-injection steps above are calibrated against.  */
    status =  fx_media_format(&ram_disk,
                            _fx_ram_driver,         // Driver entry
                            ram_disk_memory,        // RAM disk memory pointer
                            cache_buffer,           // Media buffer pointer
                            CACHE_SIZE,             // Media buffer size
                            "MY_RAM_DISK",          // Volume Name
                            1,                      // Number of FATs
                            128,                    // Directory Entries
                            0,                      // Hidden sectors

#ifdef FX_ENABLE_FAULT_TOLERANT
                            4096 * 8,               // Total sectors
                            256,                    // Sector size
                            8,                      // Sectors per cluster
#else
                            4096,                   // Total sectors
                            128,                    // Sector size
                            1,                      // Sectors per cluster
#endif

                            1,                      // Heads
                            1);                     // Sectors per track
    status += fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, cache_buffer, CACHE_SIZE);
    return_if_fail( status == FX_SUCCESS);

#ifdef FX_ENABLE_FAULT_TOLERANT
    status = fx_fault_tolerant_enable(&ram_disk, fault_tolerant_buffer, FAULT_TOLERANT_SIZE);
    return_if_fail( status == FX_SUCCESS);
#endif

    /* Moving a directory to a different parent has to update the ".."
       entry inside the moved directory: it stores the starting cluster
       of the parent directory - cluster 0 when the parent is the root
       directory. Exercise every parent transition and verify both the
       path resolution through ".." and the on-media entry itself.  */
    status =  fx_directory_create(&ram_disk, "/DOTA");
    status += fx_directory_create(&ram_disk, "/DOTB");
    status += fx_file_create(&ram_disk, "/MARKR.TXT");
    status += fx_file_create(&ram_disk, "/DOTA/MARKA.TXT");
    status += fx_file_create(&ram_disk, "/DOTB/MARKB.TXT");
    status += fx_directory_create(&ram_disk, "/DOTA/SUB");
    return_if_fail( status == FX_SUCCESS);

    /* The probe itself has to work before anything is moved: ".." of a
       new directory leads to the parent it was created in.  */
    status =  dot_dot_probe("/DOTA/SUB/../MARKA.TXT");
    return_if_fail( status == FX_SUCCESS);
    status =  dot_dot_probe("/DOTA/SUB/../MARKB.TXT");
    return_if_fail( status == FX_NOT_FOUND);

    /* Move the directory to a different parent directory. Exactly one
       directory entry write - the ".." update - is added to the two
       parent entry writes.  */
#ifndef FX_MEDIA_STATISTICS_DISABLE
    entry_writes_before =  ram_disk.fx_media_directory_entry_writes;
#endif
    status =  fx_directory_rename(&ram_disk, "/DOTA/SUB", "/DOTB/SUB");
    return_if_fail( status == FX_SUCCESS);
#ifndef FX_MEDIA_STATISTICS_DISABLE
    return_if_fail( (ram_disk.fx_media_directory_entry_writes - entry_writes_before) == 3);
#endif

    /* ".." now leads to the new parent and only to the new parent.  */
    status =  dot_dot_probe("/DOTB/SUB/../MARKB.TXT");
    return_if_fail( status == FX_SUCCESS);
    status =  dot_dot_probe("/DOTB/SUB/../MARKA.TXT");
    return_if_fail( status == FX_NOT_FOUND);

    /* The on-media ".." entry stores the new parent's starting cluster.  */
    status =  directory_cluster_read("/DOTB", &parent_cluster);
    return_if_fail( status == FX_SUCCESS);
    status =  dot_dot_cluster_read("/DOTB/SUB", &dot_dot_cluster);
    return_if_fail( status == FX_SUCCESS);
    return_if_fail( (dot_dot_cluster == parent_cluster) && (parent_cluster != 0));

    /* Move the directory into the root directory: ".." stores cluster 0
       there, per the FAT convention directory create itself writes.  */
    status =  fx_directory_rename(&ram_disk, "/DOTB/SUB", "/SUB");
    return_if_fail( status == FX_SUCCESS);
    status =  dot_dot_probe("/SUB/../MARKR.TXT");
    return_if_fail( status == FX_SUCCESS);
    status =  dot_dot_probe("/SUB/../MARKB.TXT");
    return_if_fail( status == FX_NOT_FOUND);
    status =  dot_dot_cluster_read("/SUB", &dot_dot_cluster);
    return_if_fail( (status == FX_SUCCESS) && (dot_dot_cluster == 0));

    /* Move the directory back out of the root directory: 0 becomes the
       new parent's cluster again.  */
    status =  fx_directory_rename(&ram_disk, "/SUB", "/DOTA/SUB");
    return_if_fail( status == FX_SUCCESS);
    status =  dot_dot_probe("/DOTA/SUB/../MARKA.TXT");
    return_if_fail( status == FX_SUCCESS);
    status =  dot_dot_probe("/DOTA/SUB/../MARKR.TXT");
    return_if_fail( status == FX_NOT_FOUND);
    status =  directory_cluster_read("/DOTA", &parent_cluster);
    return_if_fail( status == FX_SUCCESS);
    status =  dot_dot_cluster_read("/DOTA/SUB", &dot_dot_cluster);
    return_if_fail( (status == FX_SUCCESS) && (dot_dot_cluster == parent_cluster) && (parent_cluster != 0));

    /* A rename inside one parent does not touch ".." - and adds no third
       directory entry write.  */
    status =  fx_directory_create(&ram_disk, "/DOTC");
    return_if_fail( status == FX_SUCCESS);
#ifndef FX_MEDIA_STATISTICS_DISABLE
    entry_writes_before =  ram_disk.fx_media_directory_entry_writes;
#endif
    status =  fx_directory_rename(&ram_disk, "/DOTC", "/DOTD");
    return_if_fail( status == FX_SUCCESS);
#ifndef FX_MEDIA_STATISTICS_DISABLE
    return_if_fail( (ram_disk.fx_media_directory_entry_writes - entry_writes_before) == 2);
#endif
    status =  dot_dot_probe("/DOTD/../MARKR.TXT");
    return_if_fail( status == FX_SUCCESS);
    status =  dot_dot_cluster_read("/DOTD", &dot_dot_cluster);
    return_if_fail( (status == FX_SUCCESS) && (dot_dot_cluster == 0));

    /* Same inside a sub-directory: ".." keeps the parent's cluster.  */
    status =  fx_directory_rename(&ram_disk, "/DOTA/SUB", "/DOTA/SUB2");
    return_if_fail( status == FX_SUCCESS);
    status =  dot_dot_probe("/DOTA/SUB2/../MARKA.TXT");
    return_if_fail( status == FX_SUCCESS);
    status =  dot_dot_cluster_read("/DOTA/SUB2", &dot_dot_cluster);
    return_if_fail( (status == FX_SUCCESS) && (dot_dot_cluster == parent_cluster));

    /* Close the media.  */
    status =  fx_media_close(&ram_disk);
    return_if_fail( status == FX_SUCCESS);

    printf("SUCCESS!\n");
    test_control_return(0);
}

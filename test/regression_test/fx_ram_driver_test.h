/***************************************************************************
 * Copyright (c) 2024 Microsoft Corporation 
 * Copyright (c) 2026 Eclipse ThreadX contributors
 * 
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 * 
 * SPDX-License-Identifier: MIT
 **************************************************************************/

// Portions of this file were generated with AI assistance.

#ifndef _FX_RAM_DRIVER_TEST_H_
#define _FX_RAM_DRIVER_TEST_H_
                                
#define FX_OP_WRITE_NORMALLY                0
#define FX_OP_WRITE_INTERRUPT               1

#define FX_FAULT_TOLERANT_LOG_ERROR         0x00
#define FX_FAULT_TOLERANT_LOG_UNDO_UPDATING 0x01          
#define FX_FAULT_TOLERANT_LOG_UNDO_DONE     0x02 
#define FX_FAULT_TOLERANT_LOG_REDO_UPDATING 0x04
#define FX_FAULT_TOLERANT_LOG_REDO_DONE     0x08 
#define FX_FAULT_TOLERANT_LOG_DONE          0x10   


/* Define the memory buffer to format the media if enable fault tolerant feature.*/
extern UCHAR ram_disk_memory_large[900000000];      
extern UCHAR large_data_buffer[900000000];
#define large_data_buffer_size 900000000    

/* Define memory for tests to be run in standalone mode (without Azure RTOS: ThreadX) */
#ifdef FX_STANDALONE_ENABLE
extern UCHAR ram_disk_memory[300000000];
extern UCHAR ram_disk_memory1[30000000];
#endif

/* Selective I/O error injection in the RAM driver.  Set the request code and the sector,
   then set _fx_ram_driver_io_error_select, and the next matching driver request returns
   FX_IO_ERROR.  The driver clears the flag as it fires and counts the failure, so a test
   asserts on the count rather than trusting that the request reached the driver at all --
   a cached sector is served without any driver request, and a count of zero is how that
   shows up.

   The declarations sit beside the driver that defines them rather than with the older
   error flags in the port header, because the selector is a property of this test driver
   rather than of the port.

   _fx_ram_driver_io_error_select_sector is set to FX_RAM_DRIVER_ANY_SECTOR to fail the
   nominated request wherever it lands.  */

#define FX_RAM_DRIVER_ANY_SECTOR            ((ULONG64) ~((ULONG64) 0))

extern  ULONG   _fx_ram_driver_io_error_select;
extern  ULONG   _fx_ram_driver_io_error_select_request;
extern  ULONG64 _fx_ram_driver_io_error_select_sector;
extern  ULONG   _fx_ram_driver_io_error_select_count;


/* Define a macro for test. */
/* We do not need error code anymore but still define return_value_if_fail for compatibility backward. */
#define return_value_if_fail( p, val) if(!(p))       \
    {printf("ERROR!\n%s:%d\nError: "#p" failed.\n",  \
    __FILE__, __LINE__);test_control_return(val);}
#define return_if_fail( p) if(!(p))                  \
    {printf("ERROR!\n%s:%d\nError: "#p" failed.\n",  \
    __FILE__, __LINE__);test_control_return(254);}
                                          
/* Application define for CTEST. */
void test_application_define(void *first_unused_memory);

#endif /* _FX_RAM_DRIVER_TEST_H_ */

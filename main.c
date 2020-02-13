/******************************************************************************
*
* Copyright (C) 2013 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xqspips_g128_flash_example.c
*
*
* This file contains a design example using the QSPI driver (XQspiPs)
* with a serial Flash device greater than 128Mb. The example writes to flash
* and reads it back in I/O mode. This examples performs
* some transfers in Auto mode and Manual start mode, to illustrate the modes
* available. It is recommended to use Manual CS + Auto start for
* best performance.
* This example illustrates single, parallel and stacked modes.
* Both the flash devices have to be of the same make and size.
* The hardware which this example runs on, must have a serial Flash (Micron
* N25Q or Spansion S25FL) for it to run. This example has been
* tested with the Micron Serial Flash (N25Q256, N25Q512 & N25Q00AA) and
* Spansion (S25FL256 & S25FL512)
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 2.02a hk  05/07/13 First release
*       raw 12/10/15 Added support for Macronix 256Mb and 1Gb flash parts
*       ms  04/05/17 Modified Comment lines in functions to
*                    recognize it as documentation block for doxygen
*                    generation.
* 		tjs	06/16/17 Added support for IS25LP256D flash part (PR-4650)
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xqspips.h"		/* QSPI device driver */
#include "xil_io.h"
#include "flash.h"
#include "ps7_init.h"

#define SLICE_SIZE 16384
#define PAGE_SIZE 256

/*
 * Following global variables/defines are for communicating with the XSCT's TCL.
 * Do not rename them.
 */

#define TCL_FUNCTION_ID_RETURN		 		0x0
#define TCL_FUNCTION_ID_INIT		 		0x1
#define TCL_FUNCTION_ID_PRINT_HELLO	 		0x10
#define TCL_FUNCTION_ID_GET_VERSION	 		0x20
#define TCL_FUNCTION_ID_FLASH_ERASE 		0x100
#define TCL_FUNCTION_ID_GET_FLASH_INFO   	0x101
#define TCL_FUNCTION_ID_FLASH_INIT	 		0x105
#define TCL_FUNCTION_ID_FLASH_WRITE 		0x110
#define TCL_FUNCTION_ID_FLASH_READ  		0x120
#define TCL_FUNCTION_ID_FLASH_DUMP  		0x125
#define TCL_FUNCTION_ID_GET_SLICE_BUFFER 	0x130
#define TCL_FUNCTION_ID_GET_SLICE_BUFFER_SIZE 0x140

/*
 * tcl_function_call[0] the function ID
 * tcl_function_call[1-7] parameters of the function.
 *
 * Note, that the return value / function done will be placed
 * in this register too:
 * tcl_function_call[0] = 0 // This means, that the function done.
 * tcl_function_call[1] is the return value.
 */
u32 volatile tcl_function_call[16] = {TCL_FUNCTION_ID_INIT, 0, 0, 0, 0, 0, 0, 0};
volatile u32 volatile sync_reg[2];
volatile u8 volatile bin_slice_buffer[SLICE_SIZE+16];


/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XQspiPs QspiInstance;

//extern u8* ReadBuffer;


/*****************************************************************************/
/**
*
* Main function to call the QSPI Flash example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	u8* ReadBuffer;
	int ret;
/*
	verb_printf(INFO, "asdasdQSPI Greater than 128Mb Flash Example Test \r\n");


	FlashInit(&QspiInstance, QSPI_DEVICE_ID);

	QspiFlashDump(&QspiInstance, 0, 64);
*/
	tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
	while(1){
		switch (tcl_function_call[0]){
		case TCL_FUNCTION_ID_RETURN:
			// Wait for a real function
			break;
		case TCL_FUNCTION_ID_PRINT_HELLO:
			xil_printf("Hello parameter: %d\r\n", tcl_function_call[1]);
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_FLASH_ERASE:
			xil_printf("FLASH_ERASE: 0x%x 0x%x\r\n", tcl_function_call[1], tcl_function_call[2]);
			FlashErase2(&QspiInstance, tcl_function_call[1], tcl_function_call[2]);
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_FLASH_WRITE:
			//xil_printf("FLASH_WRITE: 0x%x 0x%x 0x%x\r\n", tcl_function_call[1], tcl_function_call[2], tcl_function_call[3]);
			FlashWrite2(&QspiInstance, tcl_function_call[1], tcl_function_call[2], (u8*)tcl_function_call[3]);
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_FLASH_READ:
			xil_printf("TCL_FUNCTION_ID_FLASH_READ: 0x%x 0x%x\r\n", tcl_function_call[1], tcl_function_call[2]);
			ReadBuffer = FlashRead2(&QspiInstance, tcl_function_call[1], tcl_function_call[2]);
			tcl_function_call[1] = (u32)ReadBuffer;
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_FLASH_INIT:
			ret = FlashInit(&QspiInstance, QSPI_DEVICE_ID);
			tcl_function_call[1] = (u32)ret;
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_FLASH_DUMP:
			QspiFlashDump(&QspiInstance, tcl_function_call[1], tcl_function_call[2]);
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_GET_SLICE_BUFFER:
			tcl_function_call[1] = (u32)bin_slice_buffer;
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_GET_SLICE_BUFFER_SIZE:
			tcl_function_call[1] = SLICE_SIZE;
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_GET_FLASH_INFO:
			tcl_function_call[1] = GetFlashInfo()->SectSize;
			tcl_function_call[2] = GetFlashInfo()->NumSect;
			tcl_function_call[3] = GetFlashInfo()->PageSize;
			tcl_function_call[4] = GetFlashInfo()->NumPage;
			tcl_function_call[5] = GetFlashInfo()->FlashDeviceSize;
			tcl_function_call[6] = GetFlashInfo()->ManufacturerID;
			tcl_function_call[7] = GetFlashInfo()->DeviceIDMemSize;
			tcl_function_call[8] = GetFlashInfo()->SectMask;
			tcl_function_call[9] = GetFlashInfo()->NumDie;
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_GET_VERSION:
			tcl_function_call[1] = 2;
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		}
	}

}


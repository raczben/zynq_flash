/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "flash.h"

#define FLASH_WRITER_MAJOR_VERSION 0x01
#define FLASH_WRITER_MINOR_VERSION 0x00
#define FLASH_WRITER_BUGFIX_VERSION 0x00
#define FLASH_WRITER_VERSION (FLASH_WRITER_MAJOR_VERSION << 16 | FLASH_WRITER_MINOR_VERSION << 8 | FLASH_WRITER_BUGFIX_VERSION << 8)


/*
 * Following global variables/defines are for communicating with the XSCT's TCL.
 * Do not rename them.
 */

#define TCL_FUNCTION_ID_RETURN		 		0x0
#define TCL_FUNCTION_ID_INIT		 		0x1
#define TCL_FUNCTION_ID_PRINT_HELLO	 		0x10
#define TCL_FUNCTION_ID_GET_VERSION	 		0x20
#define TCL_FUNCTION_ID_FLASH_ERASE 		0x100
#define TCL_FUNCTION_ID_FLASH_ERASE_ALL 	0x102
#define TCL_FUNCTION_ID_GET_FLASH_INFO   	0x101
#define TCL_FUNCTION_ID_FLASH_INIT	 		0x105
#define TCL_FUNCTION_ID_FLASH_WRITE 		0x110
#define TCL_FUNCTION_ID_FLASH_READ  		0x120
#define TCL_FUNCTION_ID_FLASH_DUMP  		0x125
#define TCL_FUNCTION_ID_GET_SLICE_BUFFER 	0x130
#define TCL_FUNCTION_ID_GET_SLICE_BUFFER_SIZE 0x140

/*
 * tcl_function_call[0] the function ID
 * tcl_function_call[1-15] parameters of the function.
 *
 * Note, that the return value / function done will be placed
 * in this register too:
 * tcl_function_call[0] = 0 // This means, that the function done.
 * tcl_function_call[1] is the return value.
 *
 * The address of this variable will be fetched by the TCL script by the `print` command.
 * See more at:
 * https://forums.xilinx.com/t5/Processor-System-Design-and-AXI/Synchronise-Zynq-with-TCL/m-p/1073951
 *
 */
u32 volatile tcl_function_call[16] = {TCL_FUNCTION_ID_INIT, 0, 0, 0, 0, 0, 0, 0};


/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XQspiPs QspiInstance;

/*****************************************************************************/
/**
*
* Main function which runs the command-parser state machine. It reads the
* `tcl_function_call` pointer to get the next command infinitely.
*
* @param	None
*
* @return	Never.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	u8* ReadBuffer;
	int ret;

	verb_printf(INFO, "Flash writer started Version: %d.\r\n", FLASH_WRITER_VERSION);

	tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
	while(1){
		switch (tcl_function_call[0]){
		case TCL_FUNCTION_ID_RETURN:
			// Wait for a real function
			break;
		case TCL_FUNCTION_ID_PRINT_HELLO:
			verb_printf(INFO, "Hello parameter: %d\r\n", tcl_function_call[1]);
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_FLASH_ERASE:
			verb_printf(INFO, "FLASH_ERASE: 0x%x 0x%x\r\n", tcl_function_call[1], tcl_function_call[2]);
			FlashErase2(&QspiInstance, tcl_function_call[1], tcl_function_call[2]);
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_FLASH_ERASE_ALL:
			verb_printf(INFO, "FLASH_ERASE_ALL: 0x%x 0x%x\r\n", tcl_function_call[1], tcl_function_call[2]);
			FlashEraseAll(&QspiInstance);
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_FLASH_WRITE:
			verb_printf(INFO, "TCL_FUNCTION_ID_FLASH_READ: 0x%x 0x%x 0x%x\r\n", tcl_function_call[1], tcl_function_call[2], tcl_function_call[3]);
			FlashWrite2(&QspiInstance, tcl_function_call[1], tcl_function_call[2], (u8*)tcl_function_call[3]);
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_FLASH_READ:
			verb_printf(INFO, "TCL_FUNCTION_ID_FLASH_READ: 0x%x 0x%x\r\n", tcl_function_call[1], tcl_function_call[2]);
			ReadBuffer = FlashRead2(&QspiInstance, tcl_function_call[1], tcl_function_call[2]);
			tcl_function_call[1] = (u32)ReadBuffer;
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_FLASH_INIT:
			verb_printf(INFO, "TCL_FUNCTION_ID_FLASH_INIT\r\n");
			ret = FlashInit(&QspiInstance, QSPI_DEVICE_ID);
			tcl_function_call[1] = (u32)ret;
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_FLASH_DUMP:
			verb_printf(INFO, "TCL_FUNCTION_ID_FLASH_DUMP: 0x%x 0x%x\r\n", tcl_function_call[1], tcl_function_call[2]);
			QspiFlashDump(&QspiInstance, tcl_function_call[1], tcl_function_call[2]);
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_GET_SLICE_BUFFER:
			verb_printf(INFO, "TCL_FUNCTION_ID_GET_SLICE_BUFFER\r\n");
			tcl_function_call[1] = (u32)get_slice_buffer();
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_GET_SLICE_BUFFER_SIZE:
			verb_printf(INFO, "TCL_FUNCTION_ID_GET_SLICE_BUFFER_SIZE\r\n");
			tcl_function_call[1] = (u32)get_slice_buffer_size();
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		case TCL_FUNCTION_ID_GET_FLASH_INFO:
			verb_printf(INFO, "TCL_FUNCTION_ID_GET_FLASH_INFO\r\n");
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
			verb_printf(INFO, "TCL_FUNCTION_ID_GET_VERSION\r\n");
			tcl_function_call[1] = FLASH_WRITER_VERSION;
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
			break;
		default:
			xil_printf("Unknown command: %d", tcl_function_call[0]);
			tcl_function_call[1] = 1;
			tcl_function_call[0] = TCL_FUNCTION_ID_RETURN;
		}
	}

}


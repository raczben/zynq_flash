/***************************** Include Files *********************************/

#include "xqspips.h"		/* QSPI device driver */
#include "xil_io.h"
#include "flash.h"
#include "sleep.h"

/************************** Variable Definitions *****************************/

FlashInfo Flash_Config_Table[34] = {
		/* Spansion */
		{0x10000, 0x100, 256, 0x10000, 0x1000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x1000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x20000, 0x100, 512, 0x10000, 0x1000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_128, 0xFFFE0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x2000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x2000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x20000, 0x200, 512, 0x20000, 0x2000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_256, 0xFFFE0000, 1},
		{0x40000, 0x100, 512, 0x20000, 0x4000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_512, 0xFFFC0000, 1},
		{0x40000, 0x200, 512, 0x40000, 0x4000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_512, 0xFFFC0000, 1},
		{0x80000, 0x100, 1024, 0x20000, 0x4000000,
				SPANSION_ID_BYTE0, SPANSION_ID_BYTE2_512, 0xFFF80000, 1},
		/* Spansion 1Gbit is handled as 512Mbit stacked */
		/* Micron */
		{0x10000, 0x100, 256, 0x10000, 0x1000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x1000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x20000, 0x100, 512, 0x10000, 0x1000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_128, 0xFFFE0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x2000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x2000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x20000, 0x200, 512, 0x20000, 0x2000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_256, 0xFFFE0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x4000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_512, 0xFFFF0000, 2},
		{0x10000, 0x800, 256, 0x80000, 0x4000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_512, 0xFFFF0000, 2},
		{0x20000, 0x400, 512, 0x40000, 0x4000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_512, 0xFFFE0000, 2},
		{0x10000, 0x800, 256, 0x80000, 0x8000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_1G, 0xFFFF0000, 4},
		{0x10000, 0x1000, 256, 0x100000, 0x8000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_1G, 0xFFFF0000, 4},
		{0x20000, 0x800, 512, 0x80000, 0x8000000,
				MICRON_ID_BYTE0, MICRON_ID_BYTE2_1G, 0xFFFE0000, 4},
		/* Winbond */
		{0x10000, 0x100, 256, 0x10000, 0x1000000,
				WINBOND_ID_BYTE0, WINBOND_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x10000, 0x200, 256, 0x20000, 0x1000000,
				WINBOND_ID_BYTE0, WINBOND_ID_BYTE2_128, 0xFFFF0000, 1},
		{0x20000, 0x100, 512, 0x10000, 0x1000000,
				WINBOND_ID_BYTE0, WINBOND_ID_BYTE2_128, 0xFFFE0000, 1},
		/* Macronix */
		{0x10000, 0x200, 256, 0x20000, 0x2000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x2000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_256, 0xFFFF0000, 1},
		{0x20000, 0x200, 512, 0x20000, 0x2000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_256, 0xFFFE0000, 1},
		{0x10000, 0x400, 256, 0x40000, 0x4000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_512, 0xFFFF0000, 1},
		{0x10000, 0x800, 256, 0x80000, 0x4000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_512, 0xFFFF0000, 1},
		{0x20000, 0x400, 512, 0x40000, 0x4000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_512, 0xFFFE0000, 1},
		{0x2000, 0x4000, 256, 0x80000, 0x8000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_1G, 0xFFFF0000, 1},
		{0x2000, 0x8000, 256, 0x100000, 0x8000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_1G, 0xFFFF0000, 1},
		{0x4000, 0x4000, 512, 0x80000, 0x8000000,
				MACRONIX_ID_BYTE0, MACRONIX_ID_BYTE2_1G, 0xFFFE0000, 1},
		/* ISSI */
		{0x10000, 0x200, 256, 0x20000, 0x2000000,
				ISSI_ID_BYTE0, ISSI_ID_BYTE2_256, 0xFFFF0000, 1}
};				/**< Flash Config Table */

u32 FlashMake;
u32 FCTIndex = -1;	/* Flash configuration table index */


/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 * The buffer size accounts for maximum page size and maximum banks -
 * for each bank separate read will be performed leading to that many
 * (overhead+dummy) bytes
 */
u8 ReadBuffer[(PAGE_COUNT * MAX_PAGE_SIZE) + (DATA_OFFSET + DUMMY_SIZE)*8];

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the Flash. Initialized to single flash page size.
 */
u32 MaxData = PAGE_COUNT*256;

FlashInfo* GetFlashInfo(){
	return &(Flash_Config_Table[FCTIndex]);
}

u8* get_slice_buffer(){
	return ReadBuffer;
}

u32 get_slice_buffer_size(){
	return (PAGE_COUNT * MAX_PAGE_SIZE);
}

int QspiFlashDump(XQspiPs *QspiInstancePtr, u32 start_addr, u32 len){
	if (!QspiInstancePtr->IsReady){
		// If the device has not been configured return error:
		return XST_FAILURE;
	}

	start_addr = start_addr & 0xfffffff0;
	len = len & 0x000007f0;
	u8* ReadBuffer;
	/*
	 * I/O Read - for any flash size
	 */
	ReadBuffer = FlashRead2(QspiInstancePtr, start_addr, len);
	MemDump(ReadBuffer, start_addr, len);
	return XST_SUCCESS;
}

u8* FlashRead2(XQspiPs *QspiInstancePtr, u32 start_addr, u32 len){
	if (!QspiInstancePtr->IsReady){
		// If the device has not been configured return error:
		return 0;
	}

	xil_printf("ReadBuffer: 0x%x\r\n", ReadBuffer);
	FlashRead(QspiInstancePtr, start_addr, len, QUAD_READ_CMD,
			ReadBuffer, ReadBuffer);
	return ReadBuffer;
}

void MemDump(void* ptr, u32 start_addr, u32 len){

	start_addr = start_addr & 0xfffffff0;
	len = len & 0x000007f0;

	for(int i=0; i<len; i+=16){
		xil_printf("%08x ", i+start_addr);
		for(int j=0; j<8; j++)
			xil_printf("%04x ", ((u16*)ptr)[(i>>1)+j]);
		xil_printf("\r\n");
	}
}


int FlashInit(XQspiPs *QspiInstancePtr, u16 QspiDeviceId) {
	int Status;
	XQspiPs_Config *QspiConfig;

	/*
	 * Initialize the QSPI driver so that it's ready to use
	 */
	QspiConfig = XQspiPs_LookupConfig(QspiDeviceId);
	if (NULL == QspiConfig) {
		return XST_FAILURE;
	}

	Status = XQspiPs_CfgInitialize(QspiInstancePtr, QspiConfig,
					QspiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build
	 */
	Status = XQspiPs_SelfTest(QspiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}



	/*
	 * Set the pre-scaler for QSPI clock
	 */
	XQspiPs_SetClkPrescaler(QspiInstancePtr, XQSPIPS_CLK_PRESCALE_8);

	/*
	 * Set Manual Start and Manual Chip select options and drive the
	 * HOLD_B high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr, XQSPIPS_FORCE_SSELECT_OPTION |
					     XQSPIPS_MANUAL_START_OPTION |
					     XQSPIPS_HOLD_B_DRIVE_OPTION);
	if(QspiConfig->ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED) {
		/*
		 * Enable two flash memories, Shared bus (NOT separate bus),
		 * L_PAGE selected by default
		 */
		XQspiPs_SetLqspiConfigReg(QspiInstancePtr, DUAL_STACK_CONFIG_WRITE);
	}

	if(QspiConfig->ConnectionMode == XQSPIPS_CONNECTION_MODE_PARALLEL) {
		/*
		 * Enable two flash memories on separate buses
		 */
		XQspiPs_SetLqspiConfigReg(QspiInstancePtr, DUAL_QSPI_CONFIG_WRITE);
	}

	/*
	 * Assert the Flash chip select.
	 */
	XQspiPs_SetSlaveSelect(QspiInstancePtr);

	/*
	 * Read flash ID and obtain all flash related information
	 * It is important to call the read id function before
	 * performing proceeding to any operation, including
	 * preparing the WriteBuffer
	 */
	Status = FlashReadID(QspiInstancePtr, ReadBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Initialize MaxData according to page size.
	 */
	MaxData = PAGE_COUNT * (Flash_Config_Table[FCTIndex].PageSize);

	return XST_SUCCESS;
}


void FlashWrite2(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 *WriteBfrPtr)
{
	u32 page_size = Flash_Config_Table[FCTIndex].PageSize;
	u32 page_cnt = ByteCount/page_size;
	u32 write_size = 0;
	/*
	 * Write the data in the write buffer to the serial Flash a page at a
	 * time, starting from TEST_ADDRESS
	 */

	verb_printf(DEBUG, "page_size 0x%x\r\n", page_size);
	verb_printf(DEBUG, "ByteCount 0x%x\r\n", ByteCount);
	verb_printf(DEBUG, "page_cnt 0x%x\r\n", page_cnt);

	for (int Page = 0; Page < page_cnt; Page++) {
		write_size = page_size < ByteCount ? page_size : ByteCount;
		FlashWrite(QspiPtr, (Page * page_size) + Address, write_size, WRITE_CMD, WriteBfrPtr + Page * page_size);
		ByteCount-=page_size;
		verb_printf(DEBUG, ".");
	}
	verb_printf(DEBUG, "\r\n");

}


/*****************************************************************************/
/**
*
*
* This function writes to the  serial Flash connected to the QSPI interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address to write data to in the Flash.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash. QSPI
*		device supports only Page Program command to write data to the
*		flash.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* Must send 2 bytes */
	u8 FlashStatus[2];
	u32 RealAddr;
	u32 BankSel;
	u8 ReadFlagSRCmd[] = {READ_FLAG_STATUS_CMD, 0};
	u8 FlagStatus[2];

	/*
	 * Translate address based on type of connection
	 * If stacked assert the slave select based on address
	 */
	RealAddr = GetRealAddr(QspiPtr, Address);
	/*
	 * Bank Select
	 */
	if(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		/*
		 * Calculate bank
		 */
		BankSel = RealAddr/SIXTEENMB;
		/*
		 * Select bank
		 */
		SendBankSelect(QspiPtr, BankSel);
	}

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				sizeof(WriteEnableCmd));


	/*
	 * Setup the write command with the specified address and data for the
	 * Flash
	 */
	/*
	 * This will ensure a 3B address is transferred even when address
	 * is greater than 128Mb.
	 */
	WriteBfrPtr[COMMAND_OFFSET]   = Command;
	WriteBfrPtr[ADDRESS_1_OFFSET] = (u8)((RealAddr & 0xFF0000) >> 16);
	WriteBfrPtr[ADDRESS_2_OFFSET] = (u8)((RealAddr & 0xFF00) >> 8);
	WriteBfrPtr[ADDRESS_3_OFFSET] = (u8)(RealAddr & 0xFF);

	/*
	 * Send the write command, address, and data to the Flash to be
	 * written, no receive buffer is specified since there is nothing to
	 * receive
	 */
	XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				ByteCount + OVERHEAD_SIZE);

	if((Flash_Config_Table[FCTIndex].NumDie > 1) &&
			(FlashMake == MICRON_ID_BYTE0)) {
		XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
					sizeof(ReadFlagSRCmd));
	}
	/*
	 * Wait for the write command to the Flash to be completed, it takes
	 * some time for the data to be written
	 */
	while (1) {
		/*
		 * Poll the status register of the Flash to determine when it
		 * completes, by sending a read status command and receiving the
		 * status byte
		 */
		XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus,
					sizeof(ReadStatusCmd));

		/*
		 * If the status indicates the write is done, then stop waiting,
		 * if a value of 0xFF in the status byte is read from the
		 * device and this loop never exits, the device slave select is
		 * possibly incorrect such that the device status is not being
		 * read
		 */
		if ((FlashStatus[1] & 0x01) == 0) {
			break;
		}
	}

		if((Flash_Config_Table[FCTIndex].NumDie > 1) &&
			(FlashMake == MICRON_ID_BYTE0)) {
		XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
					sizeof(ReadFlagSRCmd));
	}

}

int FlashEraseAll(XQspiPs *QspiPtr)
{
	if (!QspiPtr->IsReady){
		// If the device has not been configured return error:
		return XST_FAILURE;
	}

	u32 ByteCount = (Flash_Config_Table[FCTIndex]).NumSect *
				(Flash_Config_Table[FCTIndex]).SectSize;
	FlashErase(QspiPtr, 0, ByteCount, ReadBuffer);
	return XST_SUCCESS;
}


int FlashErase2(XQspiPs *QspiPtr, u32 Address, u32 ByteCount)
{
	if (!QspiPtr->IsReady){
		// If the device has not been configured return error:
		return XST_FAILURE;
	}
	FlashErase(QspiPtr, Address, ByteCount, ReadBuffer);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
*
* This function erases the sectors in the  serial Flash connected to the
* QSPI interface.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address of the first sector which needs to
*		be erased.
* @param	ByteCount contains the total size to be erased.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* Must send 2 bytes */
	u8 FlashStatus[2];
	int Sector;
	u32 RealAddr;
	u32 LqspiCr;
	u32 NumSect;
	u32 BankSel;
	u8 BankInitFlag = 1;
	u8 ReadFlagSRCmd[] = { READ_FLAG_STATUS_CMD, 0 };
	u8 FlagStatus[2];

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command or die erase command multiple times as required
	 */
	if (ByteCount == ((Flash_Config_Table[FCTIndex]).NumSect *
			(Flash_Config_Table[FCTIndex]).SectSize) ) {

		if(QspiPtr->Config.ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED){

			/*
			 * Get the current LQSPI configuration register value
			 */
			LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);
			/*
			 * Set selection to L_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));

			/*
			 * Assert the Flash chip select.
			 */
			XQspiPs_SetSlaveSelect(QspiPtr);
		}

		if(Flash_Config_Table[FCTIndex].NumDie == 1) {
			/*
			 * Call Bulk erase
			 */
			BulkErase(QspiPtr, WriteBfrPtr);
		}

		if(Flash_Config_Table[FCTIndex].NumDie > 1) {
			/*
			 * Call Die erase
			 */
			DieErase(QspiPtr, WriteBfrPtr);
		}
		/*
		 * If stacked mode, bulk erase second flash
		 */
		if(QspiPtr->Config.ConnectionMode == XQSPIPS_CONNECTION_MODE_STACKED){

			/*
			 * Get the current LQSPI configuration register value
			 */
			LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);
			/*
			 * Set selection to U_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr | XQSPIPS_LQSPI_CR_U_PAGE_MASK);

			/*
			 * Assert the Flash chip select.
			 */
			XQspiPs_SetSlaveSelect(QspiPtr);

			if(Flash_Config_Table[FCTIndex].NumDie == 1) {
				/*
				 * Call Bulk erase
				 */
				BulkErase(QspiPtr, WriteBfrPtr);
			}

			if(Flash_Config_Table[FCTIndex].NumDie > 1) {
				/*
				 * Call Die erase
				 */
				DieErase(QspiPtr, WriteBfrPtr);
			}
		}

		return;
	}

	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */

	/*
	 * Calculate no. of sectors to erase based on byte count
	 */
	NumSect = ByteCount/(Flash_Config_Table[FCTIndex].SectSize) + 1;

	/*
	 * If ByteCount to k sectors,
	 * but the address range spans from N to N+k+1 sectors, then
	 * increment no. of sectors to be erased
	 */

	if( ((Address + ByteCount) & Flash_Config_Table[FCTIndex].SectMask) ==
			((Address + (NumSect * Flash_Config_Table[FCTIndex].SectSize)) &
					Flash_Config_Table[FCTIndex].SectMask) ) {
		NumSect++;
	}

	for (Sector = 0; Sector < NumSect; Sector++) {

		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(QspiPtr, Address);

		/*
		 * Initial bank selection
		 */
		if((BankInitFlag) &&
				(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB)) {
			/*
			 * Reset initial bank select flag
			 */
			BankInitFlag = 0;
			/*
			 * Calculate initial bank
			 */
			BankSel = RealAddr/SIXTEENMB;
			/*
			 * Select bank
			 */
			SendBankSelect(QspiPtr, BankSel);
		}
		/*
		 * Check bank and send bank select if new bank
		 */
		if((BankSel != RealAddr/SIXTEENMB) &&
				(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB)) {
			/*
			 * Calculate initial bank
			 */
			BankSel = RealAddr/SIXTEENMB;
			/*
			 * Select bank
			 */
			SendBankSelect(QspiPtr, BankSel);
		}

		/*
		 * Send the write enable command to the SEEPOM so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the write
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/*
		 * Setup the write command with the specified address and data
		 * for the Flash
		 */
		/*
		 * This ensures 3B address is sent to flash even with address
		 * greater than 128Mb.
		 */
		WriteBfrPtr[COMMAND_OFFSET]   = SEC_ERASE_CMD;
		WriteBfrPtr[ADDRESS_1_OFFSET] = (u8)(RealAddr >> 16);
		WriteBfrPtr[ADDRESS_2_OFFSET] = (u8)(RealAddr >> 8);
		WriteBfrPtr[ADDRESS_3_OFFSET] = (u8)(RealAddr & 0xFF);

		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
					SEC_ERASE_SIZE);

		if((Flash_Config_Table[FCTIndex].NumDie > 1) &&
				(FlashMake == MICRON_ID_BYTE0)) {
			XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
						sizeof(ReadFlagSRCmd));
		}
		/*
		 * Wait for the sector erase command to the Flash to be completed
		 */
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
						FlashStatus,
						sizeof(ReadStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		if((Flash_Config_Table[FCTIndex].NumDie > 1) &&
				(FlashMake == MICRON_ID_BYTE0)) {
			XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
						sizeof(ReadFlagSRCmd));
		}

		Address += Flash_Config_Table[FCTIndex].SectSize;

	}
}

/*****************************************************************************/
/**
*
* This function reads serial Flash ID connected to the SPI interface.
* It then deduces the make and size of the flash and obtains the
* connection mode to point to corresponding parameters in the flash
* configuration table. The flash driver will function based on this and
* it presently supports Micron and Spansion - 128, 256 and 512Mbit and
* Winbond 128Mbit
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Pointer to the write buffer (which is to be transmitted)
* @param	Pointer to the read buffer to which valid received data should be
* 			written
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashReadID(XQspiPs *QspiPtr, u8 *buffer_ptr)
{
	int Status;
	int StartIndex;

	/*
	 * Read ID in Auto mode.
	 */
	buffer_ptr[COMMAND_OFFSET]   = READ_ID;
	buffer_ptr[ADDRESS_1_OFFSET] = 0x23;		/* 3 dummy bytes */
	buffer_ptr[ADDRESS_2_OFFSET] = 0x08;
	buffer_ptr[ADDRESS_3_OFFSET] = 0x09;

	Status = XQspiPs_PolledTransfer(QspiPtr, buffer_ptr, buffer_ptr,
				RD_ID_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Deduce flash make
	 */
	if(buffer_ptr[1] == MICRON_ID_BYTE0) {
		FlashMake = MICRON_ID_BYTE0;
		StartIndex = MICRON_INDEX_START;
	}else if(buffer_ptr[1] == SPANSION_ID_BYTE0) {
		FlashMake = SPANSION_ID_BYTE0;
		StartIndex = SPANSION_INDEX_START;
	}else if(buffer_ptr[1] == WINBOND_ID_BYTE0) {
		FlashMake = WINBOND_ID_BYTE0;
		StartIndex = WINBOND_INDEX_START;
	} else if(buffer_ptr[1] == MACRONIX_ID_BYTE0) {
		FlashMake = MACRONIX_ID_BYTE0;
		StartIndex = MACRONIX_INDEX_START;
	} else if(buffer_ptr[0] == ISSI_ID_BYTE0) {
		FlashMake = ISSI_ID_BYTE0;
		StartIndex = ISSI_INDEX_START;
	} else {
		return XST_FAILURE;
	}


	/*
	 * If valid flash ID, then check connection mode & size and
	 * assign corresponding index in the Flash configuration table
	 */
	if(((FlashMake == MICRON_ID_BYTE0) || (FlashMake == SPANSION_ID_BYTE0)||
			(FlashMake == WINBOND_ID_BYTE0)) &&
			(buffer_ptr[3] == MICRON_ID_BYTE2_128)) {

		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_128_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_128_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_128_SP + StartIndex;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}
	/* 256 and 512Mbit supported only for Micron and Spansion, not Winbond */
	if(((FlashMake == MICRON_ID_BYTE0) || (FlashMake == SPANSION_ID_BYTE0)
			|| (FlashMake == MACRONIX_ID_BYTE0)) &&
			(buffer_ptr[3] == MICRON_ID_BYTE2_256)) {

		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_256_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_256_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_256_SP + StartIndex;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}
	if((FlashMake == ISSI_ID_BYTE0) &&
			(buffer_ptr[2] == MICRON_ID_BYTE2_256)) {
		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_256_ISSI;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_256_ISSI;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_256_ISSI;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}
	if ((((FlashMake == MICRON_ID_BYTE0) || (FlashMake == SPANSION_ID_BYTE0)) &&
			(buffer_ptr[3] == MICRON_ID_BYTE2_512)) || ((FlashMake ==
			MACRONIX_ID_BYTE0) && (buffer_ptr[3] == MACRONIX_ID_BYTE2_512))) {

		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_512_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_512_SP + StartIndex;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_512_SP + StartIndex;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}
	/*
	 * 1Gbit Single connection supported for Spansion.
	 * The ConnectionMode will indicate stacked as this part has 2 SS
	 * The device ID will indicate 512Mbit.
	 * This configuration is handled as the above 512Mbit stacked configuration
	 */
	/* 1Gbit single, parallel and stacked supported for Micron */
	if(((FlashMake == MICRON_ID_BYTE0) &&
		(buffer_ptr[3] == MICRON_ID_BYTE2_1G)) ||
		((FlashMake == MACRONIX_ID_BYTE0) &&
		 (buffer_ptr[3] == MACRONIX_ID_BYTE2_1G))) {

		switch(QspiPtr->Config.ConnectionMode)
		{
			case XQSPIPS_CONNECTION_MODE_SINGLE:
				FCTIndex = FLASH_CFG_TBL_SINGLE_1GB_MC;
				break;
			case XQSPIPS_CONNECTION_MODE_PARALLEL:
				FCTIndex = FLASH_CFG_TBL_PARALLEL_1GB_MC;
				break;
			case XQSPIPS_CONNECTION_MODE_STACKED:
				FCTIndex = FLASH_CFG_TBL_STACKED_1GB_MC;
				break;
			default:
				FCTIndex = 0;
				break;
		}
	}

	xil_printf("FlashID=0x%x 0x%x 0x%x\n\r", buffer_ptr[1], buffer_ptr[2],
		   buffer_ptr[3]);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function performs an I/O read.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address of the first sector which needs to
*			be erased.
* @param	ByteCount contains the total size to be erased.
* @param	Command is the command used to read data from the flash. Supports
* 			normal, fast, dual and quad read commands.
* @param	Pointer to the write buffer which contains data to be transmitted
* @param	Pointer to the read buffer to which valid received data should be
* 			written
*
* @return	none.
*
* @note		None.
*
******************************************************************************/
void FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	u32 RealAddr;
	u32 RealByteCnt;
	u32 BankSel;
	u32 BufferIndex;
	u32 TotalByteCnt;
	u8 ShiftSize;

	/*
	 * Retain the actual byte count
	 */
	TotalByteCnt = ByteCount;

	while(((signed long)(ByteCount)) > 0) {

		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(QspiPtr, Address);

		/*
		 * Select bank
		 */
		if(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			BankSel = RealAddr/SIXTEENMB;
			SendBankSelect(QspiPtr, BankSel);
		}

		/*
		 * If data to be read spans beyond the current bank, then
		 * calculate RealByteCnt in current bank. Else
		 * RealByteCnt is the same as ByteCount
		 */
		if((Address & BANKMASK) != ((Address+ByteCount) & BANKMASK)) {
			RealByteCnt = (Address & BANKMASK) + SIXTEENMB - Address;
		}else {
			RealByteCnt = ByteCount;
		}


		/*
		 * Setup the write command with the specified address and data for the
		 * Flash
		 */
		WriteBfrPtr[COMMAND_OFFSET]   = Command;
		WriteBfrPtr[ADDRESS_1_OFFSET] = (u8)((RealAddr & 0xFF0000) >> 16);
		WriteBfrPtr[ADDRESS_2_OFFSET] = (u8)((RealAddr & 0xFF00) >> 8);
		WriteBfrPtr[ADDRESS_3_OFFSET] = (u8)(RealAddr & 0xFF);

		if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
		    (Command == QUAD_READ_CMD)) {
			RealByteCnt += DUMMY_SIZE;
		}
		/*
		 * Send the read command to the Flash to read the specified number
		 * of bytes from the Flash, send the read command and address and
		 * receive the specified number of bytes of data in the data buffer
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr,
				&(ReadBfrPtr[TotalByteCnt - ByteCount]),
				RealByteCnt + OVERHEAD_SIZE);

		/*
		 * To discard the first 5 dummy bytes, shift the data in read buffer
		 */
		if((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
			    (Command == QUAD_READ_CMD)){
			ShiftSize = OVERHEAD_SIZE + DUMMY_SIZE;
		}else{
			ShiftSize =  OVERHEAD_SIZE;
		}

		for(BufferIndex = (TotalByteCnt - ByteCount);
				BufferIndex < (TotalByteCnt - ByteCount) + RealByteCnt;
				BufferIndex++) {
			ReadBfrPtr[BufferIndex] = ReadBfrPtr[BufferIndex + ShiftSize];
		}

		/*
		 * Increase address to next bank
		 */
		Address = (Address & BANKMASK) + SIXTEENMB;
		/*
		 * Decrease byte count by bytes already read.
		 */
		if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
		    (Command == QUAD_READ_CMD)) {
			ByteCount = ByteCount - (RealByteCnt - DUMMY_SIZE);
		}else {
			ByteCount = ByteCount - RealByteCnt;
		}

	}

}

/*****************************************************************************/
/**
*
* This functions selects the current bank
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Pointer to the write buffer which contains data to be transmitted
* @param	BankSel is the bank to be selected in the flash device(s).
*
* @return	XST_SUCCESS if bank selected, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int SendBankSelect(XQspiPs *QspiPtr, u32 BankSel)
{
	u8 WriteBfrPtr[4] = { WRITE_ENABLE_CMD, 0, 0, 0 };

	/*
	 * Bank select commands for Micron and Spansion are different
	 */
	if(FlashMake == MICRON_ID_BYTE0) {
		/*
		 * For Micron command WREN should be sent first
		 * except for some specific feature set
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
					sizeof(u8));

		WriteBfrPtr[COMMAND_OFFSET]   = EXTADD_REG_WR;
		WriteBfrPtr[ADDRESS_1_OFFSET] = BankSel;

		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				BANK_SEL_SIZE);

	}
	if(FlashMake == SPANSION_ID_BYTE0) {
		WriteBfrPtr[COMMAND_OFFSET]   = BANK_REG_WR;
		WriteBfrPtr[ADDRESS_1_OFFSET] = BankSel;

		/*
		 * Send the Extended address register write command
		 * written, no receive buffer required
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				BANK_SEL_SIZE);
	}

	/* Winbond can be added here */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This functions performs a bulk erase operation when the
* flash device has a single die. Works for both Spansion and Micron
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	WritBfrPtr is the pointer to command+address to be sent
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void BulkErase(XQspiPs *QspiPtr, u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* Must send 2 bytes */
	u8 FlashStatus[2];

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer
	 * before the erase
	 */
	XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
			  sizeof(WriteEnableCmd));

	/*
	 * Setup the bulk erase command
	 */
	WriteBfrPtr[COMMAND_OFFSET]   = BULK_ERASE_CMD;

	/*
	 * Send the bulk erase command; no receive buffer is specified
	 * since there is nothing to receive
	 */
	XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				BULK_ERASE_SIZE);

	/*
	 * Wait for the erase command to the Flash to be completed
	 */
	while (1) {
		/*
		 * Poll the status register of the device to determine
		 * when it completes, by sending a read status command
		 * and receiving the status byte
		 */
		XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
					FlashStatus,
					sizeof(ReadStatusCmd));

		/*
		 * If the status indicates the write is done, then stop
		 * waiting; if a value of 0xFF in the status byte is
		 * read from the device and this loop never exits, the
		 * device slave select is possibly incorrect such that
		 * the device status is not being read
		 */
		if ((FlashStatus[1] & 0x01) == 0) {
			break;
		}
	}
}

/*****************************************************************************/
/**
*
* This functions performs a die erase operation on all the die in
* the flash device. This function uses the die erase command for
* Micron 512Mbit and 1Gbit
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	WritBfrPtr is the pointer to command+address to be sent
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void DieErase(XQspiPs *QspiPtr, u8 *WriteBfrPtr)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 DieCnt;
	u8 ReadFlagSRCmd[] = { READ_FLAG_STATUS_CMD, 0 };
	u8 FlagStatus[2];

	for(DieCnt = 0; DieCnt < Flash_Config_Table[FCTIndex].NumDie; DieCnt++) {
		/*
		 * Select bank - the lower of the 2 banks in each die
		 * This is specific to Micron flash
		 */
		SendBankSelect(QspiPtr, DieCnt*2);

		/*
		 * Send the write enable command to the SEEPOM so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the write
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/*
		 * Setup the write command with the specified address and data
		 * for the Flash
		 */
		/*
		 * This ensures 3B address is sent to flash even with address
		 * greater than 128Mb.
		 * The address is the start address of die - MSB bits will be
		 * derived from bank select by the flash
		 */
		WriteBfrPtr[COMMAND_OFFSET]   = DIE_ERASE_CMD;
		WriteBfrPtr[ADDRESS_1_OFFSET] = 0x00;
		WriteBfrPtr[ADDRESS_2_OFFSET] = 0x00;
		WriteBfrPtr[ADDRESS_3_OFFSET] = 0x00;

		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBfrPtr, NULL,
				DIE_ERASE_SIZE);

		/*
		 * Wait for the sector erase command to the Flash to be completed
		 */
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadFlagSRCmd, FlagStatus,
					sizeof(ReadFlagSRCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((FlagStatus[1] & 0x80) == 0x80) {
				break;
			}
		}

	}
}

/*****************************************************************************/
/**
*
* This functions translates the address based on the type of interconnection.
* In case of stacked, this function asserts the corresponding slave select.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address which is to be accessed (for erase, write or read)
*
* @return	RealAddr is the translated address - for single it is unchanged;
* 			for stacked, the lower flash size is subtracted;
* 			for parallel the address is divided by 2.
*
* @note		None.
*
******************************************************************************/
u32 GetRealAddr(XQspiPs *QspiPtr, u32 Address)
{
	u32 LqspiCr;
	u32 RealAddr;

	switch(QspiPtr->Config.ConnectionMode) {
	case XQSPIPS_CONNECTION_MODE_SINGLE:
		RealAddr = Address;
		break;
	case XQSPIPS_CONNECTION_MODE_STACKED:
		/*
		 * Get the current LQSPI Config reg value
		 */
		LqspiCr = XQspiPs_GetLqspiConfigReg(QspiPtr);

		/* Select lower or upper Flash based on sector address */
		if(Address & Flash_Config_Table[FCTIndex].FlashDeviceSize) {
			/*
			 * Set selection to U_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr | XQSPIPS_LQSPI_CR_U_PAGE_MASK);

			/*
			 * Subtract first flash size when accessing second flash
			 */
			RealAddr = Address &
					(~Flash_Config_Table[FCTIndex].FlashDeviceSize);

		}else{

			/*
			 * Set selection to L_PAGE
			 */
			XQspiPs_SetLqspiConfigReg(QspiPtr,
					LqspiCr & (~XQSPIPS_LQSPI_CR_U_PAGE_MASK));

			RealAddr = Address;

		}

		/*
		 * Assert the Flash chip select.
		 */
		XQspiPs_SetSlaveSelect(QspiPtr);
		break;
	case XQSPIPS_CONNECTION_MODE_PARALLEL:
		/*
		 * The effective address in each flash is the actual
		 * address / 2
		 */
		RealAddr = Address / 2;
		break;
	default:
		/* RealAddr wont be assigned in this case; */
	break;

	}

	return(RealAddr);

}

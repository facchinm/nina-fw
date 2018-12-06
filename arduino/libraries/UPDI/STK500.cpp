#include "STK500.h"

// *** Writeable Parameter Values ***
uint8_t STK500::PARAM_VTARGET_VAL;
uint8_t STK500::PARAM_VADJUST_VAL;
uint8_t STK500::PARAM_OSC_PSCALE_VAL;
uint8_t STK500::PARAM_OSC_CMATCH_VAL;
uint8_t STK500::PARAM_SCK_DURATION_VAL;
uint8_t STK500::PARAM_RESET_POLARITY_VAL;
uint8_t STK500::PARAM_CONTROLLER_INIT_VAL;

// *** STK500 packet ***
uint8_t STK500::prologue[3];
uint8_t & STK500::number = reinterpret_cast<uint8_t &> (STK500::prologue[0]);
uint8_t & STK500::size_hi = reinterpret_cast<uint8_t &> (STK500::prologue[1]);
uint8_t & STK500::size_lo = reinterpret_cast<uint8_t &> (STK500::prologue[2]);
uint8_t STK500::body [275];

// *** Address ***
uint16_t STK500::address;
uint8_t (& STK500::address_byte)[2] = (uint8_t (&)[2]) STK500::address;

#include "UPDI_lo_lvl.h"

// *** Packet functions *** 
	bool STK500::receive() {
		while (STK_io::get() != MESSAGE_START);
		uint8_t checksum = MESSAGE_START;
		checksum ^= (number = STK_io::get());
		checksum ^= (size_hi = STK_io::get());
		checksum ^= (size_lo = STK_io::get());
		if (STK_io::get() != TOKEN) return false;
		checksum ^= TOKEN;
		for (int16_t i = 0; i < (size_lo | (size_hi << 8)); i++) {
			checksum ^= (body[i] = STK_io::get());
		}
		if (STK_io::get() != checksum) body[0] = ANSWER_CKSUM_ERROR;
		return true;
	}

	void STK500::answer() {
		uint8_t checksum = STK_io::put(MESSAGE_START);
		checksum ^= STK_io::put(number);
		checksum ^= STK_io::put(size_hi);
		checksum ^= STK_io::put(size_lo);
		checksum ^= STK_io::put(TOKEN);
		for (int16_t i = 0; i < (size_lo | (size_hi << 8)); i++) {
			checksum ^= STK_io::put(body[i]);
		}
		STK_io::put(checksum);
	}

// *** Set status function ***
	void STK500::set_status(uint8_t status_code){
		size_hi = 0;
		size_lo = 2;
		body[1] = status_code;
	}

// *** General command functions ***
	// *** Signature response message ***
	FLASH(uint8_t) STK500::sgn_resp[10] {STATUS_CMD_OK, 8, 'S', 'T', 'K', '5', '0', '0', '_', '2'};
	// *** Locked MCU response message ***
	FLASH(uint8_t) STK500::lock_resp[13] {STATUS_CMD_OK, 11, 'C', 'H', 'I', 'P', '_', 'L', 'O', 'C', 'K', 'E', 'D'};
	void STK500::sign_on(){
		/* Initialize STK500v2 variables */
		PARAM_VTARGET_VAL = 55;
		PARAM_VADJUST_VAL = 32;

		using namespace UPDI;
		/* Initialize or enable UPDI */
		UPDI_io::put(UPDI_io::double_break);
		stcs(reg::Control_A, 6);
	
		/* Perform chip erase if D5 is low */
		if ((PIND & (1 << PIND5)) == 0)
			chip_erase();
	
		uint8_t status = lcds(reg::ASI_System_Status);
		if (status & 0x01) {
			size_hi = 0;
			size_lo = sizeof(lock_resp) + 1;
			for (uint8_t i = 0; i < sizeof(lock_resp); i++) {
				body[1+i] = lock_resp[i];
			}
		}
		else {
			size_hi = 0;
			size_lo = sizeof(sgn_resp) + 1;
			for (uint8_t i = 0; i < sizeof(sgn_resp); i++) {
				body[1+i] = sgn_resp[i];
			}
		}
	}

	void STK500::get_parameter(){
		uint8_t & parameter = body[1];
		uint8_t & value = body[2];
		uint8_t & status = body[1];
		size_lo = 3;
		switch (parameter) {
			case PARAM_BUILD_NUMBER_LOW:
				value = PARAM_BUILD_NUMBER_LOW_VAL;
				break;
			case PARAM_BUILD_NUMBER_HIGH:
				value = PARAM_BUILD_NUMBER_HIGH_VAL;
				break;
			case PARAM_HW_VER:
				value = PARAM_HW_VER_VAL;
				break;
			case PARAM_SW_MAJOR:
				value = PARAM_SW_MAJOR_VAL;
				break;
			case PARAM_SW_MINOR:
				value = PARAM_SW_MINOR_VAL;
				break;
			case PARAM_VTARGET:
				value = PARAM_VTARGET_VAL;
				break;
			case PARAM_VADJUST:
				value = PARAM_VADJUST_VAL;
				break;
			case PARAM_OSC_PSCALE:
				value = PARAM_OSC_PSCALE_VAL;
				break;
			case PARAM_OSC_CMATCH:
				value = PARAM_OSC_CMATCH_VAL;
				break;
			case PARAM_SCK_DURATION:
				value = PARAM_SCK_DURATION_VAL;
				break;
			case PARAM_TOPCARD_DETECT:
				value = PARAM_TOPCARD_DETECT_VAL;
				break;			
			case PARAM_STATUS:
				value = PARAM_STATUS_VAL;
				break;			
			case PARAM_DATA:
				value = PARAM_DATA_VAL;
				break;				
			case PARAM_RESET_POLARITY:
				value = PARAM_RESET_POLARITY_VAL;
				break;
			case PARAM_CONTROLLER_INIT:
				value = PARAM_CONTROLLER_INIT_VAL;
				break;
			default:
				size_lo = 2;
				status = STATUS_CMD_FAILED;
			return;
		}
		status = STATUS_CMD_OK;
		return;		
	}

	void STK500::set_parameter(){
		uint8_t & parameter = body[1];
		uint8_t & value = body[2];
		uint8_t & status = body[1];
		size_lo = 2;
		switch (parameter) {
			case PARAM_VTARGET:
				PARAM_VTARGET_VAL = value;
				break;
			case PARAM_VADJUST:
				PARAM_VADJUST_VAL = value;
				break;
			case PARAM_OSC_PSCALE:
				PARAM_OSC_PSCALE_VAL = value;
				break;
			case PARAM_OSC_CMATCH:
				PARAM_OSC_CMATCH_VAL = value;
				break;
			case PARAM_SCK_DURATION:
				PARAM_SCK_DURATION_VAL = value;
				break;
			case PARAM_RESET_POLARITY:
				PARAM_RESET_POLARITY_VAL = value;
				break;
			case PARAM_CONTROLLER_INIT:
				PARAM_CONTROLLER_INIT_VAL = value;
				break;
			default:
				status = STATUS_CMD_FAILED;
				return;
		}
		status = STATUS_CMD_OK;
		return;
	}
	
	void STK500::load_address(){
		address_byte[0] = body[4];
		address_byte[1] = body[3];
		set_status(STATUS_CMD_OK);
	}
	
// *** ISP command functions ***
	void STK500::enter_progmode(){
		switch (UPDI::lcds(UPDI::reg::ASI_System_Status)){
			// in normal operation mode
			case 0x82:
				// Write NVN unlock key (allows read access to all addressing space)
				UPDI::write_key(UPDI::NVM_Prog);
				// Request reset
				UPDI::stcs(UPDI::reg::ASI_Reset_Request, UPDI::RESET_ON);
				// Release reset (System remains in reset state until released)
				UPDI::stcs(UPDI::reg::ASI_Reset_Request, UPDI::RESET_OFF);
				// Wait for NVM unlock state
				while (UPDI::lcds(UPDI::reg::ASI_System_Status) != 0x08);
			// already in program mode
			case 0x08:
				// better clear the page buffer, just in case.
				UPDI::sts_b(NVM::NVM_base + NVM::CTRLA, NVM::PBC);
				set_status(STATUS_CMD_OK);
				// Turn on LED to indicate program mode
				PORTB |= 1 << 5;
				return;
			// in other modes just fail
			default:
				set_status(STATUS_CMD_FAILED);
				return;
		}
	}

	void STK500::leave_progmode(){
		switch (UPDI::lcds(UPDI::reg::ASI_System_Status)){
			// in program mode
			case 0x08:
				// Request reset
				UPDI::stcs(UPDI::reg::ASI_Reset_Request, UPDI::RESET_ON);
				// Release reset (System remains in reset state until released)
				UPDI::stcs(UPDI::reg::ASI_Reset_Request, UPDI::RESET_OFF);
				// Wait for normal mode
				while (UPDI::lcds(UPDI::reg::ASI_System_Status) != 0x82);
			// already in normal mode
			case 0x82:
				set_status(STATUS_CMD_OK);
				// Turn off LED to indicate normal mode
				PORTB &= ~(1 << 5);
				return;
			// in other modes just fail
			default:
				set_status(STATUS_CMD_FAILED);
				return;
		}
	}
	
	void STK500::read_mem(uint16_t mem_base, uint8_t is_word_address) {
		if (UPDI::lcds(UPDI::reg::ASI_System_Status) != 0x08){
			// fail if not in program mode
			set_status(STATUS_CMD_FAILED);
		}
		else {
			// in program mode
			const uint16_t NumBytes = (body[1] << 8) | body[2];
			uint8_t & status1 = body[1];
			uint8_t & status2 = body[NumBytes + 2];
		
			// Calculate physical address for reading
			uint16_t UPDI_address = mem_base;
			UPDI_address += is_word_address ? (address << 1) : address;
			// Update STK500 address
			address += is_word_address ? (NumBytes >> 1) : NumBytes;
		
			UPDI::stptr_w(UPDI_address);
			UPDI::rep(NumBytes - 1);
			body[2] = UPDI::ldinc_b();
			for (uint16_t i = 3; i < (NumBytes + 2); i++) {
				body[i] = UPDI_io::get();
			}
	
			status1 = status2 = STATUS_CMD_OK;
			size_hi = (NumBytes + 3) >> 8;
			size_lo = (NumBytes + 3) & 0xFF;
		}
	}
	
	void STK500::program_nvm(uint16_t mem_base, uint8_t buff_size, uint8_t is_word_address){
		if (UPDI::lcds(UPDI::reg::ASI_System_Status) == 0x08){
			// in program mode
			uint8_t NumBytes = body[2];		/* The STK500 will not request writes of >128 bytes because of chip definitions */

			uint16_t prg_address = mem_base;
			prg_address += is_word_address ? (address << 1) : address;
			// Update address
			address += is_word_address ? (NumBytes >> 1) : NumBytes;

			// Setup UPDI pointer for block transfer
			UPDI::stptr_w(prg_address);

			uint8_t current_byte = 10;		/* Index of the first byte to send inside the STK500 command body */
			
			auto updi_send_block = [] (uint8_t count, uint8_t & index) {
				uint16_t temp = UPDI::ldptr_w();
				/* Wait while NVM is busy from previous operations */
				while (UPDI::lds_b(NVM::NVM_base + NVM::STATUS) & 0x03);				
				UPDI::stptr_w(temp);
				UPDI::rep(count - 1);
				UPDI::stinc_b(body[index++]);
				for (uint8_t i = 0; i < (count - 1); i++) {
					UPDI_io::put(body[index++]);
					UPDI_io::get();
				}			
			};
			
			auto burn_buffer = [] () {
				uint16_t temp = UPDI::ldptr_w();
				/* Execute NVM erase/write */
				UPDI::sts_b(NVM::NVM_base + NVM::CTRLA, NVM::ERWP);
				UPDI::stptr_w(temp);		
			};

			/* Check address alignment, calculate number of unaligned bytes to send */
			uint8_t unaligned_bytes = (-prg_address & (buff_size - 1));
			if (unaligned_bytes > NumBytes) unaligned_bytes = NumBytes;
			/* If there are unaligned bytes, they must be sent first */
			if (unaligned_bytes) {
				// Send unaligned block
				updi_send_block(unaligned_bytes, current_byte);
				NumBytes -= unaligned_bytes;
				burn_buffer();
			}
			while (NumBytes) {
				/* Send a buff_size amount of bytes */
				if (NumBytes >= buff_size) {
					updi_send_block(buff_size, current_byte);
					NumBytes -= buff_size;
				}
				/* Send a NumBytes amount of bytes */
				else {
					updi_send_block(NumBytes, current_byte);				
					NumBytes = 0;
				}
				burn_buffer();
			}

			set_status(STATUS_CMD_OK);
		}
		else {
			// in other modes just fail
			set_status(STATUS_CMD_FAILED);
		}		
	}
	
	void STK500::program_fuse(uint16_t mem_base){
		uint16_t fuse_address = (mem_base + address);
		uint8_t & status1 = body[1];
		uint8_t & status2 = body[2];
		uint8_t & data = body[4];
		// Setup UPDI pointer
		UPDI::stptr_w(NVM::NVM_base + NVM::DATA_lo);
		// Send data to the NVM controller
		UPDI::stinc_b(data);
		UPDI::stinc_b(0x00);
		// Send address to the NVM controller
		UPDI::stinc_b(fuse_address & 0xFF);
		UPDI::stinc_b(fuse_address >> 8);
		// Execute fuse write
		UPDI::sts_b(NVM::NVM_base + NVM::CTRLA, NVM::WFU);
		size_hi = 0;
		size_lo = 3;		
		status1 = status2 = STATUS_CMD_OK;
	}
	
	void STK500::chip_erase(){
		// Write Chip Erase key 
		UPDI::write_key(UPDI::Chip_Erase);
		// Request reset
		UPDI::stcs(UPDI::reg::ASI_Reset_Request, UPDI::RESET_ON);
		// Release reset (System remains in reset state until released)
		UPDI::stcs(UPDI::reg::ASI_Reset_Request, UPDI::RESET_OFF);
		// Wait for NVM unlock state
		while (UPDI::lcds(UPDI::reg::ASI_System_Status) & 0x01);
		set_status(STATUS_CMD_OK);
		return;
	}
	
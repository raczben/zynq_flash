
####################################################################################################
# Function IDs
#
# Global constants for communicating with the flash writer executables. Each *function id* can
# "call" a C function.
#
# Do not change these values. These must be synchronous with the flash_writer executable.
#
####################################################################################################

variable TCL_FUNCTION_ID_RETURN                 0x0
variable TCL_FUNCTION_ID_INIT                   0x1
variable TCL_FUNCTION_ID_PRINT_HELLO            0x10
variable TCL_FUNCTION_ID_GET_VERSION            0x20
variable TCL_FUNCTION_ID_FLASH_ERASE            0x100
variable TCL_FUNCTION_ID_FLASH_ERASE_ALL        0x102
variable TCL_FUNCTION_ID_GET_FLASH_INFO         0x101
variable TCL_FUNCTION_ID_FLASH_INIT             0x105
variable TCL_FUNCTION_ID_FLASH_WRITE            0x110
variable TCL_FUNCTION_ID_FLASH_READ             0x120
variable TCL_FUNCTION_ID_FLASH_DUMP             0x125
variable TCL_FUNCTION_ID_GET_SLICE_BUFFER       0x130
variable TCL_FUNCTION_ID_GET_SLICE_BUFFER_SIZE  0x140

# init_flash_writer saves here the path of flash_writer executable.
if { ! [info exists flash_writer_elf] } {
    set flash_writer_elf undefined
}
if { ! [info exists boot_bin_file] } {
    set boot_bin_file undefined
}

####################################################################################################
#
# Global variables for communicating with the flash writer executables. init_flash_writer will set
# these values.
#
# Writing a function ID to `tcl_function_call_variable` will call the given function. The parameters
# can be passed via the `tcl_parameter_variable`. For more details see `_c_do` procedure.
#
# The `bin_slice_buffer` is the buffer to pass a chunk of the boot image to Zynq.
# `bin_slice_buffer_size` stores the size of this buffer.
#
####################################################################################################
set tcl_function_call_variable -1
set tcl_parameter_variable -1
set bin_slice_buffer -1
set bin_slice_buffer_size -1


####################################################################################################
#
# flash_image do a full flashing cycle. It initializes the Zynq, erase and write flash.
#
# @param    boot_bin_file the name of the file to be written.
# @param    do_erase Erase flash before writing the image. (Recommended)
# @param    do_blank_check Check the success of the erase.
# @param    do_verify Check the write of the image.
#
# @return   0 on success else 1
#
# @note     None.
#
####################################################################################################
proc flash_image { bin_file { do_erase 1 } { do_blank_check 0 } { do_verify 1 } } {

	if { ! [file exists $bin_file] } {
		puts "ERROR, $bin_file file does not exist"
		return 1
	}
	
	puts "Loading flash writer elf..."
	init_flash_writer
	
	puts "Initializing flash..."
	if { [flash_init] } {
        puts "ERROR: flash_init has failed."
        return 1
    }
	
    if { $do_erase } {
        set image_size [file size $bin_file]
        puts "Erasing flash..."
        flash_erase 0 $image_size
    }
    
    if { $do_blank_check } {
        puts "ERROR: do_blank_check has not been implemented."
    }
	
	puts "Writing flash..."
	if { [flash_write_file $bin_file] } {
        puts "ERROR: flash_write_file has failed."
        return 1
    }
    
    if { $do_verify } {
        if { [flash_verify $bin_file] } {
            puts "ERROR: flash_verify has failed. Note, that this feature has not been tested."
        }
    }
	
	puts "Done!"
    
    return 0
}


####################################################################################################
#
# init_flash_writer Initialize the flash writer to be prepare any flash operation.
#   1. Initialize Zynq's PS (processing system) (run ps7_init) 
#   2. Donwloads the flash_writer executable to Zynq's ARM.
#   3. Set variables to communicate with the flash_writer executable.
#
# @param    flash_writer_elf The path of the flash_writer_elf executable. init_flash_writer saves
#           the path, so it don't need to specify at the second run.
#
# @return   None.
#
# @note     None.
#
####################################################################################################
proc init_flash_writer { {flash_writer_elf default} } {

    #
    # Getting/saving the flash_writer_elf
    #
    if { $flash_writer_elf == "default" } {
        set flash_writer_elf $::flash_writer_elf
    }
    if { $flash_writer_elf == "undefined" } {
        puts "ERROR: Flash writer executable must be given at the first time."
        return
    }
    if { ! [file exists $flash_writer_elf ] } {
        puts "ERROR: Flash writer executable $flash_writer_elf does not exists."
        return
    }
    set ::flash_writer_elf  [file normalize $flash_writer_elf]
    
    #
    # Check and stop the processor, then run ps7_init, finally run the flash_writer executable.
    #
    if { ! [string match Stopped* [state]] } {
        puts "stop"
        stop
    }
    puts "ps7_init"
    ps7_init
    puts "ps7_post_config"
    ps7_post_config
    puts "rst -processor"
    rst -processor
    dow $::flash_writer_elf
    con
    
    #
    # Setting global variables.
    #
    # See more at:
    # https://forums.xilinx.com/t5/Processor-System-Design-and-AXI/Synchronise-Zynq-with-TCL/m-p/1073951 
    #
    puts "Getting communication variable (tcl_function_call)"
    set global_dict [print -dict &tcl_function_call]
    set ::tcl_function_call_variable [dict get $global_dict &tcl_function_call]
    set ::tcl_parameter_variable [expr $::tcl_function_call_variable + 4]
    
    puts "Setting slice buffer (bin_slice_buffer)"
    set ::bin_slice_buffer      [_c_do $::TCL_FUNCTION_ID_GET_SLICE_BUFFER]
    set ::bin_slice_buffer_size [_c_do $::TCL_FUNCTION_ID_GET_SLICE_BUFFER_SIZE]
    
    return
}


####################################################################################################
#
# _c_do The lowest-level communication procedure. _c_do will call the `func_id` C function, with
# `parameters` parameters. Do not use this function directly. Use higher level function instead.
#
# @param    func_id The ID of the C function to be call. See TCL_FUNCTION_ID* gloval variables.
# @param    parameters A list of the parameters to be passed to the C function.
#
# @return   The return value of the original C function.
#
# @note     The return value will be the previous function in case of void function.
#
####################################################################################################
proc _c_do { func_id {parameters {} } } {

    #
    # Check if init has been run earlier.
    #
    if { $::tcl_function_call_variable < 0 } {
        error "ERROR: Global variables has not been initialized. Please run the init_flash_writer function."
        return -1
    }

    #
    # Wait for the previous function run.
    #
    while {[mrd -value $::tcl_function_call_variable] != $::TCL_FUNCTION_ID_RETURN} {
    }

    #
    # Fill parameters and call the function
    #
    mwr $::tcl_parameter_variable $parameters
    mwr $::tcl_function_call_variable $func_id
    
    #
    # wait for function complete
    #
    while {[mrd -value $::tcl_function_call_variable] != $::TCL_FUNCTION_ID_RETURN} {
    }
    
    return [mrd -value $::tcl_parameter_variable]
}


####################################################################################################
#
# hello simple debug function, will print a message on the standard output.
#
# @param    hello_param The number to be written after the hello string.
#
# @return   None.
#
# @note     None.
#
####################################################################################################
proc hello { hello_param } {
    _c_do $::TCL_FUNCTION_ID_PRINT_HELLO $hello_param
    
    return
}


####################################################################################################
#
# print_flash_writer_version reads and prints out to TCL console the flash_write executable's
# version.
#
# @param    None.
#
# @return   None.
#
# @note     None.
#
####################################################################################################
proc print_flash_writer_version { } {
    set version [_c_do $::TCL_FUNCTION_ID_GET_VERSION]
    
    set major [expr ($version >> 16) & 0xff]
    set minor [expr ($version >> 8)  & 0xff]
    set fix   [expr ($version)       & 0xff]
    
    puts [format "Zynq-Flash v%d.%d.%d" $major $minor $fix]
    
    return
}


####################################################################################################
#
# flash_init after running the init_flash_writer the flash (more precisely the structures of the
# flash) can be initialized. It reads the type, size, page size, etc. parameters from the flash.
#
# @param    None.
#
# @return   None.
#
# @note     TODO more trace/debug information should be added here.
#
####################################################################################################
proc flash_init { } {
    set ret [_c_do $::TCL_FUNCTION_ID_FLASH_INIT]
    if { $ret != 0 } {
        error "ERROR flash initializationhas failed."
        return 1
    }
    
    return 0
}


####################################################################################################
#
# get_flash_info. After flash_init the flash properties aka. information can be read by the
# get_flash_info procedure. This will print the flash information to TCL's console.
#
# @param    None.
#
# @return   None.
#
# @note     None.
#
####################################################################################################
proc get_flash_info { } {
    set flash_info_ptr [_c_do $::TCL_FUNCTION_ID_GET_FLASH_INFO]
    set flash_info [mrd -value $::tcl_parameter_variable 9]
    
    puts [format "SectSize          0x%x" [lindex $flash_info 0]]
    puts [format "NumSect           0x%x" [lindex $flash_info 1]]
    puts [format "PageSize          0x%x" [lindex $flash_info 2]]
    puts [format "NumPage           0x%x" [lindex $flash_info 3]]
    puts [format "FlashDeviceSize   0x%x" [lindex $flash_info 4]]
    puts [format "ManufacturerID    0x%x" [lindex $flash_info 5]]
    puts [format "DeviceIDMemSize   0x%x" [lindex $flash_info 6]]
    puts [format "SectMask          0x%x" [lindex $flash_info 7]]
    puts [format "NumDie            0x%x" [lindex $flash_info 8]]
    
    return
}


####################################################################################################
#
# flash_erase Erase a block from given address to given length. Must be called before writing the
# flash.
#
# @param    address is the start address of the block to be erase in bytes.
# @param    length is the length of the block to be erase in bytes.
#
# @return   Return zero on success.
#
# @note     None.
#
####################################################################################################
proc flash_erase { address length } {
    set ret [_c_do $::TCL_FUNCTION_ID_FLASH_ERASE "$address $length"]
    
    return $ret
}


####################################################################################################
#
# flash_erase_all Erase all sector of the flash. Must be called before writing the flash. (It is
# faster than the flash_erase for longer sizes)
#
# @param    None.
#
# @return   Return zero on success.
#
# @note     None.
#
####################################################################################################
proc flash_erase_all { } {
    set ret [_c_do $::TCL_FUNCTION_ID_FLASH_ERASE_ALL]
    
    return $ret
}


####################################################################################################
#
# _flash_write_slice_ptr writes a slice of data to flash. Do not use this function directly. Use
# flash_write_slice instead.
#
# @param    address is the start address in the flash device in bytes.
# @param    length is the length of the block to be write in bytes.
# @param    ptr is the address of the block in the executable memory-space. Usually equals the
#           bin_slice_buffer
#
# @return   None.
#
# @note     None.
#
####################################################################################################
proc _flash_write_slice_ptr { address length ptr } {
    _c_do $::TCL_FUNCTION_ID_FLASH_WRITE "$address $length $ptr"
    
    return
}


####################################################################################################
#
# flash_write_slice writes a slice of data to flash. This function gets a slice of data as
# parameter, then copies it to ARM's memory space, then ask to write it to flash.
#
# @param    address is the start address in the flash device in bytes.
# @param    length is the length of the block to be write in bytes.
# @param    data_slice is the data to be written. Format: it must be a TCL list of bytes.
#           Ex.: {0x01 0x02 0x03 ...}
# @param    ptr is the address of the block in the executable memory-space. Default is the
#           bin_slice_buffer
#
# @return   None.
#
# @note     None.
#
####################################################################################################
proc flash_write_slice { address length data_slice {ptr "default" } } {
    if { $ptr == "default" } {
        set ptr $::bin_slice_buffer
    }
    
    #
    # Copy data-slice from TCL to Zynq on-chip-memory
    #
    mwr -size b [expr $::bin_slice_buffer +4 ] $data_slice
    
    #
    # Ask to write data-slice from Zynq on-chip-memory to flash
    #
    _flash_write_slice_ptr $address $length $ptr
    
    return
}


####################################################################################################
#
# flash_dump_zynq prints a slice of data to Zynq's standard out. I suggest to use flash_dump
# instead.
#
# @param    address is the start address in the flash device in bytes to dump.
# @param    length is the length of the block to dump in bytes.
#
# @return   None.
#
# @note     Use flash_dump instead, which prints data locally
#
####################################################################################################
proc flash_dump_zynq { address length } {
    _c_do $::TCL_FUNCTION_ID_FLASH_DUMP "$address $length"
    
    return
}


####################################################################################################
#
# flash_read Reads arbitrary size of data from the flash
#
# @param    address is the start address in the flash device in bytes to dump.
# @param    length is the length of the block to dump in bytes.
#
# @return   Zero on success.
#
# @note     None.
#
####################################################################################################
proc flash_read { address length {verbose 1} } {
    set read_data ""

    for {set start_addr 0} {$start_addr<$length} {incr start_addr $::bin_slice_buffer_size} {
        if { $verbose } {
            puts [format "Read: 0x%04x / 0x%x kB   %d%%" \
                [expr $start_addr / 1024] [expr $length / 1024] [expr 100 * $start_addr / $length]]
        }
        
        set chunk_len [expr min($length - $start_addr, $::bin_slice_buffer_size)]
        set read_data_prt [_c_do $::TCL_FUNCTION_ID_FLASH_READ "$start_addr $chunk_len"]
        
        if { $read_data_prt == 0} {
            puts "ERROR: flash_dump: got null pointer. Please run flash_init first."
            return 1
        }
        
        set len_32 [expr $chunk_len/4]
        set read_data [concat $read_data [mrd -value $read_data_prt $len_32]]
    }
    
    return $read_data
}


####################################################################################################
#
# flash_dump prints a slice of data to TCL console in a hexdump-like format.
#
# @param    address is the start address in the flash device in bytes to dump.
# @param    length is the length of the block to dump in bytes.
# @param    channel Channel to write the results. File or output stream. Default is stdout 
# @param    formatum [hexdump | binary] default is hexdump
#
# @return   Zero on success.
#
# @note     None.
#
####################################################################################################
proc flash_dump { address length { channel stdout } { formatum hexdump } {verbose 1} } {
    set read_data [flash_read $address $length $verbose]
    if {$read_data == 1} {
        puts "read_data returned error."
        return 1
    }
    
    set len_32 [expr $length/4]
    
    if { $formatum == "binary" } {
        set outBinData [binary format i* $read_data]
        puts -nonewline $channel $outBinData
        flush $channel
    } elseif { $formatum == "hexdump" } {
        # Put the hex and Latin-1 data to the channel
        set hex ""
        for {set i 0} {$i<$len_32} {incr i} {
            set hex [format "%s %08x" $hex [lindex $read_data $i]]
            if {$i % 4 == 3 } {
                puts $channel [ format {%08x %-24s} $address $hex ] 
                set address [expr $address + 16]
                set hex ""
            }
        }
    } else {
        puts "Unknown formatum: $formatum"
        return 1
    }
    
    return 0
}


####################################################################################################
#
# flash_dump_file same as flash_dump just it dumps to a file. flash_dump_file opens the file call
# `flash_dump` to dump to that file, then closes the file.
#
# @param    address is the start address in the flash device in bytes to dump.
# @param    length is the length of the block to dump in bytes.
#
# @return   None.
#
# @note     None.
#
####################################################################################################
proc flash_dump_file { address length filename { formatum hexdump } } {

    # Open the file:
    set open_options "wb"
    set fp [open $filename $open_options]
    # Dump the flash content
    flash_dump 0 $length $fp $formatum
    
    # Close the file.
    close $fp
    
    return
}


####################################################################################################
#
# flash_verify Read back the flash content to a temporary file, then compares that with a reference
# file.
#
# @param    boot_bin_file is the reference file.
# @param    tmpfilename is the temporary file. Default is: ".readback_image.bin"
#
# @return   Non-zero on error.
#
# @note     None.
#
####################################################################################################
proc flash_verify { {boot_bin_file "default"} {tmpfilename ".readback_image.bin"} } {
    #
    # Getting/saving the boot_bin_file
    #
    if { $boot_bin_file == "default" } {
        set boot_bin_file $::boot_bin_file
    }
    if { $boot_bin_file == "undefined" } {
        puts "ERROR: Boot image file must be given at the first time."
        return 1
    }
    if { ! [file exists $boot_bin_file ] } {
        puts "ERROR: Boot image file $boot_bin_file does not exists."
        return 1
    }
    
    set ::boot_bin_file  [file normalize $boot_bin_file]
    
    set image_size [file size $boot_bin_file]
    
    puts "Reading back flash content..."
    flash_dump_file 0 $image_size $tmpfilename "binary"
    
    if { ! [file_equals $tmpfilename $boot_bin_file] } {
        puts "ERROR! Verification failed. Readback file mismatches the reference file."
        return 1
    }
    puts "Verification success."
    
    return 0
}


####################################################################################################
#
# flash_write_file writes a complete file to flash.
#
# @param    boot_bin_file the name of the file to be written.
#
# @return   Non-zero on success.
#
# @note     Erase befor write.
#
####################################################################################################
proc flash_write_file { {boot_bin_file "default"} {verbose 1} } {

    #
    # Getting/saving the boot_bin_file
    #
    if { $boot_bin_file == "default" } {
        set boot_bin_file $::boot_bin_file
    }
    if { $boot_bin_file == "undefined" } {
        puts "ERROR: Boot image file must be given at the first time."
        return 1
    }
    if { ! [file exists $boot_bin_file ] } {
        puts "ERROR: Boot image file $boot_bin_file does not exists."
        return 1
    }
    set ::boot_bin_file  [file normalize $boot_bin_file]
    
    #
    # Parse the file
    #
    if { $verbose } {
        puts -nonewline "Parse file..."
        flush stdout
    }
    set hex_data [file_to_hex $boot_bin_file]
    set image_size [file size $boot_bin_file]
    if { $verbose } {
        puts "        OK."
    }
    
    #
    # Write boot image slice by slice
    #
    for {set i 0} {$i<$image_size} {incr i $::bin_slice_buffer_size} {
        if { $verbose } {
            puts [format "0x%04x / 0x%x kB   %d%%" \
                [expr $i / 1024] [expr $image_size / 1024] [expr 100 * $i / $image_size]]
        }
    
        set hex_slice [lrange $hex_data $i [expr $i+$::bin_slice_buffer_size-1]]
        
        flash_write_slice $i $::bin_slice_buffer_size $hex_slice
    }
    
    return 0
}


####################################################################################################
#
# file_to_hex reads a binary file into a TCL list.
#
# @param    filename the name of the file to be read.
#
# @return   None.
#
# @note     None.
#
####################################################################################################
proc file_to_hex { filename  } {

    # Open the file, and set up to process it in binary mode.
    set fid [open $filename r]
    fconfigure $fid -translation binary -encoding binary

    # Read the complete file.
    set s    [read $fid ]
  
    # Convert the data to hex
    binary scan $s H* hex
  
    # Convert the hex to pairs of hex digits
    regsub -all -- {..} $hex {0x& } hex
    
    return $hex
}


####################################################################################################
#
# hexdump Unix's hexdump TCL implementation.
#
# @param    filename the name of the file to be read.
#
# @return   None.
#
# @note     None.
#
####################################################################################################
proc hexdump { filename { channel stdout } } {
     # This is derived from the Tcler's WIKI, page 1599,
     # original author unknown, possibly Kevin Kenny.

     if { [ catch {
        # Open the file, and set up to process it in binary mode.
        set fid [open $filename r]
        fconfigure $fid -translation binary -encoding binary

        while { ! [ eof $fid ] } {

           # Record the seek address. Read 16 bytes from the file.
           set addr [ tell $fid ]
           set s    [read $fid 16]

           # Convert the data to hex and to characters.
           binary scan $s H*@0a* hex ascii

           # Replace non-printing characters in the data.
           regsub -all -- {[^[:graph:] ]} $ascii {.} ascii

           # Split the 16 bytes into two 8-byte chunks
           regexp -- {(.{16})(.{0,16})} $hex -> hex1 hex2

           # Convert the hex to pairs of hex digits
           regsub -all -- {..} $hex1 {& } hex1
           regsub -all -- {..} $hex2 {& } hex2

           # Put the hex and Latin-1 data to the channel
           puts $channel [ format {%08x %-24s %-24s %-16s} \
              $addr $hex1 $hex2 $ascii ]
        }
     } err ] } {
        catch { ::close $fid }
        return -code error $err
     }
     # When we're done, close the file.
     catch { ::close $fid }
     
     return
}

#
# From: https://stackoverflow.com/a/29289660/2506522
#
proc file_equals {file1 file2} {
    # optimization: check file size first
    set equal 0
    if {[file size $file1] == [file size $file2]} {
        set fh1 [open $file1 r]
        set fh2 [open $file2 r]
        set equal [string equal [read $fh1] [read $fh2]]
        close $fh1
        close $fh2
    }
    return $equal
}


#
# Check existance of ps7_init procedure
#
if { [info procs ps7_init] != "ps7_init" } {
    puts "Warning! ps7_init procedure not found! Please source your ps7_init.tcl, which is placed under the hardware platorm."
}

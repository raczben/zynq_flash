

variable TCL_FUNCTION_ID_RETURN		 		0x0
variable TCL_FUNCTION_ID_INIT		 		0x1
variable TCL_FUNCTION_ID_PRINT_HELLO	 		0x10
variable TCL_FUNCTION_ID_GET_VERSION	 		0x20
variable TCL_FUNCTION_ID_FLASH_ERASE 		0x100
variable TCL_FUNCTION_ID_GET_FLASH_INFO   	0x101
variable TCL_FUNCTION_ID_FLASH_INIT	 		0x105
variable TCL_FUNCTION_ID_FLASH_WRITE 		0x110
variable TCL_FUNCTION_ID_FLASH_READ  		0x120
variable TCL_FUNCTION_ID_FLASH_DUMP  		0x125
variable TCL_FUNCTION_ID_GET_SLICE_BUFFER 	0x130
variable TCL_FUNCTION_ID_GET_SLICE_BUFFER_SIZE 0x140

set flash_writer_elf zed_bin/flash_writer.elf

set tcl_function_call_variable -1
set tcl_parameter_variable -1
set bin_slice_buffer -1
set bin_slice_buffer_size -1

proc init { } {
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
    
    puts "Getting communication variable (tcl_function_call)"
    set global_dict [print -dict &tcl_function_call]
    set ::tcl_function_call_variable [dict get $global_dict &tcl_function_call]
    set ::tcl_parameter_variable [expr $::tcl_function_call_variable + 4]
    
    puts "Setting slice buffer (bin_slice_buffer)"
    set ::bin_slice_buffer      [c_do $::TCL_FUNCTION_ID_GET_SLICE_BUFFER]
    set ::bin_slice_buffer_size [c_do $::TCL_FUNCTION_ID_GET_SLICE_BUFFER_SIZE]
}

proc c_do { func_id {parameters {} } } {

    if { $::tcl_function_call_variable <0 } {
        puts "ERROR: Global variables has not been initialized. Please run the init function."
        return
    }

    while {[mrd -value $::tcl_function_call_variable] != $::TCL_FUNCTION_ID_RETURN} {
    }

    # Fill parameters
    mwr $::tcl_parameter_variable $parameters
    # Call the function
    mwr $::tcl_function_call_variable $func_id
    
    # wait for function complete
    
    while {[mrd -value $::tcl_function_call_variable] != $::TCL_FUNCTION_ID_RETURN} {
    }
    
    return [mrd -value $::tcl_parameter_variable]
    
}

proc hello { hello_param } {
    c_do $::TCL_FUNCTION_ID_PRINT_HELLO $hello_param
}

proc flash_erase { address length } {
    c_do $::TCL_FUNCTION_ID_FLASH_ERASE "$address $length"
}

proc flash_write { address length ptr } {
    c_do $::TCL_FUNCTION_ID_FLASH_WRITE "$address $length $ptr"
}

proc flash_init { } {
    set ret [c_do $::TCL_FUNCTION_ID_FLASH_INIT]
    if { $ret != 0 } {
        puts "ERROR flash initializationhas failed."
    }
}

proc flash_dump { address length } {
    c_do $::TCL_FUNCTION_ID_FLASH_DUMP "$address $length"
}

proc flash_read { address length } {
     set read_data_prt [c_do $::TCL_FUNCTION_ID_FLASH_READ "$address $length"]
     set read_data [mrd -value $read_data_prt $length]
    
    # Put the hex and Latin-1 data to the channel
   
    set hex ""
    set len_32 [expr $length/4]
    for {set i 0} {$i<$len_32} {incr i} {
        set hex [format "%s %08x" $hex [lindex $read_data $i]]
        if {$i % 4 == 3 } {
            puts [ format {%08x %-24s} $address $hex ] 
            set address [expr $address + 16]
            set hex ""
        }
    }
}

proc print_version { } {
    set version [c_do $::TCL_FUNCTION_ID_GET_VERSION]
    puts $version
}

proc get_flash_info { } {
    set flash_info_ptr [c_do $::TCL_FUNCTION_ID_GET_FLASH_INFO]
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
}

proc flash_write2 { filename } {

    set hex_data [file_to_hex $filename]
    set image_size [file size $filename]
    
    for {set i 0} {$i<$image_size} {incr i $::bin_slice_buffer_size} {
    
        puts "$i"
    
        set hex_slice [lrange $hex_data $i [expr $i+$::bin_slice_buffer_size-1]]
        
        mwr -size b [expr $::bin_slice_buffer +4 ] $hex_slice
        
        flash_write $i $::bin_slice_buffer_size $::bin_slice_buffer
    }
    
}


proc file_to_hex { filename  } {

    # Open the file, and set up to process it in binary mode.
    set fid [open $filename r]
    fconfigure $fid -translation binary -encoding binary

    # Record the seek address. Read 16 bytes from the file.
    set addr [ tell $fid ]
    set s    [read $fid ]
  
    # Convert the data to hex
    binary scan $s H* hex
  
    # Convert the hex to pairs of hex digits
    regsub -all -- {..} $hex {0x& } hex
    
    return $hex
}



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
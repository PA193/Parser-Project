#!/bin/bash

echo "Enter the name of the file you want to parse:"
read input_file
echo $input_file
if [ -f $input_file ]; then
	file $input_file|grep TIFF>/dev/null
	if [ `echo $?` == 0 ]; then
		if [ `hexdump -s 2 -n 2 $input_file |cut -d " " -f2|head -1` == "002a" ]; then
			#echo "This is a TIFF file"
			Byte_Order=`hexdump -n 2 $input_file |cut -d " " -f2|head -1`
			#echo $Byte_Order
			if [ $Byte_Order == "4949" ] || [ Byte_Order == "4D4D" ]; then
				if [ $Byte_Order == "4949" ]; then
					printf "The byte order is \x49\x49: Little endian order\n"
				else
                                	printf "The byte order is \x4D\x4D: Big endian order\n"
				fi
				IFD=`hexdump -s 4 -n 4 $input_file |cut -d " " -f2|head -1`
				echo "Image File Directory at 0x$IFD"
				hexdump -s '0x'$IFD $input_file|cut -c 9-|tr -d "[:space:]" > tmp.ifd
				#for i in `cat $input_file.ifd`
				#do
				#	echo $i
				#done
			else
				echo "ERROR: This is not a TIFF file"
			fi
			
		else
			echo "ERROR: This is not a TIFF file"
		fi
	else
		echo "ERROR: This is not a TIFF file"
	fi
else
	echo "ERROR: File not present"
fi

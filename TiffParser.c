#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <inttypes.h>
#include <math.h>

/* Function to find out system endianness */
int system_byteorder(){
int num = 1;

if (*(char *)&num == 1)
{
    //printf("The system byte order is Little-Endian\n");
    return 1;
}
else
{
    printf("The system byte order is Big-Endian\n");
    return 0;
}
}

/* Function to find out file endianness */
int file_byteorder(uint16_t b0, uint16_t b1){
if(b0=='I' && b1=='I'){
        printf("The file byte order is Little Endian\n");
        return 1;
        }
else if(b0=='M' && b1=='M'){
        printf("The file byte order is Big Endian\n");
        return 0;
        }
else{
        printf("ERROR: This is not a TIFF FILE\n");
        exit(-1);
        }
}

/* Function to swap 2 bytes if endianness does not match */
uint16_t byte_swap16(uint16_t value){
return (((value>> 8)) | (value << 8));
}

/* Function to swap 4 bytes if endianness does not match */
uint32_t byte_swap32(uint32_t value){
return (((value&0x000000FF)<<24)+((value&0x0000FF00)<<8)+((value&0x00FF0000)>>8)+((value&0xFF000000)>>24));
}

/*Function to read the values of IFD tags */
uint32_t read_value(FILE *fp, uint16_t tag, int ret_s, int ret_f, uint32_t len){
uint32_t value=0;
uint16_t val=0;
switch(tag){

	case 254:
		printf("New Subfile Type : ");			//Read Tag 254 for New Subfile Type Field
		fseek(fp, 6, SEEK_CUR);
		fread(&value, 1, 4, fp);
		if(ret_s!=ret_f)
		value=byte_swap32(value);
		printf("%u\n", value);
		return value;
		break;

	case 255:
		printf("Subfile Type is : ");			//Read Tag 255 for Subfile Type Field
                fseek(fp, 6, SEEK_CUR);
                fread(&val, 1, 2, fp);
                if(ret_s!=ret_f)
                val=byte_swap16(val);
                printf("%u\n", val);
		fseek(fp, 2, SEEK_CUR);
                return val;
		break;

	case 256:
                printf("Image Width is : ");			//Read Tag 256 for Image Width Field
		fread(&val, 1, 2, fp);
		if(ret_s!=ret_f)
		val=byte_swap16(val);
                fseek(fp, 4, SEEK_CUR);
		if(val==3){
		fread(&val, 1, 2, fp);
                if(ret_s!=ret_f)
                val=byte_swap16(val);
                printf("%u\n", val);
		fseek(fp, 2, SEEK_CUR);
		return val;
                }
		else if(val==4){
                fread(&value, 1, 4, fp);
                if(ret_s!=ret_f)
                value=byte_swap32(value);
                printf("%u\n", value);
                return value;
		}
		else
		printf("ERROR in image\n");
                break;

        case 257:
                printf("Image Length is : ");			//Read Tag 257 for Image Length Field
		fread(&val, 1, 2, fp);
                if(ret_s!=ret_f)
                val=byte_swap16(val);
                fseek(fp, 4, SEEK_CUR);
                if(val==3){
                fread(&val, 1, 2, fp);
                if(ret_s!=ret_f)
                val=byte_swap16(val);
                printf("%u\n", val);
                fseek(fp, 2, SEEK_CUR);
                return val;
                }
                else if(val==4){
                fread(&value, 1, 4, fp);
                if(ret_s!=ret_f)
                value=byte_swap32(value);
                printf("%u\n", value);
                return value;
                }
                else
                printf("ERROR in image\n");
                break;

	case 258:
		printf("Bits per Sample is : ");		//Read Tag 258 for Bits/Sample Field
                fseek(fp, 2, SEEK_CUR);
		uint32_t count;
		uint32_t c;
                fread(&count, 1, 4, fp);
                if(ret_s!=ret_f)
                count=byte_swap32(count);
		printf("Count=%u: ", count);
		if(count==1){
		fread(&val, 1, 2, fp);
		if(ret_s!=ret_f)
                val=byte_swap16(val);
                printf("%u\n", val);
		fseek(fp, 2, SEEK_CUR);
		}
		else if(count==2){
		for(c=1; c<=count;c++){
		fread(&val, 1, 2, fp);
		if(ret_s!=ret_f)
                val=byte_swap16(val);
                printf("%u, ", val);
		}
		}
		else{
		fread(&value, 1, 4, fp);
		if(ret_s!=ret_f)
                value=byte_swap32(value);
		if(value > len){
        	printf("ERROR: Malformed TIFF File\n");
        	exit(-1);
        	}
		long int current=ftell(fp);
		fseek(fp, value, SEEK_SET);
		for(c=1;c<=count&&c<=4;c++){
		fread(&val, 1, 2, fp);
		if(ret_s!=ret_f)
                val=byte_swap16(val);
                printf("%u, ", val);
		}
		printf("\n");
                fseek(fp, current, SEEK_SET);
		}
                return value;
		break;

	case 259:
		printf("Compression is : ");			//Read Tag 259 for Compression Field
                fseek(fp, 6, SEEK_CUR);
                fread(&val, 1, 2, fp);
                if(ret_s!=ret_f)
                val=byte_swap16(val);
                //printf("%u\n", value);
                if(val==1)
		printf("Image is uncompressed\n");
		else
		printf("Image is compressed\n");
		fseek(fp, 2, SEEK_CUR);
                return val;
		break;

	case 296:
		printf("Resolution unit is : ");		//Read Tag 296 for Resolution Unit Field
                fseek(fp, 6, SEEK_CUR);
                fread(&val, 1, 2, fp);
                if(ret_s!=ret_f)
                val=byte_swap16(val);
		if(val == 2)
		printf("inches\n");
		else if(val == 3)
		printf("centimeters\n");
		else
		printf("No unit specified\n");
                //printf("%u\n", value);
                fseek(fp, 2, SEEK_CUR);
                return val;
                break;
	
	case 282:
		printf("XResolution is : ");			//Read Tag 282 for X Resolution Field
                fseek(fp, 6, SEEK_CUR);
		fread(&value, 1, 4, fp);
		if(ret_s!=ret_f)
                value=byte_swap32(value);
		if(value > len){
        	printf("ERROR: Malformed TIFF File\n");
	        exit(-1);
        	}
		long int cur_xres=ftell(fp);
		fseek(fp, value, SEEK_SET);
		uint32_t xnum, xden;
                fread(&xnum, 1, 4, fp);
		if(ret_s!=ret_f)
                xnum=byte_swap32(xnum);
		fread(&xden, 1, 4, fp);
                if(ret_s!=ret_f)
                xden=byte_swap32(xden);
		uint32_t xres;
		if(xden==0)
		xres=0;
		else
		xres = (uint32_t)xnum/xden;
                printf("%u\n", xres);
		fseek(fp, cur_xres, SEEK_SET);
                return xres;
                break;

	case 283:
		printf("YResolution is : ");			//Read Tag 283 for Y Resolution Field
                fseek(fp, 6, SEEK_CUR);
		fread(&value, 1, 4, fp);
		if(ret_s!=ret_f)
                value=byte_swap32(value);
		if(value > len){
                printf("ERROR: Malformed TIFF File\n");
                exit(-1);
		}
                long int cur_yres=ftell(fp);
                fseek(fp, value, SEEK_SET);
                uint32_t ynum, yden;
                fread(&ynum, 1, 4, fp);
                if(ret_s!=ret_f)
                ynum=byte_swap32(ynum);
                fread(&yden, 1, 4, fp);
                if(ret_s!=ret_f)
                yden=byte_swap32(yden);
		uint32_t yres;
		if(yden==0)
		yres=0;
		else
                yres = (uint32_t)(ynum/yden);
                printf("%u\n", yres);
                fseek(fp, cur_yres, SEEK_SET);
                return yres;
                break;

	case 262:
		printf("Photometric Interpretation is : ");		//Read Tag 262 for Photometric Interpretation Field
                fseek(fp, 6, SEEK_CUR);
                fread(&val, 1, 2, fp);
                if(ret_s!=ret_f)
                val=byte_swap16(val);
		if(val==0)
		printf("Image is Monochrome Image\n");
		else if(val==1)
		printf("Image is Monochrome Image\n");
		else if(val==2)
		printf("Image is RGB Image\n");
		else if(val==3)
		printf("Image is Palette Color Image\n");
		else if(val==4)
		printf("Image has Transparency Mask\n");
		else if(val==5)
		printf("Image is CMYK Image\n");
		else if(val==6)
		printf("Image is YCBCR Image\n");
		else if(val==8)
		printf("Image is CYELab Image\n");
		else
		printf("Image is color image\n");
                //printf("%u\n", value);
                fseek(fp, 2, SEEK_CUR);
                return val;
                break;

	case 279:
		printf("Strip byte count is : ");				//Read Tag 279 for Strip byte count Field
		fread(&val, 1, 2, fp);
                if(ret_s!=ret_f)
                val=byte_swap16(val);
                fseek(fp, 4, SEEK_CUR);
                if(val==3){
                fread(&val, 1, 2, fp);
                if(ret_s!=ret_f)
                val=byte_swap16(val);
                printf("%u\n", val);
                fseek(fp, 2, SEEK_CUR);
                return val;
                }
                else if(val==4){
                fread(&value, 1, 4, fp);
                if(ret_s!=ret_f)
                value=byte_swap32(value);
                printf("%u\n", value);
                return value;
                }
                else
                printf("ERROR in image\n");
                break;

	case 278:
		printf("Rows per strip is : ");					//Read Tag 278 for Rows/Strip Field
		fread(&val, 1, 2, fp);
                if(ret_s!=ret_f)
                val=byte_swap16(val);
                fseek(fp, 4, SEEK_CUR);
                if(val==3){
                fread(&val, 1, 2, fp);
                if(ret_s!=ret_f)
                val=byte_swap16(val);
                printf("%u\n", val);
                fseek(fp, 2, SEEK_CUR);
                return val;
                }
                else if(val==4){
                fread(&value, 1, 4, fp);
                if(ret_s!=ret_f)
                value=byte_swap32(value);
                printf("%u\n", value);
                return value;
                }
                else
                printf("ERROR in image\n");
                break;

        default:
                //printf("To be Added\n");
                fseek(fp, 6, SEEK_CUR);
                fread(&value, 1, 4, fp);
                if(ret_s!=ret_f)
                value=byte_swap32(value);
                return value;
		break;
	}
return 0;
}

/* Function to read the tag value for each field */
uint16_t read_tag(FILE *fp, int ret_s, int ret_f){
uint16_t tag=0;
fread(&tag, 1, 2, fp);
if(ret_s!=ret_f)
tag=byte_swap16(tag);
//printf("Tag is : %u\n", tag);
return tag;
}

/* Main Function */
int main(int argc, char* argv[]){
	char fname[270];
	char buffer[5];
	int i;
	/*printf("Enter filename to parse:\n");					//Input file from user
	scanf("%s", &fname);
	fgets(fname, 269, stdin);*/
	if(argc!=2){
		printf("USAGE: ./TiffParser <filename>\n");
		return 0;
		exit(-1);
	}
	strncpy(fname, argv[1], 270);
	if ((strlen(fname)>0) && (fname[strlen (fname) - 1] == '\n'))
        fname[strlen (fname) - 1] = '\0';
	/*if(strlen(fname) > 269){
		printf("ERROR: File name must not exceed 256 characters\n");
		exit(-1);
	}*/

	FILE *fp=fopen(fname, "rb");
	if(!fp){
    		printf("ERROR: File not found or File name too Long\n");
    		exit(-1);
    	}

	fseek(fp, 0L, SEEK_END);
	uint32_t len=ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	fread(buffer, 2, 1, fp);

	int ret_s, ret_f;
	ret_s=system_byteorder();
	ret_f=file_byteorder(buffer[0], buffer[1]);

	/* Read TIFF magic number */	
	uint32_t ifd_offset=0;
	uint16_t ver_num=0;
	uint16_t entries=0;
	uint16_t tag=0;
	fseek(fp, 2, SEEK_SET);
	fread(&ver_num, 1, 2, fp);
	if(ret_s!=ret_f)
	ver_num=byte_swap16(ver_num);
	printf("The value of bytes 2-3 is %u\n", ver_num);
	if(ver_num==42)
	printf("This is a TIF file\n");
	else{
	printf("ERROR: Bad Input. This is not a TIF file\n");
	exit(-1);
	}
	
	/* Read IFD offset */
	fread(&ifd_offset, 1, 4, fp);
	if(ret_s!=ret_f)
        ifd_offset=byte_swap32(ifd_offset);
	if(ifd_offset > len){
	printf("ERROR: Malformed TIFF File\n");
	exit(-1);
	}
	printf("The first IFD is at  0x%02x\n", ifd_offset);

	/* Read number of directory entries */
	//The following unused variable ret has been deleted. 
	//int ret=fseek(fp, ifd_offset, SEEK_SET);
	fseek(fp, ifd_offset, SEEK_SET);
	fread(&entries, 1, 2, fp);
	if(ret_s!=ret_f)
        entries=byte_swap16(entries);
	if(entries < 1 || entries > 64){
	printf("ERROR: Malformed TIFF File\n");
	exit(-1);
	}
	printf("Number of directory entries: %u\n", entries);

	for(i=0; i<entries; i++){
		tag = read_tag(fp, ret_s, ret_f);
		read_value(fp, tag, ret_s, ret_f, len);
	
	}	

	fclose(fp);

	return 0;
}

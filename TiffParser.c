#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <inttypes.h>
#include <math.h>

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

uint16_t byte_swap16(uint16_t value){
return (((value>> 8)) | (value << 8));
}

uint32_t byte_swap32(uint32_t value){
return (((value&0x000000FF)<<24)+((value&0x0000FF00)<<8)+((value&0x00FF0000)>>8)+((value&0xFF000000)>>24));
}

uint32_t read_value(FILE *fp, uint16_t tag, int ret_s, int ret_f){
uint32_t value=0;
uint16_t val=0;
switch(tag){

	case 254:
		printf("New Subfile Type : ");
		fseek(fp, 6, SEEK_CUR);
		fread(&value, 1, 4, fp);
		if(ret_s!=ret_f)
		value=byte_swap32(value);
		printf("%u\n", value);
		return value;
		break;

	case 255:
		printf("Subfile Type is : ");
                fseek(fp, 6, SEEK_CUR);
                fread(&val, 1, 2, fp);
                if(ret_s!=ret_f)
                val=byte_swap16(val);
                printf("%u\n", val);
		fseek(fp, 2, SEEK_CUR);
                return val;
		break;

	case 256:
                printf("Image Width is : ");
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
                printf("Image Length is : ");
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
		printf("Bits per Sample is : ");
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
		printf("Compression is : ");
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
		printf("Resolution unit is : ");
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
		printf("XResolution is : ");
                fseek(fp, 6, SEEK_CUR);
		fread(&value, 1, 4, fp);
		if(ret_s!=ret_f)
                value=byte_swap32(value);
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
		printf("YResolution is : ");
                fseek(fp, 6, SEEK_CUR);
		fread(&value, 1, 4, fp);
		if(ret_s!=ret_f)
                value=byte_swap32(value);
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
		printf("Photometric Interpretation is : ");
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
		printf("Strip byte count is : ");
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
		printf("Rows per strip is : ");
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
/*fseek(fp, 6, SEEK_CUR);
fread(&value, 1, 4, fp);
if(ret_s!=ret_f)
value=byte_swap32(value);
printf("%u\n", value);
return value;*/
}

uint16_t read_tag(FILE *fp, int ret_s, int ret_f){
uint16_t tag=0;
fread(&tag, 1, 2, fp);
if(ret_s!=ret_f)
tag=byte_swap16(tag);
//printf("Tag is : %u\n", tag);
return tag;
}


int main(){
	char fname[270];
	char buffer[5];
	int i;
	printf("Enter filename to parse:\n");
	scanf("%s", &fname);
	if(strlen(fname) > 269){
	//	printf("%d\n", strlen(fname));
		printf("ERROR: File name must not exceed 256 characters\n");
		exit(-1);
	}

	FILE *fp=fopen(fname, "rb");
	if(!fp){
    		printf("ERROR: File not found\n");
    		exit(-1);
    	}

	fread(buffer, 2, 1, fp);

	int ret_s, ret_f;
	ret_s=system_byteorder();
	ret_f=file_byteorder(buffer[0], buffer[1]);

	
	uint32_t ifd_offset=0;
	uint16_t ver_num=0;
	uint16_t entries=0;
	uint16_t tag=0;
	uint32_t value=0;
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

	fread(&ifd_offset, 1, 4, fp);
	if(ret_s!=ret_f)
        ifd_offset=byte_swap32(ifd_offset);
	printf("The first IFD is at  0x%02x\n", ifd_offset);

	fseek(fp, ifd_offset, SEEK_SET);
	fread(&entries, 1, 2, fp);
	if(ret_s!=ret_f)
        entries=byte_swap16(entries);
	printf("Number of directory entries: %u\n", entries);

	for(i=0; i<entries; i++){
		tag = read_tag(fp, ret_s, ret_f);
		read_value(fp, tag, ret_s, ret_f);
	
}

	fclose(fp);

	return 0;
}

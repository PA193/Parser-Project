#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <inttypes.h>

int system_byteorder(){
/*union {
        long l;
        char c[4];
   } test;
    	
	test.l = 1;
	if( test.c[3] && !test.c[2] && !test.c[1] && !test.c[0] ){
		printf("The system byte order is Big Endian\n");
        	return 0;
	}

    	if( !test.c[3] && !test.c[2] && !test.c[1] && test.c[0] ){
		printf("The system byte order is Little Endian\n");
        	return 1;
	}

	else
    		return -1;*/
int num = 1;

if (*(char *)&num == 1)
{
    printf("The system byte order is Little-Endian\n");
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
switch(tag){

	case 254:
		printf("New Subfile Type : ");
		break;

	case 255:
		printf("Subfile Type is : ");
		break;

	case 256:
                printf("Image Width is : ");
                break;

        case 257:
                printf("Image Length is : ");
                break;

	case 258:
		printf("Bits per Sample is : ");
		break;

	case 259:
		printf("Compression is : ");
		break;

        default:
                printf("To be Added\n");
		break;
	}
fseek(fp, 6, SEEK_CUR);
fread(&value, 1, 4, fp);
if(ret_s!=ret_f)
value=byte_swap32(value);
printf("%u\n", value);
return value;
}

uint16_t read_tag(FILE *fp, int ret_s, int ret_f){
uint16_t tag=0;
//fseek(fp, 12, SEEK_CUR);
fread(&tag, 1, 2, fp);
if(ret_s!=ret_f)
tag=byte_swap16(tag);
printf("Tag is : %u\n", tag);
//fseek(fp, -2, SEEK_CUR);
//read_value(fp, tag, ret_s, ret_f);
return tag;
}

/*uint16_t read_32(FILE *fp, int ret_s, int ret_f){
uint16_t buf32=0;

fread(&buf32, 1, 4, fp);
if(ret_s!=ret_f)
buf32=byte_swap32(buf32);
return buf32;
}*/

int main(){
	char fname[20];
	char ch;
	//char *buffer=NULL;
	char buffer[5];
	int i;
	printf("Enter filename to parse:\n");
	scanf("%s", &fname);

	FILE *fp=fopen(fname, "rb");
	if(!fp){
    		printf("ERROR: File not found\n");
    		exit(-1);
    	}

	fread(buffer, 2, 1, fp);
	//printf("%s\n", buffer);

	int ret_s, ret_f;
	ret_s=system_byteorder();
	//printf("%d\n", ret_s);
	ret_f=file_byteorder(buffer[0], buffer[1]);
        //printf("%d\n", ret_f);

	
	uint16_t ifd_offset=0;
	uint16_t ver_num=0;
	uint16_t entries=0;
	uint16_t tag=0;
	uint16_t value=0;
	fseek(fp, 2, SEEK_SET);
	fread(&ver_num, 1, 2, fp);
	//printf("%s\n", buf);
	if(ret_s!=ret_f)
	ver_num=byte_swap16(ver_num);
	//ver_num = read_16(fp, ret_s, ret_f);
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
	//ifd_offset=read_32(fp, ret_s, ret_f);
	printf("The first IFD is at  %02x\n", ifd_offset);

	fseek(fp, 0x27e0, SEEK_SET);
	fread(&entries, 1, 2, fp);
	//entries=read_16(fp, ret_s, ret_f);
	printf("Number of directory entries: %u\n", entries);

	for(i=0; i<=14; i++){
		tag = read_tag(fp, ret_s, ret_f);
		read_value(fp, tag, ret_s, ret_f);
	
	/*switch(tag){

        case 254:
                printf("New Subfile Type : ");
                value = read_value(fp, tag, ret_s, ret_f);
                break;

        case 255:
                printf("Subfile Type is : ");
		value = read_value(fp, tag, ret_s, ret_f);
                break;

        case 256:
                printf("Image Width is : ");
		value = read_value(fp, tag, ret_s, ret_f);
                break;

        case 257:
                printf("Image Length is : ");
		value = read_value(fp, tag, ret_s, ret_f);
                //read_32(fp, ret_s, ret_f);
                break;
        
	case 258:
                printf("Bits per Sample is : ");
		value = read_value(fp, tag, ret_s, ret_f);
                break;

        case 259:
                printf("Compression is : ");
		value = read_value(fp, tag, ret_s, ret_f);
                break;
                
        default:
                printf("To be Added\n");
                break;
	}*/
}
	//}
	fclose(fp);

	return 0;
}

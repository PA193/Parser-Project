#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <string>

#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

#ifndef __LINUX__
# if defined(linux)
#  define __LINUX__
# endif
#endif

#define TIFF_LAST_ERROR_SIZE 1024
#define TIFF_ORDER_UNKNOWN 0
#define TIFF_ORDER_BIGENDIAN 1
#define TIFF_ORDER_LITTLEENDIAN 2

/* ----------To be Added------------*/

int TIFFReader_get_byteorder() 
{
   	union 
   	{
   	     long l;
   	     char c[4];
   	} test;
    	
	test.l = 1;
	if( test.c[3] && !test.c[2] && !test.c[1] && !test.c[0] )
        	return TIFF_ORDER_BIGENDIAN;

    	if( !test.c[3] && !test.c[2] && !test.c[1] && test.c[0] )
        	return TIFF_ORDER_LITTLEENDIAN;

    	return TIFF_ORDER_UNKNOWN;
}

struct TIFFReaderFrame {
    	uint32_t width;
    	uint32_t height;

/*----------To be Added------------*/
};


inline TIFFReaderFrame TIFFReader_getEmptyFrame() {
	TIFFReaderFrame d;
    	d.width=0;
    	d.height=0;

/*----------To be Added------------*/
    	return d;
}

struct TIFFReaderFile 
{
	FILE* file;
	
	char lastError[TIFF_LAST_ERROR_SIZE];
    	int wasError;

    	uint8_t systembyteorder;
    	uint8_t filebyteorder;

    	uint32_t firstrecord_offset;
    	uint32_t nextifd_offset;

    	uint64_t filesize;
	
    	TIFFReaderFrame currentFrame;
};

int TIFFReader_fclose(TIFFReaderFile* tiff) 
{
	int r=fclose(tiff->file);
    	tiff->file=NULL;
    	return r;
}

size_t TIFFReader_fread(void * ptr, size_t size, size_t count, TIFFReaderFile* tiff) 
{
	return fread(ptr, size, count, tiff->file);
}


inline static uint32_t TIFFReader_Byteswap32(uint32_t nLongNumber)
{
   	return (((nLongNumber&0x000000FF)<<24)+((nLongNumber&0x0000FF00)<<8)+
   		((nLongNumber&0x00FF0000)>>8)+((nLongNumber&0xFF000000)>>24));
}


inline static uint16_t TIFFReader_Byteswap16(uint16_t nValue)
{
   	return (((nValue>> 8)) | (nValue << 8));
}


inline uint32_t TIFFReader_readuint32(TIFFReaderFile* tiff) 
{
	uint32_t res=0;
	TIFFReader_fread(&res, 4,1,tiff);
    	if (tiff->systembyteorder!=tiff->filebyteorder) 
	{
	        res=TIFFReader_Byteswap32(res);
	}
    	return res;
}


inline uint16_t TIFFReader_readuint16(TIFFReaderFile* tiff) 
{
	uint16_t res=0;
    	TIFFReader_fread(&res, 2,1,tiff);
    	if (tiff->systembyteorder!=tiff->filebyteorder) 
	{
        	res=TIFFReader_Byteswap16(res);
	}
    	return res;
}


TIFFReaderFile* TIFFReader_open(const char* filename) 
{
    	TIFFReaderFile* tiff=(TIFFReaderFile*)malloc(sizeof(TIFFReaderFile));

    	tiff->filesize=0;
    	struct stat file;
    
	if(stat(filename,&file)==0) 
	{
        	tiff->filesize=file.st_size;
    	}
    	tiff->currentFrame=TIFFReader_getEmptyFrame();

    	tiff->file=fopen(filename, "rb");

    	tiff->systembyteorder=TIFFReader_get_byteorder();
    	memset(tiff->lastError, 0, TIFF_LAST_ERROR_SIZE);
    	tiff->wasError=FALSE;
    
	if ((tiff->file) && tiff->filesize>0)  
	{
        	uint8_t tiffid[3]={0,0,0};
        	fread(tiffid, 1,2,tiff->file);
        	//TIFFReader_fread(tiffid, 1,2,tiff);

        	printf("      - head=%s\n", tiffid);
        	
      		if (tiffid[0]=='I' && tiffid[1]=='I') 
			tiff->filebyteorder=TIFF_ORDER_LITTLEENDIAN;
        	else if (tiffid[0]=='M' && tiffid[1]=='M') 
			tiff->filebyteorder=TIFF_ORDER_BIGENDIAN;
        	else 
		{
            		free(tiff);
            		return NULL;
        	}
        
		uint16_t TIFFIden=TIFFReader_readuint16(tiff);
        	printf("      - TIFFIden=%u\n", TIFFIden);
        	if (TIFFIden!=42) 
		{
            		free(tiff);
            		return NULL;
        	}
        
		tiff->firstrecord_offset=TIFFReader_readuint32(tiff);

        	printf("      - filesize=%u\n", tiff->filesize);
        	printf("      - firstrecord_offset=%4X\n", tiff->firstrecord_offset);

    	} 
	else 
	{
        	free(tiff);
        	return NULL;
    	}
	
	return tiff;
}

void TIFFReader_close(TIFFReaderFile* tiff) 
{
	if (tiff) 
	{
        	fclose(tiff->file);
        	free(tiff);
    	}
}

uint32_t TIFFReader_getWidth(TIFFReaderFile* tiff) 
{
	if (tiff) 
	{
		return tiff->currentFrame.width;
	}
	return 0;
}

uint32_t TIFFReader_getHeight(TIFFReaderFile* tiff) 
{
	if (tiff) 
	{
	        return tiff->currentFrame.height;
    	}
    	return 0;
}

int main()
{
	printf("Hello World\n");

	TIFFReaderFile* tiffr=NULL;
   	tiffr=TIFFReader_open("sample.tif"); 
   	if (!tiffr) { 
        	printf("ERROR reading TIFF file)\n"); 
   	} 
	else 
	{ 
	        uint32_t width=TIFFReader_getWidth(tiffr); 
	        uint32_t height=TIFFReader_getHeight(tiffr); 
	        uint16_t* image=(uint16_t*)calloc(width*height, sizeof(uint16_t));  
		
		printf("Image Width = %d \n", width);
		printf("Image Height = %d \n", height);

	        free(image); 
	} 
    	TIFFReader_close(tiffr); 

	return 0;
}

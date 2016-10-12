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

int ByteOrder() 
{
   	union 
   	{
   	     long l;
   	     char c[4];
   	} test;
    	
	test.l = 1;
	if( test.c[3]==1)
	{
		if(!test.c[2] && !test.c[1] && !test.c[0] )
        		return 1;
	}
	if (test.c[0]==1)
	{
    		if(!test.c[3] && !test.c[2] && !test.c[1])
        		return 2;
	}
    	return 0;
}

struct TiffFrame {
    	uint32_t width;
    	uint32_t height;
	uint16_t compression;
	uint32_t rowsperstrip;
    	uint32_t* stripoffsets;
	uint32_t* stripbytecounts;
    	uint32_t stripcount;
    	uint16_t samplesperpixel;
    	uint16_t* bitspersample;
    	uint16_t planarconfiguration;
    	uint16_t sampleformat;
    	uint32_t imagelength;
	char* description;

};


inline TiffFrame GetEmptyFrame() 
{
	TiffFrame d;
    	d.width=0;
    	d.height=0;
	d.stripcount=0;
    	d.compression=1;
    	d.rowsperstrip=0;
    	d.stripoffsets=0;
    	d.stripbytecounts=0;
    	d.samplesperpixel=1;
    	d.bitspersample=0;
    	d.planarconfiguration=2;
    	d.sampleformat=1;
    	d.imagelength=0;
	d.description=0;
    	return d;
}

struct TiffFile 
{
	FILE* file;

    	uint8_t systembyteorder;
    	uint8_t filebyteorder;

    	uint32_t firstrecord_offset;
    	uint32_t nextifd_offset;

    	uint64_t filesize;
	
    	TiffFrame currentFrame;
};

inline uint8_t ReadUint8(TiffFile* tiff) 
{
	uint16_t res=0;
	fread(&res, 1,2,tiff->file);
    	return res;
}

inline static uint32_t ByteSwap32(uint32_t nLongNumber)
{
   	return (((nLongNumber&0x000000FF)<<24)+((nLongNumber&0x0000FF00)<<8)+
   		((nLongNumber&0x00FF0000)>>8)+((nLongNumber&0xFF000000)>>24));
}


inline static uint16_t ByteSwap16(uint16_t nValue)
{
   	return (((nValue>> 8)) | (nValue << 8));
}


inline uint32_t ReadUint32(TiffFile* tiff) 
{
	uint32_t res=0;
	fread(&res, 4,1,tiff->file);
    	if (tiff->systembyteorder!=tiff->filebyteorder) 
	{
	        res=ByteSwap32(res);
	}
    	return res;
}


inline uint16_t ReadUint16(TiffFile* tiff) 
{
	uint16_t res=0;
    	fread(&res, 2,1,tiff->file);
    	if (tiff->systembyteorder!=tiff->filebyteorder) 
	{
        	res=ByteSwap16(res);
	}
    	return res;
}

inline void FreeEmptyFrame(TiffFrame f) 
{
	if (f.stripoffsets) free(f.stripoffsets);
    	f.stripoffsets=NULL;
    
	if (f.stripbytecounts) free(f.stripbytecounts);
	f.stripbytecounts=NULL;
    
	if (f.bitspersample) free(f.bitspersample);
	f.bitspersample=NULL;
	
	if (f.description) free(f.description);
	f.description=NULL;
}

struct IFD 
{
	uint16_t tag;
    	uint16_t type;
    	uint32_t count;
    	uint32_t value;
    	uint32_t value2;
    	uint32_t* pvalue;
    	uint32_t* pvalue2;
};

inline IFD ReadIFD(TiffFile* tiff) 
{
	IFD d;

    	d.value=0;
    	d.value2=0;

    	d.pvalue=0;
    	d.pvalue2=0;

    	d.count=1;
    	d.tag=ReadUint16(tiff);
    	d.type=ReadUint16(tiff);
    	d.count=ReadUint32(tiff);

	fpos_t pos;
        fgetpos(tiff->file, &pos);
    	
	int changedpos=FALSE;
    
	//printf("    - pos=0x%X   tag=%d type=%d count=%u \n",pos, d.tag, d.type, d.count);
    	switch(d.type) 
	{
	        case 1:
	        case 2:
	    	if (d.count>0) 
		{
		      d.pvalue=(uint32_t*)calloc(d.count, sizeof(uint32_t));
	              if (d.count<=4) 
		      {
            	      		unsigned int i;
		    		for (i=0; i<4; i++) 
				{
				      uint32_t v=ReadUint8(tiff);
				      if (i<d.count) d.pvalue[i]=v;
		                }
	      	      } 
		      else 
		      {
		      		changedpos=TRUE;
		                uint32_t offset=ReadUint32(tiff);
		      		if (offset+d.count*1<=tiff->filesize) 
				{
				      fseek(tiff->file, offset, SEEK_SET);
			              unsigned int i;
				      for (i=0; i<d.count; i++) {
					    d.pvalue[i]=ReadUint8(tiff);
				      }
		                }
	      	      }
	    	}
            	d.pvalue2=NULL;
            	//printf("    - BYTE/CHAR: tag=%d count=%u   val[0]=%u\n",d.tag,d.count, d.pvalue[0]);
            	break;

        	case 3:
            		d.pvalue=(uint32_t*)calloc(d.count, sizeof(uint32_t));
            		if (d.count<=2) 
			{
                		unsigned int i;
                		for (i=0; i<2; i++) 
				{
                    			uint32_t v=ReadUint16(tiff);
                    			if (i<d.count) d.pvalue[i]=v;
                		}
            		} 
			else 
			{
		                changedpos=TRUE;
		                uint32_t offset=ReadUint32(tiff);
		                if (offset+d.count*2<tiff->filesize) 
				{
                    			fseek(tiff->file, offset, SEEK_SET);
                    			//TParser_fseek_set(tiff, offset);
                    			unsigned int i;
                    			for (i=0; i<d.count; i++) {
                        			d.pvalue[i]=ReadUint16(tiff);
                    			}
                	}
            	}
            	d.pvalue2=NULL;
            	//printf("    - SHORT: tag=%d count=%u   val[0]=%u\n",d.tag,d.count, d.pvalue[0]);
            	break;

        	case 4:
            	d.pvalue=(uint32_t*)calloc(d.count, sizeof(uint32_t));
            	if (d.count<=1) 
		{
	                d.pvalue[0]=ReadUint32(tiff);
	        } 
		else 
		{
	                changedpos=TRUE;
	                uint32_t offset=ReadUint32(tiff);
	                if (offset+d.count*4<tiff->filesize) 
			{
	                    	fseek(tiff->file, offset, SEEK_SET);
                		uint32_t i;
                    		for (i=0; i<d.count; i++) 
				{
                        		d.pvalue[i]=ReadUint32(tiff);
                    		}
                	}
                	//printf("- LONG: pos=0x%X   offset=0x%X   tag=%d count=%u   val[0]=%u\n",pos, offset,d.tag,d.count, d.pvalue[0]);
            	}
            	d.pvalue2=NULL;
            	//printf("    - LONG: tag=%d count=%u   val[0]=%u\n",d.tag,d.count, d.pvalue[0]);
            	break;
        
		case 5: 
		{
            		d.pvalue=(uint32_t*)calloc(d.count, sizeof(uint32_t));
            		d.pvalue2=(uint32_t*)calloc(d.count, sizeof(uint32_t));

        		changedpos=TRUE;
            		uint32_t offset=ReadUint32(tiff);
            		if (offset+d.count*4<tiff->filesize) 
			{
                		fseek(tiff->file, offset, SEEK_SET);
                		uint32_t i;
                		for (i=0; i<d.count; i++) 
				{
                    			d.pvalue[i]=ReadUint32(tiff);
                    			d.pvalue2[i]=ReadUint32(tiff);
                		}
            		}
            //printf("- RATIONAL: pos=0x%X offset=0x%X tag=%d count=%u val[0]=%u/%u\n",pos,offset,d.tag,d.count,d.pvalue[0], d.pvalue[1]);
            	} break;

       		default: d.value=ReadUint32(tiff); break;
    	}
    	if (d.pvalue) 
		d.value=d.pvalue[0];
    	if (d.pvalue2) 
		d.value2=d.pvalue2[0];
    	if (changedpos) 
	{
	        fsetpos(tiff->file, &pos);
	        fseek(tiff->file, 4, SEEK_CUR);
	}
    	return d;
}

inline void ReadNextFrame(TiffFile* tiff) 
{

	FreeEmptyFrame(tiff->currentFrame);
    	tiff->currentFrame=GetEmptyFrame();
	
	printf("Next IFD Offset = %u\n",tiff->nextifd_offset);
    	    
	if (tiff->nextifd_offset!=0 && tiff->nextifd_offset+2<tiff->filesize) 
	{
	        printf("Seeking=0x%X\n", tiff->nextifd_offset);
	        fseek(tiff->file, tiff->nextifd_offset, SEEK_SET);
	       
		uint16_t ifd_count=ReadUint16(tiff);
	        printf("No of Tags = %u\n", ifd_count);

	        uint16_t i;
	        for ( i=0; i<ifd_count; i++) 
		{
		    	IFD ifd=ReadIFD(tiff);
	        	printf("    - readIFD %d (tag: %u, type: %u, count: %u)\n", i, ifd.tag, ifd.type, ifd.count);
            
			switch(ifd.tag) 
			{
				case 254: 
				{					
					printf("New Sub File Type = %u\n", ifd.value);					
					break;
				}		                
				case 256: 
				{					
					tiff->currentFrame.width=ifd.value;  
					printf("Width = %u\n", tiff->currentFrame.width);					
					break;
				}		                
				case 257: 
				{
					tiff->currentFrame.imagelength=ifd.value; 
					printf("ImageLength = %u\n", tiff->currentFrame.imagelength); 
					break;
				}
		                case 258: 
				{
		                        tiff->currentFrame.bitspersample=(uint16_t*)malloc(ifd.count*sizeof(uint16_t));
		                        memcpy(tiff->currentFrame.bitspersample, ifd.pvalue, ifd.count*sizeof(uint16_t));
					printf("Bits per Sample = %u\n", tiff->currentFrame.bitspersample); 
		                } break;
                		
				case 259: 
				{
					tiff->currentFrame.compression=ifd.value; 
					printf("Compression = %u\n", tiff->currentFrame.compression); 
					break;
				}
				case 262: 
				{					
					printf("Photometric Value = %u\n", ifd.value);					
					break;
				}
				case 273: 
				{
		                        tiff->currentFrame.stripcount=ifd.count;
		                        tiff->currentFrame.stripoffsets=(uint32_t*)calloc(ifd.count, sizeof(uint32_t));
		                        memcpy(tiff->currentFrame.stripoffsets, ifd.pvalue, ifd.count*sizeof(uint32_t));
					printf("Strip Offsets = %lu\n", tiff->currentFrame.stripoffsets); 
		                } break;
                
				case 277: 
				{
					tiff->currentFrame.samplesperpixel=ifd.value; 
					printf("Sample per Pixel = %u\n", tiff->currentFrame.samplesperpixel); 
					break;
				}
		                case 278: 
				{
					tiff->currentFrame.rowsperstrip=ifd.value; 
					printf("Row per Strip = %u\n", tiff->currentFrame.rowsperstrip);
					break;
				}
				case 282: 
				{					
					printf("XResolution = %u\n", ifd.value);					
					break;
				}
				case 283: 
				{					
					printf("YResolution = %u\n", ifd.value);					
					break;
				}
		                case 339: 
				{
					tiff->currentFrame.sampleformat=ifd.value; 
					printf("Sample Format = %u\n", tiff->currentFrame.sampleformat);
					break;
				}
		                case 270: 
				{
					//printf("TIFF_FIELD_IMAGEDESCRIPTION: (tag: %u, type: %u, count: %u)\n", ifd.tag, ifd.type, ifd.count);
				        if (ifd.count>0) 
					{
                    				if (tiff->currentFrame.description) 
							free(tiff->currentFrame.description);
						tiff->currentFrame.description=(char*)calloc(ifd.count+1, sizeof(char));
			
                            			for (uint32_t ji=0; ji<ifd.count+1; ji++) 
						{
                                			tiff->currentFrame.description[ji]='\0';
                            			}
						for (uint32_t ji=0; ji<ifd.count; ji++) 
						{
							tiff->currentFrame.description[ji]=(char)ifd.pvalue[ji];
							printf(" %d[%d]", int(tiff->currentFrame.description[ji]), int(ifd.pvalue[ji]));
						}
						printf("\n  %s\n", tiff->currentFrame.description);
					}
                    		} break;
                
				case 279: 
				{
                        		tiff->currentFrame.stripcount=ifd.count;
                        		tiff->currentFrame.stripbytecounts=(uint32_t*)calloc(ifd.count, sizeof(uint32_t));
                        		memcpy(tiff->currentFrame.stripbytecounts, ifd.pvalue, ifd.count*sizeof(uint32_t));
					printf("Strip Byte Counts = %lu\n", tiff->currentFrame.stripbytecounts);
                    		} break;
                
				case 284: 
				{
					tiff->currentFrame.planarconfiguration=ifd.value; 
					printf("Planar Configuration = %u\n", tiff->currentFrame.planarconfiguration);
					break;         		
				}
				case 305: 
				{					
					printf("Software Type = %u\n", ifd.value);					
					break;
				}
				default: break;
            		}

        	}
        
		tiff->currentFrame.height=tiff->currentFrame.imagelength;
        	fseek(tiff->file, tiff->nextifd_offset+2+12*ifd_count, SEEK_SET);
        	tiff->nextifd_offset=ReadUint32(tiff);
        	//printf("      - nextifd_offset=%lu\n", tiff->nextifd_offset);
    	} 
	else 
	{
		printf("No more images in TIFF file\n");
    	}
}

TiffFile* OpenTiff(const char* filename) 
{
    	TiffFile* tiff=(TiffFile*)malloc(sizeof(TiffFile));

    	tiff->filesize=0;
    	struct stat file;
    
	if(stat(filename,&file)==0) 
	{
        	tiff->filesize=file.st_size;
    	}
    	
	tiff->currentFrame=GetEmptyFrame();
    	tiff->file=fopen(filename, "rb");
    	tiff->systembyteorder=ByteOrder();

	if ((tiff->file) && tiff->filesize>0)  
	{
        	uint8_t tiffid[3]={0,0,0};
        	fread(tiffid, 1,2,tiff->file);

        	printf("Header Byte = %s\n", tiffid);
        	
		//Read the first two bytes to determine byte order
      		if (tiffid[0]=='I' && tiffid[1]=='I') 
		{
			tiff->filebyteorder=2;
			printf("Byte Order is LITTLE ENDIAN \n");
		}
        	else if (tiffid[0]=='M' && tiffid[1]=='M') 
		{
			tiff->filebyteorder=1;
			printf("Byte Order is LITTLE ENDIAN \n");
		}
        	else 
		{
            		free(tiff);
			printf("0 Byte Order\n");
            		return NULL;
        	}
        	
		// Read the next two bytes to identify as TIFF File
		uint16_t TIFFIden=ReadUint16(tiff);
        	printf("TIFF Identifier = %u\n", TIFFIden);
        	if (TIFFIden!=42) 
		{
            		free(tiff);
			printf("Not a TIFF Format\n");
            		return NULL;
        	}
        
		tiff->firstrecord_offset=ReadUint32(tiff);
		tiff->nextifd_offset=tiff->firstrecord_offset;
		
		// Determine and Display the File Size and IFD Offset
        	printf("File size = %lu\n", tiff->filesize);
        	printf("First Record Offset = %4X\n", tiff->firstrecord_offset);
		ReadNextFrame(tiff);
    	} 
	else 
	{
        	free(tiff);
        	return NULL;
    	}
	
	return tiff;
}


int main()
{
	printf("Hello.....\n");
	printf("This code parses a TIFF Image and displays all the parameters and Header values.....\n\n");

	//printf("Please give the file name of the Image to be parsed : ");

	TiffFile* TFile=NULL;
   	TFile=OpenTiff("sample.tif"); 

   	if (!TFile) 
	{ 
        	printf("ERROR reading TIFF file)\n"); 
   	} 
	else 
	{ 
		uint32_t width=TFile->currentFrame.width;
		uint32_t height=TFile->currentFrame.height;

	        uint16_t* image=(uint16_t*)calloc(width*height, sizeof(uint16_t));
		
		printf("Image Width = %d \n", width);
		printf("Image Height = %d \n", height);

	        free(image); 
	} 
    	
	//Close the Input File

	fclose(TFile->file);
        free(TFile);

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

void tag_check(char *buf, FILE* fp){
char value[4];
if(strcmp(buf,"0100")==0){
	printf("Image Width\t");
	fseek(fp, 16, SEEK_CUR);
        fread(value,sizeof(char),4,fp);
        fseek(fp, -20, SEEK_CUR);
	printf("%s\n", value);
	}
else if(strcmp(buf,"0101")==0){
        printf("Image Length\t");
        fseek(fp, 16, SEEK_CUR);
        fread(value,sizeof(char),4,fp);
        fseek(fp, -20, SEEK_CUR);
        printf("%s\n", value);
        }
else if(strcmp(buf,"0102")==0){
        printf("Image Length\t");
        fseek(fp, 16, SEEK_CUR);
        fread(value,sizeof(char),4,fp);
        fseek(fp, -20, SEEK_CUR);
        printf("%s\n", value);
        }
}

int main(){

	char ch;
	char *buffer=NULL;
	system("./tif-parser.sh");
	FILE *fp=fopen("tmp.ifd", "r");

	if (fseek(fp, 0L, SEEK_END) == 0) {
        long len = ftell(fp);

        /* Allocate our buffer to that size. */
        buffer = malloc(sizeof(char) * (len + 1));

        /* Go back to the start of the file. */
        fseek(fp, 0L, SEEK_SET);

        /* Read the entire file into memory. */
        size_t newLen = fread(buffer, sizeof(char), len, fp);
        if ( ferror( fp ) != 0 ) {
            fputs("Error reading file", stderr);
        }
	 else {
            buffer[newLen++] = '\0'; 
        }
	
	int i,j;
	printf("Number of directory entries = ");
	printf("%.*s\n", 4, buffer+0);
	fseek(fp, 4, SEEK_SET);
	char buf[5];
        //buf = malloc(sizeof(char) * 5);	
	for(i=0; i<=10; i++){
		fseek(fp, 24, SEEK_CUR);
        	fread(buf,sizeof(char),4,fp);
		fseek(fp, -4, SEEK_CUR);
		printf("%s\n", buf);
		tag_check(buf,fp);
	}
	

    }
   
    fclose(fp);
    free(buffer);
}


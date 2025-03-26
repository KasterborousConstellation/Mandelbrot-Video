#include "fileUtils.h"
char* read_entire_file(const char* filename,int* size){
    FILE* f = fopen(filename,"rb");
    if(!f){
        printf("The provided file doesn't exists\n");
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *string = malloc(filesize + 1);
    fread(string, filesize, 1, f);
    fclose(f);
    if(size){
        *size = filesize;
    }
    string[filesize] = '\0';
    return string;
}
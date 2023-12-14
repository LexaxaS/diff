#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "tools.hpp"


void bufClear(void)
    {
    while (getchar() != '\n') { ; }
    }


bool isZero(double x)
    {
    return fabs(x) < AT;
    }


bool areEqual(double a, double b)
    {
    return fabs(a - b) < AT;
    }

Text setbuf(char filename_i[])
    {
    assert(filename_i);

    Text text = {};

    FILE *fileEO = fileopenerR(filename_i);
    text.size = fileLen(filename_i);  
    char* buf = (char*) calloc(text.size + 1, sizeof(char));
    buf[text.size] = '\0';
    fread(buf, sizeof(char), text.size, fileEO);
    fclose(fileEO);      
    text.bufPtr = buf;
    text.nLines = countLines(buf);
    text.lines = setPtr(buf, text.nLines, text.size);

    // printBuf(text.lines);
    return text;
    }  

const char* FileToBuf(char* filename)
    {
    FILE* file = fileopenerR(filename);
    size_t size = fileLen(filename);
    char* buf = (char*) calloc(size + 1, sizeof(char));
    buf[size] = '\0';
    fread(buf, sizeof(char), size, file);
    fclose(file);

    return buf;
    }


String* splitBuf(Text* text, char terminator)
    {
    char* buf = text->bufPtr;
    size_t nLines = text->nLines;
    size_t flen = text->size;
    size_t buf_i = 0;
    size_t ptr_j = 1;

    String* pointers = (String*) calloc(nLines + 1, sizeof(*pointers));

    size_t lineLen = 0;

    pointers[0].linePtr = &buf[0];
    // printf("%s\n", buf);
    
    while (buf_i < flen)
        {
        if (buf[buf_i] == terminator)
            {
            if (lineLen == 0)
                {
                buf[buf_i++] = '\0';
                pointers[ptr_j - 1].linePtr = &(buf[buf_i]);
                }

            else
                {
                buf[buf_i++] = '\0';
                pointers[ptr_j - 1].length = lineLen;
                lineLen = 0;
                pointers[ptr_j++].linePtr = &(buf[buf_i]);
                }
            }

        else
            {
            buf_i++;
            lineLen++;
            }
        }

    pointers[ptr_j].linePtr = 0;
    text->lines = pointers;
    return pointers;
    }

String* setPtr(char* buf, size_t nLines, size_t flen)
    {

    size_t buf_i = 0;
    size_t ptr_j = 1;

    String* pointers = (String*) calloc(nLines + 1, sizeof(*pointers));

    size_t lineLen = 0;

    pointers[0].linePtr = &buf[0];
    // printf("%s\n", buf);
    
    while (buf_i < flen)
        {
        if (buf[buf_i] == '\r' || buf[buf_i] == '\n')
            {
            if (lineLen == 0)
                {
                ptr_j--;
                buf[buf_i] = '\0';
                buf_i += 1;
                while (isalpha(buf[buf_i]) == 0 && buf_i + 2 < flen && buf[buf_i] != ';')
                    {
                    buf_i += 1;
                    }
                pointers[ptr_j].linePtr = &(buf[buf_i]);
                ptr_j++;
                }

            else
                {
                buf[buf_i] = '\0';
                pointers[ptr_j - 1].length = lineLen;
                buf_i += 1;
                lineLen = 0;
                 while (isalpha(buf[buf_i]) == 0 && buf_i < flen && buf[buf_i] != ';')
                    {
                    buf_i += 1;
                    }
                pointers[ptr_j].linePtr = &(buf[buf_i]);
                ptr_j++;
                }
            }

        else
            {
            buf_i++;
            lineLen++;
            }
        }

    pointers[ptr_j].linePtr = 0;
    return pointers;
    }

size_t countLines(const char* str)
    {
    size_t nlines = 1;
    const char* terPtr = strchr(str, '\n');
    while (terPtr != NULL)
        {
        if (*(terPtr + 1) != '\n' && *(terPtr + 1) != '\r')
            nlines++;
        terPtr = strchr(terPtr + 1, '\n');
        }
    return nlines;
    }
 
void printBuf(String* pointers)
    {
    assert(pointers);
    while (pointers->linePtr != NULL)
        {
        printf("<%s>         len = %d\n", pointers->linePtr, pointers->length);
        pointers++;
        }
    }

void bufReturn(char* buf, size_t flen)
    {
    assert(buf);
    assert(flen);

    size_t buf_i = 0;
    while (buf_i + 2 < flen)
        {
        if (buf[buf_i] == '\0')
            {
            buf[buf_i] = '\r';
            buf_i++;
            }
        else
            {
            buf_i++;
            }
        }
    }

FILE *fileopenerR(char filename[])
    {
    assert(filename);

    FILE *fileEO = fopen(filename, "r");
    if (fileEO == NULL)
        {
        perror("FILE_ERROR");
        }
    return fileEO;
    }

FILE *fileopenerRB(char filename[])
    {
    assert(filename);

    FILE *fileEO = fopen(filename, "rb");
    if (fileEO == NULL)
        {
        printf("FILE_ERROR\n");
        }
    return fileEO;
    }

FILE *fileopenerW(char filename[])
    {
    assert(filename);

    FILE *fileEO = fopen(filename, "w");
    if (fileEO == NULL)
        {
        printf("FILE_ERROR\n");
        }
    return fileEO;
    }

FILE *fileopenerWB(char filename[])
    {
    assert(filename);

    FILE *fileEO = fopen(filename, "wb");
    if (fileEO == NULL)
        {
        printf("FILE_ERROR\n");
        }
    return fileEO;
    }


size_t fileLen(const char * file)
    {
    assert(file);
    
    struct stat st = {};
    if (stat(file, &st) == 0)
        return (size_t) st.st_size;
    return 0;
    }


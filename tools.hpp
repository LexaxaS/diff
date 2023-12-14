#ifndef TOOLS_H
#define TOOLS_H 

#include <stdio.h>

typedef int error_t; 

#define MY_ASSERT_SOFT( cond, error, ... )                                                                           \
    {                                                                                                                \
    if (!(cond))                                                                                                     \
        {                                                                                                            \
        printf ("\nERROR in %s in line %d in function %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);     \
        __VA_ARGS__;                                                                                                 \
        return error;                                                                                                \
        }                                                                                                            \
    }

#define MY_ASSERT_HARD( cond, ... )                                                                                  \
    {                                                                                                                \
    if (!(cond))                                                                                                     \
        {                                                                                                            \
        printf ("\nERROR in %s in line %d in function %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);               \
        __VA_ARGS__;                                                                                                 \
        abort();                                                                                                     \
        }                                                                                                            \
    }


#define max(x, y)                                                                                                   \
({                                                                                                                  \
    __typeof__(x) _tx = x; __typeof__(y) _ty = y;                                                                   \
    _tx > _ty ? _tx : _ty;                                                                                          \
})


#define min(x, y)                                                                                                   \
({                                                                                                                  \
    __typeof__(x) _tx = x; __typeof__(y) _ty = y;                                                                   \
    _tx < _ty ? _tx : _ty;                                                                                          \
})


#define ISERROR(...)                    \
    {                                   \
    error_t error = __VA_ARGS__;        \
    if (error != NO_ERROR)              \
        return error;                   \
    }

#define RETURNwPR(error)                                                            \
    {                                                                               \
    if (error != NO_ERROR)                                                          \
        printf("error in %s line %d\n", __FILE__, __LINE__ - 1);                    \
    return error;                                                                   \
    }

#define ERR_PRINT(...)                                                      \
    {                                                                       \
    printf("error in line %d in file %S\n", __LINE__, __FILE__);            \
    __VA_ARGS__;                                                            \
    }

#define SIZE_MAX 1000000

struct String
{
    char* linePtr;
    size_t length;
};

struct Text
{
    char* bufPtr;
    String* lines;
    size_t size;
    size_t nLines;
    FILE* file;
};

enum DEFAULT_ERRORS
    {
    QUIT = -1,
    NO_ERROR = 0,
    ERR_NULL_PTR = 1,
    ERR_ALLOC_ERROR = 2, 
    ERR_TOO_BIG_IND = 3,
    ERR_SYNTAX_ERROR = 4,
    ERR_TREE_LOOP,
    };

const double AT = 1e-6;

size_t countLines(const char* str);
size_t fileLen(const char * file);
bool isZero(double x);
bool areEqual(double a, double b);

Text setbuf(char filename_i[]);
String* setPtr(char* buf, size_t nLines, size_t flen);
String* splitBuf(Text* text, char terminator);
const char* FileToBuf(char* filename);

void printBuf(String* pointers);
void bufReturn(char* buf, size_t flen);
void bufClear(void);

FILE *fileopenerR(char filename[]);
FILE *fileopenerRB(char filename[]);
FILE *fileopenerW(char filename[]);
FILE *fileopenerWB(char filename[]);


#endif

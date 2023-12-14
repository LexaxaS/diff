#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "tools.hpp"
#include "difftree.hpp"
#include "diffTreeDump.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define CreateCmdNode(cmd) DiffCreateCmdNode(cmd, nullptr, nullptr).node

const char s[] = "1000-7*100/(30+5*10-5*(100/50))+1";
int p = 0;

static DiffNode* _DiffElemParse(const char* string, size_t* pos);

DiffNode* GetG(const char* string);
DiffNode* GetN(const char* string, size_t* pos);
DiffNode* GetE(DiffNode** tokens, size_t* pos);
DiffNode* GetT(DiffNode** tokens, size_t* pos);
DiffNode* GetP(DiffNode** tokens, size_t* pos);
DiffNode* GetName(DiffNode** tokens, size_t* pos);


DiffNode** DiffTokenator(const char* string)
    {
    size_t lenstr = strlen(string);
    DiffNode** tokens = (DiffNode**) calloc(lenstr + 1, sizeof(DiffNode*));
    size_t pos = 0;
    size_t tknpos = 0;

    while (true)
        {
        printf("[%c]\n", string[pos]);
        
        if (string[pos] == '\0')
            {
            tokens[tknpos++] = CreateCmdNode(HLT);
            break;
            }

        else if (isspace(string[pos]))
            pos++;

        else if (isdigit(string[pos]))
            tokens[tknpos++] = GetN(string, &pos);

        else 
            tokens[tknpos++] = _DiffElemParse(string + pos, &pos);
        }

    return tokens;
    }

static DiffNode* _DiffElemParse(const char* string, size_t* pos)
    {
    if (*string == '+')
        {
        (*pos)++;
        return CreateCmdNode(ADD);
        }

    else if (*string == '-')
        {   
        (*pos)++;
        return CreateCmdNode(SUB);
        }

    else if (*string == '*')
        {
        (*pos)++;
        return CreateCmdNode(MUL);
        }

    else if (*string == '/')
        {
        (*pos)++;
        return CreateCmdNode(DIV);
        }

    else if (*string == '^')
        {
        (*pos)++;
        return CreateCmdNode(POW);
        }

    else if (*string == '(')
        {
        (*pos)++;
        return CreateCmdNode(OP_BR);
        }

    else if (*string == ')')
        {
        (*pos)++;
        return CreateCmdNode(CL_BR);
        }

    else if (strncasecmp(string, "sin", 3) == 0)
        {
        *pos += 3;
        return CreateCmdNode(SIN);
        }

    else if (strncasecmp(string, "cos", 3) == 0)
        {
        *pos += 3;
        return CreateCmdNode(COS);
        }

    else if (strncasecmp(string, "tg", 2) == 0)
        {
        *pos += 2;
        return CreateCmdNode(TG);
        }

    else if (strncasecmp(string, "ctg", 3) == 0)
        {
        *pos += 3;
        return CreateCmdNode(CTG);
        }

    else if (strncasecmp(string, "ln", 2) == 0)
        {
        *pos += 2;
        return CreateCmdNode(LN);
        }

    else if (strncasecmp(string, "arcsin", 6) == 0)
        {
        *pos += 6;
        return CreateCmdNode(ARCSIN);
        }

    else if (strncasecmp(string, "arccos", 6) == 0)
        {
        *pos += 6;
        return CreateCmdNode(ARCCOS);
        }

    else if (strncasecmp(string, "arctg", 5) == 0)
        {
        *pos += 5;
        return CreateCmdNode(ARCTG);
        }

    else if (strncasecmp(string, "arcctg", 6) == 0)
        {
        *pos += 6;
        return CreateCmdNode(ARCCTG);
        }

    else if (*string == 'x')
        {
        (*pos)++;
        return DiffCreateVarNode('x', nullptr, nullptr).node;
        }

    else
        return nullptr;
    
    return nullptr;
    }

DiffNode* GetN(const char* string, size_t* pos)
    {
    double val = 0;

    while ('0' <= string[*pos] && string[*pos] <= '9')
        {
        val = val * 10 + string[*pos] - '0';
        (*pos)++;
        }
        
    return DiffCreateCnstNode(val, nullptr, nullptr).node;
    }

DiffNode* GetG(const char* string)
    {
    size_t pos = 0;
    printf("beforetok\n");

    DiffNode** tokens = DiffTokenator(string);

    printf("aftertok\n");
    DiffNode* val = GetE(tokens, &pos);

    return val;
    }

DiffNode* GetE(DiffNode** tokens, size_t* pos)
    {
    DiffNode* node1 = GetT(tokens, pos);

    while (tokens[*pos]->type == COMMAND && (tokens[*pos]->value.cmd == ADD || tokens[*pos]->value.cmd == SUB))
        {
        difCommands op = tokens[*pos]->value.cmd;
        (*pos)++;
        printf("op = %d\n", tokens[*pos]->type);
        DiffNode* node2 = GetT(tokens, pos);

        node1 = DiffCreateCmdNode(op, node1, node2).node;
        }

    return node1;
    }

DiffNode* GetT(DiffNode** tokens, size_t* pos)
    {
    DiffNode* node1 = GetP(tokens, pos);
    while (tokens[*pos]->type == COMMAND && (tokens[*pos]->value.cmd == MUL || tokens[*pos]->value.cmd == DIV || tokens[*pos]->value.cmd == POW))
        {
        difCommands op = tokens[*pos]->value.cmd;
        (*pos)++;
        DiffNode* node2 = GetP(tokens, pos);

        node1 = DiffCreateCmdNode(op, node1, node2).node;
        }

    return node1;
    }

DiffNode* GetName(DiffNode** tokens, size_t* pos)
    {
    if (tokens[*pos]->type == COMMAND && tokens[*pos]->value.cmd >= SIN)
        {
        difCommands op = tokens[*pos]->value.cmd;
        (*pos)++;
        if (tokens[*pos]->type == COMMAND && tokens[*pos]->value.cmd == OP_BR)
            {
            (*pos)++;
            DiffNode* node = GetE(tokens, pos);
            node = DiffCreateCmdNode(op, nullptr, node).node;
            if (tokens[*pos]->type == COMMAND && tokens[*pos]->value.cmd == CL_BR)
                {
                (*pos)++;
                return node;
                }

            printf("noclbr %d\n", tokens[*pos]->type);
            return nullptr;
            }

        printf("noopbr\n");
        return nullptr;
        }
    return nullptr;
    }

DiffNode* GetP(DiffNode** tokens, size_t* pos)
    {
    if (tokens[*pos]->type == COMMAND && tokens[*pos]->value.cmd == OP_BR)
        {
        (*pos)++;
        DiffNode* val = GetE(tokens, pos);
        if (tokens[*pos]->type == COMMAND && tokens[*pos]->value.cmd == CL_BR)
            {
            (*pos)++;
            return val;
            }

        return nullptr;
        }

    else if (tokens[*pos]->type == CONST)
        {
        (*pos)++;
        return tokens[*pos - 1];
        }

    else if (tokens[*pos]->type == VAR)
        {
        (*pos)++;
        return tokens[*pos - 1];        
        }

    else 
        return GetName(tokens, pos);
    }


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "tools.hpp"
#include "difftree.hpp"
#include "differentiator.hpp"
#include "diffTreeDump.hpp"

#define difL DifferentiateTree(DiffCopyNode(node->left), filedest)
#define difR DifferentiateTree(DiffCopyNode(node->right), filedest)

#define copyL DiffCopyNode(node->left)
#define copyR DiffCopyNode(node->right)

#define ADD(left, right) DiffCreateCmdNode(ADD, left, right).node
#define SUB(left, right) DiffCreateCmdNode(SUB, left, right).node
#define MUL(left, right) DiffCreateCmdNode(MUL, left, right).node
#define DIV(left, right) DiffCreateCmdNode(DIV, left, right).node
#define POW(left, right) DiffCreateCmdNode(POW, left, right).node

#define SIN(val)         DiffCreateCmdNode(SIN,  nullptr, val).node
#define COS(val)         DiffCreateCmdNode(COS,  nullptr, val).node
#define TG(val)          DiffCreateCmdNode(TG,  nullptr, val).node
#define CTG(val)         DiffCreateCmdNode(CTG,  nullptr, val).node
#define LN(val)          DiffCreateCmdNode(LN,  nullptr, val).node

#define CNST(val)        DiffCreateCnstNode(val, nullptr, nullptr).node

size_t _recDiffOptimise(DiffNode* node);
bool _DiffOptimiseDelNeutral(DiffNode* node);
bool _DiffOptimiseCountCnst(DiffNode* node);
bool _OptMulDiv(DiffNode* node);
bool _OptAddSub(DiffNode* node);

DiffNode* diffAdd(DiffNode* node, FILE* filedest);
DiffNode* diffSub(DiffNode* node, FILE* filedest);
DiffNode* diffMul(DiffNode* node, FILE* filedest);
DiffNode* diffDiv(DiffNode* node, FILE* filedest);
DiffNode* diffPow(DiffNode* node, FILE* filedest);
DiffNode* diffSin(DiffNode* node, FILE* filedest);
DiffNode* diffCos(DiffNode* node, FILE* filedest);
DiffNode* diffTg(DiffNode* node, FILE* filedest);
DiffNode* diffCtg(DiffNode* node, FILE* filedest);
DiffNode* diffLn(DiffNode* node, FILE* filedest);
DiffNode* diffArcSin(DiffNode* node, FILE* filedest);
DiffNode* diffArcCos(DiffNode* node, FILE* filedest);
DiffNode* diffArcTg(DiffNode* node, FILE* filedest);
DiffNode* diffArcCtg(DiffNode* node, FILE* filedest);

DiffNode* DifferentiateTree(DiffNode* curNode, FILE* filedest)
    {
    if (!curNode)
        return nullptr;

    switch (curNode->type)
        {
        case CONST:
            printf("const %lg\n", curNode->value.cnst);
            return DiffCreateCnstNode(0, nullptr, nullptr).node;
            break;
        case VAR:
            printf("var = x\n");
            return DiffCreateCnstNode(1, nullptr, nullptr).node;
            break;
        case COMMAND:
            switch (curNode->value.cmd)
                {
                case ADD:       return diffAdd(curNode, filedest);
                case SUB:       return diffSub(curNode, filedest);
                case MUL:       return diffMul(curNode, filedest);
                case DIV:       return diffDiv(curNode, filedest);
                case POW:       return diffPow(curNode, filedest);
                case SIN:       return diffSin(curNode, filedest);
                case COS:       return diffCos(curNode, filedest);
                case TG:        return diffTg(curNode, filedest);
                case CTG:       return diffCtg(curNode, filedest);
                case LN:        return diffLn(curNode, filedest);
                case ARCSIN:    return diffArcSin(curNode, filedest);
                case ARCCOS:    return diffArcCos(curNode, filedest);
                case ARCTG:     return diffArcTg(curNode, filedest);
                case ARCCTG:    return diffArcCtg(curNode, filedest);
                default: printf("cmd unknown\n");
                }
            break;
        }

    ERR_PRINT();
    return nullptr;
    }

error_t DiffOptimise(DiffTree* tree)
    {
    size_t ischange = _recDiffOptimise(tree->root);

    while (ischange)
        ischange = _recDiffOptimise(tree->root);
    
    return RecountNodes(tree);
    }

size_t _recDiffOptimise(DiffNode* node)
    {
    if (node == nullptr)
        return false;
    
    size_t ischange = 0;
    
    ischange += _recDiffOptimise(node->left);
    ischange += _recDiffOptimise(node->right);

    if (_DiffOptimiseCountCnst(node))
        ischange++;

    // DiffTree tree = {};
    // size_t s = 10;
    // tree.root = node;
    // tree.size = &s;
    // TreeGraphicDump(&tree);

    if (_DiffOptimiseDelNeutral(node))
        ischange++;

    // TreeGraphicDump(&tree);
    return ischange;
    }


error_t _UpdateNode(DiffNode* node)
    {
    node->type = CONST;
    node->subtreeLen = 1;

    NodeDestruct(node->left);
    NodeDestruct(node->right);

    node->left = nullptr;
    node->right = nullptr;
    return NO_ERROR;
    }

bool _DiffOptimiseCountCnst(DiffNode* node)
    {
    if (node->right == nullptr || node->left == nullptr)
        return 0;
    
    if (node->type == COMMAND && node->left->type == CONST && node->right->type == CONST)
        {
        switch (node->value.cmd)
            {
            case ADD: 
                node->value.cnst = node->left->value.cnst + node->right->value.cnst;
                _UpdateNode(node);
                return true;
            case SUB: 
                node->value.cnst = node->left->value.cnst - node->right->value.cnst;
                _UpdateNode(node);
                return true;
            case MUL: 
                node->value.cnst = node->left->value.cnst * node->right->value.cnst;
                _UpdateNode(node);
                return true;
            case DIV: 
                node->value.cnst = node->left->value.cnst / node->right->value.cnst;
                _UpdateNode(node);
                return true;
            case POW: 
                node->value.cnst = pow(node->left->value.cnst, node->right->value.cnst);
                _UpdateNode(node);
                return true;
            default: return false;
            }
        }

    return false;
    }

bool _DiffOptimiseDelNeutral(DiffNode* node)
    {
    if (node->right == nullptr || node->left == nullptr)
        return 0;
    
    bool ischange = false;
    ischange += _OptMulDiv(node);
    ischange += _OptAddSub(node);
    return false;
    }

bool _OptMulDiv(DiffNode* node)
    {
    if (node->type == COMMAND && (node->value.cmd == MUL || node->value.cmd == DIV))
        {
        if ((node->left->type == CONST && node->left->value.cnst == 0) || (node->right->type == CONST && node->right->value.cnst == 0))
            {
            node->type = CONST;
            node->value.cnst = 0;
            node->subtreeLen = 0;

            NodeDestruct(node->left);
            NodeDestruct(node->right);

            node->left = nullptr;
            node->right = nullptr;

            return true;
            }

        else if (node->value.cmd == MUL && node->left->type == CONST && node->left->value.cnst == 1 && node->left->subtreeLen == 1)
            {
            node->type = node->right->type;
            node->value = node->right->value;
            node->subtreeLen = node->right->subtreeLen;
            
            NodeDestruct(node->left);
            DiffNode* oldrn = node->right;

            node->left = node->right->left;
            if (node->right->left)
                node->right->left->parent = node;

            node->right = node->right->right;
            if (node->right->right)
                node->right->right->parent = node;

            free(oldrn);
            return true;
            }
        
        else if (node->right->type == CONST && node->right->value.cnst == 1 && node->right->subtreeLen == 1)
            {
            node->type = node->left->type;
            node->value = node->left->value;
            node->subtreeLen = node->left->subtreeLen;

            NodeDestruct(node->right);
            DiffNode* oldln = node->left;

            node->right = node->left->right;
            if (node->left->right)
                node->left->right->parent = node;

            node->left = node->left->left;
            if (node->left->left)
                node->left->left->parent = node;

            
            free(oldln);
            return true;
            }
        }
    return false;
    }

bool _OptAddSub(DiffNode* node)
    {
    if (node->type == COMMAND && (node->value.cmd == ADD || node->value.cmd == SUB))
        {
        if (node->value.cmd == ADD && node->left->type == CONST && node->left->value.cnst == 0)
            {
            node->type = node->right->type;
            node->value = node->right->value;
            node->subtreeLen = node->right->subtreeLen;

            NodeDestruct(node->left);
            DiffNode* oldrn = node->right;

            node->left = node->right->left;
            if (node->right->left)
                node->right->left->parent = node;

            node->right = node->right->right;
            if (node->right->right)
                node->right->right->parent = node;

            free(oldrn);
            return true;
            }

        else if (node->right->type == CONST && node->right->value.cnst == 0)
            {
            node->type = node->left->type;
            node->value = node->left->value;
            node->subtreeLen = node->left->subtreeLen;

            NodeDestruct(node->right);
            DiffNode* oldln = node->left;

            node->right = node->left->right;
            if (node->left->right)
                node->left->right->parent = node;

            node->left = node->left->left;
            if (node->left->left)
                node->left->left->parent = node;

            free(oldln);
            return true;
            }
        }
    return false;
    }

DiffNode* diffAdd(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = ADD(difL, difR);
    return newNode;
    }

DiffNode* diffSub(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = SUB(difL, difR);
    return newNode;
    }

DiffNode* diffMul(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = ADD(MUL(difL, copyR), MUL(copyL, difR));
    return newNode;
    }

DiffNode* diffDiv(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = DIV(SUB(MUL(difL, copyR), MUL(copyL, difR)), POW(copyR, CNST(2)));
    return newNode;
    }

DiffNode* diffPow(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = MUL(MUL(POW(copyL, SUB(copyR, CNST(1))), copyR), difL);
    return newNode;
    }

DiffNode* diffSin(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = MUL(COS(copyR), difR);
    return newNode;
    }

DiffNode* diffCos(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = MUL(MUL(CNST(-1), SIN(copyR)), difR);
    return newNode;
    }

DiffNode* diffTg(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = MUL(DIV(CNST(1), POW(COS(copyR), CNST(2))), difR);
    return newNode;
    }

DiffNode* diffCtg(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = MUL(DIV(CNST(-1), POW(SIN(copyR), CNST(2))), difR);
    return newNode;
    }

DiffNode* diffLn(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = MUL(DIV(CNST(1), copyR), difR);
    return newNode;
    }

DiffNode* diffArcSin(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = MUL(DIV(CNST(1), POW(SUB(CNST(1), POW(copyR, CNST(2))), CNST(0.5))), difR);
    return newNode;
    }

DiffNode* diffArcCos(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = MUL(DIV(CNST(-1), POW(SUB(CNST(1), POW(copyR, CNST(2))), CNST(0.5))), difR);
    return newNode;
    }

DiffNode* diffArcTg(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = MUL(DIV(CNST(1), ADD(CNST(1), POW(copyR, CNST(2)))), difR);
    return newNode;
    }

DiffNode* diffArcCtg(DiffNode* node, FILE* filedest)
    {
    DiffNode* newNode = MUL(DIV(CNST(-1), ADD(CNST(1), POW(copyR, CNST(2)))), difR);
    return newNode;
    }

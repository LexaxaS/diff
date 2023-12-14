#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "tools.hpp"
#include "difftree.hpp"
#include "diffTreeDump.hpp"

static size_t NODE_ID = 1;

static char* _DiffGetStr(Text* text, size_t* elnum);

static DiffNodewErr _recDiffTreeRead(String** lines);

static error_t _DiffElemParse(char* string, DiffNode* node);

static error_t _DiffTreeBufReader(Text* text);


error_t DiffTreeRead(DiffTree* tree, char* filename)
    {
    Text text = setbuf(filename);
    _DiffTreeBufReader(&text);

    // for (size_t i = 0; i < text.nLines; i++)
    //     printf("%s, %d\n", text.lines[i].linePtr, text.lines[i].length);

    String** lines = &text.lines;
    DiffNodewErr rootRes = _recDiffTreeRead(lines);

    if (rootRes.error != NO_ERROR)
        RETURNwPR(rootRes.error);

    DiffNode* root = rootRes.node;
    tree->root = root;

    RecountNodes(tree);

    return NO_ERROR;
    }

static error_t _DiffTreeBufReader(Text* text)
    {
    char* buf = text->bufPtr;
    // printf("buf = %s\n", buf);
    
    size_t flen = text->size;
    size_t buf_i = 0;
    size_t ptr_j = 1;

    String* pointers = (String*) calloc(flen + 1, sizeof(*pointers));

    size_t lineLen = 0;

    pointers[0].linePtr = &buf[0];

    while (buf_i < flen)
        {
        if (buf[buf_i] == TREE_TERMINATOR)
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
    text->nLines = ptr_j;

    return NO_ERROR;
    }


static error_t _recNodeDestruct(DiffNode* node)
    {
    MY_ASSERT_HARD(node);

    if (node->id == ZEROID)
        ISERROR(ERR_TREE_LOOP);
    node->id = 0;

    if (node->left)
        {
        if (node->left->parent != node)
            ISERROR(ERR_TREE_LOOP);
        _recNodeDestruct(node->left);
        }

    if (node->right)
        {
        if (node->right->parent != node)
            ISERROR(ERR_TREE_LOOP);
        _recNodeDestruct(node->right);
        }
    
    node->value = {};
    node->subtreeLen = 0;

    node->left = nullptr;
    node->right = nullptr;
    node->parent = nullptr;

    free(node);
    return NO_ERROR;
    }

static error_t _updateSubtreeLen(DiffNode* node, ssize_t delta)
    {
    MY_ASSERT_HARD(node);

    if (node->id == ZEROID)
        ISERROR(ERR_TREE_LOOP);
    
    size_t nodeid = node->id;
    node->id = ZEROID;

    node->subtreeLen += delta;

    if (node->parent)
        {
        if (node->parent->right != node && node->parent->left != node)
            ISERROR(ERR_TREE_LOOP);
        _updateSubtreeLen(node->parent, delta);
        }
    
    node->id = nodeid;

    return NO_ERROR;
    }

error_t NodeDestruct(DiffNode* node)
    {
    MY_ASSERT_HARD(node);

    if (node->parent)
        {
        if (node->parent->left == node)
            {
            if (node->parent->right == node)
                ISERROR(ERR_TREE_LOOP);
            node->parent->left = nullptr;
            }
        else if (node->parent->right == node)
            node->parent->right = nullptr;
        else
            ISERROR(ERR_TREE_LOOP);
        _updateSubtreeLen(node->parent, - (ssize_t) node->subtreeLen);
        }

    return _recNodeDestruct(node);
    }

static DiffNodewErr _recDiffTreeRead(String** lines)
    {
    if (*(*lines)->linePtr != '(')
        return {nullptr, NO_ERROR};
    (*lines)++;

    DiffNode* node = DiffCreateNode({}, CONST, nullptr, nullptr).node;

    DiffNode* leftnode = nullptr;
    if (*(*lines)->linePtr == '(')
        leftnode = _recDiffTreeRead(lines).node;
    

    if (_DiffElemParse((*lines)->linePtr, node) != NO_ERROR)
        return {nullptr, ERR_SYNTAX_ERROR}; 
    (*lines)++;

    DiffNode* rightnode = nullptr;
    if (*(*lines)->linePtr == '(')
        rightnode = _recDiffTreeRead(lines).node;
    
    if (*(*lines)->linePtr != ')')
        return {nullptr, ERR_SYNTAX_ERROR};
    (*lines)++;

    node->left = leftnode;
    node->right = rightnode;
    
    if (leftnode)
        leftnode->parent = node;

    if (rightnode)
        rightnode->parent = node;

    return {node, NO_ERROR};
    }

DiffNodewErr DiffCreateCmdNode(difCommands cmd, DiffNode* left, DiffNode* right)
    {
    return DiffCreateNode({.cmd = cmd}, COMMAND, left, right);
    }

DiffNodewErr DiffCreateVarNode(char var, DiffNode* left, DiffNode* right)
    {
    return DiffCreateNode({.var = var}, VAR, left, right);
    }

DiffNodewErr DiffCreateCnstNode(double cnst, DiffNode* left, DiffNode* right)
    {
    return DiffCreateNode({.cnst = cnst}, CONST, left, right);
    }

static error_t _DiffCmdEmit(DiffNode* node, difCommands cmd)
    {
    node->value.cmd = cmd;
    node->type = COMMAND;

    return NO_ERROR;
    }

static error_t _DiffVarEmit(DiffNode* node)
    {
    node->value.var = 'x';
    node->type = VAR;

    return NO_ERROR;
    }

static error_t _DiffConstEmit(DiffNode* node, char* string)
    {
    double cnst = 0;
    sscanf(string, "%lg", &cnst);

    node->value.cnst = cnst;
    node->type = CONST;

    return NO_ERROR;
    }

static error_t _DiffElemParse(char* string, DiffNode* node)
    {
    if (strcasecmp(string, "+") == 0)
        _DiffCmdEmit(node, ADD);
    else if (strcasecmp(string, "-") == 0)
        _DiffCmdEmit(node, SUB);
    else if (strcasecmp(string, "*") == 0)
        _DiffCmdEmit(node, MUL);
    else if (strcasecmp(string, "/") == 0)
        _DiffCmdEmit(node, DIV);
    else if (strcasecmp(string, "^") == 0)
        _DiffCmdEmit(node, POW);
    else if (strcasecmp(string, "sin") == 0)
        _DiffCmdEmit(node, SIN);
    else if (strcasecmp(string, "cos") == 0)
        _DiffCmdEmit(node, COS);
    else if (strcasecmp(string, "tg") == 0)
        _DiffCmdEmit(node, TG);
    else if (strcasecmp(string, "ctg") == 0)
        _DiffCmdEmit(node, CTG);
    else if (strcasecmp(string, "ln") == 0)
        _DiffCmdEmit(node, LN);
    else if (strcasecmp(string, "arcsin") == 0)
        _DiffCmdEmit(node, ARCSIN);
    else if (strcasecmp(string, "arccos") == 0)
        _DiffCmdEmit(node, ARCCOS);
    else if (strcasecmp(string, "arctg") == 0)
        _DiffCmdEmit(node, ARCTG);
    else if (strcasecmp(string, "arcctg") == 0)
        _DiffCmdEmit(node, ARCCTG);
    else if (strcasecmp(string, "x") == 0)
        _DiffVarEmit(node);
    else if (isdigit(*string))
        _DiffConstEmit(node, string);
    else
        return ERR_SYNTAX_ERROR;
    
    return NO_ERROR;
    }

char* getCmdName(difCommands cmd)
    {
    switch (cmd)
        {
        case ADD:
            return "+";
        
        case SUB:
            return "-";

        case MUL:
            return "*";
        
        case DIV:
            return "/";
        
        case SIN:
            return "sin";
        
        case COS:
            return "cos";
        
        case TG:
            return "tg";
        
        case CTG:
            return "ctg";

        case POW:
            return "^";

        case LN:
            return "ln";
        
        case ARCSIN:
            return "arcsin";

        case ARCCOS:
            return "arccos";
        
        case ARCTG:
            return "arctg";

        case ARCCTG:
            return "arcctg";

        case OP_BR:
            return "(";
        
        case CL_BR:
            return ")";

        default:
            return "UNKNOWN";
        }
    }

error_t DiffChangeNode(DiffNode* node, DiffNode* left, DiffNode* right)
    {
    node->left = left;
    if (left)
        left->parent = node;
        
    node->right = right;
    if (right)
        right->parent = node;
    
    return NO_ERROR;
    }

DiffNode* DiffCopyNode(DiffNode* node)
    {
    if (!node)
        return nullptr;
    
    DiffNode* newNode = (DiffNode*) calloc(1, sizeof(DiffNode));
    if (!newNode)
        return nullptr;

    newNode->type = node->type;

    newNode->id = NODE_ID++;

    switch (node->type)
        {
        case CONST:
            newNode->value.cnst = node->value.cnst;
            break;
        case COMMAND:
            newNode->value.cmd = node->value.cmd;
            break;
        case VAR:
            newNode->value.var = node->value.var;
            break;
        default:
            printf("%d\n", __LINE__);
            return nullptr;
        }

    DiffNode* leftnode = DiffCopyNode(node->left);
    DiffNode* rightnode = DiffCopyNode(node->right);

    newNode->left = leftnode;
    newNode->right = rightnode;

    printf("cmd = %s, var = %c", getCmdName(newNode->value.cmd), newNode->value.var);

    // DiffTree tree = {};
    // size_t s = 10;
    // tree.root = node;
    // tree.size = &s;
    // TreeGraphicDump(&tree);
    
    return newNode;
    }

DiffNodewErr DiffCreateNode(DiffElem_t value, difType type, DiffNode* leftnode, DiffNode* rightnode)
    {
    DiffNode* node = (DiffNode*) calloc(1, sizeof(DiffNode));
    if (!node)
        return {nullptr, ERR_ALLOC_ERROR};

    node->left = leftnode;
    if (leftnode)
        leftnode->parent = node;

    node->right = rightnode;
    if (rightnode)
        rightnode->parent = node;

    node->id = NODE_ID++;

    node->value = value;
    node->type = type;

    // DiffTree tree = {};
    // size_t s = 10;
    // tree.root = node;
    // tree.size = &s;
    // TreeGraphicDump(&tree);

    return {node, NO_ERROR};
    }

bool IsVarInSubtree(DiffNode* node)
    {
    if (!node)
        return false;

    if (node->type == VAR)
        return true;
    
    if (IsVarInSubtree(node->left))
        return true;

    if (IsVarInSubtree(node->right))
        return true;

    return false;
    }
    
static error_t _recRecountNotes(DiffNode* node)
    {
    MY_ASSERT_SOFT(node, ERR_NULL_PTR);

    if (node->id == ZEROID)
        RETURNwPR(ERR_TREE_LOOP);
    
    node->subtreeLen = 1;

    if (!node->left && !node->right)
        return NO_ERROR;
        
    size_t nodeid = node->id;
    node->id = ZEROID;
    
    if (node->left)
        {
        ISERROR(_recRecountNotes(node->left));
        node->subtreeLen += node->left->subtreeLen;
        }
    
    if (node->right)
        {
        ISERROR(_recRecountNotes(node->right));
        node->subtreeLen += node->right->subtreeLen;
        }

    node->id = nodeid;

    return NO_ERROR;
    }

error_t RecountNodes(DiffTree* tree)
    {
    MY_ASSERT_SOFT(tree, ERR_NULL_PTR);
    printf("recn = %d\n", tree->root->type);
    tree->size = &tree->root->subtreeLen;
    return _recRecountNotes(tree->root);
    }
    
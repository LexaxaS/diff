#ifndef DIFFTREE_CPP
#define DIFFTREE_CPP

#include "tools.hpp"

const double POISON_TREE = 123456789;
const size_t ZEROID = 0;

const size_t MAX_TREE_LEN = 128;
const size_t PATH_MAX_LEN = 128;
const size_t CMD_MAX_LEN = 256;

#define TREE_DOT_FOLDER "TREEDOT"
#define TREE_PNG_FOLDER "TREEIMG"

#define TREE_SEPARATOR ';'
#define TREE_TERMINATOR ' '

enum difCommands
    {
    OP_BR = -2,
    CL_BR = -1,
    HLT = 0,
    ADD = 1,
    SUB = 2,
    MUL,
    DIV,
    POW,
    SIN,
    COS,
    TG,
    CTG,
    LN,
    ARCSIN,
    ARCCOS,
    ARCTG,
    ARCCTG,
    };

enum difType
    {
    CONST = 1,
    VAR,
    COMMAND,
    };

struct DiffElem_t
    {
    double cnst;
    char var;
    difCommands cmd;
    };


struct DiffNode
    {
    DiffElem_t value;

    difType type;
    size_t id;
    size_t subtreeLen;

    DiffNode* parent;
    DiffNode* right;
    DiffNode* left;
    };

struct DiffTree
    {
    DiffNode* root;
    size_t* size;
    };

struct DiffNodewErr
    {
    DiffNode* node;
    error_t error;
    };


error_t DiffTreeRead(DiffTree* tree, char* filename);

DiffNodewErr DiffCreateNode(DiffElem_t value, difType type, DiffNode* leftnode, DiffNode* rightnode);
DiffNodewErr DiffCreateCmdNode(difCommands cmd, DiffNode* left, DiffNode* right);
DiffNodewErr DiffCreateVarNode(char var, DiffNode* left, DiffNode* right);
DiffNodewErr DiffCreateCnstNode(double cnst, DiffNode* left, DiffNode* right);

char* getCmdName(difCommands cmd);

bool IsVarInSubtree(DiffNode* node);

error_t RecountNodes(DiffTree* tree);
error_t NodeDestruct(DiffNode* node);
error_t DiffChangeNode(DiffNode* node, DiffNode* left, DiffNode* right);

DiffNode* DiffCopyNode(DiffNode* node);

#endif
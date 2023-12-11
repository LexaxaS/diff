#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "tools.hpp"
#include "difftree.hpp"

static size_t GRAPH_ID = 1;


error_t TreeVerify(DiffTree* tree)
    {
    if (!tree->root)
        return ERR_NULL_PTR;
    
    if (tree->root->parent)
        return ERR_TREE_LOOP;

    return NO_ERROR;
    }

#define FONT_SIZE "20"
#define FONT_NAME "\"Sans Bold Not-Rotated\""
#define BACK_GROUND_COLOR "\"#5e67d4\""
#define NODE_COLOR "\"#faa116\""
#define NODE_FRAME_COLOR "\"#000000\""
#define ROOT_COLOR "\"#c25a90\""
#define FREE_COLOR "\"#b9e793\""
#define TREE_COLOR "\"#007be9\""

static error_t _TreeGraphBuild(DiffNode* node, FILE* outDotFile, size_t curNum, size_t maxNum)
    {
    MY_ASSERT_HARD(outDotFile);

    if (!node)
        return NO_ERROR;

    if (curNum > maxNum)
        {
        printf("cur > max build\n");
        return NO_ERROR;
        }

    fprintf(outDotFile, "NODE_%zu[style = \"filled\", fillcolor = " NODE_COLOR ", ", node->id);
    if (node->value.cnst == POISON_TREE)
        fprintf(outDotFile, "label = \"{POISON}\"];\n");
    else
        if (node->type == VAR)
            fprintf(outDotFile, "label = \"{%c}\"];\n", node->value.var);
        else if (node->type == COMMAND)
            {
            char* cmd = getCmdName(node->value.cmd);
            fprintf(outDotFile, "label = \"{%s}\"];\n", cmd);
            // fprintf(outDotFile, "label = \"{%s|{<left>Left|<right>Right}}\"];\n", cmd);
            }
        else 
            fprintf(outDotFile, "label = \"{%lg}\"];\n", node->value.cnst);          

    _TreeGraphBuild(node->left, outDotFile, curNum++, maxNum);
    _TreeGraphBuild(node->right, outDotFile, curNum++, maxNum);

    return NO_ERROR;
    }

static error_t _TreeGraphDraw(DiffNode* node, FILE* outDotFile, size_t curNum, size_t maxNum)
    {
    MY_ASSERT_HARD(outDotFile);

    if (curNum > maxNum)
        {
        printf("cur = %d > max draw = %d, line %d\n", curNum, maxNum, __LINE__);
        return NO_ERROR;
        }

    if (!node)
        return NO_ERROR;

    if (node->left)
        fprintf(outDotFile, "NODE_%zu:left->NODE_%zu;\n", node->id, node->left->id);

    if (node->right)
        fprintf(outDotFile, "NODE_%zu:right->NODE_%zu;\n", node->id, node->right->id);

    _TreeGraphDraw(node->left, outDotFile, curNum++, maxNum);
    _TreeGraphDraw(node->right, outDotFile, curNum++, maxNum);

    return NO_ERROR;
    }

error_t TreeGraphicDump(DiffTree* tree)
    {
    MY_ASSERT_HARD(tree);

    // RecountNodes(tree);

    char outFilePath[PATH_MAX_LEN] = "";
    sprintf(outFilePath, "%s/treegraph%zu.dot", TREE_DOT_FOLDER, GRAPH_ID);

    FILE* outDotFile = fileopenerW(outFilePath);

    fprintf(outDotFile,
        "digraph\n"
        "{\n"
        "rankdir = TB;\n"
        "node[shape = record, color = " NODE_FRAME_COLOR ", fontname = " FONT_NAME ", fontsize = " FONT_SIZE "];\n"
        "bgcolor = " BACK_GROUND_COLOR ";\n" 
        "TREE[rank = \"min\", style = \"filled\", fillcolor = " TREE_COLOR ", "
        "label = \"{Tree|Error: %d|Size: %zu|<root>Root}\"];\n",
        TreeVerify(tree), *(tree->size));

    size_t maxNum = min(*tree->size, MAX_TREE_LEN);

    _TreeGraphBuild(tree->root, outDotFile, 1, maxNum);

    _TreeGraphDraw(tree->root, outDotFile, 1, maxNum);

    fprintf(outDotFile, "\n");
    fprintf(outDotFile, "TREE:root->NODE_%zu\n", tree->root->id);

    fprintf(outDotFile, "}\n");
    char command[CMD_MAX_LEN] = "";
    sprintf(command, "dot %s -Tpng -o %s/TreeGraph%zu.png", outFilePath, TREE_PNG_FOLDER, GRAPH_ID);
    // printf("\n%s\n", command);

    fclose(outDotFile);

    system(command);

    GRAPH_ID++;

    return NO_ERROR;
    }

#undef FONT_SIZE "10"
#undef FONT_NAME "\"Sans Bold Not-Rotated\""
#undef BACK_GROUND_COLOR "\"#5e67d4\""
#undef NODE_COLOR "\"#faa116\""
#undef NODE_FRAME_COLOR "\"#000000\""
#undef ROOT_COLOR "\"#c25a90\""
#undef FREE_COLOR "\"#b9e793\""
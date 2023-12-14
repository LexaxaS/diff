#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "tools.hpp"
#include "difftree.hpp"
#include "differentiator.hpp"
#include "diffTreeDump.hpp"
#include "recDes.hpp"

int main()
    {
    // DiffTree tree = {};
    // DiffTreeRead(&tree, "data.txt");
    // TreeGraphicDump(&tree);

    DiffTree tree3 = {};
    const char string[] = "1000-x";
    DiffNode* root3 = GetG(string);
    tree3.root = root3;
    RecountNodes(&tree3);

    TreeGraphicDump(&tree3);

    DiffTree tree2 = {};
    DiffNode* root2 = DifferentiateTree(tree3.root, nullptr);
    tree2.root = root2;
    RecountNodes(&tree2);
    TreeGraphicDump(&tree2);

    DiffOptimise(&tree2);
    TreeGraphicDump(&tree2);
    
    return 0;
    }
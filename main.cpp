#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "tools.hpp"
#include "difftree.hpp"
#include "differentiator.hpp"
#include "diffTreeDump.hpp"

int main()
    {
    DiffTree tree = {};
    DiffTreeRead(&tree, "data.txt");
    TreeGraphicDump(&tree);

    DiffTree tree2 = {};
    DiffNode* root2 = DifferentiateTree(tree.root, nullptr);
    tree2.root = root2;
    RecountNodes(&tree2);
    TreeGraphicDump(&tree2);

    DiffOptimise(&tree2);
    TreeGraphicDump(&tree2);
    
    return 0;
    }
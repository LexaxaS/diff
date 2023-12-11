#ifndef DIFFERENTIATOR_HPP
#define DIFFERENTIATOR_HPP

DiffNode* DifferentiateTree(DiffNode* curNode, FILE* filedest);
error_t DiffOptimise(DiffTree* tree);

#endif
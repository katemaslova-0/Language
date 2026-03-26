#ifndef DIFF_H
#define DIFF_H

#include "MakeAstTreeFromFile.h"

Node * Differentiate (Node * node, Node * var);
Node * CopyNode      (Node * node);
void DiffIfNeeded (Node ** node);
Node * DiffPow    (Node * node, Node * var);

#endif // DIFF_H

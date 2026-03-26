#ifndef MIDDLEEND_H
#define MIDDLEEND_H

void OptimizeTree            (Node ** node);
void OptimizeConstNodes      (Node * node, bool * if_was_optimised);
void UniteOpAndConstArg      (Node * node);
double Calculate             (Node * node);
void UniteTwoConstNodes      (Node * node);
void OptimizeNeutralElements (Node ** node, bool * if_was_optimised);
bool OptimizeMulToNull       (Node ** node);
bool OptimizeMulToOne        (Node ** node);
bool OptimizeAddAndSubNull   (Node ** node);



bool IsNum    (Node * node);
bool IsCmdMul (Node * node);
bool IsZero   (Node * node);
bool IsOne    (Node * node);
bool IsOp     (Node * node);
bool IsCmdAdd (Node * node);
bool IsCmdSub (Node * node);

#endif // MIDDLEEND_H

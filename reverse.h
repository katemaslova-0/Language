#ifndef REVERSE_H
#define REVERSE_H

void MakeCodeOnMyLang (Node * root, const char * filename);
Node * PrintLine (Node * current, FILE * fp);
void PrintSetPixel (Node * current, FILE * fp);
void PrintKeyword (Node * current, FILE * fp);
void PrintFuncInit (Node * current, FILE * fp);
void PrintOut (Node * current, FILE * fp);
void PrintIn (Node * current, FILE * fp);
void PrintWhile (Node * current, FILE * fp);
void PrintIf (Node * current, FILE * fp);
void PrintElse (Node * current, FILE * fp);
void PrintVarOrFunc (Node * current, FILE * fp);
void PrintFunc (Node * current, FILE * fp);
void PrintVarEq (Node * current, FILE * fp);
void PrintE (Node * current, FILE * fp);
void PrintMathOp (Node * current, FILE * fp);
void PrintOp (Op_t op, FILE * fp);


#endif // REVERSE_H

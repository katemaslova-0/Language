#ifndef BACKEND_H
#define BACKEND_H

#include "data.h"

struct Files
{
    FILE * asm_code;
    FILE * asm_func;
    FILE * asm_standart_func;
};

const int NUM_OF_LABELS = 8;

enum Labels
{
    GreaterThan_L         = 1,
    GreaterThanOrEq_L     = 2,
    LessThan_L            = 3,
    LessThanOrEq_L        = 4,
    Equal_L               = 5,
    NotEqual_L            = 6,
    RetTrueAfterCompare_L = 7,
    Pow_L                 = 8

};

bool CheckFuncsFromTable       (Nametables table);
void TranslateAstToAsmCommands (const char * output_filename, Node * root, Nametables table);
void SetFuncLabels             (Nametables table, int * labels);
Node * TranslateString         (Files file, Node * node, int * labels, Nametables table, int * while_index, bool * is_inside_func);
void TranslateKeyWord          (Files file, Node * node, int * labels, Nametables table, int * while_index, bool * is_inside_func);
void TranslateFuncCall         (Files file, Node * node, int * labels, Nametables table, bool * is_inside_func);
void TranslateFuncInit         (Files file, Node * node, int * labels, Nametables table, int * while_index, bool * is_inside_func);
void TranslateOut              (Files file, Node * node, int * labels, bool * is_inside_func);
void TranslateIn               (Files file, Node * node, int * labels, bool * is_inside_func);
void TranslateBreak            (Files file, Node * node, int * labels, int * while_index, bool * is_inside_func);
void TranslateContinue         (Files file, Node * node, int * labels, int * while_index, bool * is_inside_func);
void TranslateWhile            (Files file, Node * node, int * labels, Nametables table, bool * is_inside_func);
void TranslateIf               (Files file, Node * node, int * labels, Nametables table, int * while_index, bool * is_inside_func);
void TranslateElse             (Files file, Node * node, int * labels, Nametables table, int * while_index, bool * is_inside_func);
void TranslateVarEq            (Files file, Node * node, bool * is_inside_func);
void TranslateEq               (Files file, Node * node, bool * is_inside_func);
void TranslateOp               (Files file, Node * node, bool * is_inside_func);
void TranslateSetPixel         (Files file, Node * node, int * labels, bool * is_inside_func);
void TranslateDraw             (Files file, Node * node, int * labels, bool * is_inside_func);


void WriteJaFunc  (FILE * fp);
void WriteJbFunc  (FILE * fp);
void WriteJaeFunc (FILE * fp);
void WriteJbeFunc (FILE * fp);
void WriteJeFunc  (FILE * fp);
void WriteJneFunc (FILE * fp);

void FreeNametable (Nametables table);
void CopyFuncsCode (FILE * asm_fp, const char * user_func, const char * st_func);


#endif // BACKEND_H

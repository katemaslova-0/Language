#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "MakeAstTreeFromFile.h"
#include "backend.h"
#include "data.h"

#define PRINTF_ERROR printf("%s:%d: Error\n", __FUNCTION__, __LINE__);


bool CheckFuncsFromTable (Nametables table)
{
    for (int count = 0 ; count < table.funcs->size; count++)
    {
        if (!table.funcs->data.func[count].if_initialized)
        {
            printf("Error: func %s has not been initialized\n", table.funcs->data.func[count].name);
            return false;
        }
        if (!table.funcs->data.func[count].if_defined)
        {
            printf("Error: func %s has not been defined\n", table.funcs->data.func[count].name);
            return false;
        }
    }

    return true;
}


void TranslateAstToAsmCommands (const char * output_filename, Node * root, Nametables table)
{
    assert(root);
    assert(output_filename);

    FILE * asm_fp = fopen(output_filename, "w");
    FILE * asm_func_fp = fopen("asm_func.txt", "w");
    FILE * asm_standart_func_fp = fopen("asm_standart_func.txt", "w");

    Files file = {asm_fp, asm_func_fp, asm_standart_func_fp};

    bool is_inside_func = false;

    int labels = NUM_OF_LABELS + 1; // first free label number

    SetFuncLabels(table, &labels);

    TranslateString(file, root, &labels, table, NULL, &is_inside_func);

    fprintf(asm_fp, "HLT\n\n");

    fclose(asm_func_fp);
    fclose(asm_standart_func_fp);

    CopyFuncsCode(asm_fp, "asm_func.txt", "asm_standart_func.txt");

    fclose(asm_fp);
}


void CopyFuncsCode (FILE * asm_fp, const char * user_func, const char * st_func)
{
    assert(asm_fp);
    assert(user_func);
    assert(st_func);

    FILE * asm_func_fp = fopen(user_func, "r");
    FILE * asm_standart_func_fp = fopen(st_func, "r");
    assert(asm_func_fp);
    assert(asm_standart_func_fp);

    int ch = '\0';

    while ((ch = fgetc(asm_func_fp)) != EOF)
        fputc(ch, asm_fp);

    fprintf(asm_fp, "\n");

    while ((ch = fgetc(asm_standart_func_fp)) != EOF)
        fputc(ch, asm_fp);

    fclose(asm_func_fp);
    fclose(asm_standart_func_fp);
}


void SetFuncLabels (Nametables table, int * labels)
{
    assert(labels);

    for (int count = 0; count < table.funcs->size; count++)
    {
        table.funcs->data.func[count].index = *labels;
        (*labels)++;
    }
}


Node * TranslateString (Files file, Node * node, int * labels, Nametables table, int * while_index, bool * is_inside_func)
{
    assert(node);
    assert(labels);
    assert(is_inside_func);

    printf("TRANSLATE STRING HAS BEEN CALLED\n");

    if (node->type == kKeyWord && node->value.keyword == kElse)
        return node;
    if (node->type != kDivider)
        PRINTF_ERROR;

    switch (node->left->type)
    {
        case kName:
                    {
                        if (node->left->value.name.type == kVarName)
                            TranslateVarEq(file, node->left, is_inside_func);
                        else if (node->left->value.name.type == kFuncName)
                            TranslateFuncCall(file, node->left, labels, table, is_inside_func);
                        else
                            PRINTF_ERROR;
                        break;
                    }

        case kKeyWord: TranslateKeyWord(file, node->left, labels, table, while_index, is_inside_func); break;
        case kNum:
        case kOp:
                    {
                        if (node->left->value.op == kSetPixel)
                            TranslateSetPixel(file, node->left, labels, is_inside_func);
                        else
                            PRINTF_ERROR;
                        break;
                    }
        case kDivider:
        case kComma:
        default:       PRINTF_ERROR;
    }

    if (node->right)
        return TranslateString(file, node->right, labels, table, while_index, is_inside_func);

    return NULL;
}


void TranslateSetPixel (Files file, Node * node, int * labels, bool * is_inside_func)
{
    assert(node);
    assert(labels);

    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    fprintf(fp, "PUSH 111\n");
    TranslateEq(file, node->left, is_inside_func);
    fprintf(fp, "PUSH 10\n");
    fprintf(fp, "MUL\n");
    TranslateEq(file, node->right, is_inside_func);
    fprintf(fp, "ADD\n");
    fprintf(fp, "POPREG DX\n");
    fprintf(fp, "POPM [DX]\n");
}


void TranslateKeyWord (Files file, Node * node, int * labels, Nametables table, int * while_index, bool * is_inside_func)
{
    assert(node);
    assert(labels);

    printf("TRANSLATE KEYWORD HAS BEEN CALLED\n");

    switch(node->value.keyword)
    {
        case kIf:       TranslateIf       (file, node, labels, table, while_index, is_inside_func); break;
        case kWhile:    TranslateWhile    (file, node, labels, table, is_inside_func);              break;
        case kContinue: TranslateContinue (file, node, labels, while_index, is_inside_func);        break;
        case kBreak:    TranslateBreak    (file, node, labels, while_index, is_inside_func);        break;
        case kIn:       TranslateIn       (file, node, labels, is_inside_func);                     break;
        case kOut:      TranslateOut      (file, node, labels, is_inside_func);                     break;
        case kFunc:     TranslateFuncInit (file, node, labels, table, while_index, is_inside_func); break;
        case kDraw:     TranslateDraw     (file, node, labels, is_inside_func);                     break;
        case kVarInit:
        case kReturn: // РґРѕРґРµР»Р°С‚СЊ
        case kElse:
        default:        PRINTF_ERROR;
    }
}


void TranslateDraw (Files file, Node * node, int * labels, bool * is_inside_func)
{
    assert(node);
    assert(labels);
    assert(is_inside_func);

    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    fprintf(fp, "DRAW\n");
}


void TranslateFuncCall (Files file, Node * node, int * labels, Nametables table, bool * is_inside_func)
{
    assert(node);
    assert(labels);
    assert(is_inside_func);

    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    int num_of_args = table.funcs->data.func[node->value.name.index].num_of_args;

    Node * curr_node = node->left;

    for (int count = 0; count < num_of_args; count++)
    {
        Node * curr_arg = table.funcs->data.func[node->value.name.index].array[count];
        int index_of_formal_arg = curr_arg->value.name.index;
        int index_of_real_arg = curr_node->value.name.index;

        fprintf(fp, "PUSH %d\n", index_of_real_arg);
        fprintf(fp, "POPREG DX\n");
        fprintf(fp, "PUSHM [DX]\n");
        fprintf(fp, "PUSH %d\n", index_of_formal_arg);
        fprintf(fp, "POPREG DX\n");
        fprintf(fp, "POPM [DX]\n");

        curr_node = curr_node->left;
    }

    fprintf(fp, "CALL :%d\n", table.funcs->data.func[node->value.name.index].index);
    fprintf(fp, "\n\n");
}


void TranslateFuncInit (Files file, Node * node, int * labels, Nametables table, int * while_index, bool * is_inside_func)
{
    assert(node);
    assert(labels);
    assert(is_inside_func);

    node = node->left; // move to func name

    if (!node->right)
        return;

    fprintf(file.asm_func, ":%d\n", table.funcs->data.func[node->value.name.index].index);

    *is_inside_func = true;

    TranslateString(file, node->right, labels, table, while_index, is_inside_func);

    *is_inside_func = false;

    fprintf(file.asm_func, "RET\n\n");
    fprintf(file.asm_func, "\n\n");
}


void TranslateOut (Files file, Node * node, int * labels, bool * is_inside_func)
{
    assert(node);
    assert(labels);

    printf("TRANSLATE OUT HAS BEEN CALLED\n");

    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    if (node->left->type == kName)
    {
        fprintf(fp, "PUSH %d\n", node->left->value.name.index);
        fprintf(fp, "POPREG DX\n");
        fprintf(fp, "PUSHM [DX]\n");
        fprintf(fp, "OUT\n");
    }
    else if (node->left->type == kNum)
    {
        fprintf(fp, "PUSH %d\n", node->left->value.num);
        fprintf(fp, "OUT\n");
    }
    else if (node->left->type == kOp)
    {
        TranslateEq(file, node->left, is_inside_func);
        fprintf(fp, "OUT\n");
    }
    else
        PRINTF_ERROR;
}


void TranslateIn (Files file, Node * node, int * labels, bool * is_inside_func)
{
    assert(node);
    assert(labels);

    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    fprintf(fp, "IN\n");
    fprintf(fp, "PUSH %d\n", node->left->value.name.index);
    fprintf(fp, "POPREG DX\n");
    fprintf(fp, "POPM [DX]\n");
    fprintf(fp, "\n\n");
}


void TranslateBreak (Files file, Node * node, int * labels, int * while_index, bool * is_inside_func)
{
    assert(node);
    assert(labels);

    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    if (!while_index)
        PRINTF_ERROR;

    fprintf(fp, "JMP :%d\n", *labels);

    *while_index = *labels;

    (*labels)++;

    fprintf(fp, "\n\n");
}


void TranslateContinue (Files file, Node * node, int * labels, int * while_index, bool * is_inside_func)
{
    assert(node);
    assert(labels);

    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    if (!while_index)
        PRINTF_ERROR;

    fprintf(fp, "JMP :%d\n", *while_index);

    fprintf(fp, "\n\n");
}


void TranslateWhile (Files file, Node * node, int * labels, Nametables table, bool * is_inside_func)
{
    assert(node);
    assert(labels);

    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    int while_start = *labels;
    (*labels)++;

    int le_label = *labels;
    (*labels)++;

    int break_index = while_start;

    fprintf(fp, ":%d\n", while_start);

    TranslateEq(file, node->left, is_inside_func);
    fprintf(fp, "PUSH 0\n");
    fprintf(fp, "JE :%d\n", le_label);

    TranslateString(file, node->right, labels, table, &while_start, is_inside_func);

    fprintf(fp, "JMP :%d\n\n\n", while_start);

    fprintf(fp, ":%d\n", le_label);
    if (break_index != while_start)
        fprintf(fp, ":%d\n", while_start);
}


void TranslateIf (Files file, Node * node, int * labels, Nametables table, int * while_index, bool * is_inside_func)
{
    assert(node);
    assert(labels);

    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    TranslateEq(file, node->left, is_inside_func);

    int label1 = *labels;
    int label2 = *labels + 1;
    int label3 = *labels + 2;

    *labels += 3;

    fprintf(fp, "PUSH 1\n");
    fprintf(fp, "JE :%d   // переход, если if выполнен\n", label1); // добавить комментарии к меткам (добавлять имя файла и номер стрки исходного текста - таблица в конце файла либо дерево)
    fprintf(fp, "JMP :%d  // иначе - пропуск тела if\n", label2);
    fprintf(fp, "\n\n");
    fprintf(fp, ":%d    // тело if\n", label1);

    printf("CALLING TRANSLATE STRING INSIDE OF IF\n");

    Node * ret_node = TranslateString(file, node->right, labels, table, while_index, is_inside_func);

    printf("TRANSLATE STRING ENDED. RETURNED TO IF\n");

    if (ret_node)
        fprintf(fp, "JMP :%d    // пропуск тела else, если выполнен if\n", label3);

    fprintf(fp, "\n\n");
    fprintf(fp, ":%d  // пропуск тела if\n", label2);

    if (ret_node)
    {
        TranslateElse(file, ret_node, labels, table, while_index, is_inside_func);

        fprintf(fp, ":%d  //  пропущен else, выполнен if\n", label3);
        fprintf(fp, "\n\n");
    }
}


void TranslateElse (Files file, Node * node, int * labels, Nametables table, int * while_index, bool * is_inside_func)
{
    assert(node);
    assert(labels);

    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    TranslateString(file, node->right, labels, table, while_index, is_inside_func);

    fprintf(fp, "\n\n");
}


void TranslateVarEq (Files file, Node * node, bool * is_inside_func)
{
    assert(node);

    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    TranslateEq(file, node->left->left, is_inside_func);

    fprintf(fp, "PUSH %d\n", node->value.name.index);
    fprintf(fp, "POPREG DX\n");
    fprintf(fp, "POPM [DX]\n");
    fprintf(fp, "\n\n");
}


void TranslateEq (Files file, Node * node, bool * is_inside_func)
{
    assert(node);

    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    switch (node->type)
    {
        case kNum:  fprintf(fp, "PUSH %d\n", node->value.num); break;
        case kName: {
                        fprintf(fp, "PUSH %d\n", node->value.name.index);
                        fprintf(fp, "POPREG DX\n");
                        fprintf(fp, "PUSHM [DX]\n");
                        break;
                    }
        case kOp:   {
                        TranslateEq(file, node->left, is_inside_func);

                        if (node->value.op != kSqrt && node->value.op != kPow)
                            TranslateEq(file, node->right, is_inside_func);

                        TranslateOp(file, node, is_inside_func);
                        break;
                    }
        case kKeyWord:
        case kDivider:
        case kComma:
        default:        PRINTF_ERROR;
    }
}


void TranslateOp (Files file, Node * node, bool * is_inside_func)
{
    FILE * fp = file.asm_code;

    if (*is_inside_func)
        fp = file.asm_func;

    switch(node->value.op)
    {
        case kAdd:          fprintf(fp, "ADD\n"); break;
        case kSub:          fprintf(fp, "SUB\n"); break;
        case kMul:          fprintf(fp, "MUL\n"); break;
        case kDiv:          fprintf(fp, "DIV\n"); break;
        case kGreaterThan:
                        {
                            fprintf(fp, "CALL :%d   // сравнение(>)\n", GreaterThan_L);
                            WriteJaFunc(file.asm_standart_func);
                            break;
                        }
        case kLessThan:
                        {
                            fprintf(fp, "CALL :%d   // сравнение(<)\n", LessThan_L);
                            WriteJbFunc(file.asm_standart_func);
                            break;
                        }
        case kGreaterThanOrEqual:
                        {
                            fprintf(fp, "CALL :%d   // сравнение(>=)\n", GreaterThanOrEq_L);
                            WriteJaeFunc(file.asm_standart_func);
                            break;
                        }
        case kLessThanOrEqual:
                        {
                            fprintf(fp, "CALL :%d   // сравнение(<=)\n", LessThanOrEq_L);
                            WriteJbeFunc(file.asm_standart_func);
                            break;
                        }
        case kEqual:
                        {
                            fprintf(fp, "CALL :%d   // сравнение(==)\n", Equal_L);
                            WriteJeFunc(file.asm_standart_func);
                            break;
                        }
        case kNotEqual:
                        {
                            fprintf(fp, "CALL :%d   // сравнение(!=)\n", NotEqual_L);
                            WriteJneFunc(file.asm_standart_func);
                            break;
                        }
        case kSqrt:     {
                            fprintf(fp, "SQRT\n");
                            break;
                        }
        case kSetPixel:
        case kPow:      {
                            fprintf(fp, "POW %d\n", node->right->value.num); // check! what if var?
                            break;
                        }
        case kSin: // РґРѕРґРµР»Р°С‚СЊ
        case kCos: // РґРѕРґРµР»Р°С‚СЊ
        case kTan: // РґРѕРґРµР»Р°С‚СЊ
        case kCtg: // РґРѕРґРµР»Р°С‚СЊ
        case kLn:
        case kDif:
        case kOpenBracket:
        case kCloseBracket:
        case kOpenCurlyBracket:
        case kCloseCurlyBracket:
        case kAsn:
        default:    PRINTF_ERROR;
    }
}


void WriteJaFunc (FILE * fp)
{
    assert(fp);

    static bool has_been_called = false;
    if (has_been_called) return;

    fprintf(fp, ":%d\n", GreaterThan_L);
    fprintf(fp, "JA :%d   // если >, то перейти\n", RetTrueAfterCompare_L);
    fprintf(fp, "PUSH 0   // иначе\n");
    fprintf(fp, "RET\n");
    fprintf(fp, ":%d\n", RetTrueAfterCompare_L);
    fprintf(fp, "PUSH 1\n");
    fprintf(fp, "RET\n\n\n");

    has_been_called = true;
}


void WriteJbFunc (FILE * fp)
{
    assert(fp);

    static bool has_been_called = false;
    if (has_been_called) return;

    fprintf(fp, ":%d\n", LessThan_L);
    fprintf(fp, "JB :%d   // если <, то перейти\n", RetTrueAfterCompare_L);
    fprintf(fp, "PUSH 0\n");
    fprintf(fp, "RET\n");
    fprintf(fp, ":%d\n", RetTrueAfterCompare_L);
    fprintf(fp, "PUSH 1\n");
    fprintf(fp, "RET\n\n\n");

    has_been_called = true;
}


void WriteJaeFunc (FILE * fp)
{
    assert(fp);

    static bool has_been_called = false;
    if (has_been_called) return;

    fprintf(fp, ":%d\n", GreaterThanOrEq_L);
    fprintf(fp, "JAE :%d   // если >=, то перейти\n", RetTrueAfterCompare_L);
    fprintf(fp, "PUSH 0\n");
    fprintf(fp, "RET\n");
    fprintf(fp, ":%d\n", RetTrueAfterCompare_L);
    fprintf(fp, "PUSH 1\n");
    fprintf(fp, "RET\n\n\n");

    has_been_called = true;
}


void WriteJbeFunc (FILE * fp)
{
    assert(fp);

    static bool has_been_called = false;
    if (has_been_called) return;

    fprintf(fp, ":%d\n", LessThanOrEq_L);
    fprintf(fp, "JBE :%d   // если <=, то перейти\n", RetTrueAfterCompare_L);
    fprintf(fp, "PUSH 0\n");
    fprintf(fp, "RET\n");
    fprintf(fp, ":%d\n", RetTrueAfterCompare_L);
    fprintf(fp, "PUSH 1\n");
    fprintf(fp, "RET\n\n\n");

    has_been_called = true;
}


void WriteJeFunc (FILE * fp)
{
    assert(fp);

    static bool has_been_called = false;
    if (has_been_called) return;

    fprintf(fp, ":%d\n", Equal_L);
    fprintf(fp, "JE :%d   // если ==, то перейти\n", RetTrueAfterCompare_L);
    fprintf(fp, "PUSH 0\n");
    fprintf(fp, "RET\n");
    fprintf(fp, ":%d\n", RetTrueAfterCompare_L);
    fprintf(fp, "PUSH 1\n");
    fprintf(fp, "RET\n\n\n");

    has_been_called = true;
}


void WriteJneFunc (FILE * fp)
{
    assert(fp);

    static bool has_been_called = false;
    if (has_been_called) return;

    fprintf(fp, ":%d\n", NotEqual_L);
    fprintf(fp, "JNE :%d   // если !=, то перейти\n", RetTrueAfterCompare_L);
    fprintf(fp, "PUSH 0\n");
    fprintf(fp, "RET\n");
    fprintf(fp, ":%d\n", RetTrueAfterCompare_L);
    fprintf(fp, "PUSH 1\n");
    fprintf(fp, "RET\n\n\n");

    has_been_called = true;
}


void FreeNametable (Nametables table)
{
    for (int count = 0; count < table.funcs->size; count++)
        free(table.funcs->data.func[count].array);
}

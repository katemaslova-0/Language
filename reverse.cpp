#include <stdio.h>
#include <assert.h>

#include "data.h"
#include "reverse.h"
#include "MakeAstTreeFromFile.h"

const int NUM_OF_KEY_WORDS = 10;

#define PRINTF_ERROR printf("%s:%d: Error\n", __FUNCTION__, __LINE__);


void MakeCodeOnMyLang (Node * root, const char * filename)
{
    assert(root);
    assert(filename);

    FILE * fp = fopen(filename, "w");

    Node * current = root;

    PrintLine(current, fp);

    fclose(fp);
}


Node * PrintLine (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    if (current->type == kKeyWord && current->value.keyword == kElse)
        return current;
    else if (current->type != kDivider)
    {
        PRINTF_ERROR;
        printf("CURR TYPE ENUM IS %d\n", current->type);
        fclose(fp);
        assert(0);
    }

    if (!current->left)
        fclose(fp);
    assert(current->left);
    switch(current->left->type)
    {
        case kName:    PrintVarOrFunc(current->left, fp); break;
        case kKeyWord: PrintKeyword(current->left, fp);   break;
        case kOp:      PrintSetPixel(current->left, fp);  break;
        case kNum:
        case kDivider:
        case kComma:
        default: PRINTF_ERROR;
    }

    fprintf(fp, ";\n");

    if (current->right)
        return PrintLine(current->right, fp);

    return NULL;
}


void PrintSetPixel (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    fprintf(fp, "setpixel(");
    PrintE(current->left, fp);
    fprintf(fp, ", ");
    PrintE(current->right, fp);
    fprintf(fp, ")");
}


void PrintKeyword (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    switch(current->value.keyword)
    {
        case kIf:       PrintIf(current, fp);           break;
        case kWhile:    PrintWhile(current, fp);        break;
        case kContinue: fprintf(fp, "удержи-меня");     break;
        case kBreak:    fprintf(fp, "переступи-порог"); break;
        case kReturn:   fprintf(fp, "вернулся-волком"); break;
        case kIn:       PrintIn(current, fp);           break;
        case kOut:      PrintOut(current, fp);          break;
        case kFunc:     PrintFuncInit(current, fp);     break;
        case kVarInit:
        case kElse:
        default:        PRINTF_ERROR;
    }
}


void PrintFuncInit (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    fprintf(fp, "я-покажу-тебе-маршрут ");
    PrintFunc(current->left, fp);
}


void PrintOut (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    fprintf(fp, "говорила-она(");
    PrintE(current->left, fp);
    fprintf(fp, ")");
}


void PrintIn (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    fprintf(fp, "отвечай-же-скорей(");
    PrintE(current->left, fp);
    fprintf(fp, ")");
}


void PrintWhile (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    fprintf(fp, "покуда-льется-свет (");
    PrintE(current->left, fp);
    fprintf(fp, ")\n{\n");

    PrintLine(current->right, fp);

    fprintf(fp, "\n}");
}


void PrintIf (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    fprintf(fp, "сделаешь-шаг (");
    PrintE(current->left, fp);
    fprintf(fp, ")\n{\n");

    Node * ret = PrintLine(current->right, fp);

    fprintf(fp, "\n}");

    if (ret)
        PrintElse(ret, fp);
}


void PrintElse (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    fprintf(fp, "спешит-в-другую-сторону\n{\n");
    PrintLine(current->right, fp);
    fprintf(fp, "}");
}


void PrintVarOrFunc (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    if (current->value.name.type == kVarName)
        PrintVarEq(current, fp);
    else
        PrintFunc(current, fp);
}


void PrintFunc (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    fprintf(fp, "%s(", current->value.name.name);

    if (current->left)
        PrintE(current->left, fp);

    fprintf(fp, ")");

    if (current->right)
    {
        fprintf(fp, "\n{\n");
        PrintLine(current->right->right, fp);
        fprintf(fp, "\n}");
    }
}


void PrintVarEq (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    fprintf(fp, "знай-же %s = ", current->value.name.name);

    current = current->left;

    if (current->value.op != kAsn)
        PRINTF_ERROR;

    current = current->left;

    PrintE(current, fp);
}


void PrintE (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    switch(current->type)
    {
        case kNum:  fprintf(fp, "%d ", current->value.num);         break;
        case kName: fprintf(fp, "%s ", current->value.name.name);   break;
        case kOp:
                {
                    if (current->value.op >= kAdd && current->value.op < kAsn)
                    {
                        fprintf(fp, "(");
                        PrintE(current->left, fp);
                        fprintf(fp, ")");
                        PrintOp(current->value.op, fp);
                        fprintf(fp, "(");
                        PrintE(current->right, fp);
                        fprintf(fp, ")");
                    }
                    else
                        PrintMathOp(current, fp);
                    break;
                }
        case kKeyWord: printf("Keyword isn't expected! enum: %d\n", current->value.keyword); break;
        case kDivider:
        case kComma:
        default:    {PRINTF_ERROR; fclose(fp);}
    }
}


void PrintMathOp (Node * current, FILE * fp)
{
    assert(current);
    assert(fp);

    PrintOp(current->value.op, fp);
    fprintf(fp, "(");
    PrintE(current->left, fp);

    if (current->value.op == kDif)
    {
        fprintf(fp, ", ");
        PrintE(current->right, fp);
    }
    fprintf(fp, ")");
}


void PrintOp (Op_t op, FILE * fp)
{
    assert(fp);

    switch(op)
    {
        case kAdd:                fprintf(fp, "+");        break;
        case kSub:                fprintf(fp, "-");        break;
        case kMul:                fprintf(fp, "*");        break;
        case kDiv:                fprintf(fp, "/");        break;
        case kGreaterThan:        fprintf(fp, ">");        break;
        case kLessThan:           fprintf(fp, "<");        break;
        case kGreaterThanOrEqual: fprintf(fp, ">=");       break;
        case kLessThanOrEqual:    fprintf(fp, "<=");       break;
        case kEqual:              fprintf(fp, "==");       break;
        case kNotEqual:           fprintf(fp, "!=");       break;
        case kAsn:                fprintf(fp, "=");        break;
        case kSin:                fprintf(fp, "sin");      break;
        case kCos:                fprintf(fp, "cos");      break;
        case kTan:                fprintf(fp, "tan");      break;
        case kCtg:                fprintf(fp, "ctg");      break;
        case kPow:                fprintf(fp, "^");        break;
        case kSqrt:               fprintf(fp, "sqrt");     break;
        case kOpenBracket:        fprintf(fp, "(");        break;
        case kCloseBracket:       fprintf(fp, ")");        break;
        case kOpenCurlyBracket:   fprintf(fp, "{");        break;
        case kCloseCurlyBracket:  fprintf(fp, "}");        break;
        case kSetPixel:           fprintf(fp, "setpixel"); break;
        case kDif:                fprintf(fp, "dif");      break;
        case kLn:                 fprintf(fp, "ln");       break;
        default:                  PRINTF_ERROR;
    }
}

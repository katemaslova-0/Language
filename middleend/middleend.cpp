#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "data.h"
#include "middleend.h"
#include "MakeAstTreeFromFile.h"

const double SMALL_NUMBER = 0.00001;

#define PRINTF_ERROR printf("%s:%d: Error\n", __FUNCTION__, __LINE__)

#define Ln node->left
#define Rn node->right

#define CALC_ADD(node1, node2) Calculate(node1) + Calculate(node2)
#define CALC_MUL(node1, node2) Calculate(node1) * Calculate(node2)
#define CALC_DIV(node1, node2) Calculate(node1) / Calculate(node2);
#define CALC_SIN(node)         sin(Calculate(node));
#define CALC_COS(node)         cos(Calculate(node));
#define CALC_TAN(node)         tan(Calculate(node));
#define CALC_CTG(node)         1 / Calculate(node);
#define CALC_SUB(node1, node2) Calculate(node1) - Calculate(node2);
#define CALC_SQRT(node)        sqrt(Calculate(node));

#define CALC_GREATER_THAN(node1, node2)       if (Calculate(node1) > Calculate(node2))  return 1; else return 0;
#define CALC_GREATER_THAN_OR_EQ(node1, node2) if (Calculate(node1) >= Calculate(node2)) return 1; else return 0;
#define CALC_LESS_THAN(node1, node2)          if (Calculate(node1) < Calculate(node2))  return 1; else return 0;
#define CALC_LESS_THAN_OR_EQ(node1, node2)    if (Calculate(node1) <= Calculate(node2)) return 1; else return 0;
#define CALC_EQUAL(node1, node2)              if (Calculate(node1) - Calculate(node2)) return 1; else return 0;
#define CALC_NOT_EQUAL(node1, node2)          if (Calculate(node1) != Calculate(node2)) return 1; else return 0;

void OptimizeTree (Node ** node)
{
    assert(node);
    assert(*node);

    bool if_was_optimised = true;

    while (if_was_optimised)
    {
        if_was_optimised = false;
        OptimizeConstNodes(*node, &if_was_optimised);
        OptimizeNeutralElements(node, &if_was_optimised);
    }
}


void OptimizeConstNodes (Node * node, bool * if_was_optimised)
{
    assert(node);
    assert(if_was_optimised);

    if (Ln)
        OptimizeConstNodes(Ln, if_was_optimised);
    if (Rn)
        OptimizeConstNodes(Rn, if_was_optimised);

    if (Rn && Ln)
    {
        if (IsNum(Rn) && IsNum(Ln))
        {
            UniteTwoConstNodes(node);
            *if_was_optimised = true;
        }
    }
    else if (Rn && !Ln)
    {
        if (IsNum(Rn))
        {
            UniteOpAndConstArg(node);
            *if_was_optimised = true;
        }
    }
}


bool IsNum (Node * node)
{
    assert(node);

    if (node->type == kNum)
        return true;

    return false;
}


void UniteOpAndConstArg (Node * node)
{
    assert(node);

    int res = (int)Calculate(node);

    free(Rn);

    node->type = kNum;
    node->value.num = res;
    node->left = NULL;
    node->right = NULL;
}


double Calculate (Node * node)
{
    assert(node);

    switch(node->type)
    {
        case kNum: return node->value.num;
        case kName: PRINTF_ERROR; break;
        case kOp:
        {
            switch((node->value).op)
            {
                case kAdd:                return CALC_ADD(Ln, Rn);
                case kMul:                return CALC_MUL(Ln, Rn);
                case kDiv:                return CALC_DIV(Ln, Rn);
                case kGreaterThan:        CALC_GREATER_THAN(Ln, Rn);
                case kLessThan:           CALC_LESS_THAN(Ln, Rn);
                case kGreaterThanOrEqual: CALC_GREATER_THAN_OR_EQ(Ln, Rn);
                case kLessThanOrEqual:    CALC_LESS_THAN_OR_EQ(Ln, Rn);
                case kEqual:              CALC_EQUAL(Ln, Rn);
                case kNotEqual:           CALC_NOT_EQUAL(Ln, Rn);
                case kSin:                return CALC_SIN(Ln);
                case kCos:                return CALC_COS(Ln);
                case kTan:                return CALC_TAN(Ln);
                case kCtg:                return CALC_CTG(Ln);
                case kSqrt:               return CALC_SQRT(Ln);
                case kSub:                return CALC_SUB(Ln, Rn);
                case kPow: // добавить
                case kLn:
                case kDif:
                case kAsn:
                case kSetPixel:
                case kOpenBracket:
                case kCloseBracket:
                case kOpenCurlyBracket:
                case kCloseCurlyBracket: PRINTF_ERROR; break;
                default: printf("%s: Command is not found\n", __FUNCTION__);
            }
            break;
        }
        case kKeyWord:
        case kDivider:
        case kComma:    PRINTF_ERROR; break;
        default:    printf("%s: Type is not found\n", __FUNCTION__);
    }

    printf("%s:%d: Error\n", __FUNCTION__, __LINE__); //FIXME
    return 0;
}


void UniteTwoConstNodes (Node * node)
{
    assert(node);

    int res = (int)Calculate(node);

    free(Ln);
    free(Rn);

    node->type = kNum;
    node->value.num = res;
    node->left = NULL;
    node->right = NULL;
}


void OptimizeNeutralElements (Node ** node, bool * if_was_optimised)
{
    assert(node);
    assert(*node);
    assert(if_was_optimised);

    if ((*node)->left)
        OptimizeNeutralElements(&((*node)->left), if_was_optimised);
    if ((*node)->right)
        OptimizeNeutralElements(&((*node)->right), if_was_optimised);

    if (IsOp(*node) && (*node)->value.op == kMul)
    {
        if(*if_was_optimised = OptimizeMulToNull(node)) return;
        if(*if_was_optimised = OptimizeMulToOne(node))  return;
    }

    if (IsOp(*node) && (IsCmdAdd(*node) || IsCmdSub(*node)))
    {
        if(*if_was_optimised = OptimizeAddAndSubNull(node)) return;
    }
}


bool IsCmdSub (Node * node)
{
    assert(node);

    if (node->value.op == kSub)
        return true;

    return false;
}


bool IsCmdAdd (Node * node)
{
    assert(node);

    if (node->value.op == kAdd)
        return true;

    return false;
}


bool IsOp (Node * node)
{
    assert(node);

    if (node->type == kOp)
        return true;

    return false;
}


bool OptimizeMulToNull (Node ** node)
{
    assert(node);
    assert(*node);

    if ((IsZero((*node)->left)) || IsZero((*node)->right))
    {
        NodeDtor((*node)->left);
        NodeDtor((*node)->right);

        (*node)->type = kNum;
        (*node)->value.num = 0;
        (*node)->left = NULL;
        (*node)->right = NULL;

        return true;
    }

    return false;
}


bool OptimizeMulToOne (Node ** node)
{
    assert(node);
    assert(*node);

    if (IsOne((*node)->left))
    {
        free((*node)->left);
        Node * ptr = *node;
        *node = (*node)->right;
        free(ptr);  // NodeDtor

        return true;
    }
    else if(IsOne((*node)->right))
    {
        free((*node)->right);
        Node * ptr = *node;
        *node = (*node)->left;
        free(ptr);

        return true;
    }

    return false;
}


bool OptimizeAddAndSubNull (Node ** node)
{
    assert(node);
    assert(*node);

    if (IsZero((*node)->left))
    {
        free((*node)->left);
        Node * ptr = *node;
        *node = (*node)->right;
        free(ptr);

        return true;
    }
    else if (IsZero((*node)->right))
    {
        free((*node)->right);
        Node * ptr = *node;
        *node = (*node)->left;
        free(ptr);

        return true;
    }

    return false;
}

bool IsCmdMul (Node * node)
{
    assert(node);

    if (node->value.op == kMul)
        return true;

    return false;
}


bool IsZero (Node * node)
{
    assert(node);

    if (IsNum(node) && node->value.num < SMALL_NUMBER && node->value.num > - SMALL_NUMBER)
        return true;

    return false;
}

bool IsOne (Node * node)
{
    assert(node);

    if (IsNum(node) && node->value.num - 1 < SMALL_NUMBER && node->value.num - 1 > - SMALL_NUMBER)
        return true;

    return false;
}


#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "data.h"
#include "diff.h"
#include "GraphDump.h"

#define dR Differentiate(node->right, var)
#define dL Differentiate(node->left,  var)
#define cR CopyNode(node->right)
#define cL CopyNode(node->left)

#define ADD_(node1, node2) MakeNewNode(kOp, {.op = kAdd}, node1, node2)
#define SUB_(node1, node2) MakeNewNode(kOp, {.op = kSub}, node1, node2)
#define MUL_(node1, node2) MakeNewNode(kOp, {.op = kMul}, node1, node2)
#define DIV_(node1, node2) MakeNewNode(kOp, {.op = kDiv}, node1, node2)
#define POW_(node1, node2) MakeNewNode(kOp, {.op = kPow}, node1, node2)
#define SIN_(node)         MakeNewNode(kOp, {.op = kSin}, NULL, node)
#define COS_(node)         MakeNewNode(kOp, {.op = kCos}, NULL, node)
#define TAN_(node)         MakeNewNode(kOp, {.op = kTan}, NULL, node)
#define LN_(node)          MakeNewNode(kOp, {.op = kLn},  NULL, node)
#define NUM_(value)        MakeNewNode(kNum, {.num = value}, NULL, NULL)


#define RETURN_IF_NOT_ALLOCATED(node) if (!node) {printf("Memory allocation error\n"); return NULL;}
#define PRINTF_ERROR printf("%s:%d: Error\n", __FUNCTION__, __LINE__);


Node * Differentiate (Node * node, Node * var)
{
    assert(node);

    switch(node->type)
    {
        case kNum: return NUM_(0); break;
        case kName: {if (strcmp(var->value.name.name, node->value.name.name) == 0)
                        return NUM_(1);
                    else
                        return NUM_(0);
                    break;}
        case kOp:
        {
            switch(node->value.op)
            {
                case kAdd:    return ADD_(dL, dR);    break;
                case kMul:    return ADD_(MUL_(dL, cR), MUL_(cL, dR));    break;
                case kDiv:    return DIV_(ADD_(MUL_(dL, cR), MUL_(MUL_(cL, dR), NUM_(-1))), MUL_(cR, cR)); break;
                case kSin:    return MUL_(COS_(cR), dR);    break;
                case kCos:    return MUL_(MUL_(SIN_(cR), NUM_(-1)), dR);    break;
                case kTan:    return MUL_(DIV_(NUM_(1), MUL_(COS_(cR), COS_(cR))), dR);    break;
                case kCtg:    return MUL_(DIV_(NUM_(-1), MUL_(SIN_(cR), SIN_(cR))), dR);    break;
                case kPow:    return DiffPow(node, var);    break;
                case kSub:    return SUB_(dL, dR); break;
                case kLn:
                case kAsn:
                case kDif:
                case kSqrt:
                case kSetPixel:
                case kOpenBracket:
                case kCloseBracket:
                case kOpenCurlyBracket:
                case kCloseCurlyBracket:
                case kGreaterThan:
                case kLessThan:
                case kGreaterThanOrEqual:
                case kLessThanOrEqual:
                case kEqual:
                case kNotEqual: PRINTF_ERROR; break;
                default:        printf("%s: Command is not found\n", __FUNCTION__);
            }
            break;
        }
        case kKeyWord:
        case kDivider:
        case kComma: PRINTF_ERROR; break;
        default: printf("%s: Type is not found\n", __FUNCTION__);
    }

    return NULL;
}


void DiffIfNeeded (Node ** node)
{
    assert(node);
    assert(*node);

    if ((*node)->left)
        DiffIfNeeded(&((*node)->left));

    if ((*node)->right)
        DiffIfNeeded(&((*node)->right));

    if ((*node)->type == kOp && (*node)->value.op == kDif)
        *node = Differentiate((*node)->left, (*node)->right);
}


Node * CopyNode (Node * node)
{
    assert(node);

    Node * new_node = AllocateNodeMemory();
    RETURN_IF_NOT_ALLOCATED(new_node);

    new_node->value = node->value;
    new_node->type = node->type;

    if (node->left)
        new_node->left = CopyNode(node->left);
    else
        new_node->left = NULL;

    if (node->right)
        new_node->right = CopyNode(node->right);
    else
        new_node->right = NULL;

    return new_node;
}


Node * DiffPow (Node * node, Node * var)
{
    assert(node);
    assert(var);

    if (node->right->type == kNum)
        return MUL_(cR, MUL_(dL, POW_(cL, ADD_(cR, NUM_(-1)))));
    //else if (node->left->type == kNum && node->right->type != kNum)
        //return MUL_(POW_(cL, cR), MUL_(LN_(cL), dR));
    //else if (node->left->type != kNum && node->right->type != kNum)
        //return MUL_(POW_(NUM_(exp(1)), MUL_(LN_(cL), cR)), ADD_(MUL_(dR, LN_(cL)), DIV_(MUL_(dL, cR), cL)));
    else
    {
        printf("%s:%d: ERROR\n", __FUNCTION__, __LINE__);
        return NULL;
    }
}

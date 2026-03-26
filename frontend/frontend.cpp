#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>

#include "data.h"
#include "frontend.h"

#define PRINTF_ERROR printf("%s:%d: Error\n", __FUNCTION__, __LINE__);
#define MAKE_NODE_FROM_CURR_TOKEN MakeNewNode(tokens->data.token[*current].type, tokens->data.token[*current].value, NULL, NULL)

const int NUM_OF_TOKENS  = 1000;
const int LENGTH_OF_WORD = 40;
const int LENTH_OF_ASN_PHRASE = 5;
const int NUM_OF_KEY_WORDS = 11;
const int ALPH_LENGTH = 33;

const KeyWord key_words[NUM_OF_KEY_WORDS] = {{"ñäåëàåøü-øàã", kIf},
                                             {"ïîêóäà-ëüåòñÿ-ñâåò", kWhile},
                                             {"óäåðæè-ìåíÿ", kContinue},
                                             {"ïåðåñòóïè-ïîðîã", kBreak},
                                             {"âåðíóëñÿ-âîëêîì", kReturn},
                                             {"îòâå÷àé-æå-ñêîðåé", kIn},
                                             {"ãîâîðèëà-îíà", kOut},
                                             {"ÿ-ïîêàæó-òåáå-ìàðøðóò", kFunc},
                                             {"çíàé-æå", kVarInit},
                                             {"ñïåøèò-â-äðóãóþ-ñòîðîíó", kElse},
                                             {"íàðèñóé", kDraw}};

Buffer Tokenize (const char * filename)
{
    assert(filename);

    Buffer tokens = AllocateBuffer(kTokenBuff, NUM_OF_TOKENS);

    Buffer program_text = ReadFileToBuffer(filename);

    printf("BUFFER: %s\n", program_text.data.sym);

    char * pos = program_text.data.sym;

    while (*pos != '\0')
    {
        if (isspace(*pos)) // FIXME change?
        {
            pos++;
            printf("SPACE SKIPPED\n");
        }
        else if (isdigit(*pos))
            ReadNum(&pos, &tokens);
        else if (*pos == ';')
            ReadDivider(&pos, &tokens);
        else if (*pos == ',')
            ReadComma(&pos, &tokens);
        else if (IsMathOp(pos))
            ReadOp(&pos, &tokens);
        else
        {
            printf("BEFORE READWORD: [%d][%c]\n", *pos, *pos);
            ReadWord(&pos, &tokens);
        }
    }

    return tokens;
}


void ReadComma (char ** pos, Buffer * tokens)
{
    assert(pos);
    assert(*pos);
    assert(tokens);

    (*pos)++;
    tokens->data.token[tokens->size] = {kComma, {}};
    tokens->size++;
}


void ReadWord (char ** pos, Buffer * tokens)
{
    assert(pos);
    assert(*pos);
    assert(tokens);

    if (**pos == 's' && *(*pos + 1) == 'i' && *(*pos + 2) == 'n')
    {
        tokens->data.token[tokens->size] = {kOp, {.op = kSin}};
        (*pos) += 3; // FIXME
        tokens->size++;
    }
    else if (**pos == 'c' && *(*pos + 1) == 'o' && *(*pos + 2) == 's')
    {
        tokens->data.token[tokens->size] = {kOp, {.op = kCos}};
        (*pos) += 3;
        tokens->size++;
    }
    else if (**pos == 't' && *(*pos + 1) == 'a' && *(*pos + 2) == 'n')
    {
        tokens->data.token[tokens->size] = {kOp, {.op = kTan}};
        (*pos) += 3;
        tokens->size++;
    }
    else if (**pos == 'c' && *(*pos + 1) == 't' && *(*pos + 2) == 'g')
    {
        tokens->data.token[tokens->size] = {kOp, {.op = kCtg}};
        (*pos) += 3;
        tokens->size++;
    }
    else if (**pos == 's' && *(*pos + 1) == 'q' && *(*pos + 2) == 'r' && *(*pos + 3) == 't')
    {
        tokens->data.token[tokens->size] = {kOp, {.op = kSqrt}};
        (*pos) += 4;
        tokens->size++;
    }
    else if (**pos == 's' && *(*pos + 1) == 'e' && *(*pos + 2) == 't')
    {
        tokens->data.token[tokens->size] = {kOp, {.op = kSetPixel}};
        (*pos) += 8;
        tokens->size++;
    }
    else if (**pos == 'd' && *(*pos + 1) == 'i' && *(*pos + 2) == 'f')
    {
        tokens->data.token[tokens->size] = {kOp, {.op = kDif}};
        (*pos) += 3;
        tokens->size++;
    }
    else
        ReadKeyWord(pos, tokens);
}


void ReadKeyWord (char ** pos, Buffer * tokens)
{
    assert(pos);
    assert(*pos);
    assert(tokens);

    char * name = ReadName(pos);

    int index = 0;
    if ((index = FindKeywordIndex(name)) >= 0)
    {
        tokens->data.token[tokens->size] = {kKeyWord, {.keyword = key_words[index].number}};
        tokens->size++;
        free(name);
    }
    else
        ReadVarOrFunc(pos, tokens, name);
}


int FindKeywordIndex (const char * name)
{
    int count = 0;

    for (count = 0; count < NUM_OF_KEY_WORDS; count++)
    {                                                  // àëãîðèòì Ðàáèíà-Êàðïà (õýøèðîâàíèå)
        if (strcmp(name, key_words[count].name) == 0)
            return count;
    }

    return -1;
}


char * ReadName (char ** pos)
{
    assert(pos);
    assert(*pos);

    char * name = (char *) calloc (LENGTH_OF_WORD, sizeof(char));
    assert(name); // FIXME check

    char * curr = name;

    while ((**pos < 0 && **pos > - ALPH_LENGTH) || **pos == '-' || isalpha(**pos))
    {
        *curr = **pos;
        printf("before ++ [%d][%c]\n", **pos, **pos);
        (*pos)++;
        printf("after ++ [%d][%c]\n", **pos, **pos);
        curr++;
    }
    *curr = '\0';

    printf("NAME: %s (inside readname)\n", name);

    return name;
}


void ReadVarOrFunc (char ** pos, Buffer * tokens, char * name)
{
    assert(pos);
    assert(*pos);
    assert(name);
    assert(tokens);
    printf("NAME: [%s]\n", name);

    printf("size: %d\n", tokens->size);

    tokens->data.token[tokens->size].type = kName;
    tokens->data.token[tokens->size].value.name.name = name;

    tokens->size++;
}


void ReadOp (char ** pos, Buffer * tokens)
{
    assert(pos);
    assert(*pos);
    assert(tokens);

    char sym = **pos;

    printf("BEFORE SWITCH: POS [%d][%c]\n", **pos, **pos);
    switch(sym)
    {
        case '(':
                {
                    (*pos)++;
                    tokens->data.token[tokens->size] = {kOp, {.op = kOpenBracket}};
                    tokens->size++;
                    break;
                }
        case ')':
                {
                    (*pos)++;
                    tokens->data.token[tokens->size] = {kOp, {.op = kCloseBracket}};
                    tokens->size++;
                    break;
                }
        case '{':
                {
                    (*pos)++;
                    tokens->data.token[tokens->size] = {kOp, {.op = kOpenCurlyBracket}};
                    tokens->size++;
                    break;
                }
        case '}':
                {
                    (*pos)++;
                    tokens->data.token[tokens->size] = {kOp, {.op = kCloseCurlyBracket}};
                    tokens->size++;
                    break;
                }
        case '+':
                {
                    (*pos)++;
                    tokens->data.token[tokens->size] = {kOp, {.op = kAdd}};
                    tokens->size++;
                    break;
                }
        case '-':
                {
                    (*pos)++;
                    tokens->data.token[tokens->size] = {kOp, {.op = kSub}};
                    tokens->size++;
                    break;
                }
        case '*':
                {
                    (*pos)++;
                    tokens->data.token[tokens->size] = {kOp, {.op = kMul}};
                    tokens->size++;
                    break;
                }
        case '/':
                {
                    (*pos)++;
                    tokens->data.token[tokens->size] = {kOp, {.op = kDiv}};
                    tokens->size++;
                    break;
                }
        case '^':
                {
                    (*pos)++;
                    tokens->data.token[tokens->size] = {kOp, {.op = kPow}};
                    tokens->size++;
                    break;
                }
        case '>':
                {
                    (*pos)++;
                    if (**pos == '=')
                    {
                        (*pos)++;
                        tokens->data.token[tokens->size] = {kOp, {.op = kGreaterThanOrEqual}};
                    }
                    else
                        tokens->data.token[tokens->size] = {kOp, {.op = kGreaterThan}};

                    tokens->size++;
                    break;
                }
        case '<':
                {
                    (*pos)++;
                    if (**pos == '=')
                    {
                        (*pos)++;
                        tokens->data.token[tokens->size] = {kOp, {.op = kLessThanOrEqual}};
                    }
                    else
                        tokens->data.token[tokens->size] = {kOp, {.op = kLessThan}};

                    tokens->size++;
                    break;
                }
        case '!':
                {
                    (*pos)++;
                    if (**pos != '=') {PRINTF_ERROR; return;}
                    (*pos)++;

                    tokens->data.token[tokens->size] = {kOp, {.op = kNotEqual}};
                    tokens->size++;
                    break;
                }
        case '=':
                {
                    (*pos)++;
                    if (**pos == '=')
                    {
                        (*pos)++;
                        tokens->data.token[tokens->size] = {kOp, {.op = kEqual}};
                    }
                    else
                        tokens->data.token[tokens->size] = {kOp, {.op = kAsn}};

                    tokens->size++;
                    break;


                    /*(*pos)++;
                    tokens->data.token[tokens->size] = {kOp, {.op = kAsn}};

                    tokens->size++;
                    break;*/
                }
        default:    PRINTF_ERROR;
    }
}


bool IsMathOp (char * pos)
{
    assert(pos);

    if (*pos == '>' || *pos == '<' || *pos == '!' || *pos == '=' ||
        *pos == '(' || *pos == ')' || *pos == '{' || *pos == '}' ||
        *pos == '+' || *pos == '-' || *pos == '*' || *pos == '/'||
        *pos == '^')
        return true;
    //else if (strcmp(pos, "áóäåò") == 0)
        //return true;

    return false;
}


void ReadNum (char ** pos, Buffer * tokens)
{
    assert(pos);
    assert(*pos);
    assert(tokens);

    int val = 0;
    while ('0' <= **pos && **pos <= '9')
    {
        val = **pos - '0' + val * 10;
        (*pos)++;
    }

    tokens->data.token[tokens->size] = {kNum, {.num = val}};
    tokens->size++;
}


void ReadDivider (char ** pos, Buffer * tokens)
{
    assert(pos);
    assert(*pos);
    assert(tokens);

    (*pos)++;
    tokens->data.token[tokens->size] = {kDivider, {}};
    tokens->size++;
}


Node * MakeNewNode (Type_t type, Value_t value, Node * left, Node * right)
{
    Node * node = AllocateNodeMemory();
    assert(node); // FIXME

    node->value = value;
    node->type  = type;

    node->left  = left;
    node->right = right;

    return node;
}


Node * AllocateNodeMemory (void)
{
    Node * node = (Node *) calloc (1, sizeof(Node));

    return node;
}


Node * MakeTree (Buffer * tokens)
{
    assert(tokens);

    Node * root = GetG(tokens);

    return root;
}

Node * GetG (Buffer * tokens)
{
    assert(tokens);

    int current = 0;
    Node * root = GetString(tokens, &current);
    Node * curr_node = root;
    assert(root);

    while (current < tokens->size)
    {
        Node * node_to_connect = GetString(tokens, &current);
        if(!node_to_connect) {PRINTF_ERROR; break;}
        curr_node->right = node_to_connect;
        curr_node = curr_node->right;
    }

    return root;
}


Node * GetString (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    Node * node = NULL;

    printf("Curr at get string %d\n", *current);

    switch (tokens->data.token[*current].type)
    {
        case kName:     node = GetVarOrFunc(tokens, current); break;
        case kKeyWord:  node = GetKeyword(tokens, current);   break;
        case kOp:       node = GetSetPixel(tokens, current);  break;
        case kNum:
        case kDivider:
        case kComma:
        default: return NULL;
    }

    if (IsDivider(tokens, *current))
    {
        node = MakeNewNode(kDivider, {}, node, NULL);
        //tokens->data.node[*current]->left = node;
        (*current)++;
    }
    else
    {
        printf("No divider! curr = %d\n", *current);
        return NULL;
    }

    return node;
}


Node * GetSetPixel (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    Node * node = NULL;

    if (tokens->data.token[*current].value.op == kSetPixel)
    {
        node = MAKE_NODE_FROM_CURR_TOKEN;
        (*current)++;

        if (!IsOpenBracket(tokens, *current))
            return NULL;
        else
            (*current)++;

        node->left = GetP(tokens, current);
        (*current)++; // skip comma
        node->right = GetP(tokens, current);

        if (!IsCloseBracket(tokens, *current))
            return NULL;
        else
            (*current)++;
    }
    else
        PRINTF_ERROR;

    return node;
}


Node * GetVarOrFunc (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    if (tokens->data.token[*current + 1].type == kOp &&
        tokens->data.token[*current + 1].value.op == kOpenBracket)
        return GetFunc(tokens, current);
    else
        return GetVarEq(tokens, current);
}


Node * GetVarEq (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    printf("CURRENT: %d\n", *current);

    Node * node = MAKE_NODE_FROM_CURR_TOKEN;

    (*current)++;

    if (IsAsn(tokens, *current))
        node->left = MAKE_NODE_FROM_CURR_TOKEN;
    else
        return NULL;

    (*current)++;

    node->left->left = GetE(tokens, current);

    return node;
}


Node * GetE (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    Node * node = GetC(tokens, current);

    printf("curr after GetC: %d\n", *current);

    while (tokens->data.token[*current].type == kOp &&
           tokens->data.token[*current].value.op < kAsn &&
           tokens->data.token[*current].value.op > kDiv) // FIXME
    {
        printf("TAG\n");
        Node * op = MAKE_NODE_FROM_CURR_TOKEN;

        (*current)++;

        op->left = node;
        op->right = GetC(tokens, current);
        node = op;
    }

    return node;
}


Node * GetC (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    Node * node = GetT(tokens, current);

    printf("curr after GetT: %d\n", *current);

    while (tokens->data.token[*current].type == kOp &&
          (tokens->data.token[*current].value.op == kAdd ||
           tokens->data.token[*current].value.op == kSub))
    {
        Node * op = MAKE_NODE_FROM_CURR_TOKEN;

        (*current)++;

        op->left = node;
        op->right = GetT(tokens, current);
        node = op;
    }

    return node;
}


Node * GetT (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    Node * node = GetS(tokens, current);

    printf("curr after GetS: %d\n", *current);

    while (tokens->data.token[*current].type == kOp &&
          (tokens->data.token[*current].value.op == kMul ||
           tokens->data.token[*current].value.op == kDiv))
    {
        Node * op = MAKE_NODE_FROM_CURR_TOKEN;

        (*current)++;

        op->left = node;
        op->right = GetS(tokens, current);
        node = op;
    }

    return node;
}


Node * GetS (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    Node * node = GetP(tokens, current);
    printf("curr after GetP: %d\n", *current);

    while (tokens->data.token[*current].type == kOp &&
           tokens->data.token[*current].value.op == kPow)
    {
        Node * op = MAKE_NODE_FROM_CURR_TOKEN;

        (*current)++;

        op->left = node;
        op->right = GetP(tokens, current);
        node = op;
    }

    return node;
}


Node * GetP (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    Node * node = NULL;

    if (tokens->data.token[*current].type == kOp &&
        tokens->data.token[*current].value.op == kOpenBracket)
    {
        printf("Current %d is open bracket\n", *current);

        (*current)++;
        node = GetE(tokens, current);

        if (!(tokens->data.token[*current].type == kOp &&
              tokens->data.token[*current].value.op == kCloseBracket))
        {PRINTF_ERROR; printf("current enum is %d\n", tokens->data.token[*current].value.op); return NULL;}

        (*current)++;
    }
    else if (tokens->data.token[*current].type == kNum)
    {
        node = MAKE_NODE_FROM_CURR_TOKEN;
        (*current)++;
    }
    else if (tokens->data.token[*current].type == kName) // check if var(not func)
    {
        node = MAKE_NODE_FROM_CURR_TOKEN;
        (*current)++;
    }
    else if (tokens->data.token[*current].type == kOp &&
             tokens->data.token[*current].value.op > kAsn &&
             tokens->data.token[*current].value.op < kOpenBracket) // FIXME
        node = GetMathFunc(tokens, current);

    return node;
}


Node * GetMathFunc (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    Node * node = MAKE_NODE_FROM_CURR_TOKEN;

    if(node->value.op != kSin && node->value.op != kCos &&
       node->value.op != kTan && node->value.op != kCtg &&
       node->value.op != kSqrt && node->value.op != kSetPixel &&
       node->value.op != kDif)
        return NULL;

    (*current)++;

    if (!IsOpenBracket(tokens, *current))
        return NULL;
    else
        (*current)++;

    node->left = GetE(tokens, current);

    if (node->value.op == kSetPixel || node->value.op == kDif)
    {
        node->right = GetP(tokens, current);
        (*current)++; // from comma - check?

        if (tokens->data.token[*current].type == kName)
        {
            node->right = MAKE_NODE_FROM_CURR_TOKEN;
            (*current)++;
        }
        else
            PRINTF_ERROR;
    }

    if (!IsCloseBracket(tokens, *current))
        return NULL;
    else
        (*current)++;

    return node;
}


bool IsAsn (Buffer * tokens, int current)
{
    assert(tokens);

    if (tokens->data.token[current].type == kOp
     || tokens->data.token[current].value.op == kAsn)
        return true;

    return false;
}


Node * GetFunc (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    printf("curr at get func: %d\n", *current);

    Node * node = MAKE_NODE_FROM_CURR_TOKEN;

    Node * curr_node = node;

    (*current)++;

    if (IsOpenBracket(tokens, *current))
        (*current)++;
    else
        return NULL;

    printf("read ( at get func\n");

    while (tokens->data.token[*current].type == kNum
       || (tokens->data.token[*current].type == kName)) // check if here is var(n not func)
    {
        printf("found var at get func\n");
        curr_node->left = MAKE_NODE_FROM_CURR_TOKEN;
        curr_node = curr_node->left;
        (*current)++;
    }

    if (IsCloseBracket(tokens, *current))
        (*current)++;
    else
        return NULL;

    if (IsOpenCurlyBracket(tokens, *current))
    {
        (*current)++;

        node->right = GetString(tokens, current);
        Node * curr_node2 = node->right;

        while (!IsCloseCurlyBracket(tokens, *current) && *current < tokens->size)
        {
            curr_node2->right = GetString(tokens, current);
            curr_node2 = curr_node2->right; // FIXME names
        }

        if (*current == tokens->size)
        {PRINTF_ERROR; return NULL;}

        (*current)++; // move from close bracket
    }

    return node;
}


bool IsDivider (Buffer * tokens, int current)
{
    assert(tokens);

    if (tokens->data.token[current].type == kDivider)
        return true;

    return false;
}


Node * GetKeyword (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    Node * node = NULL;

    printf("INSIDE KEYWORD %d\n", *current);
    switch(tokens->data.token[*current].value.keyword)
    {
        case kIf:       node = GetIfOrWhile (tokens, current, kIf); break;
        case kWhile:    node = GetIfOrWhile (tokens, current, kWhile); break;
        case kReturn:   node = GetReturn    (tokens, current); break;
        case kIn:
        case kOut:      node = GetInOrOut   (tokens, current); break;
        case kContinue:
        case kBreak:    {node = MAKE_NODE_FROM_CURR_TOKEN; (*current)++; break;}
        case kFunc:     node = GetFuncInit  (tokens, current); break;
        case kVarInit:  {printf("TAG\n");(*current)++; node = GetVarEq(tokens, current);}break;
        case kDraw:     {node = MAKE_NODE_FROM_CURR_TOKEN; (*current)++; break;}
        case kElse:
        default:        PRINTF_ERROR;
    }
    printf("BEFORE RET FROM KEYWORD %d\n", *current);
    return node;
}


Node * GetFuncInit (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    printf("CURR in func init %d\n", *current);
    Node * node = MAKE_NODE_FROM_CURR_TOKEN;

    (*current)++;

    node->left = GetFunc(tokens, current);

    return node;
}


Node * GetInOrOut (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    Node * node = MAKE_NODE_FROM_CURR_TOKEN;

    (*current)++;

    if (IsOpenBracket(tokens, *current))
        (*current)++;
    else
        {printf("ERROR WHILE GETTING ( IN OR OUT: curr = %d\n", *current);return NULL;}

    node->left = GetE(tokens, current);

    if (IsCloseBracket(tokens, *current))
        (*current)++;
    else
        {printf("ERROR WHILE GETTING ) IN OR OUT: curr = %d\n", *current);return NULL;}

    return node;
}


Node * GetReturn (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    Node * node = MAKE_NODE_FROM_CURR_TOKEN;

    (*current)++;

    if (IsOpenBracket(tokens, *current))
        (*current)++;

    node->left = GetE(tokens, current);

    if (IsCloseBracket(tokens, *current))
        (*current)++;

    return node;
}


Node * GetIfOrWhile (Buffer * tokens, int * current, KeyWord_t keyword)
{
    assert(tokens);
    assert(current);

    Node * node = MAKE_NODE_FROM_CURR_TOKEN;

    (*current)++; // move to open bracket
    printf("AFTER MOVE TO (: %d\n", *current);

    if (IsOpenBracket(tokens, *current))
        (*current)++;
    else
        {printf("Error! no open bracket");return NULL;}

    printf("AFTER MOVE FROM (: %d\n", *current);

    node->left = GetE(tokens, current);

    printf("AFTER GET E: %d\n", *current);

    if (IsCloseBracket(tokens, *current))
    {
        printf("Curr %d is close bracket\n", *current);
        (*current)++;
    }
    else
        {printf("Error! no close bracket");return NULL;}

    if (IsOpenCurlyBracket(tokens, *current))
    {
        printf("Curr %d is open curly bracket\n", *current);
        (*current)++;
    }
    else
        {printf("%d: Error! no open curly bracket", __LINE__);return NULL;}

    Node * curr_node = node;

    while (!(tokens->data.token[*current].type == kOp
          && tokens->data.token[*current].value.op == kCloseCurlyBracket))
    {
        curr_node->right = GetString(tokens, current);
        if(!curr_node->right) {PRINTF_ERROR; break;}
        curr_node = curr_node->right;
    }

    if (IsCloseCurlyBracket(tokens, *current))
        (*current)++;
    else
        {printf("Error! no close curly bracket");return NULL;}

    if (keyword == kIf)
    {
        if (tokens->data.token[*current].type == kKeyWord
         && tokens->data.token[*current].value.keyword == kElse)
            curr_node->right = GetElse(tokens, current);

    }

    printf("TO THE END OF GET IF OR WHILE!\n");
    return node;
}


Node * GetElse (Buffer * tokens, int * current)
{
    assert(tokens);
    assert(current);

    printf("Entered ELSE\n");

    Node * node = MAKE_NODE_FROM_CURR_TOKEN;
    (*current)++;

    if (IsOpenCurlyBracket(tokens, *current))
    {
        printf("Curr %d is open curly bracket\n", *current);
        (*current)++;
    }
    else
    {printf("%d: Error! no open curly bracket\n", __LINE__);return NULL;}

    Node * curr_node = node;

    while (!(tokens->data.token[*current].type == kOp
          && tokens->data.token[*current].value.op == kCloseCurlyBracket))
    {
        curr_node->right = GetString(tokens, current);
        if(!curr_node->right) {PRINTF_ERROR; break;}
        curr_node = curr_node->right;
    }

    if (IsCloseCurlyBracket(tokens, *current))
        (*current)++;
    else
        {printf("Error! no close curly bracket");return NULL;}

    return node;
}


bool IsCloseCurlyBracket (Buffer * tokens, int current)
{
    assert(tokens);

    if (tokens->data.token[current].type == kOp
     && tokens->data.token[current].value.op == kCloseCurlyBracket)
        return true;

    return false;
}


bool IsOpenCurlyBracket (Buffer * tokens, int current)
{
    assert(tokens);

    if (tokens->data.token[current].type == kOp
     && tokens->data.token[current].value.op == kOpenCurlyBracket)
        return true;

    return false;
}


bool IsCloseBracket (Buffer * tokens, int current)
{
    assert(tokens);

    if (tokens->data.token[current].type == kOp
     && tokens->data.token[current].value.op == kCloseBracket)
        return true;

    return false;
}


bool IsOpenBracket (Buffer * tokens, int current)
{
    assert(tokens);

    if (tokens->data.token[current].type == kOp
     && tokens->data.token[current].value.op == kOpenBracket)
        return true;

    return false;
}


void PutTreeToFile (Node * root)
{
    assert(root);

    FILE * fp = fopen("frontend_output.txt", "w");
    if (!fp) PRINTF_ERROR;

    PrintNode(fp, root);

    fclose(fp);
}


void PrintNode (FILE * fp, Node * node)
{
    assert(fp);
    assert(node);

    fprintf(fp, "(");
    PrintNodeValue(fp, node);

    if (node->left)
        PrintNode(fp, node->left);
    if (node->right)
        PrintNode(fp, node->right);

    fprintf(fp, ")");
}


void PrintNodeValue (FILE * fp, Node * node)
{
    assert(fp);
    assert(node);

    switch(node->type)
    {
        case kNum: fprintf(fp, "%d", node->value.num);             break;
        case kOp:
        {
            switch(node->value.op)
            {
                case kAdd:                fprintf(fp, "+");        break;
                case kSub:                fprintf(fp, "-");        break;
                case kMul:                fprintf(fp, "*");        break;
                case kDiv:                fprintf(fp, "\\");       break;
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
                case kSetPixel:           fprintf(fp, "setpixel"); break;
                case kDif:                fprintf(fp, "dif");      break;
                case kLn:
                case kOpenBracket:
                case kCloseBracket:
                case kOpenCurlyBracket:
                case kCloseCurlyBracket:
                default:                  PRINTF_ERROR;
            }
            break;
        }
        case kKeyWord:
        {
            switch(node->value.keyword)
            {
                case kIf:                 fprintf(fp, "if");       break;
                case kWhile:              fprintf(fp, "while");    break;
                case kContinue:           fprintf(fp, "continue"); break;
                case kBreak:              fprintf(fp, "break");    break;
                case kReturn:             fprintf(fp, "return");   break;
                case kIn:                 fprintf(fp, "in");       break;
                case kOut:                fprintf(fp, "out");      break;
                case kFunc:               fprintf(fp, "func");     break;
                case kElse:               fprintf(fp, "else");     break;
                case kDraw:               fprintf(fp, "draw");     break;
                case kVarInit:                                     break;
                default:                  PRINTF_ERROR;
            }
            break;
        }
        case kName:    fprintf(fp, "%s", node->value.name.name);   break;
        case kDivider: fprintf(fp, ";");                           break;
        case kComma:
        default:       PRINTF_ERROR;
    }
}


void ScanfStringUntilSpace (char ** pos, char * word)
{
    assert(pos);
    assert(*pos);
    assert(word);

    char * start = *pos;

    while (**pos != ' ')
        (*pos)++;

    **pos = '\0';
    sscanf(start, "%s", word);
    **pos = ' ';
}


char * AllocateWord (void)
{
    char * word = (char *) calloc (LENGTH_OF_WORD, sizeof(char));
    if (!word) PRINTF_ERROR;

    return word;
}


Buffer ReadFileToBuffer (const char * filename)
{
    assert(filename);

    FILE * fp = fopen(filename, "r");
    if (!fp) PRINTF_ERROR;

    int buffer_size = GetFileSize(filename);

    Buffer buffer = AllocateBuffer(kCharBuff, buffer_size + 1);
    buffer.size = buffer_size;

    printf("size %d\n", buffer_size);

    fread(buffer.data.sym, sizeof(char), (size_t)(buffer.size), fp);

    fclose(fp);

    for (int count = 0; count < buffer_size + 1; count++)
        printf("[%d][%c]\n", count, buffer.data.sym[count]);

    return buffer;
}


int GetFileSize (const char * filename)
{
    assert(filename);

    struct stat st;
    stat(filename, &st);

    return (int)st.st_size;
}


Buffer AllocateBuffer (TypeOfBuffer type, int size)
{
    Buffer buffer = {};
    buffer.size = 0;

    switch(type)
    {
        case kNodeBuff:
                        if (!(buffer.data.node = AllocateNodeBuff(size)))
                            PRINTF_ERROR;
                        break;
        case kCharBuff:
                        if (!(buffer.data.sym  = AllocateCharBuff(size)))
                            PRINTF_ERROR;
                        break;

        case kTokenBuff:
                        if (!(buffer.data.token  = AllocateTokenBuff(size)))
                            PRINTF_ERROR;
                        break;
        case kFuncBuff:
        case kVarBuff:
        default:            PRINTF_ERROR;
    }

    return buffer;
}


Node ** AllocateNodeBuff (int size)
{
    Node ** buffer = (Node **) calloc ((size_t)size, sizeof(Node *));
    if (!buffer) PRINTF_ERROR;

    return buffer;
}


char * AllocateCharBuff (int size)
{
    char * buffer = (char *) calloc ((size_t)size, sizeof(char));
    if (!buffer) PRINTF_ERROR;

    return buffer;
}


Token * AllocateTokenBuff (int size)
{
    Token * buffer = (Token *) calloc ((size_t)size, sizeof(Token));
    if (!buffer) PRINTF_ERROR;

    return buffer;
}


void NodeDtor (Node * node)
{
    assert(node);

    if (node->left)
        NodeDtor(node->left);
    if (node->right)
        NodeDtor(node->right);

    free(node);
}

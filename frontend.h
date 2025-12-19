#ifndef FRONTEND_H
#define FRONTEND_H

Buffer Tokenize       (const char * filename);
Buffer ReadFileToBuffer (const char * filename);

Node * MakeTree       (Buffer * tokens);
Node * MakeNewNode    (Type_t type, Value_t value, Node * left, Node * right);
void PutTreeToFile    (Node * root);
void PrintNode        (FILE * fp, Node * node);
void PrintNodeValue   (FILE * fp, Node * node);

// Read functions

void ReadNum     (char ** pos, Buffer * tokens);
void ReadDivider (char ** pos, Buffer * tokens);
void ReadWord    (char ** pos, Buffer * tokens);

// Recursive descent parser functions

Node * GetG         (Buffer * tokens);
Node * GetS         (Buffer * tokens, int * current);
Node * GetString    (Buffer * tokens, int * current);
Node * GetVarEq     (Buffer * tokens, int * current);
Node * GetE         (Buffer * tokens, int * current);
Node * GetC         (Buffer * tokens, int * current);
Node * GetT         (Buffer * tokens, int * current);
Node * GetP         (Buffer * tokens, int * current);
Node * GetMathFunc  (Buffer * tokens, int * current);
Node * GetFunc      (Buffer * tokens, int * current);
Node * GetKeyword   (Buffer * tokens, int * current);
Node * GetInOrOut   (Buffer * tokens, int * current);
Node * GetReturn    (Buffer * tokens, int * current);
Node * GetIfOrWhile (Buffer * tokens, int * current, KeyWord_t keyword);
Node * GetElse      (Buffer * tokens, int * current);
Node * GetFuncInit  (Buffer * tokens, int * current);
Node * GetVarOrFunc (Buffer * tokens, int * current);
Node * GetSetPixel  (Buffer * tokens, int * current);

// Utils

Buffer       AllocateBuffer     (TypeOfBuffer type, int size);
Node **      AllocateNodeBuff   (int size);
char *       AllocateCharBuff   (int size);
char *       AllocateWord       (void);
Node *       AllocateNodeMemory (void);
Token *      AllocateTokenBuff  (int size);

void ScanfStringUntilSpace      (char ** pos, char * word);
int GetFileSize                 (const char * filename);

bool IsAsn                      (Buffer * tokens, int current);
bool IsDivider                  (Buffer * tokens, int current);
bool IsCloseCurlyBracket        (Buffer * tokens, int current);
bool IsOpenCurlyBracket         (Buffer * tokens, int current);
bool IsCloseBracket             (Buffer * tokens, int current);
bool IsOpenBracket              (Buffer * tokens, int current);

void NodeDtor (Node * node);

bool IsMathOp      (char * pos);
void ReadOp        (char ** pos, Buffer * tokens);
void ReadKeyWord   (char ** pos, Buffer * tokens);
void ReadVarOrFunc (char ** pos, Buffer * tokens, char * name);
char * ReadName    (char ** pos);

int FindKeywordIndex (const char * name);
void ReadComma (char ** pos, Buffer * tokens);

#endif // FRONTEND_H

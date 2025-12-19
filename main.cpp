#include <stdio.h>

#include "data.h"
#include "reverse.h"
#include "MakeAstTreeFromFile.h"
#include "GraphDump.h"

const int NUM_OF_VARS = 20;
const int NUM_OF_FUNCS = 20;

int main (int argc, char * argv[])
{
    const char * input_file = "frontend_output.txt";
    const char * output_file = "reversend_output.txt";

    if (argc == 2)
        input_file = argv[1];
    else if (argc > 2)
    {
        printf("Invalid cmd line arguments\n");
        return -1;
    }

    Buffer tokens = Tokenize(input_file);

    PrintfTokens(&tokens);

    Buffer var_names = AllocateBuffer(kVarBuff, NUM_OF_VARS);
    Buffer func_names = AllocateBuffer(kFuncBuff, NUM_OF_FUNCS);

    Nametables table = {&var_names, &func_names};

    Node * root = MakeTreeFromTokens(&tokens, table);

    MakeGraphCodeFile(root);

    MakeCodeOnMyLang(root, output_file);

    return 0;
}

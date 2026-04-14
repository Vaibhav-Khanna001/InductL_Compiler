#include <iostream>
#include <cstdio>
#include "ast.hpp"
#include "parser.hpp"

// These are defined in the auto-generated parser/lexer files
extern int yyparse();
extern FILE* yyin;
extern ASTNode* root;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file.ind>" << std::endl;
        return 1;
    }

    // Open the InductL source file
    FILE* infile = fopen(argv[1], "r");
    if (!infile) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    // Set Flex to read from our file instead of stdin
    yyin = infile;

    std::cout << "--- Starting Compilation ---" << std::endl;

    // Start parsing
    int result = yyparse();

    if (result == 0 && root != nullptr) {
        std::cout << "--- Parse Successful! AST Output: ---" << std::endl;
        // This prints the entire tree structure starting from the root
        root->print(0);
    } else {
        std::cerr << "--- Parse Failed ---" << std::endl;
    }

    fclose(infile);
    return result;
}
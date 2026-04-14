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

    // 2. Check if parsing was successful before doing anything else
    if (result == 0 && root != nullptr) {
        std::cout << "--- Parse Successful! ---" << std::endl;

        // 3. Now start Semantic Analysis
        try {
            SymbolTable st;
            std::cout << "--- Starting Semantic Analysis ---" << std::endl;
            root->analyze(st); 
            std::cout << "--- Semantic Analysis Successful! ---" << std::endl;
            
            // 4. If semantics are good, print the tree or move to codegen
            std::cout << "--- AST Output: ---" << std::endl;
            root->print(0);

        } catch (const std::exception& e) {
            // Using a try-catch is a professional way to handle semantic errors
            // without just crashing the whole program.
            std::cerr << "Semantic Error: " << e.what() << std::endl;
            fclose(infile);
            return 1;
        }
    } else {
        std::cerr << "--- Parse Failed (Syntax Error) ---" << std::endl;
    }

    fclose(infile);
    return result;
}
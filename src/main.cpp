#include <iostream>
#include <cstdio>
#include "ast.hpp"
#include "parser.hpp"
#include "codegen.hpp" // Added for 3AC generation

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

    // 1. Start parsing
    int result = yyparse();

    // 2. Check if parsing was successful before doing anything else
    if (result == 0 && root != nullptr) {
        std::cout << "--- Parse Successful! ---" << std::endl;

        // 3. Start Semantic Analysis
        try {
            SymbolTable st;
            std::cout << "--- Starting Semantic Analysis ---" << std::endl;
            root->analyze(st); 
            std::cout << "--- Semantic Analysis Successful! ---" << std::endl;
            
            // 4. Print the AST (Visual representation)
            std::cout << "\n--- AST Output: ---" << std::endl;
            root->print(0);

            // 5. Start Three-Address Code (3AC) Generation
            std::cout << "\n--- Generating Intermediate Code ---" << std::endl;
            CodeGenerator cg;
            root->codegen(cg);
            
            // 6. Output the final 3AC
            cg.dump();

        } catch (const std::exception& e) {
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
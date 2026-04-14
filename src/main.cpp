#include <iostream>
#include <cstdio>
#include <exception>
#include "ast.hpp"
#include "parser.hpp"
#include "codegen.hpp"
#include "verifier.hpp"
#include "transpiler.hpp"

// Global pointers and functions defined in Bison/Flex
extern int yyparse();
extern FILE* yyin;
extern ASTNode* root;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file.ind>" << std::endl;
        return 1;
    }

    // 1. Open the InductL source file
    FILE* infile = fopen(argv[1], "r");
    if (!infile) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    yyin = infile;
    std::cout << "--- InductL Compiler: Starting Compilation ---" << std::endl;

    // 2. Syntax Analysis (Parsing)
    if (yyparse() == 0 && root != nullptr) {
        std::cout << "--- [1/4] Parse Successful! ---" << std::endl;

        try {
            // 3. Semantic Analysis (Name & Type Checking)
            SymbolTable st;
            std::cout << "--- [2/4] Starting Semantic Analysis ---" << std::endl;
            root->analyze(st); 
            std::cout << "    >> Semantic Pass: OK" << std::endl;
            
            // 4. Static Logic Verification (Inductive Verifier)
            // This proves the mathematical safety of the code
            std::cout << "--- [3/4] Starting Static Logic Verification ---" << std::endl;
            VerifierState vs;
            root->verify(vs);
            vs.dump(); 
            std::cout << "    >> Verification Pass: OK" << std::endl;

            // 5. Code Generation (Intermediate Representation)
            std::cout << "--- [4/4] Generating Three-Address Code (3AC) ---" << std::endl;
            CodeGenerator cg;
            root->codegen(cg);
            cg.dump();

            // 6. Transpilation (Target Code Generation)
            // This turns your 3AC logic into a runnable C program
            std::cout << "\n--- Finalizing Target Code ---" << std::endl;
            Transpiler transpiler;
            transpiler.generateC(cg.getInstructions(), "output.c");
            
            std::cout << "\nCOMPILATION COMPLETE!" << std::endl;
            std::cout << "To run your program, use:" << std::endl;
            std::cout << "gcc output.c -o induct_app && ./induct_app" << std::endl;

        } catch (const std::exception& e) {
            // Catching logic or semantic errors gracefully
            std::cerr << "\n[!] COMPILER ERROR: " << e.what() << std::endl;
            fclose(infile);
            return 1;
        }
    } else {
        std::cerr << "--- Parse Failed: Review syntax errors above ---" << std::endl;
    }

    fclose(infile);
    return 0;
}
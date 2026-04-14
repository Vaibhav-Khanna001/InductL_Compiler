#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include <string>
#include <vector>
#include <iostream>

class CodeGenerator {
    int tempCount = 0;
    int labelCount = 0;
    std::vector<std::string> instructions;
public:
    std::string newTemp() { return "_t" + std::to_string(tempCount++); }
    std::string newLabel() { return "L" + std::to_string(labelCount++); }
    
    void addInstruction(std::string instr) {
        instructions.push_back(instr);
    }

    void dump() {
        std::cout << "--- Generated Three-Address Code (3AC) ---" << std::endl;
        for (const auto& instr : instructions) {
            std::cout << instr << std::endl;
        }
    }
};

#endif
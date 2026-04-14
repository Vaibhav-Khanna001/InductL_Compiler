#ifndef CODEGEN_HPP
#define CODEGEN_HPP

#include <string>
#include <vector>
#include <iostream>

class CodeGenerator {
    int tempCount = 0;
    int labelCount = 0;
    std::vector<std::string> instructions; // This is private by default

public: // Everything below this is accessible from main.cpp
    std::string newTemp() { return "_t" + std::to_string(tempCount++); }
    std::string newLabel() { return "L" + std::to_string(labelCount++); }
    
    void addInstruction(const std::string& instr) {
        instructions.push_back(instr);
    }

    // THIS IS THE KEY FIX: The Public Getter
    const std::vector<std::string>& getInstructions() const {
        return instructions;
    }

    void dump() {
        std::cout << "\n--- Generated Three-Address Code (3AC) ---" << std::endl;
        for (const auto& instr : instructions) {
            std::cout << instr << std::endl;
        }
    }
};

#endif
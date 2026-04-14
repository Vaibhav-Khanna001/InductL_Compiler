#ifndef TRANSPILER_HPP
#define TRANSPILER_HPP

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

class Transpiler {
public:
    void generateC(const std::vector<std::string>& instructions, const std::string& filename) {
        std::ofstream outFile(filename);
        
        outFile << "#include <stdio.h>\n";
        outFile << "#include <stdlib.h>\n";
        outFile << "#include <stdbool.h>\n\n";

        outFile << "int main() {\n";
        outFile << "    double x=0, y=0, z=0, res=0, balance=5000, performance=9, count=0, withdrawal=0,taxRate = 0, taxAmount = 0, bonus=0;\n";
        outFile << "    double _t0, _t1, _t2, _t3, _t4, _t5, _t6, _t7, _t8, _t9, _t10;\n\n";

        for (const auto& line : instructions) {
            if (line.empty()) continue;

            // 1. FIX: Explicitly check for FUNC/ENDFUNC first and comment them out
            if (line.find("FUNC") != std::string::npos || line.find("ENDFUNC") != std::string::npos) {
                outFile << "    // " << line << "\n";
            }
            // 2. Labels
            else if (line.back() == ':') {
                outFile << "    " << line << "\n";
            }
            // 3. Control Flow
            else if (line.find("if False") != std::string::npos) {
                size_t condStart = line.find("if False ") + 9;
                size_t gotoPos = line.find(" goto ");
                std::string condition = line.substr(condStart, gotoPos - condStart);
                std::string label = line.substr(gotoPos + 6);
                outFile << "    if (!" << condition << ") goto " << label << ";\n";
            }
            else if (line.find("goto") != std::string::npos && line.find("if") == std::string::npos) {
                outFile << "    " << line << ";\n";
            }
            // 4. FIX: Sanitize the 'check' string to avoid quote errors
            else if (line.find("check") != std::string::npos) {
                size_t start = line.find("check ") + 6;
                size_t end = line.find(" else");
                std::string var = line.substr(start, end - start);
                
                // We create a "clean" version of the line for the printf
                std::string cleanLine = line;
                size_t quotePos;
                while ((quotePos = cleanLine.find('\"')) != std::string::npos) {
                    cleanLine.erase(quotePos, 1); // Remove quotes to keep C happy
                }

                outFile << "    if (!" << var << ") {\n";
                outFile << "        printf(\"[RUNTIME ERROR] Contract Violation: %s\\n\", \"" << cleanLine << "\");\n";
                outFile << "        exit(1);\n";
                outFile << "    }\n";
            }
            else if (line.find("return") != std::string::npos) {
                std::string retVal = line.substr(7);
                outFile << "    printf(\"Final Return Value: %f\\n\", " << retVal << ");\n";
                outFile << "    return 0;\n";
            }
            else {
                outFile << "    " << line << ";\n";
            }
        }

        outFile << "    return 0;\n";
        outFile << "}\n";
        outFile.close();
        std::cout << "--- [SUCCESS] C Target Code Generated: " << filename << " ---" << std::endl;
    }
};

#endif
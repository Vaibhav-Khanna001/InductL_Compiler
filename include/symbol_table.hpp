#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

struct Symbol {
    std::string name;
    std::string type;
    int line;
};

class SymbolTable {
private:
    // Stack of maps for nested scopes
    std::vector<std::unordered_map<std::string, Symbol>> scopes;

public:
    SymbolTable() {
        enterScope(); // Global scope
    }

    void enterScope() {
        scopes.push_back(std::unordered_map<std::string, Symbol>());
    }

    void exitScope() {
        if (scopes.size() > 1) scopes.pop_back();
    }

    bool declare(const std::string& name, const std::string& type, int line) {
        // Check if already declared in the CURRENT scope
        if (scopes.back().find(name) != scopes.back().end()) {
            return false; 
        }
        scopes.back()[name] = {name, type, line};
        return true;
    }

    bool lookup(const std::string& name) {
        // Check from current scope upwards to global
        for (int i = scopes.size() - 1; i >= 0; i--) {
            if (scopes[i].find(name) != scopes[i].end()) return true;
        }
        return false;
    }
};

#endif
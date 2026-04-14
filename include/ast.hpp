#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include "symbol_table.hpp"

// Professional Type System Enum
enum class DataType { NUMBER, STRING, BOOLEAN, VOID, ERROR };

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent) const = 0;
    virtual void analyze(SymbolTable& st) = 0;
    virtual DataType getType() const = 0; // Ensures type safety
};

// --- EXPRESSIONS ---

class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double val) : value(val) {}
    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "[Literal: " << value << "]" << std::endl;
    }
    void analyze(SymbolTable& st) override {} // Literals are always valid
    DataType getType() const override { return DataType::NUMBER; }
};

class VariableNode : public ASTNode {
    std::string name;
public:
    VariableNode(const std::string& name) : name(name) {}
    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "[Variable: " << name << "]" << std::endl;
    }
    void analyze(SymbolTable& st) override {
        if (!st.lookup(name)) {
            throw std::runtime_error("Semantic Error: Variable '" + name + "' used before declaration.");
        }
    }
    DataType getType() const override { return DataType::NUMBER; } // For now, all variables are numbers
};

class BinaryOpNode : public ASTNode {
    std::string op;
    ASTNode *left, *right;
public:
    BinaryOpNode(std::string op, ASTNode* l, ASTNode* r) : op(op), left(l), right(r) {}
    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "[BinaryOp: " << op << "]" << std::endl;
        left->print(indent + 1);
        right->print(indent + 1);
    }
    void analyze(SymbolTable& st) override {
        left->analyze(st);
        right->analyze(st);

        // Pro-Level Type Check: Ensure both sides are numeric for arithmetic
        if (left->getType() != DataType::NUMBER || right->getType() != DataType::NUMBER) {
            throw std::runtime_error("Type Mismatch: Operator '" + op + "' requires numeric operands.");
        }
    }
    DataType getType() const override {
        // Comparison operators return boolean logic, others return numbers
        if (op == ">" || op == "<" || op == "==" || op == ">=" || op == "<=") 
            return DataType::BOOLEAN;
        return DataType::NUMBER;
    }
};

// --- STATEMENTS ---

class AssignmentNode : public ASTNode {
    std::string varName;
    ASTNode* value;
public:
    AssignmentNode(std::string name, ASTNode* val) : varName(name), value(val) {}
    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "[Assign: " << varName << " = ]" << std::endl;
        value->print(indent + 1);
    }
    void analyze(SymbolTable& st) override {
        value->analyze(st);
        
        // Logical Polish: If not in scope, initialize it. If it is, ensure types match.
        if (!st.lookup(varName)) {
            st.declare(varName, "number", 0);
        } else {
            // Future: Check if updating a 'number' with a 'string'
            if (value->getType() != DataType::NUMBER) {
                throw std::runtime_error("Type Mismatch: Cannot assign non-number to '" + varName + "'.");
            }
        }
    }
    DataType getType() const override { return DataType::VOID; }
};

class BlockNode : public ASTNode {
    std::vector<ASTNode*> statements;
public:
    // This constructor takes the raw vector pointer from Bison
    BlockNode(std::vector<ASTNode*>* stmts) {
        if (stmts) {
            statements = *stmts;
            delete stmts; // Clean up the vector pointer to prevent memory leaks
        }
    }
    void print(int indent) const override {
        for (auto s : statements) s->print(indent);
    }
    void analyze(SymbolTable& st) override {
        for (auto s : statements) s->analyze(st);
    }
    DataType getType() const override { return DataType::VOID; }
};

class IfNode : public ASTNode {
    ASTNode *condition;
    BlockNode *thenBranch;
public:
    // Note: The second argument is exactly what Bison ($6) provides
    IfNode(ASTNode* cond, std::vector<ASTNode*>* thenB) 
        : condition(cond), thenBranch(new BlockNode(thenB)) {}

    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "[If]" << std::endl;
        condition->print(indent + 1);
        thenBranch->print(indent + 1);
    }
    void analyze(SymbolTable& st) override {
        condition->analyze(st);
        if (condition->getType() != DataType::BOOLEAN) {
            throw std::runtime_error("Semantic Error: 'if' condition must be boolean.");
        }
        st.enterScope();
        thenBranch->analyze(st);
        st.exitScope();
    }
    DataType getType() const override { return DataType::VOID; }
};

class WhileNode : public ASTNode {
    ASTNode *condition;
    BlockNode *body;
public:
    WhileNode(ASTNode* cond, std::vector<ASTNode*>* b) 
        : condition(cond), body(new BlockNode(b)) {}

    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "[While]" << std::endl;
        condition->print(indent + 1);
        body->print(indent + 1);
    }
    void analyze(SymbolTable& st) override {
        condition->analyze(st);
        st.enterScope();
        body->analyze(st);
        st.exitScope();
    }
    DataType getType() const override { return DataType::VOID; }
};

class ReturnNode : public ASTNode {
    ASTNode* value;
public:
    ReturnNode(ASTNode* val) : value(val) {}
    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "[Return]" << std::endl;
        value->print(indent + 1);
    }
    void analyze(SymbolTable& st) override {
        value->analyze(st);
    }
    DataType getType() const override { return value->getType(); }
};

// --- INDUCTIVE NOVELTY ---

class ConditionNode : public ASTNode {
    std::string type;
    ASTNode* condition;
public:
    ConditionNode(std::string type, ASTNode* cond) : type(type), condition(cond) {}
    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "Contract [" << type << "]:" << std::endl;
        condition->print(indent + 1);
    }
    void analyze(SymbolTable& st) override {
        condition->analyze(st);
        if (condition->getType() != DataType::BOOLEAN) {
            throw std::runtime_error("Contract Error: '" + type + "' must be a logical comparison.");
        }
    }
    DataType getType() const override { return DataType::VOID; }
};

class FunctionNode : public ASTNode {
    std::string name;
    std::vector<std::string> params;
    std::vector<ConditionNode*> preconditions;
    std::vector<ASTNode*> body;
    std::vector<ConditionNode*> postconditions;
public:
    FunctionNode(std::string name, std::vector<std::string> params) : name(name), params(params) {}
    
    void addPre(ConditionNode* c) { preconditions.push_back(c); }
    void addStmt(ASTNode* s) { body.push_back(s); }
    void addPost(ConditionNode* c) { postconditions.push_back(c); }

    void print(int indent) const override {
        std::cout << "Function: " << name << std::endl;
        for(auto p : preconditions) p->print(indent + 1);
        for(auto s : body) s->print(indent + 2);
        for(auto p : postconditions) p->print(indent + 1);
    }

    void analyze(SymbolTable& st) override {
        st.enterScope();
        // Register parameters
        for (const auto& p : params) {
            st.declare(p, "number", 0);
        }
        // Analyze logic and body
        for (auto p : preconditions) p->analyze(st);
        for (auto s : body) s->analyze(st);
        for (auto p : postconditions) p->analyze(st);
        st.exitScope();
    }
    DataType getType() const override { return DataType::VOID; }
};



#endif
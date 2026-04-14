#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include "symbol_table.hpp"
#include "codegen.hpp"

enum class DataType { NUMBER, STRING, BOOLEAN, VOID, ERROR };

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent) const = 0;
    virtual void analyze(SymbolTable& st) = 0;
    virtual DataType getType() const = 0;
    
    // The Back-End: Returns the 3AC code or the name of the temporary result
    virtual std::string codegen(CodeGenerator& cg) = 0;
};

// --- 1. BlockNode: Manages lists of statements ---
class BlockNode : public ASTNode {
    std::vector<ASTNode*> statements;
public:
    BlockNode(std::vector<ASTNode*>* stmts) {
        if (stmts) {
            statements = *stmts;
            delete stmts;
        }
    }
    void print(int indent) const override {
        for (auto s : statements) s->print(indent);
    }
    void analyze(SymbolTable& st) override {
        for (auto s : statements) s->analyze(st);
    }
    DataType getType() const override { return DataType::VOID; }

    std::string codegen(CodeGenerator& cg) override {
        std::string code = "";
        for (auto s : statements) code += s->codegen(cg);
        return code;
    }
};

// --- 2. Expressions ---

class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double val) : value(val) {}
    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "[Literal: " << value << "]" << std::endl;
    }
    void analyze(SymbolTable& st) override {}
    DataType getType() const override { return DataType::NUMBER; }

    std::string codegen(CodeGenerator& cg) override {
        return std::to_string(value); // Returns the value itself for use in 3AC
    }
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
        if (!st.lookup(name)) throw std::runtime_error("Undeclared variable: " + name);
    }
    DataType getType() const override { return DataType::NUMBER; }

    std::string codegen(CodeGenerator& cg) override {
        return name; // Returns the variable name for the 3AC instruction
    }
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
        left->analyze(st); right->analyze(st);
        if (left->getType() != DataType::NUMBER || right->getType() != DataType::NUMBER)
            throw std::runtime_error("Arithmetic error: Numbers required.");
    }
    DataType getType() const override {
        if (op == ">" || op == "<" || op == "==") return DataType::BOOLEAN;
        return DataType::NUMBER;
    }

    std::string codegen(CodeGenerator& cg) override {
        std::string l = left->codegen(cg);
        std::string r = right->codegen(cg);
        std::string temp = cg.newTemp();
        // Here is the actual 3AC instruction generation
        cg.addInstruction(temp + " = " + l + " " + op + " " + r);
        return temp; // Return the temp where result is stored
    }
};

// --- 3. Statements ---

class AssignmentNode : public ASTNode {
    std::string varName;
    ASTNode* value;
public:
    AssignmentNode(std::string name, ASTNode* val) : varName(name), value(val) {}
    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "[Assign: " << varName << "]" << std::endl;
        value->print(indent + 1);
    }
    void analyze(SymbolTable& st) override {
        value->analyze(st);
        st.declare(varName, "number", 0);
    }
    DataType getType() const override { return DataType::VOID; }

    std::string codegen(CodeGenerator& cg) override {
        std::string val = value->codegen(cg);
        cg.addInstruction(varName + " = " + val);
        return "";
    }
};

class IfNode : public ASTNode {
    ASTNode *condition;
    BlockNode *thenBranch;
public:
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
        st.enterScope(); thenBranch->analyze(st); st.exitScope();
    }
    DataType getType() const override { return DataType::VOID; }

    std::string codegen(CodeGenerator& cg) override {
        std::string cond = condition->codegen(cg);
        std::string labelExit = cg.newLabel();
        
        cg.addInstruction("if False " + cond + " goto " + labelExit);
        thenBranch->codegen(cg);
        cg.addInstruction(labelExit + ":");
        return "";
    }
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

    std::string codegen(CodeGenerator& cg) override {
        std::string startLabel = cg.newLabel();
        std::string exitLabel = cg.newLabel();

        cg.addInstruction(startLabel + ":");
        std::string cond = condition->codegen(cg);
        cg.addInstruction("if False " + cond + " goto " + exitLabel);
        
        body->codegen(cg);
        
        cg.addInstruction("goto " + startLabel);
        cg.addInstruction(exitLabel + ":");
        return "";
    }
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
    void analyze(SymbolTable& st) override { value->analyze(st); }
    DataType getType() const override { return value->getType(); }

    std::string codegen(CodeGenerator& cg) override {
        std::string val = value->codegen(cg);
        cg.addInstruction("return " + val);
        return "";
    }
};

// --- 4. Function & Contracts ---

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
    void analyze(SymbolTable& st) override { condition->analyze(st); }
    DataType getType() const override { return DataType::VOID; }

    std::string codegen(CodeGenerator& cg) override {
        std::string cond = condition->codegen(cg);
        std::string labelPass = cg.newLabel();
        cg.addInstruction("check " + cond + " else throw \"" + type + " Violation\"");
        return "";
    }
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
        for (const auto& p : params) st.declare(p, "number", 0);
        for (auto p : preconditions) p->analyze(st);
        for (auto s : body) s->analyze(st);
        for (auto p : postconditions) p->analyze(st);
        st.exitScope();
    }
    DataType getType() const override { return DataType::VOID; }

    std::string codegen(CodeGenerator& cg) override {
        cg.addInstruction("FUNC " + name + ":");
        for (auto p : preconditions) p->codegen(cg);
        for (auto s : body) s->codegen(cg);
        for (auto p : postconditions) p->codegen(cg);
        cg.addInstruction("ENDFUNC");
        return "";
    }
};

#endif
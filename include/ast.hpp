#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <map>
#include "symbol_table.hpp"
#include "codegen.hpp"
#include "verifier.hpp"

enum class DataType { NUMBER, STRING, BOOLEAN, VOID, ERROR };

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent) const = 0;
    virtual void analyze(SymbolTable& st) = 0;
    virtual DataType getType() const = 0;
    virtual std::string codegen(CodeGenerator& cg) = 0;
    
    // Pass 2: The Inductive Verifier
    virtual ValueRange verify(VerifierState& vs) = 0;
};

// --- BlockNode: Handles sequences of statements ---
class BlockNode : public ASTNode {
    std::vector<ASTNode*> statements;
public:
    BlockNode(std::vector<ASTNode*>* stmts) {
        if (stmts) { statements = *stmts; delete stmts; }
    }
    void print(int indent) const override { for (auto s : statements) s->print(indent); }
    void analyze(SymbolTable& st) override { for (auto s : statements) s->analyze(st); }
    DataType getType() const override { return DataType::VOID; }
    std::string codegen(CodeGenerator& cg) override {
        std::string code = "";
        for (auto s : statements) code += s->codegen(cg);
        return code;
    }
    ValueRange verify(VerifierState& vs) override {
        for (auto s : statements) s->verify(vs);
        return ValueRange();
    }
};

// --- Expression Nodes ---
class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double val) : value(val) {}
    void print(int indent) const override { std::cout << std::string(indent*2, ' ') << "[Literal: " << value << "]" << std::endl; }
    void analyze(SymbolTable& st) override {}
    DataType getType() const override { return DataType::NUMBER; }
    std::string codegen(CodeGenerator& cg) override { return std::to_string(value); }
    ValueRange verify(VerifierState& vs) override { return ValueRange(value, value); }
};

class VariableNode : public ASTNode {
    std::string name;
public:
    VariableNode(const std::string& name) : name(name) {}
    void print(int indent) const override { std::cout << std::string(indent*2, ' ') << "[Variable: " << name << "]" << std::endl; }
    void analyze(SymbolTable& st) override { if (!st.lookup(name)) throw std::runtime_error("Undeclared variable: " + name); }
    DataType getType() const override { return DataType::NUMBER; }
    std::string codegen(CodeGenerator& cg) override { return name; }
    ValueRange verify(VerifierState& vs) override { return vs.getConstraint(name); }
};

class BinaryOpNode : public ASTNode {
    std::string op;
    ASTNode *left, *right;
public:
    BinaryOpNode(std::string op, ASTNode* l, ASTNode* r) : op(op), left(l), right(r) {}
    void print(int indent) const override {
        std::cout << std::string(indent*2, ' ') << "[BinaryOp: " << op << "]" << std::endl;
        left->print(indent + 1); right->print(indent + 1);
    }
    void analyze(SymbolTable& st) override { left->analyze(st); right->analyze(st); }
    DataType getType() const override {
        if (op == ">" || op == "<" || op == "==" || op == ">=" || op == "<=") return DataType::BOOLEAN;
        return DataType::NUMBER;
    }
    std::string codegen(CodeGenerator& cg) override {
        std::string l = left->codegen(cg); std::string r = right->codegen(cg);
        std::string temp = cg.newTemp();
        cg.addInstruction(temp + " = " + l + " " + op + " " + r);
        return temp;
    }
    ValueRange verify(VerifierState& vs) override {
        ValueRange L = left->verify(vs);
        ValueRange R = right->verify(vs);
        if (op == "+") return ValueRange(L.min + R.min, L.max + R.max);
        if (op == "-") return ValueRange(L.min - R.max, L.max - R.min);
        if (op == "*") {
            double a = L.min * R.min, b = L.min * R.max, c = L.max * R.min, d = L.max * R.max;
            return ValueRange(std::min({a,b,c,d}), std::max({a,b,c,d}));
        }
        return ValueRange(); 
    }
    std::string getOp() { return op; }
    ASTNode* getLeft() { return left; }
    ASTNode* getRight() { return right; }
};

// --- Statement Nodes ---
class AssignmentNode : public ASTNode {
    std::string varName;
    ASTNode* value;
public:
    AssignmentNode(std::string name, ASTNode* val) : varName(name), value(val) {}
    void print(int indent) const override { std::cout << std::string(indent*2, ' ') << "[Assign: " << varName << "]" << std::endl; value->print(indent + 1); }
    void analyze(SymbolTable& st) override { value->analyze(st); st.declare(varName, "number", 0); }
    DataType getType() const override { return DataType::VOID; }
    std::string codegen(CodeGenerator& cg) override {
        std::string val = value->codegen(cg);
        cg.addInstruction(varName + " = " + val);
        return "";
    }
    ValueRange verify(VerifierState& vs) override {
        ValueRange res = value->verify(vs);
        vs.setConstraint(varName, res.min, res.max);
        return res;
    }
};

class ConditionNode : public ASTNode {
    std::string type; // "given" or "ensure"
    ASTNode* condition;
public:
    ConditionNode(std::string type, ASTNode* cond) : type(type), condition(cond) {}
    void print(int indent) const override { std::cout << std::string(indent*2, ' ') << "Contract [" << type << "]" << std::endl; condition->print(indent + 1); }
    void analyze(SymbolTable& st) override { condition->analyze(st); }
    DataType getType() const override { return DataType::VOID; }
    std::string codegen(CodeGenerator& cg) override {
        std::string cond = condition->codegen(cg);
        cg.addInstruction("check " + cond + " else throw \"" + type + " Violation\"");
        return "";
    }

    ValueRange verify(VerifierState& vs) override {
        BinaryOpNode* opNode = dynamic_cast<BinaryOpNode*>(condition);
        if (!opNode) return ValueRange();

        VariableNode* varNode = dynamic_cast<VariableNode*>(opNode->getLeft());
        NumberNode* numNode = dynamic_cast<NumberNode*>(opNode->getRight());
        std::string op = opNode->getOp();

        if (type == "given" && varNode && numNode) {
            // Logic: Strengthen the known range of the variable
            if (op == ">") vs.setConstraint(varNode->codegen(*(new CodeGenerator())), numNode->verify(vs).min + 0.001, 1e18);
            if (op == "<") vs.setConstraint(varNode->codegen(*(new CodeGenerator())), -1e18, numNode->verify(vs).max - 0.001);
        } 
        else if (type == "ensure") {
            // Logic: STATIC VERIFICATION CHECK
            // We calculate the current range of the expression being tested
            ValueRange actualRange = varNode ? vs.getConstraint(varNode->codegen(*(new CodeGenerator()))) : condition->verify(vs);
            ValueRange requiredRange = numNode->verify(vs);

            // If the math shows our 'actual' range could fail the 'required' range
            if (op == ">" && actualRange.min <= requiredRange.min) {
                throw std::runtime_error("INDL PROOF FAILED: Post-condition '" + type + "' cannot be guaranteed. \n" +
                                         "Minimum possible value (" + std::to_string(actualRange.min) + 
                                         ") is not strictly greater than " + std::to_string(requiredRange.min));
            }
        }
        return ValueRange();
    }
};

class IfNode : public ASTNode {
    ASTNode *condition;
    BlockNode *thenBranch;
public:
    IfNode(ASTNode* cond, std::vector<ASTNode*>* thenB) : condition(cond), thenBranch(new BlockNode(thenB)) {}
    void print(int indent) const override { std::cout << std::string(indent*2, ' ') << "[If]" << std::endl; condition->print(indent + 1); thenBranch->print(indent + 1); }
    void analyze(SymbolTable& st) override { condition->analyze(st); st.enterScope(); thenBranch->analyze(st); st.exitScope(); }
    DataType getType() const override { return DataType::VOID; }
    std::string codegen(CodeGenerator& cg) override {
        std::string cond = condition->codegen(cg);
        std::string labelExit = cg.newLabel();
        cg.addInstruction("if False " + cond + " goto " + labelExit);
        thenBranch->codegen(cg);
        cg.addInstruction(labelExit + ":");
        return "";
    }
    ValueRange verify(VerifierState& vs) override {
        thenBranch->verify(vs);
        return ValueRange();
    }
};

class WhileNode : public ASTNode {
    ASTNode *condition;
    BlockNode *body;
public:
    WhileNode(ASTNode* cond, std::vector<ASTNode*>* b) : condition(cond), body(new BlockNode(b)) {}
    void print(int indent) const override { std::cout << std::string(indent*2, ' ') << "[While]" << std::endl; condition->print(indent+1); body->print(indent+1); }
    void analyze(SymbolTable& st) override { condition->analyze(st); st.enterScope(); body->analyze(st); st.exitScope(); }
    DataType getType() const override { return DataType::VOID; }
    std::string codegen(CodeGenerator& cg) override {
        std::string start = cg.newLabel(); std::string exit = cg.newLabel();
        cg.addInstruction(start + ":");
        std::string cond = condition->codegen(cg);
        cg.addInstruction("if False " + cond + " goto " + exit);
        body->codegen(cg);
        cg.addInstruction("goto " + start);
        cg.addInstruction(exit + ":");
        return "";
    }
    ValueRange verify(VerifierState& vs) override { body->verify(vs); return ValueRange(); }
};

class ReturnNode : public ASTNode {
    ASTNode* value;
public:
    ReturnNode(ASTNode* val) : value(val) {}
    void print(int indent) const override { std::cout << std::string(indent*2, ' ') << "[Return]" << std::endl; value->print(indent + 1); }
    void analyze(SymbolTable& st) override { value->analyze(st); }
    DataType getType() const override { return value->getType(); }
    std::string codegen(CodeGenerator& cg) override {
        std::string val = value->codegen(cg);
        cg.addInstruction("return " + val);
        return "";
    }
    ValueRange verify(VerifierState& vs) override { return value->verify(vs); }
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
        for (auto p : preconditions) p->print(indent + 1);
        for (auto s : body) s->print(indent + 2);
        for (auto p : postconditions) p->print(indent + 1);
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
    
    ValueRange verify(VerifierState& vs) override {
        for (auto p : preconditions) p->verify(vs);
        for (auto s : body) s->verify(vs);
        for (auto p : postconditions) p->verify(vs);
        return ValueRange();
    }
};

#endif
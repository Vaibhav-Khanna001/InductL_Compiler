#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <vector>
#include <string>

// Base class for all nodes in the Abstract Syntax Tree
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent) const = 0; 
};

// --- EXPRESSIONS (Things that return a value) ---

class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double val) : value(val) {}
    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "[Literal: " << value << "]" << std::endl;
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
};

// --- STATEMENTS (Actions/Control Flow) ---

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
};

class IfNode : public ASTNode {
    ASTNode *condition, *thenBranch, *elseBranch;
public:
    IfNode(ASTNode* cond, ASTNode* thenB, ASTNode* elseB = nullptr) 
        : condition(cond), thenBranch(thenB), elseBranch(elseB) {}
    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "[If]" << std::endl;
        condition->print(indent + 1);
        thenBranch->print(indent + 1);
        if(elseBranch) elseBranch->print(indent + 1);
    }
};

class WhileNode : public ASTNode {
    ASTNode *condition, *body;
public:
    WhileNode(ASTNode* cond, ASTNode* b) : condition(cond), body(b) {}
    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "[While Loop]" << std::endl;
        condition->print(indent + 1);
        body->print(indent + 1);
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
};

// --- INDUCTIVE NOVELTY NODES ---

class ConditionNode : public ASTNode {
    std::string type; // "given" or "ensure"
    ASTNode* condition;
public:
    ConditionNode(std::string type, ASTNode* cond) : type(type), condition(cond) {}
    void print(int indent) const override {
        for(int i=0; i<indent; i++) std::cout << "  ";
        std::cout << "Contract [" << type << "]:" << std::endl;
        condition->print(indent + 1);
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
        std::cout << "Function Definition: " << name << std::endl;
        for(auto p : preconditions) p->print(indent + 1);
        std::cout << "  Implementation Body:" << std::endl;
        for(auto s : body) s->print(indent + 2);
        for(auto p : postconditions) p->print(indent + 1);
    }
};

#endif
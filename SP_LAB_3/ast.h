#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using Variables = std::unordered_map<std::string, int>;

struct ReturnSignal {
    int value;
};

namespace AstDot {
    inline std::string escape(const std::string& text) {
        std::string result;
        for (char ch : text) {
            if (ch == '\\' || ch == '"') result += '\\';
            if (ch == '\n') { result += "\\n"; continue; }
            result += ch;
        }
        return result;
    }

    inline int node(std::ostream& out, int& nextId, const std::string& label) {
        const int id = nextId++;
        out << "  node" << id << " [label=\"" << escape(label) << "\"];\n";
        return id;
    }

    inline void edge(std::ostream& out, int parent, int child, const std::string& label = "") {
        out << "  node" << parent << " -> node" << child;
        if (!label.empty()) out << " [label=\"" << escape(label) << "\"]";
        out << ";\n";
    }
}

class Expression {
public:
    virtual ~Expression() = default;
    virtual int evaluate(Variables& variables) const = 0;
    virtual int toDot(std::ostream& out, int& nextId) const = 0;
};

class NumberExpression : public Expression {
    int value;
public:
    explicit NumberExpression(int value) : value(value) {}
    int evaluate(Variables&) const override { return value; }
    int toDot(std::ostream& out, int& nextId) const override {
        return AstDot::node(out, nextId, std::to_string(value));
    }
};

class VariableExpression : public Expression {
    std::string name;
public:
    explicit VariableExpression(std::string name) : name(std::move(name)) {}
    int evaluate(Variables& variables) const override {
        auto it = variables.find(name);
        if (it == variables.end()) throw std::runtime_error("Undefined variable: " + name);
        return it->second;
    }
    int toDot(std::ostream& out, int& nextId) const override {
        return AstDot::node(out, nextId, "Variable: " + name);
    }
};

class BinaryExpression : public Expression {
    char operation;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
public:
    BinaryExpression(char operation, Expression* left, Expression* right)
        : operation(operation), left(left), right(right) {
    }
    int evaluate(Variables& variables) const override {
        int a = left->evaluate(variables);
        int b = right->evaluate(variables);
        switch (operation) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) throw std::runtime_error("Division by zero");
            return a / b;
        default: throw std::runtime_error("Unknown operator");
        }
    }
    int toDot(std::ostream& out, int& nextId) const override {
        int id = AstDot::node(out, nextId, std::string("Operator: ") + operation);
        int lhs = left->toDot(out, nextId);
        int rhs = right->toDot(out, nextId);
        AstDot::edge(out, id, lhs, "left");
        AstDot::edge(out, id, rhs, "right");
        return id;
    }
};

class Statement {
public:
    virtual ~Statement() = default;
    virtual void execute(Variables& variables) const = 0;
    virtual int toDot(std::ostream& out, int& nextId) const = 0;
};

class DeclarationStatement : public Statement {
    std::string name;
    std::unique_ptr<Expression> expression;

public:
    DeclarationStatement(
        std::string name,
        Expression* expression
    ) : name(std::move(name)), expression(expression) {
    }

    void execute(Variables& variables) const override {
        if (variables.count(name) != 0) {
            throw std::runtime_error(
                "Variable already declared: " + name);
        }

        int value = expression->evaluate(variables);
        variables[name] = value;
    }

    int toDot(std::ostream& out, int& nextId) const override {
        int id = AstDot::node(
            out, nextId, "Declare: " + name);

        AstDot::edge(
            out, id,
            expression->toDot(out, nextId),
            "value");

        return id;
    }
};

class AssignmentStatement : public Statement {
    std::string name;
    std::unique_ptr<Expression> expression;
public:
    AssignmentStatement(std::string name, Expression* expression)
        : name(std::move(name)), expression(expression) {
    }
    void execute(Variables& variables) const override {
        auto it = variables.find(name);

        if (it == variables.end()) {
            throw std::runtime_error(
                "Undefined variable: " + name);
        }

        int value = expression->evaluate(variables);
        it->second = value;
    }
    int toDot(std::ostream& out, int& nextId) const override {
        int id = AstDot::node(out, nextId, "Assign: " + name);
        AstDot::edge(out, id, expression->toDot(out, nextId), "value");
        return id;
    }
};

class PrintStatement : public Statement {
    std::unique_ptr<Expression> expression;
public:
    explicit PrintStatement(Expression* expression) : expression(expression) {}
    void execute(Variables& variables) const override {
        std::cout << expression->evaluate(variables) << '\n';
    }
    int toDot(std::ostream& out, int& nextId) const override {
        int id = AstDot::node(out, nextId, "Print");
        AstDot::edge(out, id, expression->toDot(out, nextId));
        return id;
    }
};

class ReturnStatement : public Statement {
    std::unique_ptr<Expression> expression;
public:
    explicit ReturnStatement(Expression* expression) : expression(expression) {}
    void execute(Variables& variables) const override {
        int result = expression->evaluate(variables);
        throw ReturnSignal{ result };
    }

    int toDot(std::ostream& out, int& nextId) const override {
        int id = AstDot::node(out, nextId, "Return");
        AstDot::edge(out, id, expression->toDot(out, nextId));
        return id;
    }
};

class ComparisonExpression : public Expression {
    std::string operation;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
public:
    ComparisonExpression(std::string operation, Expression* left, Expression* right)
        : operation(std::move(operation)), left(left), right(right) {
    }
    int evaluate(Variables& variables) const override {
        int a = left->evaluate(variables);
        int b = right->evaluate(variables);
        if (operation == "==") return a == b;
        if (operation == "!=") return a != b;
        if (operation == "<") return a < b;
        if (operation == ">") return a > b;
        if (operation == "<=") return a <= b;
        if (operation == ">=") return a >= b;
        throw std::runtime_error("Unknown comparison operator");
    }
    int toDot(std::ostream& out, int& nextId) const override {
        int id = AstDot::node(out, nextId, "Compare: " + operation);
        int lhs = left->toDot(out, nextId);
        int rhs = right->toDot(out, nextId);
        AstDot::edge(out, id, lhs, "left");
        AstDot::edge(out, id, rhs, "right");
        return id;
    }
};

class IfStatement : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenBranch;
    std::unique_ptr<Statement> elseBranch;
public:
    IfStatement(Expression* condition, Statement* thenBranch, Statement* elseBranch)
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {
    }
    void execute(Variables& variables) const override {
        if (condition->evaluate(variables)) thenBranch->execute(variables);
        else if (elseBranch) elseBranch->execute(variables);
    }
    int toDot(std::ostream& out, int& nextId) const override {
        int id = AstDot::node(out, nextId, "If");
        AstDot::edge(out, id, condition->toDot(out, nextId), "condition");
        AstDot::edge(out, id, thenBranch->toDot(out, nextId), "then");
        if (elseBranch) AstDot::edge(out, id, elseBranch->toDot(out, nextId), "else");
        return id;
    }
};

class BlockStatement : public Statement {
    std::vector<std::unique_ptr<Statement>> statements;
public:
    void addStatement(Statement* statement) { statements.emplace_back(statement); }
    void execute(Variables& variables) const override {
        for (const auto& statement : statements) statement->execute(variables);
    }
    int toDot(std::ostream& out, int& nextId) const override {
        int id = AstDot::node(out, nextId, "Block");
        for (const auto& statement : statements)
            AstDot::edge(out, id, statement->toDot(out, nextId));
        return id;
    }
};

class Program {
    std::vector<std::unique_ptr<Statement>> statements;
public:
    void addStatement(Statement* statement) { statements.emplace_back(statement); }
    std::vector<std::unique_ptr<Statement>> releaseStatements() {
        return std::move(statements);
    }
    int execute() const {
        Variables variables;

        try {
            for (const auto& statement : statements) {
                statement->execute(variables);
            }
        }
        catch (const ReturnSignal& signal) {
            return signal.value;
        }

        return 0;
    }
    void exportToDot(const std::string& path) const {
        std::ofstream out(path);
        if (!out) throw std::runtime_error("Cannot create AST file: " + path);
        out << "digraph AST {\n"
            << "  rankdir=TB;\n"
            << "  node [shape=box, style=rounded, fontname=Arial];\n"
            << "  edge [fontname=Arial];\n";
        int nextId = 0;
        int root = AstDot::node(out, nextId, "Program");
        for (const auto& statement : statements)
            AstDot::edge(out, root, statement->toDot(out, nextId));
        out << "}\n";
        if (!out) throw std::runtime_error("Failed to write AST file: " + path);
    }
};

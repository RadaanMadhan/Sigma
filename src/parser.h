#pragma once

#include <functional>

#include "tokenization.h"
#include <variant>

#include "arena.h"

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeTermParen {
    NodeExpr* expr;
};

struct NodeBinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprMulti {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprSub {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprDiv {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd *, NodeBinExprMulti *, NodeBinExprSub*, NodeBinExprDiv*> var;
};

struct NodeTerm {
    std::variant<NodeTermIntLit *, NodeTermIdent *, NodeTermParen*> var;
};

struct NodeExpr {
    std::variant<NodeTerm *, NodeBinExpr *> var;
};


struct NodeStmtExit {
    NodeExpr *expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr *expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit *, NodeStmtLet *> var;
};

struct NodeProg {
    std::vector<NodeStmt *> stmt;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)),
          m_allocator(1024 * 1024 * 4) //4mb
    {
    }


    std::optional<NodeTerm *> parse_term() {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = int_lit.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        } else if (auto ident = try_consume(TokenType::ident)) {
            auto term_ident = m_allocator.alloc<NodeTermIdent>();
            term_ident->ident = ident.value();
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        } else if (auto open_paren = try_consume(TokenType::open_paren)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                std::cerr<<"Expected expression"<<std::endl;
                exit(EXIT_FAILURE);
            }
            if (auto close_paren = try_consume(TokenType::close_paren)) {
                auto term_paren = m_allocator.alloc<NodeTermParen>();
                term_paren -> expr = expr.value();
                auto term = m_allocator.alloc<NodeTerm>();
                term->var = term_paren;
                return term;
            } else {
                std::cerr<<"Expected ')' "<<std::endl;
                exit(EXIT_FAILURE);
            }

        }else {
            return {};
        }
    }

    std::optional<NodeExpr *> parse_expr(int min_prec = 0) {
        auto term_left = parse_term();
        if (!term_left.has_value()) {
            return {};
        }
        auto expr_lhs = m_allocator.alloc<NodeExpr>();
        expr_lhs->var = term_left.value();
        while (true) {
            std::optional<int> precedence;
            if (auto cur_token = peek()) {
                precedence = get_precedence(cur_token->type);
                if (!precedence.has_value() || precedence.value() < min_prec) {
                    break;
                }
            }else {
                break;
            }

            Token op = consume();
            int next_min_precedence = precedence.value() + 1;
            auto expr_rhs = parse_expr(next_min_precedence);
            if (!expr_rhs.has_value()) {
                std::cerr << "Error parsing expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto bin_expr = m_allocator.alloc<NodeBinExpr>();
            auto expr_lhs_node = m_allocator.alloc<NodeExpr>();
            if (op.type == TokenType::plus) {
                auto add = m_allocator.alloc<NodeBinExprAdd>();
                expr_lhs_node->var = expr_lhs -> var;
                add->lhs = expr_lhs_node;
                add->rhs = expr_rhs.value();
                bin_expr->var = add;
            } else if (op.type == TokenType::multi) {
                auto multi = m_allocator.alloc<NodeBinExprMulti>();
                expr_lhs_node->var = expr_lhs -> var;
                multi->lhs = expr_lhs_node;
                multi->rhs = expr_rhs.value();
                bin_expr->var = multi;
            } else if (op.type == TokenType::minus) {
                auto sub = m_allocator.alloc<NodeBinExprSub>();
                expr_lhs_node->var = expr_lhs -> var;
                sub->lhs = expr_lhs_node;
                sub->rhs = expr_rhs.value();
                bin_expr->var = sub;
            } else if (op.type == TokenType::div) {
                auto div = m_allocator.alloc<NodeBinExprDiv>();
                expr_lhs_node->var = expr_lhs -> var;
                div->lhs = expr_lhs_node;
                div->rhs = expr_rhs.value();
                bin_expr->var = div;
            }

            expr_lhs->var = bin_expr;
        }
        return expr_lhs;
    }

    std::optional<NodeStmt *> parse_stmt() {
        if (peek().value().type == TokenType::exit && peek(1).has_value() && peek(1).value().type ==
            TokenType::open_paren) {
            //creating variable inside if statement causes type to become bool and is true if expression has as value and goes into else if it doesn't
            consume();
            consume();
            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
            if (auto expr = parse_expr()) {
                stmt_exit->expr = *expr;
            } else {
                std::cerr << "invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "Expected ')'");
            try_consume(TokenType::semi, "Expected ';'");
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_exit;
            return stmt;
        } else if (peek().has_value() && peek().value().type == TokenType::let &&
                   peek(1).has_value() && peek(1).value().type == TokenType::ident &&
                   peek(2).has_value() && peek(2).value().type == TokenType::eq) {
            consume();
            auto stmt_let = m_allocator.alloc<NodeStmtLet>();
            stmt_let->ident = consume();
            consume();
            if (auto expr = parse_expr()) {
                stmt_let->expr = expr.value();
            } else {
                std::cerr << "invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semi, "Expected ';'");
            auto stmt = m_allocator.alloc<NodeStmt>();
            stmt->var = stmt_let;
            return stmt;
        } else {
            return {};
        }
    }

    std::optional<NodeProg> parse_prog() {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmt.push_back(stmt.value());
            } else {
                std::cerr << "invalid statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

private:
    [[nodiscard]] std::optional<Token> peek(int const offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        } else {
            return m_tokens.at(m_index + offset);
        }
    }

    inline Token consume() {
        return m_tokens.at(m_index++);
    }


    inline std::optional<Token> try_consume(TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        } else {
            return {};
        }
    }

    inline std::optional<Token> try_consume(TokenType type, const std::string &err_msg) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        } else {
            std::cerr << err_msg << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    ArenaAllocator m_allocator;
};

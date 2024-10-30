#pragma once

#include <unordered_map>
#include <cassert>

#include "tokenization.h"
#include "parser.h"


class Generator {
public:
    inline explicit Generator(NodeProg prog)
        : m_prog(std::move(prog))
    {
    }

    void gen_term(const NodeTerm* term){
        struct TermVisitor {
            Generator* gen;
            void operator()(const NodeTermIntLit* term_int_lit) const{
                gen->m_output << "\tmov rax, " << term_int_lit -> int_lit.value.value() << "\n";
                gen->push("rax");
            }
            void operator()(const NodeTermIdent* term_ident) const{
                if (!gen->m_vars.contains(term_ident->ident.value.value())) {
                    std::cerr << "ERROR: Identifier " << term_ident->ident.value.value() << " does not exist\n";
                    exit(EXIT_FAILURE);
                }
                const auto& var = gen->m_vars.at(term_ident->ident.value.value());
                std::stringstream offset;
                //just the syntax for int lit in nasm assembly
                offset << "qword [rsp + " << 8 * (gen->m_stack_size - var.stack_loc - 1)  << "]";
                gen->push(offset.str());
            }
            void operator() (const NodeTermParen* term_paren) {
                gen->gen_expr(term_paren->expr);
            }
        };
        TermVisitor visitor{.gen = this};
        std::visit(visitor, term->var);
    }

    void gen_bin_expr(const NodeBinExpr* bin_expr) {
        struct BinExprVisitor {
            Generator* gen;
            void operator()(const NodeBinExprAdd* bin_expr_add) const {
                gen->gen_expr(bin_expr_add->lhs);
                gen->gen_expr(bin_expr_add->rhs);
                gen->pop("rbx");
                gen->pop("rax");
                gen->m_output<<"\tadd rax, rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprMulti* bin_expr_multi) const {
                gen->gen_expr(bin_expr_multi->lhs);
                gen->gen_expr(bin_expr_multi->rhs);
                gen->pop("rbx");
                gen->pop("rax");
                gen->m_output<<"\tmul rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprSub* bin_expr_sub) const {
                gen->gen_expr(bin_expr_sub->lhs);
                gen->gen_expr(bin_expr_sub->rhs);
                gen->pop("rbx");
                gen->pop("rax");
                gen->m_output<<"\tsub rax, rbx\n";
                gen->push("rax");
            }
            void operator()(const NodeBinExprDiv* bin_expr_div) const {
                gen->gen_expr(bin_expr_div->lhs);
                gen->gen_expr(bin_expr_div->rhs);
                gen->pop("rbx");
                gen->pop("rax");
                gen->m_output<<"\tdiv rbx\n";
                gen->push("rax");
            }
        };
        BinExprVisitor visitor{.gen = this};
        std::visit(visitor, bin_expr->var);
    }

    void gen_expr(const NodeExpr* expr) {
        struct ExprVisitor {
            Generator* gen;
            void operator()(const NodeTerm* term) const
            {
                gen->gen_term(term);
            }
            void operator()(const NodeBinExpr* bin_expr)const
            {
                gen->gen_bin_expr(bin_expr);
            }
        };

        ExprVisitor visitor{.gen = this};
        std::visit(visitor, expr->var);
    }

     void gen_stmt(const NodeStmt* stmt) {
        struct StmtVisitor {
            Generator* gen;
            void operator() (const NodeStmtExit* stmt_exit) const
            {
                gen->gen_expr(stmt_exit->expr);
                gen -> m_output <<   "\tmov rax, 60\n";
                gen -> pop("rdi");
                gen -> m_output <<   "\tsyscall\n";
            }
            void operator()(const NodeStmtLet* stmt_let) const
            {
                if(gen->m_vars.contains(stmt_let->ident.value.value())) {
                    std::cerr << "Identifier " << stmt_let->ident.value.value() << " already exists" << std::endl;
                    exit(EXIT_FAILURE);
                }
                gen->m_vars.insert({stmt_let->ident.value.value(), Var {.stack_loc = gen->m_stack_size}});
                gen->gen_expr(stmt_let->expr);
            }
        };

        StmtVisitor visitor{.gen = this};
        std::visit(visitor, stmt->var);
    }



    [[nodiscard]] std::string gen_prog() {
        m_output << "global _start\n_start:\n";

        for (const NodeStmt* stmt: m_prog.stmt) {
            gen_stmt(stmt);
        }



        m_output <<   "\tmov rax, 60\n";
        m_output <<   "\tmov rdi, 0\n";
        m_output <<   "\tsyscall";
        return m_output.str();
    }

private:

    void push (const std::string& reg) {
        m_output << "\tpush "<< reg <<"\n";
        m_stack_size++;
    }

    void pop (const std::string& reg) {
        m_output << "\tpop " << reg <<"\n";
        m_stack_size--;
    }

    struct Var {
        size_t stack_loc;
    };
    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Var> m_vars {};

};
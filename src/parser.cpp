#include "parser.h"
#include "lexer.h"
#include "misc.h"
#include "exceptions.h"
#include <fstream>
#include <map>
#include <cassert>
#include <array>
#include <memory>
#include <algorithm>
#include <format>
#include "var_manager.h"
#include "bytecode_emitter.h"
#include "expr_semantic.h"
#include "lang_stdlib.h"

namespace {
    using namespace ast;
    using namespace parser;

    interpreter::BytecodeEmitter *emitter;
    VarManager vars;
    std::vector<LoopManager> loops;
    int jmp_uid = 0;

    bool match(int token_type) {
        if (token_type != cur.token) return false;
        get_tok();
        return true;
    }

    bool test(int token_type) {
        return (token_type == cur.token);
    }

    unique_ptr<Node> parse_precedence(int prec);

    template<typename T>
    bool parse_param_list(int end_token, T check_add) {
        if (match(end_token)) return true;
        while (true) {
            if (!check_add()) return false;
            if (match(end_token)) return true;
            if (!match(TOKEN_COMMA)) parser_throws(error_msg(", in parameter list"));
        }
    }

    //   cur.token = ->
    std::unique_ptr<FunctionSignature> parse_fn_params(std::string first = "") {
        auto res = std::make_unique<FunctionSignature>();
        res->name = "";
        if (!first.empty()) res->params.push_back(first);
        if (!parse_param_list(TOKEN_RPAREN, [&]() {
            if (cur.token != TOKEN_IDENTIFIER)
                parser_throws(error_msg("identifier in function definition"));
            res->params.push_back(cur.identifier);
            get_tok();
            return true;
        }))
            return nullptr;
        return res;
    }

    std::unique_ptr<Node> pfn_identifier() {
        auto res = std::make_unique<VarExpr>(cur.identifier);
        return (get_tok(), std::move(res));
    }

    std::unique_ptr<Node> pfn_number() {
        int64_t mul = 1;
        int start = 0;
        if (cur.identifier.size() > 1 && cur.identifier[0] == '-') {
            if (cur.identifier.size() > 20 ||
                cur.identifier.size() == 20 && cur.identifier > "-9223372036854775808")
                parser_throws("Invalid number format: " + cur.identifier);
            mul = -1;
            start = 1;
        } else if (cur.identifier.size() > 19 ||
                   cur.identifier.size() == 19 && cur.identifier > "9223372036854775807")
            parser_throws("Invalid number format: " + cur.identifier);
        int64_t res = 0;
        for (int i = start; i < cur.identifier.size(); ++i) {
            res *= 10ll;
            res += (int64_t) (cur.identifier[i] - '0') * mul;
        }
        get_tok();
        return std::make_unique<IntLitExpr>(res);
    }

    std::unique_ptr<Node> pfn_unary() {
        get_tok();
        if (test(TOKEN_INT_LIT)) {
            cur.identifier = "-" + cur.identifier;
            return pfn_number();
        }
        auto res = parse_precedence(PREC_UNARY);
        return res ? std::make_unique<UnaryExpr<UnaryOpType::MINUS>>(std::move(res)) : nullptr;
    }

    std::unique_ptr<Node> pfn_grouping();

    std::unique_ptr<Node> ifn_call(std::unique_ptr<Node> lhs) {
        std::unique_ptr<FunctionCall> f = std::make_unique<FunctionCall>(std::move(lhs));
        get_tok();
        if (!parse_param_list(TOKEN_RPAREN, [&]() {
            auto res = parse_expression();
            return !(res == nullptr) && (f->args.push_back(std::move(res)), true);
        }))
            return nullptr;
        return f;
    }

    std::unique_ptr<Node> ifn_arrayget(std::unique_ptr<Node> lhs) {
        get_tok();
        auto x = parse_expression();
        if (!x || !match(TOKEN_RBRACKET)) parser_throws(error_msg("]"));
        return std::make_unique<ArrayGet>(std::move(lhs), std::move(x));
    }

    std::unique_ptr<Node> ifn_member(std::unique_ptr<Node> lhs) {
        get_tok();
        if (!match(TOKEN_IDENTIFIER)) parser_throws(error_msg("member name after ."));
        return std::make_unique<MemberGet>(std::move(lhs), prv.identifier);
    }

    std::unique_ptr<Node> ifn_binary(std::unique_ptr<Node> lhs);

    std::unique_ptr<Node> pfn_string_lit() {
        get_tok();
        return std::make_unique<StringLitExpr>(prv.identifier);
    }
// clang-format off
// @formatter:off

    RuleInfo rules[] = {
    /* TOKEN_EOF */        {nullptr,        nullptr,    PREC_NONE},
    /* TOKEN_ADD */        {nullptr,        ifn_binary, PREC_ADD},
    /* TOKEN_SUB */        {pfn_unary,        ifn_binary, PREC_ADD},
    /* TOKEN_MUL */        {nullptr,        ifn_binary, PREC_FACTOR},
    /* TOKEN_DIV */        {nullptr,        ifn_binary, PREC_FACTOR},
    /* TOKEN_MOD */        {nullptr,        ifn_binary, PREC_FACTOR},
    /* TOKEN_IDENTIFIER */ {pfn_identifier, nullptr,    PREC_PRIMARY},
    /* TOKEN_INT_LIT */    {pfn_number,     nullptr,    PREC_PRIMARY},
    /* TOKEN_STR_LIT */    {pfn_string_lit,     nullptr,    PREC_PRIMARY},
    /* TOKEN_LBRACKET */   {nullptr,   ifn_arrayget,    PREC_CALL},
    /* TOKEN_RBRACKET */   {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_LPAREN */     {pfn_grouping,   ifn_call,    PREC_CALL},
    /* TOKEN_RPAREN */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_LCURLY */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_RCURLY */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_FN */         {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_CONTINUE */   {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_BREAK */      {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_IF */         {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_ELSE */       {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_FOR */        {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_WHILE */      {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_RETURN */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_ASSIGN */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_PLUS_EQ */    {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_MINUS_EQ */   {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_DIV_EQ */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_MUL_EQ */     {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_EQ */         {nullptr,   ifn_binary,    PREC_EQ},
    /* TOKEN_NEQ */        {nullptr,   ifn_binary,    PREC_EQ},
    /* TOKEN_AND */        {nullptr,   ifn_binary,    PREC_AND},
    /* TOKEN_OR */         {nullptr,   ifn_binary,    PREC_OR},
    /* TOKEN_LS */         {nullptr,   ifn_binary,    PREC_CMP},
    /* TOKEN_LE */         {nullptr,   ifn_binary,    PREC_CMP},
    /* TOKEN_GR */         {nullptr,   ifn_binary,    PREC_CMP},
    /* TOKEN_GE */         {nullptr,   ifn_binary,    PREC_CMP},
    /* TOKEN_SEMICOLON */  {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_COMMA */      {nullptr,   nullptr,    PREC_NONE},
    /* TOKEN_DOT */        {nullptr,   ifn_member,    PREC_CALL},
    /* TOKEN_UNKNOWN */    {nullptr,   nullptr,    PREC_NONE},
    };
// clang-format on
// @formatter:on

    std::unique_ptr<Node> pfn_grouping() {
        if (!match(TOKEN_LPAREN))
            parser_throws(
                    error_msg("("));
        //either (), (<identifier>), (<identifier>, ...)
        if (cur.token == TOKEN_RPAREN) {
            get_tok();
            return std::make_unique<FunctionSignature>();
        }
        if (cur.token == TOKEN_IDENTIFIER) {
            const std::string first = cur.identifier;
            if (get_tok() == TOKEN_COMMA) {
                return get_tok(), parse_fn_params(first);
            }
            roll_back();
        }

        auto expr = parse_expression();
        if (cur.token != TOKEN_RPAREN)
            parser_throws(
                    error_msg(")"));
        get_tok();
        return expr;
    }

    std::unique_ptr<Node> ifn_binary(std::unique_ptr<Node> lhs) {
        int op = cur.token;
        get_tok();
        auto rhs = parse_precedence(rules[op].precedence +
                                    rules[op].left_assoc);//because we want all operators to be left-assoc(if right-assoc needed just do not add 1)
        if (rhs == nullptr)
            return nullptr;
        switch (op) {
            case TOKEN_ADD:
                return std::make_unique<AddExpr>(std::move(lhs), std::move(rhs));
            case TOKEN_SUB:
                return std::make_unique<SubExpr>(std::move(lhs), std::move(rhs));
            case TOKEN_MUL:
                return std::make_unique<MulExpr>(std::move(lhs), std::move(rhs));
            case TOKEN_DIV:
                return std::make_unique<DivExpr>(std::move(lhs), std::move(rhs));
            case TOKEN_EQ:
                return std::make_unique<BinaryExpr<BinaryOpType::EQ>>(std::move(lhs), std::move(rhs));
            case TOKEN_LE:
                return std::make_unique<BinaryExpr<BinaryOpType::LE>>(std::move(lhs), std::move(rhs));
            case TOKEN_LS:
                return std::make_unique<BinaryExpr<BinaryOpType::LS>>(std::move(lhs), std::move(rhs));
            case TOKEN_GR:
                return std::make_unique<BinaryExpr<BinaryOpType::GR>>(std::move(lhs), std::move(rhs));
            case TOKEN_GE:
                return std::make_unique<BinaryExpr<BinaryOpType::GE>>(std::move(lhs), std::move(rhs));
            case TOKEN_AND:
                return std::make_unique<BinaryExpr<BinaryOpType::AND>>(std::move(lhs), std::move(rhs));
            case TOKEN_OR:
                return std::make_unique<BinaryExpr<BinaryOpType::OR>>(std::move(lhs), std::move(rhs));
            case TOKEN_MOD:
                return std::make_unique<BinaryExpr<BinaryOpType::MOD>>(std::move(lhs), std::move(rhs));
            case TOKEN_NEQ:
                return std::make_unique<BinaryExpr<BinaryOpType::NEQ>>(std::move(lhs), std::move(rhs));
            default:
                parser_throws("Unknown binary operator | or internal error");
        }
        return nullptr;
    }

    unique_ptr<Node> parse_precedence(int prec) {
        if (rules[cur.token].prefix == nullptr)
            parser_throws(error_msg("identifier"));
        auto lhs = rules[cur.token].prefix();
        int op = cur.token;
        while (lhs && prec <= rules[op].precedence) {
            if (rules[op].infix == nullptr)
                parser_throws(error_msg("operator"));
            lhs = rules[op].infix(std::move(lhs));
            op = cur.token;
        }
        return lhs;
    }
}
namespace parser {

    unique_ptr<Node> parse_expression() {
        return parse_precedence(PREC_TEMP);
    }


    template<bool createNewScope = true, bool closeScope = true>
    void parse_block() {
        if constexpr (createNewScope)
            vars.new_scope();
        while (true) {
            switch (cur.token) {
                case TOKEN_RCURLY:
                    get_tok();
                    if constexpr (closeScope)
                        vars.close_scope();
                    return;
                case TOKEN_EOF:
                    parser_throws("expected } but found EOF");
                default:
                    parse_statement();
            }
        }
    }

    //cur_tok = '('
    void if_statement() {
        if (!match(TOKEN_LPAREN))
            parser_throws(error_msg("( after if keyword"));
        if (!epush(parse_expression())) return;
        if (!match(TOKEN_RPAREN))
            parser_throws(error_msg("closing ) in if statement"));
        const int cur_id1 = jmp_uid++;
        emitter->jmpf_label(vars.pop_var(), cur_id1);

        if (match(TOKEN_LCURLY)) {
            parse_block();
        } else parse_statement();


        //TODO: test if with else if
        if (is_panic()) return;
        if (match(TOKEN_ELSE)) {
            const int cur_id2 = jmp_uid++;
            emitter->jmp_label(cur_id2);
            emitter->label(cur_id1);
            if (match(TOKEN_IF)) {
//                throw std::runtime_error("not supported for now");
                if_statement();
                emitter->label(cur_id2);
            } else {
                if (match(TOKEN_LCURLY)) {
                    parse_block();
                } else parse_statement();
                emitter->label(cur_id2);
                //elsebody = match(TOKEN_LCURLY) ? parse_block() : parse_statement();
            }
        } else emitter->label(cur_id1);
    }

    std::unique_ptr<ast::Node> parse_expr_sc() {
        auto res = parse_expression();
        if (!match(TOKEN_SEMICOLON))
            parser_throws(error_msg("; after expression"));
        return res;
    }

    void parse_while() {
        if (!match(TOKEN_LPAREN))
            parser_throws(error_msg("( after while"));
        int start_id = jmp_uid++;
        int end_id = jmp_uid++;
        loops.emplace_back(start_id, end_id);
        vars.new_scope();
        emitter->label(start_id);
        if (!epush(parse_expression())) return;
        emitter->jmpf_label(vars.pop_var(), end_id);
        if (!match(TOKEN_RPAREN))
            parser_throws(error_msg(") in while statement"));
        if (match(TOKEN_LCURLY)) {
            parse_block();
        } else {
            parse_statement();
        }
        emitter->jmp_label(start_id);
        emitter->label(end_id);

        vars.close_scope();
        loops.pop_back();
    }


    //if is_assignment: pushes assingment;
    //           else: pushes res, pops back;
    void push_assign(std::unique_ptr<Node> val, int is_assignment) {
        if (!is_assignment) {
            epush(std::move(val));
            vars.pop_var();
            return;
        }
        auto curv = dynamic_cast<BinaryExpr<BinaryOpType::ASSIGN> *>(val.release());
        auto lhs = std::move(curv->l);
        auto rhs = std::move(curv->r);
        delete curv;
        if (!epush(std::move(rhs))) {
            return;
        }
        if (lhs->get_type() == ast::NodeType::Var) {
            const std::string &m_name = dynamic_cast<VarExpr *>(lhs.get())->name;
            const int res = vars.get_var(m_name);
            //if variable not found, then we create new at the same place and it has almost no scope
            if (res == -1) {
                if (is_assignment != 1) parser_throws("cannot find variable used in modifying assignment");
                vars.pop_var();
                vars.push_var(m_name);
                return;
            }
            if (is_assignment == 1) emitter->emit_move(res, vars.pop_var());
            else if (is_assignment == 2) emitter->emit_add(res, vars.pop_var(), res);
            else if (is_assignment == 3) emitter->emit_sub(res, res, vars.pop_var());
            else if (is_assignment == 4) emitter->emit_mul(res, res, vars.pop_var());
            else if (is_assignment == 5) emitter->emit_div(res, res, vars.pop_var());
        }
        if (lhs->get_type() == ast::NodeType::ArrayGet) {
            if (!parser::check_lvalue(lhs.get(), *emitter, vars)) parser_throws(error_msg("lvalue failed"));
            auto cur = dynamic_cast<ast::ArrayGet *>(lhs.get());
            parser::eval_expr(cur->name_expr.get(), *emitter, vars);
            parser::eval_expr(cur->index.get(), *emitter, vars);
            if (is_assignment != 1) {
                emitter->emit_arrayget(vars.last() + 1, vars.last() - 1, vars.last());
                switch (is_assignment) {
                    case 2:
                        emitter->emit_add(vars.last() - 2, vars.last() - 2, vars.last() + 1);
                        break;
                    case 3:
                        emitter->emit_sub(vars.last() - 2, vars.last() - 2, vars.last() + 1);
                        break;
                    case 4:
                        emitter->emit_mul(vars.last() - 2, vars.last() - 2, vars.last() + 1);
                        break;
                    case 5:
                        emitter->emit_div(vars.last() - 2, vars.last() - 2, vars.last() + 1);
                        break;
                    default:
                        break;
                }
            }
            emitter->emit_arrayset(vars.last() - 1, vars.last(), vars.last() - 2);
            vars.drop(3);
        }

    }

    //parses <expressiona>: ?(<lvalue> =) <expression> ;
    //true if result is a binary expression(assingment)
    std::unique_ptr<Node> parse_assignment(int &res) {
        res = 0;
        auto lhs = parse_expression();
        if (!lhs) { return lhs; }
        res = test(TOKEN_ASSIGN)
              + 2 * test(TOKEN_PLUS_EQ)
              + 3 * test(TOKEN_MINUS_EQ)
              + 4 * test(TOKEN_MUL_EQ)
              + 5 * test(TOKEN_DIV_EQ);
        if (res == 0)
            return lhs;
        get_tok();
        auto rhs = parse_expression();
        if (!rhs) return res = 0, nullptr;
        return std::make_unique<BinaryExpr<BinaryOpType::ASSIGN>>(std::move(lhs), std::move(rhs));
    }

    void parse_push_assign() {
        int res = 0;
        auto x = parse_assignment(res);
        push_assign(std::move(x), res);
    }

    void parse_for() {
        if (!match(TOKEN_LPAREN)) parser_throws(error_msg("( after for"));
        vars.new_scope();
        if (!match(TOKEN_SEMICOLON)) {
            parse_push_assign();
            if (!match(TOKEN_SEMICOLON))
                parser_throws(error_msg("; after assignment"));
        }

        int start_id = jmp_uid++;
        int end_id = jmp_uid++;
        loops.emplace_back(start_id, end_id);

        auto cond = parse_expr_sc();
        epush(cond.get());
        emitter->jmpf_label(vars.pop_var(), end_id);
        emitter->label(start_id);
        //HERE: should be parse_assignment
        int is_assignment = 0;
        auto incExpr = parse_assignment(is_assignment);
        if (!match(TOKEN_RPAREN)) parser_throws(error_msg(")"));
        if (match(TOKEN_LCURLY)) parse_block<false, false>();
        else parse_statement();
        push_assign(std::move(incExpr), is_assignment);
        epush(cond.get());
        emitter->jmpt_label(vars.pop_var(), start_id);
        emitter->label(end_id);

        vars.close_scope();
        loops.pop_back();
    }

    void parse_continue() {
        if (!match(TOKEN_SEMICOLON)) parser_throws(";");
        if (loops.empty())
            parser_throws(parser_throws("continue without loop; continue statements are allowed only inside loops"));
        emitter->jmp_label(loops.back().label_start);
    }


    void parse_break() {
        if (!match(TOKEN_SEMICOLON)) parser_throws(";");
        if (loops.empty())
            parser_throws(parser_throws("break without loop; break statements are allowed only inside loops"));
        emitter->jmp_label(loops.back().label_end);
    }


    //make sure that cur.token != TOKEN_EOF && cur.token != TOKEN_RCURLY on call
    void parse_statement() {
        if (match(TOKEN_IF)) if_statement();
        else if (match(TOKEN_FOR)) parse_for();
        else if (match(TOKEN_WHILE)) parse_while();
        else if (match(TOKEN_RETURN)) parse_return();
        else if (match(TOKEN_BREAK)) parse_continue();
        else if (match(TOKEN_CONTINUE)) parse_continue();
        else {
            //TODO: fails here
            parse_push_assign();
            if (!match(TOKEN_SEMICOLON))
                parser_throws(error_msg("; after assignment"));
        }
    }

    ast::Program parse_program(interpreter::VMData &vm) {
        Program res;
        jmp_uid = 0;
        while (cur.token != TOKEN_EOF) {
            if (match(TOKEN_FN)) {
                parse_function();
            } else
                parser_throws(error_msg("expected function declaration"));
        }
        emitter->initVM(vm);
        res.instructions = {};
        delete emitter;
        return res;
    }

    //if anonymous: cur = first argument
    //otherwise: cur = function name
    void parse_function() {
        if (!match(TOKEN_IDENTIFIER))
            parser_throws(error_msg("function name"));
        std::string name = std::move(prv.identifier);
        if (!match(TOKEN_LPAREN))
            parser_throws(error_msg("("));
        auto header = parse_fn_params();
        if (header == nullptr) { return; }
        if (!match(TOKEN_LCURLY))
            parser_throws(error_msg("{ in function body"));
        int fid = emitter->begin_func(header->params.size(), name);
        if (!vars.add_func(name, fid)) {
            parser_throws(error_msg("function '" + name + "' already exsists"));
        }
        vars.new_scope();
        for (auto &cur: header->params) {
            auto it = vars.get_var(cur);
            if (it != -1)
                parser_throws(std::format("illegal argument names, argument {} already exsists", cur));
            vars.push_var(cur);
        }
        parse_block<false>();
        //TODO: tail call
        //add return in case control flow leaks
        emitter->emit_retnil();
        emitter->end_func();
    }


    void init_parser(std::istream &in, interpreter::BytecodeEmitter *emit) {
        init_lexer(in);
        emitter = emit;
        vars = VarManager();
        cote_stdlib::initStdlib(interpreter::vm_instance(), vars);
        init_exceptions();
    }

    bool epush(ast::Node *expr) {
        if (expr == nullptr) return false;
        if (!parser::eval_expr(expr, *emitter, vars))
            throw std::runtime_error("error parsing expression");
        return true;
    }

    void parse_return() {
        epush(parse_expr_sc());
        //TODO: tail call
        emitter->emit_return(vars.pop_var());
    }

    void parse_block_() {
        parse_block();
    }

}
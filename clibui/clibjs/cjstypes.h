﻿//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBJS_CJSTYPES_H
#define CLIBJS_CJSTYPES_H

#include <string>

#define JS_NAN std::numeric_limits<double>::quiet_NaN()

namespace clib {
    namespace types {

        using decimal = double;

        enum disp_t {
            D_PS,
            D_HTOP,
            D_HANDLE,
            D_WINDOW,
            D_MEM,
            D_STAT,
        };

        enum js_lexer_t {
            NONE,
            NUMBER,
            ID,
            REGEX,
            STRING,
            SPACE,
            NEWLINE,
            COMMENT,
            END,
            RULE_START,
            RULE_NO_LINE,
            RULE_LINE,
            RULE_RBRACE,
            RULE_EOF,
            RULE_END,
            KEYWORD_START,
            K_NEW,
            K_VAR,
            K_LET,
            K_CONST,
            K_FUNCTION,
            K_IF,
            K_ELSE,
            K_FOR,
            K_WHILE,
            K_IN,
            K_DO,
            K_BREAK,
            K_CONTINUE,
            K_RETURN,
            K_SWITCH,
            K_DEFAULT,
            K_CASE,
            K_NULL,
            K_UNDEFINED,
            K_TRUE,
            K_FALSE,
            K_INSTANCEOF,
            K_TYPEOF,
            K_VOID,
            K_DELETE,
            K_CLASS,
            K_THIS,
            K_SUPER,
            K_WITH,
            K_TRY,
            K_THROW,
            K_CATCH,
            K_FINALLY,
            K_DEBUGGER,
            KEYWORD_END,
            OPERATOR_START,
            T_ADD,
            T_SUB,
            T_MUL,
            T_DIV,
            T_MOD,
            T_POWER,
            T_INC,
            T_DEC,
            T_ASSIGN,
            T_ASSIGN_ADD,
            T_ASSIGN_SUB,
            T_ASSIGN_MUL,
            T_ASSIGN_DIV,
            T_ASSIGN_MOD,
            T_ASSIGN_LSHIFT,
            T_ASSIGN_RSHIFT,
            T_ASSIGN_URSHIFT,
            T_ASSIGN_AND,
            T_ASSIGN_OR,
            T_ASSIGN_XOR,
            T_ASSIGN_POWER,
            T_LESS,
            T_LESS_EQUAL,
            T_GREATER,
            T_GREATER_EQUAL,
            T_EQUAL,
            T_FEQUAL,
            T_NOT_EQUAL,
            T_FNOT_EQUAL,
            T_LOG_NOT,
            T_LOG_AND,
            T_LOG_OR,
            T_BIT_NOT,
            T_BIT_AND,
            T_BIT_OR,
            T_BIT_XOR,
            T_DOT,
            T_COMMA,
            T_SEMI,
            T_COLON,
            T_QUERY,
            T_LSHIFT,
            T_RSHIFT,
            T_URSHIFT,
            T_LPARAN,
            T_RPARAN,
            T_LSQUARE,
            T_RSQUARE,
            T_LBRACE,
            T_RBRACE,
            T_COALESCE,
            T_SHARP,
            T_ELLIPSIS,
            T_ARROW,
            OPERATOR_END,
            LEXER_END,
        };

        enum js_ast_t {
            a_root,
            a_collection,
            a_keyword,
            a_operator,
            a_literal,
            a_string,
            a_regex,
            a_number,
            a_rule,
            AST_END,
        };

        enum js_ast_to_t {
            to_parent,
            to_prev,
            to_next,
            to_child,
        };

        enum js_ast_attr_t {
            a_none,
            a_exp = 1,
            a_reverse = 2,
        };

        enum js_coll_t {
            c_program,
            c_sourceElement,
            c_statement,
            c_block,
            c_statementList,
            c_variableStatement,
            c_variableDeclarationList,
            c_variableDeclaration,
            c_emptyStatement,
            c_expressionStatement,
            c_ifStatement,
            c_iterationStatement,
            c_doStatement,
            c_whileStatement,
            c_forStatement,
            c_forInStatement,
            c_continueStatement,
            c_breakStatement,
            c_returnStatement,
            c_withStatement,
            c_switchStatement,
            c_functionStatement,
            c_caseBlock,
            c_caseClauses,
            c_caseClause,
            c_defaultClause,
            c_labelledStatement,
            c_throwStatement,
            c_tryStatement,
            c_catchProduction,
            c_finallyProduction,
            c_debuggerStatement,
            c_functionDeclaration,
            c_classDeclaration,
            c_classTail,
            c_classElement,
            c_classElements,
            c_methodDefinition,
            c_formalParameterList,
            c_formalParameterArg,
            c_lastFormalParameterArg,
            c_functionBody,
            c_sourceElements,
            c_arrayLiteral,
            c_elementList,
            c_arrayElement,
            c_commaList,
            c_objectLiteral,
            c_propertyAssignment,
            c_propertyAssignments,
            c_propertyName,
            c_arguments,
            c_argument,
            c_expressionSequence,
            c_singleExpression,
            c_assignable,
            c_anonymousFunction,
            c_arrowFunctionParameters,
            c_arrowFunctionBody,
            c_literal,
            c_numericLiteral,
            c_identifierName,
            c_reservedWord,
            c_keyword,
            c_eos,
            c_propertyExpressionAssignment,
            c_computedPropertyExpressionAssignment,
            c_propertyShorthand,
            c_functionDecl,
            c_anonymousFunctionDecl,
            c_arrowFunction,
            c_functionExpression,
            c_classExpression,
            c_memberIndexExpression,
            c_memberDotExpression,
            c_argumentsExpression,
            c_newExpression,
            c_newExpressionArgument,
            c_primaryExpression,
            c_prefixExpression,
            c_prefixExpressionList,
            c_postIncrementExpression,
            c_postDecreaseExpression,
            c_postfixExpression,
            c_deleteExpression,
            c_voidExpression,
            c_typeofExpression,
            c_preIncrementExpression,
            c_preDecreaseExpression,
            c_unaryPlusExpression,
            c_unaryMinusExpression,
            c_bitNotExpression,
            c_notExpression,
            c_powerExpression,
            c_multiplicativeExpression,
            c_additiveExpression,
            c_coalesceExpression,
            c_bitShiftExpression,
            c_relationalExpression,
            c_instanceofExpression,
            c_inExpression,
            c_equalityExpression,
            c_bitAndExpression,
            c_bitXOrExpression,
            c_bitOrExpression,
            c_logicalAndExpression,
            c_logicalOrExpression,
            c_ternaryExpression,
            c_assignmentExpression,
            c_assignmentOperatorExpression,
            c_thisExpression,
            c_identifierExpression,
            c_superExpression,
            c_literalExpression,
            c_arrayLiteralExpression,
            c_objectLiteralExpression,
            c_parenthesizedExpression,
            COLL_END,
        };

        enum js_unit_t {
            u_none,
            u_token,
            u_token_ref,
            u_rule,
            u_rule_ref,
            u_sequence,
            u_branch,
            u_optional,
        };

        const char *js_lexer_string(js_lexer_t t);
        const char *js_ast_string(js_lexer_t t);
        const char *js_coll_string(js_coll_t t);

        // Refer: https://github.com/python/cpython/blob/master/Include/opcode.h
        enum js_ins_t {
            LOAD_EMPTY,
            LOAD_NULL,
            LOAD_UNDEFINED,
            LOAD_TRUE,
            LOAD_FALSE,
            LOAD_ZERO,
            LOAD_THIS,
            POP_TOP,
            DUP_TOP,
            NOP,
            INSTANCE_OF,
            OBJECT_IN,
            UNARY_POSITIVE,
            UNARY_NEGATIVE,
            UNARY_NOT,
            UNARY_INVERT,
            UNARY_NEW,
            UNARY_DELETE,
            UNARY_TYPEOF,
            BINARY_MATRIX_MULTIPLY,
            INPLACE_MATRIX_MULTIPLY,
            BINARY_POWER,
            BINARY_MULTIPLY,
            BINARY_MODULO,
            BINARY_ADD,
            BINARY_SUBTRACT,
            BINARY_SUBSCR,
            BINARY_FLOOR_DIVIDE,
            BINARY_TRUE_DIVIDE,
            BINARY_INC,
            BINARY_DEC,
            STORE_SUBSCR,
            BINARY_LSHIFT,
            BINARY_RSHIFT,
            BINARY_URSHIFT,
            BINARY_AND,
            BINARY_XOR,
            BINARY_OR,
            GET_ITER,
            RETURN_VALUE,
            STORE_NAME,
            DELETE_NAME,
            UNPACK_SEQUENCE,
            FOR_ITER,
            UNPACK_EX,
            STORE_ATTR,
            STORE_GLOBAL,
            LOAD_CONST,
            LOAD_NAME,
            BUILD_LIST,
            BUILD_MAP,
            LOAD_ATTR,
            COMPARE_LESS,
            COMPARE_LESS_EQUAL,
            COMPARE_EQUAL,
            COMPARE_NOT_EQUAL,
            COMPARE_GREATER,
            COMPARE_GREATER_EQUAL,
            COMPARE_FEQUAL,
            COMPARE_FNOT_EQUAL,
            JUMP_FORWARD,
            JUMP_IF_FALSE_OR_POP,
            JUMP_IF_TRUE_OR_POP,
            JUMP_ABSOLUTE,
            POP_JUMP_IF_FALSE,
            POP_JUMP_IF_TRUE,
            LOAD_GLOBAL,
            SETUP_FINALLY,
            POP_FINALLY,
            THROW,
            EXIT_FINALLY,
            LOAD_FAST,
            STORE_FAST,
            CALL_FUNCTION,
            MAKE_FUNCTION,
            LOAD_CLOSURE,
            LOAD_DEREF,
            STORE_DEREF,
            REST_ARGUMENT,
            CALL_FUNCTION_EX,
            LOAD_METHOD,
            CALL_METHOD,
            INS_END,
        };
        const char *js_ins_string(js_ins_t t);

        enum js_runtime_t {
            r_number,
            r_string,
            r_boolean,
            r_regex,
            r_object,
            r_function,
            r_null,
            r_undefined,
            r__end,
        };
        const char *js_runtime_string(js_runtime_t t);
    }

    class cjs_exception : public std::exception {
    public:
        explicit cjs_exception(const std::string &msg) noexcept;

        cjs_exception(const cjs_exception &e) = default;
        cjs_exception &operator=(const cjs_exception &e) = default;

        std::string message() const;

    private:
        std::string msg;
    };
}

namespace std
{
    std::vector<std::string> split(const std::string& s, char delim);
}

#endif //CLIBJS_CJSTYPES_H

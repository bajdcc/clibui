﻿//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include <array>
#include "cjstypes.h"

namespace clib {
    namespace types {

        const char *js_lexer_string(js_lexer_t t) {
            static std::array<const char *, LEXER_END> p = {
                    "NONE",
                    "NUMBER",
                    "ID",
                    "REGEX",
                    "STRING",
                    "SPACE",
                    "NEWLINE",
                    "COMMENT",
                    "END",
                    "RULE_START",
                    "RULE_NO_LINE",
                    "RULE_LINE",
                    "RULE_RBRACE",
                    "RULE_EOF",
                    "RULE_END",
                    "KEYWORD_START",
                    "K_NEW",
                    "K_VAR",
                    "K_LET",
                    "K_CONST",
                    "K_FUNCTION",
                    "K_IF",
                    "K_ELSE",
                    "K_FOR",
                    "K_WHILE",
                    "K_IN",
                    "K_DO",
                    "K_BREAK",
                    "K_CONTINUE",
                    "K_RETURN",
                    "K_SWITCH",
                    "K_DEFAULT",
                    "K_CASE",
                    "K_NULL",
                    "K_UNDEFINED",
                    "K_TRUE",
                    "K_FALSE",
                    "K_INSTANCEOF",
                    "K_TYPEOF",
                    "K_VOID",
                    "K_DELETE",
                    "K_CLASS",
                    "K_THIS",
                    "K_SUPER",
                    "K_WITH",
                    "K_TRY",
                    "K_THROW",
                    "K_CATCH",
                    "K_FINALLY",
                    "K_DEBUGGER",
                    "KEYWORD_END",
                    "OPERATOR_START",
                    "T_ADD",
                    "T_SUB",
                    "T_MUL",
                    "T_DIV",
                    "T_MOD",
                    "T_POWER",
                    "T_INC",
                    "T_DEC",
                    "T_ASSIGN",
                    "T_ASSIGN_ADD",
                    "T_ASSIGN_SUB",
                    "T_ASSIGN_MUL",
                    "T_ASSIGN_DIV",
                    "T_ASSIGN_MOD",
                    "T_ASSIGN_LSHIFT",
                    "T_ASSIGN_RSHIFT",
                    "T_ASSIGN_URSHIFT",
                    "T_ASSIGN_AND",
                    "T_ASSIGN_OR",
                    "T_ASSIGN_XOR",
                    "T_ASSIGN_POWER",
                    "T_LESS",
                    "T_LESS_EQUAL",
                    "T_GREATER",
                    "T_GREATER_EQUAL",
                    "T_EQUAL",
                    "T_FEQUAL",
                    "T_NOT_EQUAL",
                    "T_FNOT_EQUAL",
                    "T_LOG_NOT",
                    "T_LOG_AND",
                    "T_LOG_OR",
                    "T_BIT_NOT",
                    "T_BIT_AND",
                    "T_BIT_OR",
                    "T_BIT_XOR",
                    "T_DOT",
                    "T_COMMA",
                    "T_SEMI",
                    "T_COLON",
                    "T_QUERY",
                    "T_LSHIFT",
                    "T_RSHIFT",
                    "T_URSHIFT",
                    "T_LPARAN",
                    "T_RPARAN",
                    "T_LSQUARE",
                    "T_RSQUARE",
                    "T_LBRACE",
                    "T_RBRACE",
                    "T_COALESCE",
                    "T_SHARP",
                    "T_ELLIPSIS",
                    "T_ARROW",
                    "OPERATOR_END",
            };
            return p.at(t);
        }

        const char *js_coll_string(js_coll_t t) {
            static std::array<const char *, COLL_END> p = {
                    "Program",
                    "SourceElement",
                    "Statement",
                    "Block",
                    "StatementList",
                    "VariableStatement",
                    "VariableDeclarationList",
                    "VariableDeclaration",
                    "EmptyStatement",
                    "ExpressionStatement",
                    "IfStatement",
                    "IterationStatement",
                    "DoStatement",
                    "WhileStatement",
                    "ForStatement",
                    "ForInStatement",
                    "ContinueStatement",
                    "BreakStatement",
                    "ReturnStatement",
                    "WithStatement",
                    "SwitchStatement",
                    "FunctionStatement",
                    "CaseBlock",
                    "CaseClauses",
                    "CaseClause",
                    "DefaultClause",
                    "LabelledStatement",
                    "ThrowStatement",
                    "TryStatement",
                    "CatchProduction",
                    "FinallyProduction",
                    "DebuggerStatement",
                    "FunctionDeclaration",
                    "ClassDeclaration",
                    "ClassTail",
                    "ClassElement",
                    "ClassElements",
                    "MethodDefinition",
                    "FormalParameterList",
                    "FormalParameterArg",
                    "LastFormalParameterArg",
                    "FunctionBody",
                    "SourceElements",
                    "ArrayLiteral",
                    "ElementList",
                    "ArrayElement",
                    "CommaList",
                    "ObjectLiteral",
                    "PropertyAssignment",
                    "PropertyAssignments",
                    "PropertyName",
                    "Arguments",
                    "Argument",
                    "ExpressionSequence",
                    "SingleExpression",
                    "Assignable",
                    "AnonymousFunction",
                    "ArrowFunctionParameters",
                    "ArrowFunctionBody",
                    "Literal",
                    "NumericLiteral",
                    "IdentifierName",
                    "ReservedWord",
                    "Keyword",
                    "Eos",
                    "PropertyExpressionAssignment",
                    "ComputedPropertyExpressionAssignment",
                    "PropertyShorthand",
                    "FunctionDecl",
                    "AnonymousFunctionDecl",
                    "ArrowFunction",
                    "FunctionExpression",
                    "ClassExpression",
                    "MemberIndexExpression",
                    "MemberDotExpression",
                    "ArgumentsExpression",
                    "NewExpression",
                    "NewExpressionArgument",
                    "PrimaryExpression",
                    "PrefixExpression",
                    "PrefixExpressionList",
                    "PostIncrementExpression",
                    "PostDecreaseExpression",
                    "PostfixExpression",
                    "DeleteExpression",
                    "VoidExpression",
                    "TypeofExpression",
                    "PreIncrementExpression",
                    "PreDecreaseExpression",
                    "UnaryPlusExpression",
                    "UnaryMinusExpression",
                    "BitNotExpression",
                    "NotExpression",
                    "PowerExpression",
                    "MultiplicativeExpression",
                    "AdditiveExpression",
                    "CoalesceExpression",
                    "BitShiftExpression",
                    "RelationalExpression",
                    "InstanceofExpression",
                    "InExpression",
                    "EqualityExpression",
                    "BitAndExpression",
                    "BitXOrExpression",
                    "BitOrExpression",
                    "LogicalAndExpression",
                    "LogicalOrExpression",
                    "TernaryExpression",
                    "AssignmentExpression",
                    "AssignmentOperatorExpression",
                    "ThisExpression",
                    "IdentifierExpression",
                    "SuperExpression",
                    "LiteralExpression",
                    "ArrayLiteralExpression",
                    "ObjectLiteralExpression",
                    "ParenthesizedExpression",
            };
            return p.at(t);
        }

        const char *js_ast_string(js_ast_t t) {
            static std::array<const char *, AST_END> p = {
                    "root",
                    "collection",
                    "keyword",
                    "operator",
                    "literal",
                    "string",
                    "regex",
                    "number",
                    "rule",
            };
            return p.at(t);
        }

        const char *js_ins_string(js_ins_t t) {
            static std::array<const char *, INS_END> p = {
                    "LOAD_EMPTY",
                    "LOAD_NULL",
                    "LOAD_UNDEFINED",
                    "LOAD_TRUE",
                    "LOAD_FALSE",
                    "LOAD_ZERO",
                    "LOAD_THIS",
                    "POP_TOP",
                    "DUP_TOP",
                    "NOP",
                    "INSTANCE_OF",
                    "OBJECT_IN",
                    "UNARY_POSITIVE",
                    "UNARY_NEGATIVE",
                    "UNARY_NOT",
                    "UNARY_INVERT",
                    "UNARY_NEW",
                    "UNARY_DELETE",
                    "UNARY_TYPEOF",
                    "BINARY_MATRIX_MULTIPLY",
                    "INPLACE_MATRIX_MULTIPLY",
                    "BINARY_POWER",
                    "BINARY_MULTIPLY",
                    "BINARY_MODULO",
                    "BINARY_ADD",
                    "BINARY_SUBTRACT",
                    "BINARY_SUBSCR",
                    "BINARY_FLOOR_DIVIDE",
                    "BINARY_TRUE_DIVIDE",
                    "BINARY_INC",
                    "BINARY_DEC",
                    "STORE_SUBSCR",
                    "BINARY_LSHIFT",
                    "BINARY_RSHIFT",
                    "BINARY_URSHIFT",
                    "BINARY_AND",
                    "BINARY_XOR",
                    "BINARY_OR",
                    "GET_ITER",
                    "RETURN_VALUE",
                    "STORE_NAME",
                    "DELETE_NAME",
                    "UNPACK_SEQUENCE",
                    "FOR_ITER",
                    "UNPACK_EX",
                    "STORE_ATTR",
                    "STORE_GLOBAL",
                    "LOAD_CONST",
                    "LOAD_NAME",
                    "BUILD_LIST",
                    "BUILD_MAP",
                    "LOAD_ATTR",
                    "COMPARE_LESS",
                    "COMPARE_LESS_EQUAL",
                    "COMPARE_EQUAL",
                    "COMPARE_NOT_EQUAL",
                    "COMPARE_GREATER",
                    "COMPARE_GREATER_EQUAL",
                    "COMPARE_FEQUAL",
                    "COMPARE_FNOT_EQUAL",
                    "JUMP_FORWARD",
                    "JUMP_IF_FALSE_OR_POP",
                    "JUMP_IF_TRUE_OR_POP",
                    "JUMP_ABSOLUTE",
                    "POP_JUMP_IF_FALSE",
                    "POP_JUMP_IF_TRUE",
                    "LOAD_GLOBAL",
                    "SETUP_FINALLY",
                    "POP_FINALLY",
                    "THROW",
                    "EXIT_FINALLY",
                    "LOAD_FAST",
                    "STORE_FAST",
                    "CALL_FUNCTION",
                    "MAKE_FUNCTION",
                    "LOAD_CLOSURE",
                    "LOAD_DEREF",
                    "STORE_DEREF",
                    "REST_ARGUMENT",
                    "CALL_FUNCTION_EX",
                    "LOAD_METHOD",
                    "CALL_METHOD",
            };
            return p.at(t);
        }

        const char *js_runtime_string(js_runtime_t t) {
            static std::array<const char *, r__end> p = {
                    "number",
                    "string",
                    "boolean",
                    "regex",
                    "object",
                    "function",
                    "null",
                    "undefined",
            };
            return p.at(t);
        }
    }

    cjs_exception::cjs_exception(const std::string &msg) noexcept : msg(msg) {}

    std::string cjs_exception::message() const {
        return msg;
    }
}

template<typename Out>
void split(const std::string& s, char delim, Out result)
{
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        *(result++) = item;
    }
}

std::vector<std::string> std::split(const std::string& s, char delim)
{
    std::vector<std::string> elems;
    ::split(s, delim, std::back_inserter(elems));
    return elems;
}
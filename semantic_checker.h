#ifndef SEMANTIC_CHECKER_H
#define SEMANTIC_CHECKER_H

#include "parser.h"
#include "symbol_table.h"

#include <string>

class SemanticChecker {
    public:
        // writes to stderr all the errors that it finds and returns whether
        // the program is semantically correct
        static bool check(Program * program, SymbolTable * symbol_table);

    private: // variables
        Program * m_program;
        SymbolTable * m_symbol_table;
        // current class id in parsing
        std::string m_class_id;
        // current function_id in parsing
        std::string m_function_id;
        // true if we have not encountered any errors
        bool m_success;

        // true if we have any recursive data structures
        bool m_recursive_error;

    private: // methods
        SemanticChecker(Program * program, SymbolTable * symbol_table);
        bool internal_check();

        void check_statement_list(StatementList * _statement_list);
        void check_statement(Statement * statement);
        void check_variable_declaration_list(VariableDeclarationList * variable_list);
        void check_variable_declaration(VariableDeclaration * variable);

        // functions to get the types of structures
        // null means semantic error occured
        TypeDenoter * check_expression(Expression * expression);
        TypeDenoter * check_additive_expression(AdditiveExpression * additive_expression);
        TypeDenoter * check_multiplicitive_expression(MultiplicativeExpression * multiplicative_expression);
        TypeDenoter * check_negatable_expression(NegatableExpression * negatable_expression);
        TypeDenoter * check_primary_expression(PrimaryExpression * primary_expression);
        TypeDenoter * check_variable_access(VariableAccess * variable_access, bool allow_function_return_value = false);
        TypeDenoter * check_method_designator(MethodDesignator * method_designator);
        TypeDenoter * check_object_instantiation(ObjectInstantiation * object_instantiation);
        TypeDenoter * check_indexed_variable(IndexedVariable * indexed_variable);
        TypeDenoter * check_attribute_designator(AttributeDesignator * attribute_designator);

        Identifier * find_identifier(VariableAccess * variable_access);

        // get the result type of adding/multiplying/etc types
        TypeDenoter * combined_type(TypeDenoter * left_type, TypeDenoter * right_type);
        // determine if types are exactly equal
        bool types_equal(TypeDenoter * type1, TypeDenoter * type2);
        // returns true if you can assign the right_type to the left_type
        bool assignment_valid(TypeDenoter * left_type, TypeDenoter * right_type);
        // programmer-friendly display of a type
        std::string type_to_string(TypeDenoter * type);
        // returns whether the expression is a constant integer and its value if it is
        LiteralInteger * constant_integer(Expression * expression);
        bool is_ancestor(TypeDenoter * child, TypeDenoter * ancestor);
        Identifier * variable_access_identifier(VariableAccess * variable_access);
        FunctionDeclaration * class_method(std::string class_name, FunctionDesignator * function_designator);
        TypeDenoter * class_variable_type(std::string class_name, Identifier * variable);
        void check_type(TypeDenoter * type);
        bool structurally_equivalent(TypeDenoter * left_type, TypeDenoter * right_type);
        bool class_contains_class(TypeDenoter * owner, TypeDenoter * owned);
};

#endif

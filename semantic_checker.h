#ifndef SEMANTIC_CHECKER_H
#define SEMANTIC_CHECKER_H

#include "parser.h"
#include "symbol_table.h"

#include <string>

class SemanticChecker {
    public:
        SemanticChecker(Program * program, SymbolTable * symbol_table);

        // writes to stderr all the errors that it finds and returns whether
        // the program is semantically correct
        bool check();

    private: // variables
        Program * m_program;
        SymbolTable * m_symbol_table;
        // current class id in parsing
        std::string m_class_id;
        // current function_id in parsing
        std::string m_function_id;
        // true if we have not encountered any errors
        bool m_success;
    private: // methods
        void check_statement_list(StatementList * _statement_list);
        void check_statement(Statement * statement);

        // functions to get the types of structures
        // null means semantic error occured
        TypeDenoter * check_expression(Expression * expression);
        TypeDenoter * check_additive_expression(AdditiveExpression * additive_expression);
        TypeDenoter * check_multiplicitive_expression(MultiplicativeExpression * multiplicative_expression);
        TypeDenoter * check_negatable_expression(NegatableExpression * negatable_expression);
        TypeDenoter * check_primary_expression(PrimaryExpression * primary_expression);
        TypeDenoter * check_variable_access(VariableAccess * variable_access);
        TypeDenoter * check_function_designator(FunctionDesignator * function_designator);
        TypeDenoter * check_method_designator(MethodDesignator * method_designator);
        TypeDenoter * check_object_instantiation(ObjectInstantiation * object_instantiation);
        TypeDenoter * check_indexed_variable(IndexedVariable * indexed_variable);
        TypeDenoter * check_attribute_designator(AttributeDesignator * attribute_designator);

        // get the result type of adding/multiplying/etc types
        TypeDenoter * combined_type(TypeDenoter * left_type, TypeDenoter * right_type);
};

#endif

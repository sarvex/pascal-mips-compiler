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
    private: // methods
        bool check_program();
        bool check_statement_list(StatementList * _statement_list);
        bool check_statement(Statement * statement);
        bool check_indexed_variable(IndexedVariable * indexed_variable);

        // functions to get the types of structures
        // null means semantic error occured
        TypeDenoter * expression_type(Expression * expression);
        TypeDenoter * additive_expression_type(AdditiveExpression * additive_expression);
        TypeDenoter * multiplicitive_expression_type(MultiplicativeExpression * multiplicative_expression);
        TypeDenoter * negatable_expression_type(NegatableExpression * negatable_expression);
        TypeDenoter * primary_expression_type(PrimaryExpression * primary_expression);
        TypeDenoter * variable_access_type(VariableAccess * variable_access);
        TypeDenoter * function_designator_type(FunctionDesignator * function_designator);
        TypeDenoter * method_designator_type(MethodDesignator * method_designator);
        TypeDenoter * object_instantiation_type(ObjectInstantiation * object_instantiation);
        TypeDenoter * indexed_variable_type(IndexedVariable * indexed_variable);
        TypeDenoter * attribute_designator_type(AttributeDesignator * attribute_designator);

        // get the result type of adding/multiplying/etc types
        TypeDenoter * combined_type(TypeDenoter * left_type, TypeDenoter * right_type);
};

#endif

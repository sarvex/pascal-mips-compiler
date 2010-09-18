#include "semantic_checker.h"

#include <map>
#include <iostream>
#include <cassert>

SemanticChecker::SemanticChecker(Program * program, SymbolTable * symbol_table) :
    m_program(program),
    m_symbol_table(symbol_table),
    m_success(true) {}

bool SemanticChecker::check()
{
    for (ClassList * class_list = m_program->class_list; class_list != NULL; class_list = class_list->next) {
        ClassDeclaration * class_declaration = class_list->item;
        m_class_id = class_declaration->identifier->text;
        
        // make sure array indicies are integers
        for (FunctionDeclarationList * function_list = class_declaration->class_block->function_list; function_list != NULL; function_list = function_list->next) {
            FunctionDeclaration * function_declaration = function_list->item;
            m_function_id = function_declaration->identifier->text;

            StatementList * statement_list = function_declaration->block->statement_list;
            check_statement_list(statement_list);
        }
    }

    return m_success;
}

void SemanticChecker::check_statement_list(StatementList * _statement_list)
{
    for (StatementList * statement_list = _statement_list; statement_list != NULL; statement_list = statement_list->next)
        check_statement(statement_list->item);
}

void SemanticChecker::check_statement(Statement * statement)
{
    switch(statement->type) {
        case Statement::ASSIGNMENT:
            check_variable_access(statement->assignment->variable);
        case Statement::IF:
            check_expression(statement->if_statement->expression);
            check_statement(statement->if_statement->then_statement);
            if (statement->if_statement->else_statement == NULL)
                check_statement(statement->if_statement->else_statement);
        case Statement::PRINT:
            check_expression(statement->print_statement->expression);
        case Statement::WHILE:
            check_expression(statement->while_statement->expression);
            check_statement(statement->while_statement->statement);
        case Statement::COMPOUND:
            check_statement_list(statement->compound_statement);
        default:
            assert(false);
    }
}

TypeDenoter * SemanticChecker::check_expression(Expression * expression)
{
    if (expression->right == NULL) {
        // it's just the type of the first additive expression
        return check_additive_expression(expression->left);
    } else {
        // we're looking at a compare operator, so it always returns a boolean
        return new TypeDenoter(TypeDenoter::BOOLEAN);
    }
}

// when we do a multiplicitive or additive operation, what is the return type?
TypeDenoter * SemanticChecker::combined_type(TypeDenoter * left_type, TypeDenoter * right_type)
{
    // valid addition types:
    // char +       char =      char
    // integer +    integer =   integer
    // integer +    char =      integer
    // real +       integer =   real
    // real +       real =      real
    // real +       char =      real
    if (left_type->type == TypeDenoter::CHAR && right_type->type == TypeDenoter::CHAR) {
        return new TypeDenoter(TypeDenoter::CHAR);
    } else if (left_type->type == TypeDenoter::INTEGER && right_type->type == TypeDenoter::INTEGER) {
        return new TypeDenoter(TypeDenoter::INTEGER);
    } else if (left_type->type == TypeDenoter::REAL && right_type->type == TypeDenoter::REAL) {
        return new TypeDenoter(TypeDenoter::REAL);
    } else if ((left_type->type == TypeDenoter::INTEGER && right_type->type == TypeDenoter::CHAR) ||
        (left_type->type == TypeDenoter::CHAR && right_type->type == TypeDenoter::INTEGER))
    {
        return new TypeDenoter(TypeDenoter::INTEGER);
    } else if ((left_type->type == TypeDenoter::REAL && right_type->type == TypeDenoter::INTEGER) ||
        (left_type->type == TypeDenoter::INTEGER && right_type->type == TypeDenoter::REAL))
    {
        return new TypeDenoter(TypeDenoter::REAL);
    } else if ((left_type->type == TypeDenoter::REAL && right_type->type == TypeDenoter::CHAR) ||
        (left_type->type == TypeDenoter::CHAR && right_type->type == TypeDenoter::REAL))
    {
        return new TypeDenoter(TypeDenoter::REAL);
    } else {
        // anything else is invalid
        std::cerr << "ERROR:" << 100 << ":Cannot combine these types" << std::endl;
        return NULL;
    }
}

TypeDenoter * SemanticChecker::check_additive_expression(AdditiveExpression * additive_expression)
{
    TypeDenoter * right_type = check_multiplicitive_expression(additive_expression->right);
    if (additive_expression->left == NULL) {
        // it's just the type of the right
        return right_type;
    } else {
        TypeDenoter * left_type = check_additive_expression(additive_expression->left);
        if (left_type == NULL || right_type == NULL) {
            // semantic error occurred down the stack
            return NULL;
        }
        return combined_type(left_type, right_type);
    }
}

TypeDenoter * SemanticChecker::check_multiplicitive_expression(MultiplicativeExpression * multiplicative_expression) {
    TypeDenoter * right_type = check_negatable_expression(multiplicative_expression->right);
    if (multiplicative_expression->left == NULL) {
        // it's just the type of the right
        return right_type;
    } else {
        TypeDenoter * left_type = check_multiplicitive_expression(multiplicative_expression->left);
        if (left_type == NULL || right_type == NULL) {
            // semantic error occurred down the stack
            return NULL;
        }
        return combined_type(left_type, right_type);
    }
}

TypeDenoter * SemanticChecker::check_negatable_expression(NegatableExpression * negatable_expression) {
    if (negatable_expression->type == NegatableExpression::SIGN) {
        return check_negatable_expression(negatable_expression->next);
    } else if (negatable_expression->type == NegatableExpression::PRIMARY) {
        return check_primary_expression(negatable_expression->primary_expression);
    } else {
        assert(false);
        return NULL;
    }
}

TypeDenoter * SemanticChecker::check_primary_expression(PrimaryExpression * primary_expression) {
    switch (primary_expression->type) {
        case PrimaryExpression::VARIABLE:
            return check_variable_access(primary_expression->variable);
        case PrimaryExpression::INTEGER:
            return new TypeDenoter(TypeDenoter::INTEGER);
        case PrimaryExpression::FUNCTION:
            return check_function_designator(primary_expression->function);
        case PrimaryExpression::METHOD:
            return check_method_designator(primary_expression->method);
        case PrimaryExpression::OBJECT_INSTANTIATION:
            return check_object_instantiation(primary_expression->object_instantiation);
        case PrimaryExpression::PARENS:
            return check_expression(primary_expression->parens_expression);
        case PrimaryExpression::NOT:
            return check_primary_expression(primary_expression->not_expression);
        default:
            assert(false);
            return NULL;
    }
}

TypeDenoter * SemanticChecker::check_variable_access(VariableAccess * variable_access)
{
    switch (variable_access->type) {
        case VariableAccess::IDENTIFIER:
        {
            // it's the type of the declaration
            // figure out what variable this is referencing
            ClassSymbolTable * class_symbols = (*m_symbol_table)[m_class_id];
            FunctionSymbolTable * function_symbols = (*class_symbols->function_symbols)[m_function_id];
            if (function_symbols->variables->count(variable_access->identifier->text) > 0) {
                // local variable or parameter
                return (*function_symbols->variables)[variable_access->identifier->text]->type;
            } else if (class_symbols) {
                // class variable
                return (*class_symbols->variables)[variable_access->identifier->text]->type;
            } else {
                // undeclared variable
                std::cerr << "ERROR:" << variable_access->identifier->line_number <<
                    ":Undeclared varable: " << variable_access->identifier->text << std::endl;
                return NULL;
            }
            break;
        }
        case VariableAccess::INDEXED_VARIABLE:
            return check_indexed_variable(variable_access->indexed_variable);
        case VariableAccess::ATTRIBUTE:
            return check_attribute_designator(variable_access->attribute);
        default:
            assert(false);
            return NULL;
    }
}

// this is for functions; use method_designator_type for methods
TypeDenoter * SemanticChecker::check_function_designator(FunctionDesignator * function_designator)
{
    // look it up in the symbol table
    ClassSymbolTable * class_symbols = (*m_symbol_table)[m_class_id];
    if (class_symbols->function_symbols->count(function_designator->identifier->text)) {
        return (*class_symbols->function_symbols)[function_designator->identifier->text]->function_declaration->type;
    } else {
        std::cerr << "ERROR:" << function_designator->identifier->line_number <<
            ":Undeclared function: " << function_designator->identifier->text << std::endl;
        return NULL;
    }
}

TypeDenoter * SemanticChecker::check_method_designator(MethodDesignator * method_designator)
{
    // look it up in the symbol table
    TypeDenoter * owner_type = check_variable_access(method_designator->owner);
    assert(owner_type->type == TypeDenoter::CLASS);
    ClassSymbolTable * class_symbols = (*m_symbol_table)[owner_type->class_identifier->text];
    if (class_symbols->function_symbols->count(method_designator->function->identifier->text)) {
        return (*class_symbols->function_symbols)[method_designator->function->identifier->text]->function_declaration->type;
    } else {
        std::cerr << "ERROR:" << method_designator->function->identifier->line_number <<
            ":Undeclared method: " << method_designator->function->identifier->text << std::endl;
        return NULL;
    }
}

TypeDenoter * SemanticChecker::check_object_instantiation(ObjectInstantiation * object_instantiation)
{
    // look it up in the symbol table
    if (m_symbol_table->count(object_instantiation->class_identifier->text) > 0) {
        return new TypeDenoter(object_instantiation->class_identifier);
    } else {
        std::cerr << "ERROR:" << object_instantiation->class_identifier->line_number <<
            ":Undeclared class: " << object_instantiation->class_identifier->text << std::endl;
        return NULL;
    }
}

TypeDenoter * SemanticChecker::check_indexed_variable(IndexedVariable * indexed_variable)
{
    // every expression in the list should be an integer
    for (ExpressionList * expression_list = indexed_variable->expression_list; expression_list != NULL; expression_list = expression_list->next) {
        TypeDenoter * index_type = check_expression(expression_list->item);
        if (index_type == NULL) {
            // semantic error occured while determining type
            break;
        }
        if (index_type->type != TypeDenoter::INTEGER) {
            std::cerr << "ERROR:" << indexed_variable->variable->identifier->line_number <<
                ":Array index not an integer for variable \"" << indexed_variable->variable->identifier->text << "\"" << std::endl;
            m_success = false;
            break;
        }
    }

    return check_variable_access(indexed_variable->variable);
}

TypeDenoter * SemanticChecker::check_attribute_designator(AttributeDesignator * attribute_designator)
{
    TypeDenoter * owner_type = check_variable_access(attribute_designator->owner);
    assert(owner_type->type == TypeDenoter::CLASS);
    ClassSymbolTable * class_symbols = (*m_symbol_table)[owner_type->class_identifier->text];
    if (class_symbols->variables->count(attribute_designator->identifier->text) > 0) {
        return (*class_symbols->variables)[attribute_designator->identifier->text]->type;
    } else {
        std::cerr << "ERROR:" << attribute_designator->identifier->line_number <<
            ":Undeclared class: " << owner_type->class_identifier->text;
        return NULL;
    }
}

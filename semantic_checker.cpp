#include "semantic_checker.h"

#include <map>
#include <iostream>
#include <cassert>

SemanticChecker::SemanticChecker(Program * program, SymbolTable * symbol_table) :
    m_program(program),
    m_symbol_table(symbol_table) {}

bool SemanticChecker::check()
{
    bool ok = true;
    for (ClassList * class_list = m_program->class_list; class_list != NULL; class_list = class_list->next) {
        ClassDeclaration * class_declaration = class_list->item;
        m_class_id = class_declaration->identifier->text;
        
        // make sure array indicies are integers
        for (FunctionDeclarationList * function_list = class_declaration->class_block->function_list; function_list != NULL; function_list = function_list->next) {
            FunctionDeclaration * function_declaration = function_list->item;
            m_function_id = function_declaration->identifier->text;

            StatementList * statement_list = function_declaration->block->statement_list;
            ok = check_statement_list(statement_list) && ok;
        }
    }

    return ok;
}

bool SemanticChecker::check_statement_list(StatementList * _statement_list)
{
    bool ok = true;
    for (StatementList * statement_list = _statement_list; statement_list != NULL; statement_list = statement_list->next) {
        ok = check_statement(statement_list->item) && ok;
    }
    return ok;
}

bool SemanticChecker::check_statement(Statement * statement)
{
    bool ok = true;
    switch(statement->type) {
        case Statement::ASSIGNMENT:
        {
            VariableAccess * variable_access = statement->assignment->variable;
            switch (variable_access->type) {
                case VariableAccess::IDENTIFIER:
                    // TODO
                    break;
                case VariableAccess::INDEXED_VARIABLE:
                    ok = check_indexed_variable(variable_access->indexed_variable) && ok;
                    break;
                case VariableAccess::ATTRIBUTE:
                    // TODO
                    break;
            }
            break;
        }
        case Statement::IF:
            // TODO
            break;
        case Statement::PRINT:
            // TODO
            break;
        case Statement::WHILE:
            // TODO
            break;
        case Statement::COMPOUND:
            // TODO
            break;
    }
    return ok;
}

bool SemanticChecker::check_indexed_variable(IndexedVariable * indexed_variable)
{
    bool ok = true;
    // every expression in the list should be an integer
    for (ExpressionList * expression_list = indexed_variable->expression_list; expression_list != NULL; expression_list = expression_list->next) {
        TypeDenoter * index_type = expression_type(expression_list->item);
        if (index_type == NULL) {
            // semantic error occured while determining type
            break;
        }
        if (index_type->type != TypeDenoter::INTEGER) {
            std::cerr << "ERROR:" << indexed_variable->variable->identifier->line_number <<
                ":Array index not an integer for variable \"" << indexed_variable->variable->identifier->text << "\"" << std::endl;
            ok = false;
            break;
        }
    }
    return ok;
}

TypeDenoter * SemanticChecker::expression_type(Expression * expression)
{
    if (expression->right == NULL) {
        // it's just the type of the first additive expression
        return additive_expression_type(expression->left);
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

TypeDenoter * SemanticChecker::additive_expression_type(AdditiveExpression * additive_expression)
{
    TypeDenoter * right_type = multiplicitive_expression_type(additive_expression->right);
    if (additive_expression->left == NULL) {
        // it's just the type of the right
        return right_type;
    } else {
        TypeDenoter * left_type = additive_expression_type(additive_expression->left);
        if (left_type == NULL || right_type == NULL) {
            // semantic error occurred down the stack
            return NULL;
        }
        return combined_type(left_type, right_type);
    }
}

TypeDenoter * SemanticChecker::multiplicitive_expression_type(MultiplicativeExpression * multiplicative_expression) {
    TypeDenoter * right_type = negatable_expression_type(multiplicative_expression->right);
    if (multiplicative_expression->left == NULL) {
        // it's just the type of the right
        return right_type;
    } else {
        TypeDenoter * left_type = multiplicitive_expression_type(multiplicative_expression->left);
        if (left_type == NULL || right_type == NULL) {
            // semantic error occurred down the stack
            return NULL;
        }
        return combined_type(left_type, right_type);
    }
}

TypeDenoter * SemanticChecker::negatable_expression_type(NegatableExpression * negatable_expression) {
    if (negatable_expression->type == NegatableExpression::SIGN) {
        return negatable_expression_type(negatable_expression->next);
    } else if (negatable_expression->type == NegatableExpression::PRIMARY) {
        return primary_expression_type(negatable_expression->primary_expression);
    } else {
        assert(false);
        return NULL;
    }
}

TypeDenoter * SemanticChecker::primary_expression_type(PrimaryExpression * primary_expression) {
    switch (primary_expression->type) {
        case PrimaryExpression::VARIABLE:
            return variable_access_type(primary_expression->variable);
        case PrimaryExpression::FUNCTION:
            return function_designator_type(primary_expression->function);
        case PrimaryExpression::METHOD:
            return method_designator_type(primary_expression->method);
        case PrimaryExpression::OBJECT_INSTANTIATION:
            return object_instantiation_type(primary_expression->object_instantiation);
        case PrimaryExpression::PARENS:
            return expression_type(primary_expression->parens_expression);
        case PrimaryExpression::NOT:
            return primary_expression_type(primary_expression->not_expression);
        default:
            assert(false);
            return NULL;
    }
}

TypeDenoter * SemanticChecker::variable_access_type(VariableAccess * variable_access)
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
            return indexed_variable_type(variable_access->indexed_variable);
        case VariableAccess::ATTRIBUTE:
            return attribute_designator_type(variable_access->attribute);
        default:
            assert(false);
            return NULL;
    }
}

TypeDenoter * SemanticChecker::function_designator_type(FunctionDesignator * function_designator)
{
    // look it up in the symbol table
    return NULL; // TODO
}

TypeDenoter * SemanticChecker::method_designator_type(MethodDesignator * method_designator)
{
    // look it up in the symbol table
    return NULL; // TODO
}

TypeDenoter * SemanticChecker::object_instantiation_type(ObjectInstantiation * object_instantiation)
{
    // look it up in the symbol table
    return NULL; // TODO
}

TypeDenoter * SemanticChecker::indexed_variable_type(IndexedVariable * indexed_variable)
{
    return variable_access_type(indexed_variable->variable);
}

TypeDenoter * SemanticChecker::attribute_designator_type(AttributeDesignator * attribute_designator)
{
    // look it up in the symbol table
    return NULL; // TODO
}

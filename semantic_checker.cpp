#include "semantic_checker.h"
#include "utils.h"
using Utils::err_header;

#include <map>
#include <iostream>
#include <sstream>
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

        check_variable_declaration_list(class_declaration->class_block->variable_list);
        
        // make sure array indicies are integers
        for (FunctionDeclarationList * function_list = class_declaration->class_block->function_list; function_list != NULL; function_list = function_list->next) {
            FunctionDeclaration * function_declaration = function_list->item;
            m_function_id = function_declaration->identifier->text;

            check_variable_declaration_list(function_declaration->parameter_list);

            StatementList * statement_list = function_declaration->block->statement_list;
            check_statement_list(statement_list);
        }
    }

    return m_success;
}

void SemanticChecker::check_variable_declaration_list(VariableDeclarationList * _variable_list)
{
    for (VariableDeclarationList * variable_list = _variable_list; variable_list != NULL; variable_list = variable_list->next)
        check_variable_declaration(variable_list->item);
}

void SemanticChecker::check_variable_declaration(VariableDeclaration * variable)
{
    TypeDenoter * type = variable->type;
    switch(type->type) {
        case TypeDenoter::INTEGER:
            break;
        case TypeDenoter::REAL:
            break;
        case TypeDenoter::CHAR:
            break;
        case TypeDenoter::BOOLEAN:
            break;
        case TypeDenoter::CLASS:
            break;
        case TypeDenoter::ARRAY:
            // make sure the range is valid
            if (! (type->array_type->max >= type->array_type->min)) {
                std::cerr << err_header(type->array_type->line_number) << "invalid array range: [" <<
                    type->array_type->min << ".." << type->array_type->max << "]" << std::endl;
                m_success = false;
            }
            break;
        default:
            assert(false);
    }
}

void SemanticChecker::check_statement_list(StatementList * _statement_list)
{
    for (StatementList * statement_list = _statement_list; statement_list != NULL; statement_list = statement_list->next)
        check_statement(statement_list->item);
}

bool SemanticChecker::types_equal(TypeDenoter * type1, TypeDenoter * type2)
{
    if (type1->type == type2->type) {
        if (type1->type == TypeDenoter::ARRAY) {
            // make sure arrays are same size and of same type
            bool size_equal = (type1->array_type->max - type1->array_type->min) ==
                (type2->array_type->max - type2->array_type->min);
            return size_equal && types_equal(type1->array_type->type, type2->array_type->type);
        } else if (type1->type == TypeDenoter::CLASS) {
            return type1->class_identifier->text.compare(type2->class_identifier->text) == 0;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

bool SemanticChecker::assignment_valid(TypeDenoter * left_type, TypeDenoter * right_type)
{
    // rules for assignment
    // X = X - OK, but if it's an array, has to be the same size
    // integer = char - OK
    // real = integer/char - OK
    // A = B - OK if A is an ancestor of B or if A and B's fields are respectively compatible
    if (left_type->type == right_type->type) {
        if (left_type->type == TypeDenoter::ARRAY) {
            bool size_equal = (left_type->array_type->max - left_type->array_type->min) ==
                (right_type->array_type->max - right_type->array_type->min);
            return size_equal && assignment_valid(left_type->array_type->type, right_type->array_type->type);
        } else {
            return true;
        }
    } else if (left_type->type == TypeDenoter::INTEGER && right_type->type == TypeDenoter::CHAR) {
        return true;
    } else if (left_type->type == TypeDenoter::REAL && (right_type->type == TypeDenoter::INTEGER || right_type->type == TypeDenoter::CHAR)) {
        return true;
    } else {
        return false;
    }
}

std::string SemanticChecker::type_to_string(TypeDenoter * type)
{
    std::stringstream ss;
    switch(type->type) {
        case TypeDenoter::INTEGER:
            ss << "integer";
            break;
        case TypeDenoter::REAL:
            ss << "real";
            break;
        case TypeDenoter::CHAR:
            ss << "char";
            break;
        case TypeDenoter::BOOLEAN:
            ss << "boolean";
            break;
        case TypeDenoter::CLASS:
            ss << type->class_identifier->text;
            break;
        case TypeDenoter::ARRAY:
            ss << "array[" << type->array_type->min << ".." << type->array_type->max << "] of " <<
                type_to_string(type->array_type->type);
            break;
        default:
            assert(false);
    }
    return ss.str();
}

void SemanticChecker::check_statement(Statement * statement)
{
    switch(statement->type) {
        case Statement::ASSIGNMENT:
        {
            TypeDenoter * left_type = check_variable_access(statement->assignment->variable);
            TypeDenoter * right_type = check_expression(statement->assignment->expression);
            if (! assignment_valid(left_type, right_type)) {
                std::cerr << err_header(statement->assignment->variable->identifier->line_number) <<
                    "cannot assign \"" << type_to_string(right_type) << "\" to \"" <<
                    type_to_string(left_type) << "\"" << std::endl;
                m_success = false;
            }
            break;
        }
        case Statement::IF:
            check_expression(statement->if_statement->expression);
            check_statement(statement->if_statement->then_statement);
            if (statement->if_statement->else_statement == NULL)
                check_statement(statement->if_statement->else_statement);
            break;
        case Statement::PRINT:
            check_expression(statement->print_statement->expression);
            break;
        case Statement::WHILE:
            check_expression(statement->while_statement->expression);
            check_statement(statement->while_statement->statement);
            break;
        case Statement::COMPOUND:
            check_statement_list(statement->compound_statement);
            break;
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
        case PrimaryExpression::REAL:
            return new TypeDenoter(TypeDenoter::REAL);
        case PrimaryExpression::BOOLEAN:
            return new TypeDenoter(TypeDenoter::BOOLEAN);
        case PrimaryExpression::STRING:
        {
            std::string str = primary_expression->literal_string;
            int str_len = (int) str.length();
            if (str_len == 1) {
                return new TypeDenoter(TypeDenoter::CHAR);
            } else {
                return new TypeDenoter(new ArrayType(0, 0, str_len-1, new TypeDenoter(TypeDenoter::CHAR)));
            }
        }
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
                std::cerr << err_header(variable_access->identifier->line_number) <<
                    "Undeclared varable: " << variable_access->identifier->text << std::endl;
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
        std::cerr << err_header(function_designator->identifier->line_number) <<
            "Undeclared function: " << function_designator->identifier->text << std::endl;
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
        std::cerr << err_header(method_designator->function->identifier->line_number) <<
            "Undeclared method: " << method_designator->function->identifier->text << std::endl;
        return NULL;
    }
}

TypeDenoter * SemanticChecker::check_object_instantiation(ObjectInstantiation * object_instantiation)
{
    // look it up in the symbol table
    if (m_symbol_table->count(object_instantiation->class_identifier->text) > 0) {
        return new TypeDenoter(object_instantiation->class_identifier);
    } else {
        std::cerr << err_header(object_instantiation->class_identifier->line_number) <<
            "Undeclared class: " << object_instantiation->class_identifier->text << std::endl;
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
            std::cerr << err_header(indexed_variable->variable->identifier->line_number) <<
                "Array index not an integer for variable \"" << indexed_variable->variable->identifier->text << "\"" << std::endl;
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
        std::cerr << err_header(attribute_designator->identifier->line_number) <<
            "Undeclared class: " << owner_type->class_identifier->text;
        return NULL;
    }
}

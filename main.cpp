#include "parser.h"

#include <map>
#include <iostream>

bool check_program(Program * program);
bool check_indexed_variable(IndexedVariable * indexed_variable);
bool check_statement_list(StatementList * _statement_list);
bool check_statement(Statement * statement);

// functions to get the types of structures
// null means semantic error occured
TypeDenoter * expression_type(Expression * expression);
TypeDenoter * additive_expression_type(AdditiveExpression * _additive_expression);
TypeDenoter * multiplicitive_expression_type(MultiplicativeExpression * multiplicative_expression);
TypeDenoter * negatable_expression_type(NegatableExpression * negatable_expression);
TypeDenoter * combined_type(TypeDenoter * left_type, TypeDenoter * right_type);
TypeDenoter * primary_expression_type(PrimaryExpression * primary_expression);
TypeDenoter * variable_access_type(VariableAccess * variable_access);
TypeDenoter * function_designator_type(FunctionDesignator * function_designator);
TypeDenoter * method_designator_type(MethodDesignator * method_designator);
TypeDenoter * object_instantiation_type(ObjectInstantiation * object_instantiation);
TypeDenoter * indexed_variable_type(IndexedVariable * indexed_variable);
TypeDenoter * attribute_designator_type(AttributeDesignator * attribute_designator);

int main() {
    Program * program = parse_input();

    if (! check_program(program))
        return -1;

    return 0;
}

// check everything that could possibly go wrong. 
// return true if everything is OK
bool check_program(Program * program) {
    // one pass to build symbol table
    // collect all the classes that are declared
    std::map<char *, ClassDeclaration *> class_map;
    for (ClassList * class_list = program->class_list; class_list != NULL; class_list = class_list->next) {
        ClassDeclaration * class_declaration = class_list->item;
        class_map[class_declaration->id] = class_declaration;
        // collect all the variables that are declared
        // TODO
    }

    // now pass to validate
    bool ok = true;
    for (ClassList * class_list = program->class_list; class_list != NULL; class_list = class_list->next) {
        ClassDeclaration * class_declaration = class_list->item;
        
        // make sure array indicies are integers
        for (FunctionDeclarationList * function_list = class_declaration->class_block->function_list; function_list != NULL; function_list = function_list->next) {
            FunctionDeclaration * function_declaration = function_list->item;
            StatementList * statement_list = function_declaration->block->statement_list;
            ok = check_statement_list(statement_list) && ok;
        }
    }

    return ok;
}

bool check_statement_list(StatementList * _statement_list) {
    bool ok = true;
    for (StatementList * statement_list = _statement_list; statement_list != NULL; statement_list = statement_list->next) {
        ok = check_statement(statement_list->item) && ok;
    }
    return ok;
}

bool check_statement(Statement * statement) {
    bool ok = true;
    switch(statement->type) {
        case Statement::ASSIGNMENT:
        {
            VariableAccess * variable_access = statement->assignment->variable;
            switch (variable_access->type) {
                case VariableAccess::IDENTIFIER:
                    break;
                case VariableAccess::INDEXED_VARIABLE:
                    ok = check_indexed_variable(variable_access->indexed_variable) && ok;
                    break;
                case VariableAccess::ATTRIBUTE:
                    break;
            }
            break;
        }
        case Statement::IF:
            break;
        case Statement::PRINT:
            break;
        case Statement::WHILE:
            break;
        case Statement::COMPOUND:
            break;
    }
    return ok;
}

bool check_indexed_variable(IndexedVariable * indexed_variable) {
    bool ok = true;
    // every expression in the list should be an integer
    for (ExpressionList * expression_list = indexed_variable->expression_list; expression_list != NULL; expression_list = expression_list->next) {
        TypeDenoter * index_type = expression_type(expression_list->item);
        if (index_type == NULL) {
            // semantic error occured while determining type
            break;
        }
        if (index_type->type != TypeDenoter::INTEGER) {
            std::cerr << "ERROR:" << 100 << ":Array index not an integer for variable \"" <<
                indexed_variable->variable->id << "\"" << std::endl;
            ok = false;
            break;
        }
    }
    return ok;
}

TypeDenoter * expression_type(Expression * expression) {
    if (expression->right == NULL) {
        // it's just the type of the first additive expression
        return additive_expression_type(expression->left);
    } else {
        // we're looking at a compare operator, so it always returns a boolean
        return new TypeDenoter(TypeDenoter::BOOLEAN);
    }

}

// when we do a multiplicitive or additive operation, what is the return type?
TypeDenoter * combined_type(TypeDenoter * left_type, TypeDenoter * right_type) {
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

TypeDenoter * additive_expression_type(AdditiveExpression * additive_expression) {
    TypeDenoter * left_type = multiplicitive_expression_type(additive_expression->left);
    if (additive_expression->right == NULL) {
        // it's just the type of the left
        return left_type;
    } else {
        TypeDenoter * right_type = additive_expression_type(additive_expression->right);
        if (left_type == NULL || right_type == NULL) {
            // semantic error occurred down the stack
            return NULL;
        }
        return combined_type(left_type, right_type);
    }
}

TypeDenoter * multiplicitive_expression_type(MultiplicativeExpression * multiplicative_expression) {
    TypeDenoter * left_type = negatable_expression_type(multiplicative_expression->left);
    if (multiplicative_expression->right == NULL) {
        // it's just the type of the left
        return left_type;
    } else {
        TypeDenoter * right_type = multiplicitive_expression_type(multiplicative_expression->right);
        if (left_type == NULL || right_type == NULL) {
            // semantic error occurred down the stack
            return NULL;
        }
        return combined_type(left_type, right_type);
    }
}

TypeDenoter * negatable_expression_type(NegatableExpression * negatable_expression) {
    if (negatable_expression->type == NegatableExpression::SIGN) {
        return negatable_expression_type(negatable_expression->next);
    } else if (negatable_expression->type == NegatableExpression::PRIMARY) {
        return primary_expression_type(negatable_expression->primary_expression);
    }
}

TypeDenoter * primary_expression_type(PrimaryExpression * primary_expression) {
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
    }
}

TypeDenoter * variable_access_type(VariableAccess * variable_access) {
    switch (variable_access->type) {
        case VariableAccess::IDENTIFIER:
            // look up the identifier in the symbol table to determine type
            // TODO
            break;
        case VariableAccess::INDEXED_VARIABLE:
            return indexed_variable_type(variable_access->indexed_variable);
        case VariableAccess::ATTRIBUTE:
            return attribute_designator_type(variable_access->attribute);
    }
}

TypeDenoter * function_designator_type(FunctionDesignator * function_designator) {
    // look it up in the symbol table
    // TODO
}

TypeDenoter * method_designator_type(MethodDesignator * method_designator) {
    // look it up in the symbol table
    // TODO
}

TypeDenoter * object_instantiation_type(ObjectInstantiation * object_instantiation) {
    // look it up in the symbol table
    // TODO
}

TypeDenoter * indexed_variable_type(IndexedVariable * indexed_variable) {
    return variable_access_type(indexed_variable->variable);
}

TypeDenoter * attribute_designator_type(AttributeDesignator * attribute_designator) {
    // look it up in the symbol table
    // TODO
}

#include "code_generation.h"

#include <vector>
#include <cassert>

class Instruction {
};

class CodeGenerator {
private:
    std::vector<Instruction *> m_instructions;
    int next_register_number;

public:
    void generate(FunctionDeclaration * function_declaration);

private:
    int current_index() { return m_instructions.size(); }

    void scan_statement_list(StatementList * statement_list);
    void scan_statement(Statement * statement);
    int scan_expression(Expression * expression);


    void add_assignment(VariableAccess * variable, int value_register);
};

void generate_code(Program * program) {
    for (ClassList * class_list_node = program->class_list; class_list_node != NULL; class_list_node = class_list_node->next) {
        ClassDeclaration * class_declaration = class_list_node->item;
        for (FunctionDeclarationList * function_list_node = class_declaration->class_block->function_list; function_list_node != NULL; function_list_node = function_list_node->next) {
            FunctionDeclaration * function_declaration = function_list_node->item;
            CodeGenerator generator;
            generator.generate(function_declaration);
        }
    }
}

void CodeGenerator::generate(FunctionDeclaration * function_declaration) {
    scan_statement_list(function_declaration->block->statement_list);
}

void CodeGenerator::scan_statement_list(StatementList * statement_list) {
    for (StatementList * statement_list_node = statement_list; statement_list_node != NULL; statement_list_node = statement_list_node->next)
        scan_statement(statement_list_node->item);
}

void CodeGenerator::scan_statement(Statement * statement) {
    switch (statement->type) {
        case Statement::ASSIGNMENT:
        {
            int value_register = scan_expression(statement->assignment->expression);
            add_assignment(statement->assignment->variable, value_register);
            break;
        }
        /*
        case Statement::IF:
            check_expression(statement->if_statement->expression);
            check_statement(statement->if_statement->then_statement);
            if (statement->if_statement->else_statement != NULL)
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
        case Statement::METHOD:
            check_method_designator(statement->method);
            break;
        case Statement::FUNCTION:
            check_function_designator(statement->function);
            break;
        case Statement::ATTRIBUTE:
            // actually this is a method call
            check_method_designator(new MethodDesignator(statement->attribute->owner, new FunctionDesignator(statement->attribute->identifier, NULL)));
            break;
        */
        default:
            assert(false);

    }
}


int CodeGenerator::scan_expression(Expression * expression) {
    return 0;
}


void CodeGenerator::add_assignment(VariableAccess * variable, int value_register) {
    switch (variable->type) {
        case VariableAccess::IDENTIFIER:
        {
            break;
        }
        /*
        case VariableAccess::INDEXED_VARIABLE:
            return check_indexed_variable(variable_access->indexed_variable);
        case VariableAccess::ATTRIBUTE:
            return check_attribute_designator(variable_access->attribute);
        case VariableAccess::THIS:
            return new TypeDenoter(m_symbol_table->item(m_class_id)->class_declaration->identifier);
        */
        default:
            assert(false);
    }
}


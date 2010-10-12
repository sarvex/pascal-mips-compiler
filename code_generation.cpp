#include "code_generation.h"
#include "insensitive_map.h"

#include <vector>
#include <set>
#include <cassert>
#include <iostream>

struct Instruction {
    enum Type {
        COPY,
        OPERATOR,
        UNARY,
        IMMEDIATE_BOOLEAN,
        IMMEDIATE_INT,
        IMMEDIATE_REAL,
        IF,
        GOTO,
        RETURN,
        PRINT,
    };
    Type type;

    Instruction(Type type) : type(type) {}
};

struct CopyInstruction : public Instruction {
    int dest; // register number
    int source; // register number
    CopyInstruction(int dest, int source) : Instruction(COPY), dest(dest), source(source) {}
};

struct OperatorInstruction : public Instruction {
    enum Operator {
        EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
        PLUS, MINUS, OR,
        TIMES, DIVIDE, MOD, AND,
    };

    int dest;
    int left;
    Operator _operator;
    int right;

    OperatorInstruction(int dest, int left, Operator _operator, int right) : Instruction(OPERATOR), dest(dest), left(left), _operator(_operator), right(right) {}
};

struct UnaryInstruction : public Instruction {
    enum Operator {
        NOT,
        NEGATE,
    };

    int dest;
    Operator _operator;
    int source;

    UnaryInstruction(int dest, Operator _operator, int source) : Instruction(UNARY), dest(dest), _operator(_operator), source(source) {}
};

struct ImmediateBoolean : public Instruction {
    int dest;
    bool constant;

    ImmediateBoolean(int dest, bool constant) : Instruction(IMMEDIATE_BOOLEAN), dest(dest), constant(constant) {}
};

struct ImmediateInteger : public Instruction {
    int dest;
    int constant;

    ImmediateInteger(int dest, int constant) : Instruction(IMMEDIATE_INT), dest(dest), constant(constant) {}
};

struct ImmediateReal : public Instruction {
    int dest;
    float constant;

    ImmediateReal(int dest, float constant) : Instruction(IMMEDIATE_REAL), dest(dest), constant(constant) {}
};

struct IfInstruction : public Instruction {
    int condition;
    int goto_index;

    IfInstruction(int condition, int goto_index) : Instruction(IF), condition(condition), goto_index(goto_index) {}
};

struct GotoInstruction : public Instruction {
    int goto_index;

    GotoInstruction(int goto_index) : Instruction(GOTO), goto_index(goto_index) {}
};

struct ReturnInstruction : public Instruction {
    ReturnInstruction() : Instruction(RETURN) {}
};

struct PrintInstruction : public Instruction {
    int value;
    PrintInstruction(int value) : Instruction(PRINT), value(value) {}
};


struct BasicBlock {
    int start;
    int end;
    // parent and child indexes in m_basic_blocks
    std::set<int> children;
    std::set<int> parents;
    BasicBlock(int start, int end) : start(start), end(end) {}
};

class CodeGenerator {
public:
    CodeGenerator() : m_register_count(0) {}
    void generate(FunctionDeclaration * function_declaration);
    void build_basic_blocks();

    void print_disassembly(int i);
    void print_basic_blocks();
    void print_control_flow_graph();

private:
    std::vector<Instruction *> m_instructions;
    OrderedInsensitiveMap<int> m_variable_numbers;
    int m_register_count;
    std::vector<BasicBlock *> m_basic_blocks;

private:
    int current_index() { return m_instructions.size(); }
    int next_available_register() { return m_register_count++; }

    void gen_statement_list(StatementList * statement_list);
    void gen_statement(Statement * statement);
    int gen_expression(Expression * expression);
    int gen_additive_expression(AdditiveExpression * additive_expression);
    int gen_multiplicitive_expression(MultiplicativeExpression * multiplicative_expression);
    int gen_negatable_expression(NegatableExpression * negatable_expression);
    int gen_primary_expression(PrimaryExpression * primary_expression);
    int gen_variable_access(VariableAccess * variable);

    void gen_assignment(VariableAccess * variable, int value_register);
    void link_parent_and_child(int parent_index, int child_index);
};

void generate_code(Program * program) {
    for (ClassList * class_list_node = program->class_list; class_list_node != NULL; class_list_node = class_list_node->next) {
        ClassDeclaration * class_declaration = class_list_node->item;
        for (FunctionDeclarationList * function_list_node = class_declaration->class_block->function_list; function_list_node != NULL; function_list_node = function_list_node->next) {
            FunctionDeclaration * function_declaration = function_list_node->item;

            std::cout << "Method " << class_declaration->identifier->text << "." << function_declaration->identifier->text << std::endl;
            std::cout << "--------------------------" << std::endl;

            CodeGenerator generator;
            generator.generate(function_declaration);
            generator.build_basic_blocks();

            std::cout << "Disassembly" << std::endl;
            std::cout << "--------------------------" << std::endl;
            generator.print_basic_blocks();
            std::cout << "--------------------------" << std::endl;

            std::cout << "Control Flow Graph" << std::endl;
            std::cout << "--------------------------" << std::endl;
            generator.print_control_flow_graph();
            std::cout << "--------------------------" << std::endl;
        }
    }
}

void CodeGenerator::print_disassembly(int i) {
    std::cout << i << ":\t";
    Instruction * instruction = m_instructions[i];
    switch (instruction->type) {
        case Instruction::COPY:
        {
            CopyInstruction * copy_instruction = (CopyInstruction *) instruction;
            std::cout << "$" << copy_instruction->dest << " = $" << copy_instruction->source;
            break;
        }
        case Instruction::OPERATOR:
        {
            OperatorInstruction * operator_instruction = (OperatorInstruction *) instruction;
            std::cout << "$" << operator_instruction->dest << " = $" << operator_instruction->left << " ";
            switch (operator_instruction->_operator) {
                case OperatorInstruction::EQUAL:
                    std::cout << "=="; break;
                case OperatorInstruction::NOT_EQUAL:
                    std::cout << "!="; break;
                case OperatorInstruction::LESS:
                    std::cout << "<"; break;
                case OperatorInstruction::GREATER:
                    std::cout << ">"; break;
                case OperatorInstruction::LESS_EQUAL:
                    std::cout << "<="; break;
                case OperatorInstruction::GREATER_EQUAL:
                    std::cout << ">="; break;
                case OperatorInstruction::PLUS:
                    std::cout << "+"; break;
                case OperatorInstruction::MINUS:
                    std::cout << "-"; break;
                case OperatorInstruction::OR:
                    std::cout << "||"; break;
                case OperatorInstruction::TIMES:
                    std::cout << "*"; break;
                case OperatorInstruction::DIVIDE:
                    std::cout << "/"; break;
                case OperatorInstruction::MOD:
                    std::cout << "%"; break;
                case OperatorInstruction::AND:
                    std::cout << "&&"; break;
                default:
                    assert(false);
            }
            std::cout << " $" << operator_instruction->right;
            break;
        }
        case Instruction::UNARY:
        {
            UnaryInstruction * unary_instruction = (UnaryInstruction *) instruction;
            std::cout << "$" << unary_instruction->dest << " = ";
            if (unary_instruction->_operator == UnaryInstruction::NEGATE)
                std::cout << "-";
            else if (unary_instruction->_operator == UnaryInstruction::NOT)
                std::cout << "!";
            else
                assert(false);
            std::cout << "$" << unary_instruction->source;
            break;
        }
        case Instruction::IMMEDIATE_BOOLEAN:
        {
            ImmediateBoolean * immediate_bool_instruction = (ImmediateBoolean *) instruction;
            std::cout << "$" << immediate_bool_instruction->dest << " = " << (immediate_bool_instruction->constant ? "true" : "false");
            break;
        }
        case Instruction::IMMEDIATE_INT:
        {
            ImmediateInteger * immediate_int_instruction = (ImmediateInteger *) instruction;
            std::cout << "$" << immediate_int_instruction->dest << " = " << immediate_int_instruction->constant;
            break;
        }
        case Instruction::IMMEDIATE_REAL:
        {
            ImmediateReal * immediate_real_instruction = (ImmediateReal *) instruction;
            std::cout << "$" << immediate_real_instruction->dest << " = " << immediate_real_instruction->constant;
            break;
        }
        case Instruction::IF:
        {
            IfInstruction * if_instruction = (IfInstruction *) instruction;
            std::cout << "if !$" << if_instruction->condition << " goto " << if_instruction->goto_index;
            break;
        }
        case Instruction::GOTO:
        {
            GotoInstruction * goto_instruction = (GotoInstruction *) instruction;
            std::cout << "goto " << goto_instruction->goto_index;
            break;
        }
        case Instruction::RETURN:
            std::cout << "return";
            break;
        case Instruction::PRINT:
            PrintInstruction * print_instruction = (PrintInstruction *) instruction;
            std::cout << "print $" << print_instruction->value;
            break;
    }
    std::cout << ";" << std::endl;
}

void CodeGenerator::print_basic_blocks() {
    for (unsigned int b = 0; b < m_basic_blocks.size(); b++) {
        std::cout << "block_" << b << ":" << std::endl;
        BasicBlock * block = m_basic_blocks[b];
        for (int i = block->start; i < block->end; i++)
            print_disassembly(i);
    }
}

void CodeGenerator::print_control_flow_graph() {
    for (unsigned int parent = 0; parent < m_basic_blocks.size(); parent++) {
        BasicBlock * parent_block = m_basic_blocks[parent];
        for (unsigned int child = 0; child < m_basic_blocks.size(); child++) {
            if (parent == child) {
                // for self, print the number (ugly if more than 1 digit)
                std::cout << parent;
            } else if (parent_block->children.count(child)) {
                // child is a child of parent
                std::cout << (child < parent ? "^" : "v");
            } else {
                // nothing to see here
                std::cout << " ";
            }
            std::cout << " ";
        }
        std::cout << std::endl;
    }
}


void CodeGenerator::generate(FunctionDeclaration * function_declaration) {
    for (VariableDeclarationList * variable_list = function_declaration->block->variable_list; variable_list != NULL; variable_list = variable_list->next) {
        for (IdentifierList * id_list = variable_list->item->id_list; id_list != NULL; id_list = id_list->next)
            m_variable_numbers.put(id_list->item->text, next_available_register());
    }

    gen_statement_list(function_declaration->block->statement_list);

    m_instructions.push_back(new ReturnInstruction());
}

void CodeGenerator::gen_statement_list(StatementList * statement_list) {
    for (StatementList * statement_list_node = statement_list; statement_list_node != NULL; statement_list_node = statement_list_node->next) {
        if (statement_list_node->item != NULL)
            gen_statement(statement_list_node->item);
    }
}

void CodeGenerator::gen_statement(Statement * statement) {
    switch (statement->type) {
        case Statement::ASSIGNMENT:
        {
            int value_register = gen_expression(statement->assignment->expression);
            gen_assignment(statement->assignment->variable, value_register);
            break;
        }
        case Statement::IF:
        {
            int condition = gen_expression(statement->if_statement->expression);

            IfInstruction * if_instruction = new IfInstruction(condition, -1);
            m_instructions.push_back(if_instruction);

            gen_statement(statement->if_statement->then_statement);
            if (statement->if_statement->else_statement != NULL) {
                GotoInstruction * goto_instruction = new GotoInstruction(-1);
                m_instructions.push_back(goto_instruction);
                if_instruction->goto_index = m_instructions.size();
                gen_statement(statement->if_statement->else_statement);
                goto_instruction->goto_index = m_instructions.size();
            } else {
                if_instruction->goto_index = m_instructions.size();
            }

            break;
        }
        case Statement::PRINT:
        {
            int value = gen_expression(statement->print_statement->expression);
            m_instructions.push_back(new PrintInstruction(value));
            break;
        }
        case Statement::WHILE:
        {
            int while_start = m_instructions.size();
            int condition = gen_expression(statement->while_statement->expression);
            IfInstruction * if_instruction = new IfInstruction(condition, -1);
            m_instructions.push_back(if_instruction);
            gen_statement(statement->while_statement->statement);
            m_instructions.push_back(new GotoInstruction(while_start));
            if_instruction->goto_index = m_instructions.size();
            break;
        }
        case Statement::COMPOUND:
            gen_statement_list(statement->compound_statement);
            break;
            /*
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


int CodeGenerator::gen_expression(Expression * expression) {
    if (expression->right == NULL) {
        // it's just the type of the first additive expression
        return gen_additive_expression(expression->left);
    } else {
        // we're looking at a compare operator
        int left = gen_additive_expression(expression->left);
        int right = gen_additive_expression(expression->right);
        int dest = next_available_register();
        OperatorInstruction::Operator _operator = (OperatorInstruction::Operator)(expression->_operator->type + OperatorInstruction::EQUAL); // LOL HAX!
        m_instructions.push_back(new OperatorInstruction(dest, left, _operator, right));
        return dest;
    }
}

int CodeGenerator::gen_additive_expression(AdditiveExpression * additive_expression) {
    int right = gen_multiplicitive_expression(additive_expression->right);

    if (additive_expression->left == NULL) {
        // pass the right through
        return right;
    } else {
        int left = gen_additive_expression(additive_expression->left);
        int dest = next_available_register();
        OperatorInstruction::Operator _operator = (OperatorInstruction::Operator)(additive_expression->_operator->type + OperatorInstruction::PLUS);
        m_instructions.push_back(new OperatorInstruction(dest, left, _operator, right));
        return dest;
    }
}

int CodeGenerator::gen_multiplicitive_expression(MultiplicativeExpression * multiplicative_expression) {
    int right = gen_negatable_expression(multiplicative_expression->right);

    if (multiplicative_expression->left == NULL) {
        // pass the right through
        return right;
    } else {
        int left = gen_multiplicitive_expression(multiplicative_expression->left);
        int dest = next_available_register();
        OperatorInstruction::Operator _operator = (OperatorInstruction::Operator)(multiplicative_expression->_operator->type + OperatorInstruction::TIMES);
        m_instructions.push_back(new OperatorInstruction(dest, left, _operator, right));
        return dest;
    }
}

int CodeGenerator::gen_negatable_expression(NegatableExpression * negatable_expression) {
    if (negatable_expression->type == NegatableExpression::PRIMARY) {
        return gen_primary_expression(negatable_expression->primary_expression);
    } else if (negatable_expression->type == NegatableExpression::SIGN) {
        int source = gen_negatable_expression(negatable_expression->next);
        int dest = next_available_register();
        m_instructions.push_back(new UnaryInstruction(dest, UnaryInstruction::NEGATE, source));
        return dest;
    } else {
        assert(false);
        return -1;
    }
}

int CodeGenerator::gen_primary_expression(PrimaryExpression * primary_expression) {
    switch (primary_expression->type) {
        case PrimaryExpression::VARIABLE:
            return gen_variable_access(primary_expression->variable);
        case PrimaryExpression::INTEGER:
        {
            int dest = next_available_register();
            int constant = primary_expression->literal_integer->value;
            m_instructions.push_back(new ImmediateInteger(dest, constant));
            return dest;
        }
        case PrimaryExpression::BOOLEAN:
        {
            int dest = next_available_register();
            bool constant = primary_expression->literal_boolean->value;
            m_instructions.push_back(new ImmediateBoolean(dest, constant));
            return dest;
        }
        case PrimaryExpression::REAL:
        {
            int dest = next_available_register();
            float constant = primary_expression->literal_real->value;
            m_instructions.push_back(new ImmediateReal(dest, constant));
            return dest;
        }
        case PrimaryExpression::PARENS:
            return gen_expression(primary_expression->parens_expression);
        case PrimaryExpression::NOT:
        {
            int dest = next_available_register();
            int source = gen_primary_expression(primary_expression->not_expression);
            m_instructions.push_back(new UnaryInstruction(dest, UnaryInstruction::NOT, source));
            return dest;
        }
            /*
        case PrimaryExpression::STRING:
        case PrimaryExpression::FUNCTION:
        case PrimaryExpression::METHOD:
        case PrimaryExpression::OBJECT_INSTANTIATION:
             */

        default:
            assert(false);
    }
}

int CodeGenerator::gen_variable_access(VariableAccess * variable) {
    switch (variable->type) {
        case VariableAccess::IDENTIFIER:
            return m_variable_numbers.get(variable->identifier->text);
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

void CodeGenerator::gen_assignment(VariableAccess * variable, int value_register) {
    switch (variable->type) {
        case VariableAccess::IDENTIFIER:
        {
            m_instructions.push_back(new CopyInstruction(m_variable_numbers.get(variable->identifier->text), value_register));
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

void CodeGenerator::link_parent_and_child(int parent_index, int child_index) {
    m_basic_blocks[parent_index]->children.insert(child_index);
    m_basic_blocks[child_index]->parents.insert(parent_index);
}

void CodeGenerator::build_basic_blocks() {
    // identify breaks between blocks
    std::set<int> block_break_indexes;
    // 0 is a break, but leave it out for easier iteration later.
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        Instruction * instruction = m_instructions[i];
        switch (instruction->type) {
            case Instruction::IF:
            {
                IfInstruction * if_instruction = (IfInstruction *) instruction;
                block_break_indexes.insert(if_instruction->goto_index);
                block_break_indexes.insert(i + 1);
                break;
            }
            case Instruction::GOTO:
            {
                GotoInstruction * goto_instruction = (GotoInstruction *) instruction;
                block_break_indexes.insert(goto_instruction->goto_index);
                block_break_indexes.insert(i + 1);
                break;
            }
            case Instruction::RETURN:
                // throw one in at the end for easier iteration later.
                block_break_indexes.insert(i + 1);
            default:
                break;
        }
    }

    // construct blocks
    std::map<int, int> instruction_index_to_block_index;
    int start_index = 0; // first block starts at 0
    for (std::set<int>::iterator iter = block_break_indexes.begin(); iter != block_break_indexes.end(); iter++) {
        int end_index = *iter;
        BasicBlock * block = new BasicBlock(start_index, end_index);
        instruction_index_to_block_index[start_index] = m_basic_blocks.size();
        m_basic_blocks.push_back(block);

        start_index = end_index;
    }

    // connect blocks together
    for (unsigned int i = 0; i < m_basic_blocks.size(); i++) {
        BasicBlock * block = m_basic_blocks[i];
        int last_index = block->end- 1;
        Instruction * instruction = m_instructions[last_index];
        switch (instruction->type) {
            case Instruction::IF:
            {
                // two children
                IfInstruction * if_instruction = (IfInstruction *) instruction;
                int child1 = instruction_index_to_block_index[if_instruction->goto_index];
                link_parent_and_child(i, child1);
                int child2 = i + 1;
                link_parent_and_child(i, child2);
                break;
            }
            case Instruction::GOTO:
            {
                // one distant child
                GotoInstruction * goto_instruction = (GotoInstruction *) instruction;
                int child = instruction_index_to_block_index[goto_instruction->goto_index];
                link_parent_and_child(i, child);
                break;
            }
            case Instruction::RETURN:
                // no children
                break;
            default:
            {
                // next block is only child
                int child = i + 1;
                link_parent_and_child(i, child);
                break;
            }
        }
    }
}


#include "code_generation.h"
#include "insensitive_map.h"

#include <vector>
#include <set>
#include <cassert>
#include <iostream>
#include <list>
#include <sstream>


class CodeGenerator {
public:
    CodeGenerator() : m_register_count(0) {}
    void generate(FunctionDeclaration * function_declaration);
    void build_basic_blocks();

    void print_basic_blocks();
    void print_control_flow_graph();

private:
    struct Instruction {
        enum Type {
            COPY,
            OPERATOR,
            UNARY,
            IF,
            GOTO,
            RETURN,
            PRINT,
        };
        Type type;

        Instruction(Type type) : type(type) {}
    };

    struct RegisterOrConstant {
        enum Type {
            REGISTER,
            CONST_INT,
            CONST_BOOL,
            CONST_REAL,
        };
        Type type;
        union {
            int register_number;
            int const_int;
            bool const_bool;
            float const_real;
        };

        RegisterOrConstant(int register_number) : type(REGISTER), register_number(register_number) {}
        RegisterOrConstant(int const_int, bool) : type(CONST_INT), const_int(const_int) {}
        RegisterOrConstant(bool const_bool) : type(CONST_BOOL), const_bool(const_bool) {}
        RegisterOrConstant(float const_real) : type(CONST_REAL), const_real(const_real) {}

        std::string str() {
            std::stringstream ss;
            switch (type) {
                case REGISTER:
                    ss << "$" << register_number;
                    break;
                case CONST_INT:
                    ss << const_int;
                    break;
                case CONST_BOOL:
                    ss << (const_bool ? "true" : "false");
                    break;
                case CONST_REAL:
                    ss << const_real;
                    break;
            }
            return ss.str();
        }
    };


    struct CopyInstruction : public Instruction {
        int dest; // register number
        RegisterOrConstant source; // register number
        CopyInstruction(int dest, RegisterOrConstant source) : Instruction(COPY), dest(dest), source(source) {}
    };
    
    struct OperatorInstruction : public Instruction {
        enum Operator {
            EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
            PLUS, MINUS, OR,
            TIMES, DIVIDE, MOD, AND,
        };

        int dest;
        RegisterOrConstant left;
        Operator _operator;
        RegisterOrConstant right;

        OperatorInstruction(int dest, RegisterOrConstant left, Operator _operator, RegisterOrConstant right) :
            Instruction(OPERATOR), dest(dest), left(left), _operator(_operator), right(right) {}
    };

    struct UnaryInstruction : public Instruction {
        enum Operator {
            NOT,
            NEGATE,
        };

        int dest;
        Operator _operator;
        RegisterOrConstant source;

        UnaryInstruction(int dest, Operator _operator, RegisterOrConstant source) : Instruction(UNARY), dest(dest), _operator(_operator), source(source) {}
    };

    struct IfInstruction : public Instruction {
        RegisterOrConstant condition;
        int goto_index;

        IfInstruction(RegisterOrConstant condition, int goto_index) : Instruction(IF), condition(condition), goto_index(goto_index) {}
    };

    struct GotoInstruction : public Instruction {
        int goto_index;

        GotoInstruction(int goto_index) : Instruction(GOTO), goto_index(goto_index) {}
    };

    struct ReturnInstruction : public Instruction {
        ReturnInstruction() : Instruction(RETURN) {}
    };

    struct PrintInstruction : public Instruction {
        RegisterOrConstant value;
        PrintInstruction(RegisterOrConstant value) : Instruction(PRINT), value(value) {}
    };

    struct BasicBlock {
        // indexes in m_instructions
        int start;
        int end;
        // indexes in m_basic_blocks
        int jump_child;
        int fallthrough_child;
        std::set<int> parents;
        std::list<Instruction *> instructions;
        BasicBlock(int start, int end) : start(start), end(end) {}
    };

    typedef std::list<Instruction *> InstructionList;

    std::vector<Instruction *> m_instructions;
    OrderedInsensitiveMap<int> m_variable_numbers;
    int m_register_count;
    std::vector<BasicBlock *> m_basic_blocks;

private:
    int next_available_register() { return m_register_count++; }

    void gen_statement_list(StatementList * statement_list);
    void gen_statement(Statement * statement);
    RegisterOrConstant gen_expression(Expression * expression);
    RegisterOrConstant gen_additive_expression(AdditiveExpression * additive_expression);
    RegisterOrConstant gen_multiplicitive_expression(MultiplicativeExpression * multiplicative_expression);
    RegisterOrConstant gen_negatable_expression(NegatableExpression * negatable_expression);
    RegisterOrConstant gen_primary_expression(PrimaryExpression * primary_expression);
    RegisterOrConstant gen_variable_access(VariableAccess * variable);

    void gen_assignment(VariableAccess * variable, RegisterOrConstant source);
    void link_parent_and_child(int parent_index, int jump_child, int fallthrough_child);

    void print_disassembly(int address, Instruction * instruction);
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

void CodeGenerator::print_disassembly(int address, Instruction * instruction) {
    std::cout << address << ":\t";
    switch (instruction->type) {
        case Instruction::COPY:
        {
            CopyInstruction * copy_instruction = (CopyInstruction *) instruction;
            std::cout << "$" << copy_instruction->dest << " = " << copy_instruction->source.str();
            break;
        }
        case Instruction::OPERATOR:
        {
            OperatorInstruction * operator_instruction = (OperatorInstruction *) instruction;
            std::cout << "$" << operator_instruction->dest << " = " << operator_instruction->left.str() << " ";
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
            std::cout << " " << operator_instruction->right.str();
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
            std::cout << unary_instruction->source.str();
            break;
        }
        case Instruction::IF:
        {
            IfInstruction * if_instruction = (IfInstruction *) instruction;
            std::cout << "if !" << if_instruction->condition.str() << " goto " << if_instruction->goto_index;
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
            std::cout << "print " << print_instruction->value.str();
            break;
    }
    std::cout << ";" << std::endl;
}

void CodeGenerator::print_basic_blocks() {
    for (unsigned int b = 0; b < m_basic_blocks.size(); b++) {
        std::cout << "block_" << b << ":" << std::endl;
        BasicBlock * block = m_basic_blocks[b];
        int i = block->start;
        for (InstructionList::iterator it = block->instructions.begin(); it != block->instructions.end(); ++it, ++i)
            print_disassembly(i, *it);
    }
}

void CodeGenerator::print_control_flow_graph() {
    for (int parent = 0; parent < (int)m_basic_blocks.size(); parent++) {
        BasicBlock * parent_block = m_basic_blocks[parent];
        for (int child = 0; child < (int)m_basic_blocks.size(); child++) {
            if (parent == child) {
                // for self, print the number (ugly if more than 1 digit)
                std::cout << parent;
            } else if (parent_block->jump_child == child || parent_block->fallthrough_child == child) {
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
            RegisterOrConstant source = gen_expression(statement->assignment->expression);
            gen_assignment(statement->assignment->variable, source);
            break;
        }
        case Statement::IF:
        {
            RegisterOrConstant condition = gen_expression(statement->if_statement->expression);

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
            RegisterOrConstant value = gen_expression(statement->print_statement->expression);
            m_instructions.push_back(new PrintInstruction(value));
            break;
        }
        case Statement::WHILE:
        {
            int while_start = m_instructions.size();
            RegisterOrConstant condition = gen_expression(statement->while_statement->expression);
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


CodeGenerator::RegisterOrConstant CodeGenerator::gen_expression(Expression * expression) {
    if (expression->right == NULL) {
        // it's just the type of the first additive expression
        return gen_additive_expression(expression->left);
    } else {
        // we're looking at a compare operator
        RegisterOrConstant left = gen_additive_expression(expression->left);
        RegisterOrConstant right = gen_additive_expression(expression->right);
        int dest = next_available_register();
        OperatorInstruction::Operator _operator = (OperatorInstruction::Operator)(expression->_operator->type + OperatorInstruction::EQUAL); // LOL HAX!
        m_instructions.push_back(new OperatorInstruction(dest, left, _operator, right));
        return RegisterOrConstant(dest);
    }
}

CodeGenerator::RegisterOrConstant CodeGenerator::gen_additive_expression(AdditiveExpression * additive_expression) {
    RegisterOrConstant right = gen_multiplicitive_expression(additive_expression->right);

    if (additive_expression->left == NULL) {
        // pass the right through
        return right;
    } else {
        RegisterOrConstant left = gen_additive_expression(additive_expression->left);
        int dest = next_available_register();
        OperatorInstruction::Operator _operator = (OperatorInstruction::Operator)(additive_expression->_operator->type + OperatorInstruction::PLUS);
        m_instructions.push_back(new OperatorInstruction(dest, left, _operator, right));
        return RegisterOrConstant(dest);
    }
}

CodeGenerator::RegisterOrConstant CodeGenerator::gen_multiplicitive_expression(MultiplicativeExpression * multiplicative_expression) {
    RegisterOrConstant right = gen_negatable_expression(multiplicative_expression->right);

    if (multiplicative_expression->left == NULL) {
        // pass the right through
        return right;
    } else {
        RegisterOrConstant left = gen_multiplicitive_expression(multiplicative_expression->left);
        int dest = next_available_register();
        OperatorInstruction::Operator _operator = (OperatorInstruction::Operator)(multiplicative_expression->_operator->type + OperatorInstruction::TIMES);
        m_instructions.push_back(new OperatorInstruction(dest, left, _operator, right));
        return RegisterOrConstant(dest);
    }
}

CodeGenerator::RegisterOrConstant CodeGenerator::gen_negatable_expression(NegatableExpression * negatable_expression) {
    if (negatable_expression->type == NegatableExpression::PRIMARY) {
        return gen_primary_expression(negatable_expression->primary_expression);
    } else if (negatable_expression->type == NegatableExpression::SIGN) {
        RegisterOrConstant source = gen_negatable_expression(negatable_expression->next);
        int dest = next_available_register();
        m_instructions.push_back(new UnaryInstruction(dest, UnaryInstruction::NEGATE, source));
        return RegisterOrConstant(dest);
    } else {
        assert(false);
        return -1;
    }
}

CodeGenerator::RegisterOrConstant CodeGenerator::gen_primary_expression(PrimaryExpression * primary_expression) {
    switch (primary_expression->type) {
        case PrimaryExpression::VARIABLE:
            return gen_variable_access(primary_expression->variable);
        case PrimaryExpression::INTEGER:
        {
            int dest = next_available_register();
            int constant = primary_expression->literal_integer->value;
            m_instructions.push_back(new CopyInstruction(dest, RegisterOrConstant(constant, true)));
            return RegisterOrConstant(dest);
        }
        case PrimaryExpression::BOOLEAN:
        {
            int dest = next_available_register();
            bool constant = primary_expression->literal_boolean->value;
            m_instructions.push_back(new CopyInstruction(dest, RegisterOrConstant(constant)));
            return RegisterOrConstant(dest);
        }
        case PrimaryExpression::REAL:
        {
            int dest = next_available_register();
            float constant = primary_expression->literal_real->value;
            m_instructions.push_back(new CopyInstruction(dest, RegisterOrConstant(constant)));
            return RegisterOrConstant(dest);
        }
        case PrimaryExpression::PARENS:
            return gen_expression(primary_expression->parens_expression);
        case PrimaryExpression::NOT:
        {
            int dest = next_available_register();
            RegisterOrConstant source = gen_primary_expression(primary_expression->not_expression);
            m_instructions.push_back(new UnaryInstruction(dest, UnaryInstruction::NOT, source));
            return RegisterOrConstant(dest);
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

CodeGenerator::RegisterOrConstant CodeGenerator::gen_variable_access(VariableAccess * variable) {
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

void CodeGenerator::gen_assignment(VariableAccess * variable, RegisterOrConstant source) {
    switch (variable->type) {
        case VariableAccess::IDENTIFIER:
        {
            m_instructions.push_back(new CopyInstruction(m_variable_numbers.get(variable->identifier->text), source));
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

void CodeGenerator::link_parent_and_child(int parent_index, int jump_child, int fallthrough_child) {
    m_basic_blocks[parent_index]->jump_child = jump_child;
    m_basic_blocks[parent_index]->fallthrough_child = fallthrough_child;

    if (jump_child >= 0)
        m_basic_blocks[jump_child]->parents.insert(parent_index);
    if (fallthrough_child >= 0)
        m_basic_blocks[fallthrough_child]->parents.insert(parent_index);
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
        
        // build linked list of instructions
        for (int i = start_index; i < end_index; ++i)
            block->instructions.push_back(m_instructions[i]);

        start_index = end_index;
    }

    m_instructions.clear();

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
                int jump_child = instruction_index_to_block_index[if_instruction->goto_index];
                int fallthrough_child = i + 1;
                link_parent_and_child(i, jump_child, fallthrough_child);
                break;
            }
            case Instruction::GOTO:
            {
                // one distant child
                GotoInstruction * goto_instruction = (GotoInstruction *) instruction;
                int jump_child = instruction_index_to_block_index[goto_instruction->goto_index];
                link_parent_and_child(i, jump_child, -1);
                break;
            }
            case Instruction::RETURN:
                // no children :(
                link_parent_and_child(i, -1, -1);
                break;
            default:
            {
                // next block is only child
                int fallthrough_child = i + 1;
                link_parent_and_child(i, -1, fallthrough_child);
                break;
            }
        }
    }
}


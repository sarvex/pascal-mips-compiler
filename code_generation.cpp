#include "code_generation.h"
#include "insensitive_map.h"
#include "two_way_map.h"

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
    void value_numbering();

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

    class Variant {
    public:
        enum Type {
            REGISTER, // int
            VALUE_NUMBER, // int
            CONST_INT, // int
            CONST_BOOL, // bool
            CONST_REAL, // float
        };
        Type type;
        union {
            int _int;
            bool _bool;
            float _float;
        };

        // don't use this, stupid face
        Variant(){}
        Variant(int _int, Type type) : type(type), _int(_int) {}
        Variant(bool _bool) : type(CONST_BOOL), _bool(_bool) {}
        Variant(float _float) : type(CONST_REAL), _float(_float) {}

        std::string str() {
            std::stringstream ss;
            switch (type) {
                case REGISTER:
                    ss << "$" << _int;
                    break;
                case VALUE_NUMBER:
                    ss << _int;
                    break;
                case CONST_INT:
                    ss << _int;
                    break;
                case CONST_BOOL:
                    ss << (_bool ? "true" : "false");
                    break;
                case CONST_REAL:
                    ss << _float;
                    break;
            }
            return ss.str();
        }

        bool operator< (Variant right) const {
            if (type != right.type) {
                return type < right.type;
            } else {
                switch (type) {
                case REGISTER: // int
                case VALUE_NUMBER: // int
                case CONST_INT: // int
                    return _int < right._int;
                case CONST_BOOL: // bool
                    return _bool < right._bool;
                case CONST_REAL: // float
                    return _float < right._float;
                }
            }
            assert(false);
            return false;
        }
    };


    struct CopyInstruction : public Instruction {
        Variant dest; // register number
        Variant source; // register number
        CopyInstruction(Variant dest, Variant source) : Instruction(COPY), dest(dest), source(source) {}
    };

    struct OperatorInstruction : public Instruction {
        enum Operator {
            EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
            PLUS, MINUS, OR,
            TIMES, DIVIDE, MOD, AND,
        };

        Variant dest;
        Variant left;
        Operator _operator;
        Variant right;

        OperatorInstruction(Variant dest, Variant left, Operator _operator, Variant right) :
            Instruction(OPERATOR), dest(dest), left(left), _operator(_operator), right(right) {}
    };

    struct UnaryInstruction : public Instruction {
        enum Operator {
            NOT,
            NEGATE,
        };

        Variant dest;
        Operator _operator;
        Variant source;

        UnaryInstruction(Variant dest, Operator _operator, Variant source) : Instruction(UNARY), dest(dest), _operator(_operator), source(source) {}
    };

    struct IfInstruction : public Instruction {
        Variant condition;
        int goto_index;

        IfInstruction(Variant condition, int goto_index) : Instruction(IF), condition(condition), goto_index(goto_index) {}
    };

    struct GotoInstruction : public Instruction {
        int goto_index;

        GotoInstruction(int goto_index) : Instruction(GOTO), goto_index(goto_index) {}
    };

    struct ReturnInstruction : public Instruction {
        ReturnInstruction() : Instruction(RETURN) {}
    };

    struct PrintInstruction : public Instruction {
        Variant value;
        PrintInstruction(Variant value) : Instruction(PRINT), value(value) {}
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

        TwoWayMap<int, Variant> value_numbers;

        BasicBlock(int start, int end) : start(start), end(end) {}
    };

    typedef std::list<Instruction *> InstructionList;

    std::vector<Instruction *> m_instructions;
    OrderedInsensitiveMap<Variant> m_variable_numbers;
    int m_register_count;
    std::vector<BasicBlock *> m_basic_blocks;

private:
    Variant next_available_register() { return Variant(m_register_count++, Variant::REGISTER); }

    void gen_statement_list(StatementList * statement_list);
    void gen_statement(Statement * statement);
    Variant gen_expression(Expression * expression);
    Variant gen_additive_expression(AdditiveExpression * additive_expression);
    Variant gen_multiplicitive_expression(MultiplicativeExpression * multiplicative_expression);
    Variant gen_negatable_expression(NegatableExpression * negatable_expression);
    Variant gen_primary_expression(PrimaryExpression * primary_expression);
    Variant gen_variable_access(VariableAccess * variable);

    void gen_assignment(VariableAccess * variable, Variant source);
    void link_parent_and_child(int parent_index, int jump_child, int fallthrough_child);

    void print_disassembly(int address, Instruction * instruction);

    Variant get_value_number(BasicBlock * block, Variant register_or_const);
    Variant inline_value(BasicBlock * block, Variant register_or_const);
    Instruction * constant_folded(BasicBlock * block, OperatorInstruction * instruction);

    CopyInstruction * make_copy(BasicBlock * block, OperatorInstruction * operator_instruction);
    bool operands_same(OperatorInstruction * instruction);
    bool constant_is(OperatorInstruction * instruction, int constant);
    CopyInstruction * make_immediate(BasicBlock * block, OperatorInstruction * operator_instruction, int constant);
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

            std::cout << "3 Address Code" << std::endl;
            std::cout << "--------------------------" << std::endl;
            generator.print_basic_blocks();
            std::cout << "--------------------------" << std::endl;

            std::cout << "Control Flow Graph" << std::endl;
            std::cout << "--------------------------" << std::endl;
            generator.print_control_flow_graph();
            std::cout << "--------------------------" << std::endl;

            generator.value_numbering();

            std::cout << "3 Address Code After Value Numbering" << std::endl;
            std::cout << "--------------------------" << std::endl;
            generator.print_basic_blocks();
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
            std::cout << copy_instruction->dest.str() << " = " << copy_instruction->source.str();
            break;
        }
        case Instruction::OPERATOR:
        {
            OperatorInstruction * operator_instruction = (OperatorInstruction *) instruction;
            std::cout << operator_instruction->dest.str() << " = " << operator_instruction->left.str() << " ";
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
            std::cout << unary_instruction->dest.str() << " = ";
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
            Variant source = gen_expression(statement->assignment->expression);
            gen_assignment(statement->assignment->variable, source);
            break;
        }
        case Statement::IF:
        {
            Variant condition = gen_expression(statement->if_statement->expression);

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
            Variant value = gen_expression(statement->print_statement->expression);
            m_instructions.push_back(new PrintInstruction(value));
            break;
        }
        case Statement::WHILE:
        {
            int while_start = m_instructions.size();
            Variant condition = gen_expression(statement->while_statement->expression);
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


CodeGenerator::Variant CodeGenerator::gen_expression(Expression * expression) {
    if (expression->right == NULL) {
        // it's just the type of the first additive expression
        return gen_additive_expression(expression->left);
    } else {
        // we're looking at a compare operator
        Variant left = gen_additive_expression(expression->left);
        Variant right = gen_additive_expression(expression->right);
        Variant dest = next_available_register();
        OperatorInstruction::Operator _operator = (OperatorInstruction::Operator)(expression->_operator->type + OperatorInstruction::EQUAL); // LOL HAX!
        m_instructions.push_back(new OperatorInstruction(dest, left, _operator, right));
        return dest;
    }
}

CodeGenerator::Variant CodeGenerator::gen_additive_expression(AdditiveExpression * additive_expression) {
    Variant right = gen_multiplicitive_expression(additive_expression->right);

    if (additive_expression->left == NULL) {
        // pass the right through
        return right;
    } else {
        Variant left = gen_additive_expression(additive_expression->left);
        Variant dest = next_available_register();
        OperatorInstruction::Operator _operator = (OperatorInstruction::Operator)(additive_expression->_operator->type + OperatorInstruction::PLUS);
        m_instructions.push_back(new OperatorInstruction(dest, left, _operator, right));
        return dest;
    }
}

CodeGenerator::Variant CodeGenerator::gen_multiplicitive_expression(MultiplicativeExpression * multiplicative_expression) {
    Variant right = gen_negatable_expression(multiplicative_expression->right);

    if (multiplicative_expression->left == NULL) {
        // pass the right through
        return right;
    } else {
        Variant left = gen_multiplicitive_expression(multiplicative_expression->left);
        Variant dest = next_available_register();
        OperatorInstruction::Operator _operator = (OperatorInstruction::Operator)(multiplicative_expression->_operator->type + OperatorInstruction::TIMES);
        m_instructions.push_back(new OperatorInstruction(dest, left, _operator, right));
        return dest;
    }
}

CodeGenerator::Variant CodeGenerator::gen_negatable_expression(NegatableExpression * negatable_expression) {
    if (negatable_expression->type == NegatableExpression::PRIMARY) {
        return gen_primary_expression(negatable_expression->primary_expression);
    } else if (negatable_expression->type == NegatableExpression::SIGN) {
        Variant source = gen_negatable_expression(negatable_expression->next);
        Variant dest = next_available_register();
        m_instructions.push_back(new UnaryInstruction(dest, UnaryInstruction::NEGATE, source));
        return dest;
    } else {
        assert(false);
        return Variant(false);
    }
}

CodeGenerator::Variant CodeGenerator::gen_primary_expression(PrimaryExpression * primary_expression) {
    switch (primary_expression->type) {
        case PrimaryExpression::VARIABLE:
            return gen_variable_access(primary_expression->variable);
        case PrimaryExpression::INTEGER:
        {
            Variant dest = next_available_register();
            int constant = primary_expression->literal_integer->value;
            m_instructions.push_back(new CopyInstruction(dest, Variant(constant, Variant::CONST_INT)));
            return dest;
        }
        case PrimaryExpression::BOOLEAN:
        {
            Variant dest = next_available_register();
            bool constant = primary_expression->literal_boolean->value;
            m_instructions.push_back(new CopyInstruction(dest, Variant(constant)));
            return dest;
        }
        case PrimaryExpression::REAL:
        {
            Variant dest = next_available_register();
            float constant = primary_expression->literal_real->value;
            m_instructions.push_back(new CopyInstruction(dest, Variant(constant)));
            return dest;
        }
        case PrimaryExpression::PARENS:
            return gen_expression(primary_expression->parens_expression);
        case PrimaryExpression::NOT:
        {
            Variant dest = next_available_register();
            Variant source = gen_primary_expression(primary_expression->not_expression);
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

CodeGenerator::Variant CodeGenerator::gen_variable_access(VariableAccess * variable) {
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

void CodeGenerator::gen_assignment(VariableAccess * variable, Variant source) {
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

void CodeGenerator::value_numbering() {
    for (int i = 0; i < (int)m_basic_blocks.size(); i++) {
        BasicBlock * block = m_basic_blocks[i];
        for (int register_index = 0; register_index < m_register_count; ++register_index) {
            block->value_numbers.associate(register_index, Variant(register_index, Variant::VALUE_NUMBER));
        }

        for (InstructionList::iterator it = block->instructions.begin(); it != block->instructions.end(); ++it) {
            Instruction * instruction = *it;
            switch (instruction->type) {
                case Instruction::COPY:
                {
                    CopyInstruction * copy_instruction = (CopyInstruction *) instruction;
                    copy_instruction->source = inline_value(block, copy_instruction->source);
                    block->value_numbers.associate(copy_instruction->dest._int, get_value_number(block, copy_instruction->source));
                    break;
                }
                case Instruction::OPERATOR:
                {
                    OperatorInstruction * operator_instruction = (OperatorInstruction *) instruction;
                    operator_instruction->left = inline_value(block, operator_instruction->left);
                    operator_instruction->right = inline_value(block, operator_instruction->right);

                    // normalize parameter order
                    // constant on the right and lower register first
                    bool swap = false;
                    if (operator_instruction->left.type != Variant::REGISTER) {
                        // left is constant, swap
                        swap = true;
                    } else if (operator_instruction->right.type == Variant::REGISTER) {
                        // right is not constant. order by lower register first
                        swap = operator_instruction->left._int > operator_instruction->right._int;
                    }
                    if (swap) {
                        Variant tmp = operator_instruction->left;
                        operator_instruction->left = operator_instruction->right;
                        operator_instruction->right = tmp;
                    }

                    *it = constant_folded(block, operator_instruction);

                    break;
                }
                case Instruction::UNARY:
                    break;
                case Instruction::IF:
                    break;
                case Instruction::GOTO:
                    break;
                case Instruction::RETURN:
                    break;
                case Instruction::PRINT:
                    break;
            }
        }
    }
}

CodeGenerator::CopyInstruction * CodeGenerator::make_copy(BasicBlock * block, OperatorInstruction * operator_instruction) {
    CopyInstruction * copy_instruction = new CopyInstruction(operator_instruction->dest, operator_instruction->left);
    delete operator_instruction;
    block->value_numbers.associate(copy_instruction->dest._int, get_value_number(block, copy_instruction->source));
    return copy_instruction;
}

bool CodeGenerator::operands_same(OperatorInstruction * instruction) {
    return (instruction->left.type == Variant::REGISTER && instruction->right.type == Variant::REGISTER &&
            instruction->left._int == instruction->right._int);
}

bool CodeGenerator::constant_is(OperatorInstruction * instruction, int constant) {
    return (instruction->right.type == Variant::CONST_INT && instruction->right._int == constant) ||
            (instruction->right.type == Variant::CONST_REAL && instruction->right._float == (float)constant);
}

CodeGenerator::CopyInstruction * CodeGenerator::make_immediate(BasicBlock * block, OperatorInstruction * operator_instruction, int constant) {
    // TODO: need to know what type the register is for this constant to have meaning
    CopyInstruction * copy_instruction = new CopyInstruction(operator_instruction->dest, Variant(constant, Variant::CONST_INT));
    delete operator_instruction;
    block->value_numbers.associate(copy_instruction->dest._int, get_value_number(block, copy_instruction->source));
    return copy_instruction;
}

CodeGenerator::Instruction * CodeGenerator::constant_folded(BasicBlock * block, OperatorInstruction * instruction) {
    // we know that constants are on the right
    switch (instruction->_operator) {
        case OperatorInstruction::PLUS:
        {
            // a + 0 = a
            if (constant_is(instruction, 0))
                return make_copy(block, instruction);
            break;
        }
        case OperatorInstruction::MINUS:
        {
            // b := a - 0;
            // b := a - a;
            if (constant_is(instruction, 0))
                return make_copy(block, instruction);
            else if (operands_same(instruction))
                return make_immediate(block, instruction, 0);
            break;
        }
        case OperatorInstruction::TIMES:
        {
            // a * 1 = a
            // b := a * 0;
            // b := a * 2;
            if (constant_is(instruction, 1))
                return make_copy(block, instruction);
            else if (constant_is(instruction, 0))
                return make_immediate(block, instruction, 0);
            else if (constant_is(instruction, 2)) {
                instruction->_operator = OperatorInstruction::PLUS;
                instruction->right = instruction->left;
                return instruction;
            }
            break;
        }
        case OperatorInstruction::DIVIDE:
        {
            // b := a / 1;
            // b := a / a;
            if (constant_is(instruction, 1))
                return make_copy(block, instruction);
            else if (operands_same(instruction))
                return make_immediate(block, instruction, 1);
            break;
        }
        case OperatorInstruction::AND:
        case OperatorInstruction::OR:
        {
            // d := c and c;
            // d := c or c;
            if (operands_same(instruction))
                return make_copy(block, instruction);
            break;
        }
        default:
            break;
    }
    return instruction;
}

CodeGenerator::Variant CodeGenerator::inline_value(BasicBlock * block, Variant register_or_const) {
    if (register_or_const.type == Variant::REGISTER) {
        Variant value = block->value_numbers.get(register_or_const._int);
        if (value.type == Variant::VALUE_NUMBER) {
            std::set<int> * registers = block->value_numbers.keys(value);
            int lowest = *(registers->begin());
            return Variant(lowest, Variant::REGISTER);
        } else {
            return value;
        }
    } else {
        return register_or_const;
    }
}

CodeGenerator::Variant CodeGenerator::get_value_number(BasicBlock * block, Variant register_or_const) {
    if (register_or_const.type == Variant::REGISTER)
        return block->value_numbers.get(register_or_const._int);
    else
        return register_or_const;
}

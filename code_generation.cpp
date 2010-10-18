#include "code_generation.h"
#include "insensitive_map.h"
#include "two_way_map.h"
#include "utils.h"

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
    void calculate_mangle_sets();
    void dependency_management();
    void compute_addresses();

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
            VALUE_NUMBER, // string
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
        std::string _string;

        // don't use this, stupid face
        Variant(){}
        Variant(int _int, Type type) : type(type), _int(_int) {}
        Variant(bool _bool) : type(CONST_BOOL), _bool(_bool) {}
        Variant(float _float) : type(CONST_REAL), _float(_float) {}
        Variant(std::string _string) : type(VALUE_NUMBER), _string(_string) {}

        std::string str() {
            std::stringstream ss;
            switch (type) {
                case REGISTER:
                    ss << "$" << _int;
                    break;
                case VALUE_NUMBER:
                    ss << _string;
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
                case CONST_INT: // int
                    return _int < right._int;
                case CONST_BOOL: // bool
                    return _bool < right._bool;
                case CONST_REAL: // float
                    return _float < right._float;
                case VALUE_NUMBER: // string
                    return _string < right._string;
                }
            }
            assert(false);
            return false;
        }

        bool operator== (Variant right) const {
            if (type != right.type) {
                return false;
            } else {
                switch (type) {
                case REGISTER: // int
                case CONST_INT: // int
                    return _int == right._int;
                case CONST_BOOL: // bool
                    return _bool == right._bool;
                case CONST_REAL: // float
                    return _float == right._float;
                case VALUE_NUMBER: // string
                    return _string == right._string;
                }
            }
            assert(false);
            return false;
        }

        bool operator != (Variant right) const {
            return !this->operator ==(right);
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

        std::string str() {
            std::stringstream ss;
            ss << dest.str() << " = " << left.str() << " " << operator_str() << " " << right.str();
            return ss.str();
        }

        std::string operator_str() {
            switch (_operator) {
                case EQUAL:
                    return "==";
                case NOT_EQUAL:
                    return "!=";
                case LESS:
                    return "<";
                case GREATER:
                    return ">";
                case LESS_EQUAL:
                    return "<=";
                case GREATER_EQUAL:
                    return ">=";
                case PLUS:
                    return "+";
                case MINUS:
                    return "-";
                case OR:
                    return "||";
                case TIMES:
                    return "*";
                case DIVIDE:
                    return "/";
                case MOD:
                    return "%";
                case AND:
                    return "&&";
                default:
                    assert(false);
            }
            return std::string();
        }
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
        std::set<int> mangled_registers;
        std::set<int> used_registers;

        // for detecting loops during mangle set calculations
        bool is_destination;
        bool is_source;

        // maps registers to value number/ constant
        TwoWayMap<int, Variant> value_numbers;

        BasicBlock(int start, int end) : start(start), end(end), is_destination(false), is_source(false) {}
    };

    typedef std::list<Instruction *> InstructionList;

    std::vector<Instruction *> m_instructions;
    OrderedInsensitiveMap<Variant> m_variable_numbers;
    int m_register_count;
    int m_unique_value_count;
    std::vector<BasicBlock *> m_basic_blocks;

    enum RegisterType {
        INTEGER,
        REAL,
        BOOL,
    };

    std::vector<RegisterType> m_register_type;

private:
    Variant next_available_register(RegisterType type);

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
    CopyInstruction * make_immediate(BasicBlock * block, UnaryInstruction * operator_instruction, int constant);
    CopyInstruction * constant_expression_evaluated(BasicBlock * block, OperatorInstruction * operator_instruction);
    CopyInstruction * constant_expression_evaluated(BasicBlock * block, UnaryInstruction * operator_instruction);
    std::string next_unique_value();
    std::string hash_operator_instruction(BasicBlock * block, OperatorInstruction * instruction);
    void basic_block_value_numbering(BasicBlock * block);
    std::string hash_operand(BasicBlock * block, Variant operand);
    Variant typed_constant(Variant _register, int constant);
    RegisterType type_denoter_to_register_type(TypeDenoter * type);
    void calculate_mangle_set(int block_index);
    void record_mangled_registers(BasicBlock * block, BasicBlock * block_that_gets_the_resutls);
    void record_used_registers(BasicBlock * block, BasicBlock * block_that_gets_the_results);
    void calculate_downward_mangle_set(int block_index);
    void calculate_upward_mangle_set(int block_index);
    void add_if_register(std::set<int> & set, Variant possibly_a_register);
};

CodeGenerator::Variant CodeGenerator::next_available_register(RegisterType type) {
    m_register_type.push_back(type);
    return Variant(m_register_count++, Variant::REGISTER);
}

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

            generator.calculate_mangle_sets();
            generator.value_numbering();

            std::cout << "3 Address Code After Value Numbering" << std::endl;
            std::cout << "--------------------------" << std::endl;
            generator.print_basic_blocks();
            std::cout << "--------------------------" << std::endl;

            generator.dependency_management();
            generator.compute_addresses();

            std::cout << "3 Address Code After Dependency Management" << std::endl;
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
            std::cout << operator_instruction->str();
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
        {
            PrintInstruction * print_instruction = (PrintInstruction *) instruction;
            std::cout << "print " << print_instruction->value.str();
            break;
        }
        default:
            assert(false);
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

CodeGenerator::RegisterType CodeGenerator::type_denoter_to_register_type(TypeDenoter * type) {
    switch (type->type) {
        case TypeDenoter::BOOLEAN:
            return BOOL;
        case TypeDenoter::INTEGER:
            return INTEGER;
        case TypeDenoter::REAL:
            return REAL;
        default:
            assert(false);
            return BOOL; // BOOL is COOL
    }
}

void CodeGenerator::generate(FunctionDeclaration * function_declaration) {
    for (VariableDeclarationList * variable_list = function_declaration->block->variable_list; variable_list != NULL; variable_list = variable_list->next) {
        for (IdentifierList * id_list = variable_list->item->id_list; id_list != NULL; id_list = id_list->next)
            m_variable_numbers.put(id_list->item->text, next_available_register(type_denoter_to_register_type(variable_list->item->type)));
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
        Variant dest = next_available_register(type_denoter_to_register_type(expression->type));
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
        Variant dest = next_available_register(type_denoter_to_register_type(additive_expression->type));
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
        Variant dest = next_available_register(type_denoter_to_register_type(multiplicative_expression->type));
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
        Variant dest = next_available_register(type_denoter_to_register_type(negatable_expression->variable_type));
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
            Variant dest = next_available_register(INTEGER);
            int constant = primary_expression->literal_integer->value;
            m_instructions.push_back(new CopyInstruction(dest, Variant(constant, Variant::CONST_INT)));
            return dest;
        }
        case PrimaryExpression::BOOLEAN:
        {
            Variant dest = next_available_register(BOOL);
            bool constant = primary_expression->literal_boolean->value;
            m_instructions.push_back(new CopyInstruction(dest, Variant(constant)));
            return dest;
        }
        case PrimaryExpression::REAL:
        {
            Variant dest = next_available_register(REAL);
            float constant = primary_expression->literal_real->value;
            m_instructions.push_back(new CopyInstruction(dest, Variant(constant)));
            return dest;
        }
        case PrimaryExpression::PARENS:
            return gen_expression(primary_expression->parens_expression);
        case PrimaryExpression::NOT:
        {
            Variant dest = next_available_register(BOOL);
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

void CodeGenerator::basic_block_value_numbering(BasicBlock * block) {
    // initialize our value numbers to something that has to do with parent value numbers.
    // for each register
    for (int register_index = 0; register_index < m_register_count; ++register_index) {
        // if any of our parents disagree about the value number, or if any of the parents mangle our value number,
        // then we have to use a unique one. As long as at least one parent provides a value number, and nobody mangles it,
        // then we can use that.
        std::set<int>::iterator it = block->parents.begin();
        // consult all of the parents with value numbers (normal parents).
        // find the first parent with value numbers and start with that parent's opinion of what the value number is.
        Variant value;
        bool has_a_value_at_all = false;
        for (; it != block->parents.end(); ++it) {
            BasicBlock * parent = m_basic_blocks[*it];
            if (parent->value_numbers.is_empty())
                continue; // no value numbers
            value = parent->value_numbers.get(register_index);
            block->value_numbers.associate(register_index, value);
            has_a_value_at_all = true;
            it++;
            break;
        }
        // then check the value with all the rest of the parents who have value numbers
        for (; it != block->parents.end(); ++it) {
            BasicBlock * parent = m_basic_blocks[*it];
            if (parent->value_numbers.is_empty())
                continue; // no value numbers
            Variant other_value = parent->value_numbers.get(register_index);
            if (other_value != value) {
                // at least two parents disagree about this register. gotta say it's unique.
                block->value_numbers.associate(register_index, next_unique_value());
                // this register can't get any worse. move on to the next one.
                goto continue_register_loop;
            }
        }
        // finally check with any parents who want to mangle our values.
        it = block->parents.begin();
        for (; it != block->parents.end(); ++it) {
            BasicBlock * parent = m_basic_blocks[*it];
            if (parent->mangled_registers.count(register_index)) {
                block->value_numbers.associate(register_index, next_unique_value());
                has_a_value_at_all = true;
                // one bit of bad news is all it takes. we have nothing left to lose.
                goto continue_register_loop;
            }
        }
        // if none of our parents will help us out (like if we have no parents), then we'll assume our value is garbage.
        if (!has_a_value_at_all)
            block->value_numbers.associate(register_index, next_unique_value());

        // we wouldn't have to use goto if C++ had labeld loops like Java.
        continue_register_loop:;
    }

    // now do our own value numbers.
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

                CopyInstruction * copy_instruction = constant_expression_evaluated(block, operator_instruction);
                if (copy_instruction != NULL) {
                    *it = copy_instruction;
                    break;
                }

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

                instruction = constant_folded(block, operator_instruction);
                *it = instruction;

                if (instruction->type != Instruction::OPERATOR)
                    break;

                operator_instruction = (OperatorInstruction *) instruction;

                // replace operator instruction with a copy instruction if we can
                std::string hash = hash_operator_instruction(block, operator_instruction);
                std::set<int> * registers = block->value_numbers.keys(hash);
                if (registers != NULL && registers->size() > 0) {
                    int lowest = *(registers->begin());
                    CopyInstruction * copy_instruction = new CopyInstruction(operator_instruction->dest, Variant(lowest, Variant::REGISTER));
                    instruction = copy_instruction;
                    *it = copy_instruction;
                    delete operator_instruction;
                    block->value_numbers.associate(copy_instruction->dest._int, get_value_number(block, copy_instruction->source));
                } else {
                    block->value_numbers.associate(operator_instruction->dest._int, Variant(hash));
                }
                break;
            }
            case Instruction::UNARY:
            {
                UnaryInstruction * unary_instruction = (UnaryInstruction *) instruction;
                unary_instruction->source = inline_value(block, unary_instruction->source);

                CopyInstruction * copy_instruction = constant_expression_evaluated(block, unary_instruction);
                if (copy_instruction != NULL) {
                    *it = copy_instruction;
                    break;
                }

                break;
            }
            case Instruction::IF:
            {
                IfInstruction * if_instruction = (IfInstruction *) instruction;
                if_instruction->condition = inline_value(block, if_instruction->condition);
                break;
            }
            case Instruction::GOTO:
                break;
            case Instruction::RETURN:
                break;
            case Instruction::PRINT:
            {
                PrintInstruction * print_instruction = (PrintInstruction *) instruction;
                print_instruction->value = inline_value(block, print_instruction->value);
                break;
            }
        }
    }
}

void CodeGenerator::value_numbering() {

    for (int i = 0; i < (int)m_basic_blocks.size(); i++) {
        BasicBlock * block = m_basic_blocks[i];
        basic_block_value_numbering(block);
    }
}

void CodeGenerator::calculate_mangle_sets() {
    for (int i = 0; i < (int)m_basic_blocks.size(); i++)
        calculate_mangle_set(i);
}

void CodeGenerator::calculate_mangle_set(int block_index) {
    BasicBlock * block = m_basic_blocks[block_index];
    for (std::set<int>::iterator it = block->parents.begin(); it != block->parents.end(); it++) {
        int parent_index = *it;
        // only consider blocks that have parents later in the program.
        // these blocks are usually the beginings of loops
        if (parent_index > block_index) {
            // search for every path from this block to the later parent (which is the entire
            // scope of the loop) and record which registers are mangled along the way.
            calculate_downward_mangle_set(block_index);
            calculate_upward_mangle_set(parent_index);
            BasicBlock * end_node = m_basic_blocks[parent_index];
            // now add in all the mangles from the between nodes.
            for (int i = 0; i < (int)m_basic_blocks.size(); i++) {
                BasicBlock * node = m_basic_blocks[i];
                if (node->is_destination && node->is_source) {
                    // this block is on the path to victory.
                    // record the mangled registers and store them in the later parent.
                    record_mangled_registers(node, end_node);
                }
                // reset state for next go around.
                node->is_destination = false;
                node->is_source = false;
            }
        }
    }
}

void CodeGenerator::calculate_downward_mangle_set(int block_index) {
    BasicBlock * block = m_basic_blocks[block_index];
    // prevent infinite recursion
    if (block->is_destination)
        return;
    block->is_destination = true;

    if (block->jump_child != -1)
        calculate_downward_mangle_set(block->jump_child);
    if (block->fallthrough_child != -1)
        calculate_downward_mangle_set(block->fallthrough_child);
}

void CodeGenerator::calculate_upward_mangle_set(int block_index) {
    BasicBlock * block = m_basic_blocks[block_index];
    // prevent infinite recursion
    if (block->is_source)
        return;
    block->is_source = true;

    for (std::set<int>::iterator it = block->parents.begin(); it != block->parents.end(); ++it) {
        int parent_index = *it;
        calculate_upward_mangle_set(parent_index);
    }
}

void CodeGenerator::record_mangled_registers(BasicBlock * block, BasicBlock * block_that_gets_the_results) {
    std::set<int> * mangled_registers = & block_that_gets_the_results->mangled_registers;
    for (InstructionList::iterator it = block->instructions.begin(); it != block->instructions.end(); ++it) {
        Instruction * instruction = *it;
        switch (instruction->type) {
        case Instruction::COPY:
            mangled_registers->insert(((CopyInstruction *)instruction)->dest._int);
            break;
        case Instruction::OPERATOR:
            mangled_registers->insert(((OperatorInstruction *)instruction)->dest._int);
            break;
        case Instruction::UNARY:
            mangled_registers->insert(((UnaryInstruction *)instruction)->dest._int);
            break;
        case Instruction::IF:
        case Instruction::GOTO:
        case Instruction::RETURN:
        case Instruction::PRINT:
            break;
        }
    }
}

void CodeGenerator::add_if_register(std::set<int> & set, Variant possibly_a_register) {
    if (possibly_a_register.type == Variant::REGISTER)
        set.insert(possibly_a_register._int);
}

void CodeGenerator::record_used_registers(BasicBlock * block, BasicBlock * block_that_gets_the_results) {
    std::set<int> & used_registers = block_that_gets_the_results->used_registers;
    for (InstructionList::iterator it = block->instructions.begin(); it != block->instructions.end(); ++it) {
        Instruction * instruction = *it;
        switch (instruction->type) {
        case Instruction::COPY:
            add_if_register(used_registers, ((CopyInstruction *)instruction)->source);
            break;
        case Instruction::OPERATOR:
            add_if_register(used_registers, ((OperatorInstruction *)instruction)->left);
            add_if_register(used_registers, ((OperatorInstruction *)instruction)->right);
            break;
        case Instruction::UNARY:
            add_if_register(used_registers, ((UnaryInstruction *)instruction)->source);
            break;
        case Instruction::PRINT:
            add_if_register(used_registers, ((PrintInstruction *)instruction)->value);
            break;
        case Instruction::IF:
            add_if_register(used_registers, ((IfInstruction *)instruction)->condition);
            break;
        case Instruction::GOTO:
        case Instruction::RETURN:
            break;
        }
    }
}

void CodeGenerator::compute_addresses() {
    int address = 0;
    for (int i = 0; i < (int)m_basic_blocks.size(); ++i) {
        BasicBlock * block = m_basic_blocks[i];
        block->start = address;
        address += block->instructions.size();
        block->end = address;
    }
    for (int i = 0; i < (int)m_basic_blocks.size(); ++i) {
        BasicBlock * block = m_basic_blocks[i];
        for (InstructionList::iterator it = block->instructions.begin(); it != block->instructions.end(); ++it) {
            Instruction * instruction = *it;
            if (instruction->type == Instruction::IF) {
                IfInstruction * if_instruction = (IfInstruction *) instruction;
                if_instruction->goto_index = m_basic_blocks[block->jump_child]->start;
            } else if (instruction->type == Instruction::GOTO) {
                GotoInstruction * goto_instruction = (GotoInstruction *) instruction;
                goto_instruction->goto_index = m_basic_blocks[block->jump_child]->start;
            }
        }
    }
}

void CodeGenerator::dependency_management() {
    for (int block_index = 0; block_index < (int)m_basic_blocks.size(); ++block_index) {
        BasicBlock * block = m_basic_blocks[block_index];
        for (std::set<int>::iterator it = block->parents.begin(); it != block->parents.end(); it++) {
            int parent_index = *it;
            // only consider blocks that have parents later in the program.
            // these blocks are usually the beginings of loops
            if (parent_index > block_index) {
                // search for every path from this block to the later parent (which is the entire
                // scope of the loop) and record which registers are mangled along the way.
                calculate_downward_mangle_set(block_index);
                calculate_upward_mangle_set(parent_index);
                BasicBlock * end_node = m_basic_blocks[parent_index];
                // now add in all the mangles from the between nodes.
                for (int i = 0; i < (int)m_basic_blocks.size(); i++) {
                    BasicBlock * node = m_basic_blocks[i];
                    if (node->is_destination && node->is_source) {
                        // this block is on the path to victory.
                        // record the mangled registers and store them in the later parent.
                        record_used_registers(node, end_node);
                    }
                    // reset state for next go around.
                    node->is_destination = false;
                    node->is_source = false;
                }
            }
        }

    }

    for (int i = m_basic_blocks.size() - 1; i >= 0; --i) {
        BasicBlock * block = m_basic_blocks[i];

        if (block->jump_child != -1) {
            BasicBlock * child = m_basic_blocks[block->jump_child];
            block->used_registers.insert(child->used_registers.begin(), child->used_registers.end());
        }
        if (block->fallthrough_child != -1) {
            BasicBlock * child = m_basic_blocks[block->fallthrough_child];
            block->used_registers.insert(child->used_registers.begin(), child->used_registers.end());
        }

        InstructionList::iterator it = block->instructions.end();
        while (it != block->instructions.begin()) {
            --it;
            Instruction * instruction = *it;
            switch (instruction->type) {
                case Instruction::COPY:
                {
                    CopyInstruction * copy_instruction = (CopyInstruction *) instruction;

                    int dest_register = copy_instruction->dest._int;

                    // see if this is unecessary
                    if (! block->used_registers.count(dest_register)) {
                        bool beg = (block->instructions.begin() == it);
                        block->instructions.erase(it);
                        if (beg)
                            it = block->instructions.begin();

                        delete instruction;
                        instruction = NULL;
                    } else {
                        // add the source to used set
                        add_if_register(block->used_registers, copy_instruction->source);
                    }

                    // delete dest from used set
                    block->used_registers.erase(dest_register);

                    break;
                }
                case Instruction::OPERATOR:
                {
                    OperatorInstruction * operator_instruction = (OperatorInstruction *) instruction;

                    int dest_register = operator_instruction->dest._int;

                    // see if this is unecessary
                    if (! block->used_registers.count(dest_register)) {
                        bool beg = (block->instructions.begin() == it);
                        block->instructions.erase(it);
                        if (beg)
                            it = block->instructions.begin();

                        delete instruction;
                        instruction = NULL;
                    } else {
                        // add the operands to used set
                        add_if_register(block->used_registers, operator_instruction->left);
                        add_if_register(block->used_registers, operator_instruction->right);
                    }

                    // delete dest from used set
                    block->used_registers.erase(dest_register);


                    break;
                }
                case Instruction::UNARY:
                {
                    UnaryInstruction * unary_instruction = (UnaryInstruction *) instruction;


                    int dest_register = unary_instruction->dest._int;

                    // see if this is unecessary
                    if (! block->used_registers.count(dest_register)) {
                        bool beg = (block->instructions.begin() == it);
                        block->instructions.erase(it);
                        if (beg)
                            it = block->instructions.begin();

                        delete instruction;
                        instruction = NULL;
                    } else {
                        // add the source to used set
                        add_if_register(block->used_registers, unary_instruction->source);
                    }

                    // delete dest from used set
                    block->used_registers.erase(dest_register);


                    break;
                }
                case Instruction::PRINT:
                {
                    PrintInstruction * print_instruction = (PrintInstruction *) instruction;
                    add_if_register(block->used_registers, print_instruction->value);
                    break;
                }
                case Instruction::IF:
                {
                    IfInstruction * if_instruction = (IfInstruction *) instruction;
                    add_if_register(block->used_registers, if_instruction->condition);
                    break;
                }
                case Instruction::GOTO:
                case Instruction::RETURN:
                    break;
            }
        }
    }

}

CodeGenerator::CopyInstruction * CodeGenerator::constant_expression_evaluated(BasicBlock * block, UnaryInstruction * instruction) {
    if (instruction->source.type == Variant::REGISTER)
        return NULL;
    switch (instruction->_operator) {
        case UnaryInstruction::NEGATE:
            switch (instruction->source.type) {
                case Variant::CONST_INT:
                    return make_immediate(block, instruction, -instruction->source._int);
                case Variant::CONST_REAL:
                    return make_immediate(block, instruction, -instruction->source._float);
                default:
                    assert(false);
            }
            break;
        case UnaryInstruction::NOT:
            switch (instruction->source.type) {
                case Variant::CONST_BOOL:
                    return make_immediate(block, instruction, !instruction->source._bool);
                default:
                    assert(false);
            }
            break;
    }
    return NULL;
}


CodeGenerator::CopyInstruction * CodeGenerator::constant_expression_evaluated(BasicBlock * block, OperatorInstruction * instruction) {
    if (instruction->left.type == Variant::REGISTER || instruction->right.type == Variant::REGISTER)
        return NULL;

    switch (instruction->_operator) {
        case OperatorInstruction::EQUAL:
            switch (instruction->left.type) {
                case Variant::CONST_BOOL:
                    return make_immediate(block, instruction, instruction->left._bool == instruction->right._bool);
                case Variant::CONST_INT:
                    return make_immediate(block, instruction, instruction->left._int == instruction->right._int);
                case Variant::CONST_REAL:
                    return make_immediate(block, instruction, instruction->left._float == instruction->right._float);
                default:
                    assert(false);
            }
            break;
        case OperatorInstruction::NOT_EQUAL:
            switch (instruction->left.type) {
                case Variant::CONST_BOOL:
                    return make_immediate(block, instruction, instruction->left._bool != instruction->right._bool);
                case Variant::CONST_INT:
                    return make_immediate(block, instruction, instruction->left._int != instruction->right._int);
                case Variant::CONST_REAL:
                    return make_immediate(block, instruction, instruction->left._float != instruction->right._float);
                default:
                    assert(false);
            }
            break;
        case OperatorInstruction::LESS:
            switch (instruction->left.type) {
                case Variant::CONST_INT:
                    return make_immediate(block, instruction, instruction->left._int < instruction->right._int);
                case Variant::CONST_REAL:
                    return make_immediate(block, instruction, instruction->left._float < instruction->right._float);
                default:
                    assert(false);
            }
            break;
        case OperatorInstruction::GREATER:
            switch (instruction->left.type) {
                case Variant::CONST_INT:
                    return make_immediate(block, instruction, instruction->left._int > instruction->right._int);
                case Variant::CONST_REAL:
                    return make_immediate(block, instruction, instruction->left._float > instruction->right._float);
                default:
                    assert(false);
            }
            break;
        case OperatorInstruction::LESS_EQUAL:
            switch (instruction->left.type) {
                case Variant::CONST_INT:
                    return make_immediate(block, instruction, instruction->left._int <= instruction->right._int);
                case Variant::CONST_REAL:
                    return make_immediate(block, instruction, instruction->left._float <= instruction->right._float);
                default:
                    assert(false);
            }
            break;
        case OperatorInstruction::GREATER_EQUAL:
            switch (instruction->left.type) {
                case Variant::CONST_INT:
                    return make_immediate(block, instruction, instruction->left._int >= instruction->right._int);
                case Variant::CONST_REAL:
                    return make_immediate(block, instruction, instruction->left._float >= instruction->right._float);
                default:
                    assert(false);
            }
            break;
        case OperatorInstruction::PLUS:
            switch (instruction->left.type) {
                case Variant::CONST_INT:
                    return make_immediate(block, instruction, instruction->left._int + instruction->right._int);
                case Variant::CONST_REAL:
                    return make_immediate(block, instruction, instruction->left._float + instruction->right._float);
                default:
                    assert(false);
            }
            break;
        case OperatorInstruction::MINUS:
            switch (instruction->left.type) {
                case Variant::CONST_INT:
                    return make_immediate(block, instruction, instruction->left._int - instruction->right._int);
                case Variant::CONST_REAL:
                    return make_immediate(block, instruction, instruction->left._float - instruction->right._float);
                default:
                    assert(false);
            }
            break;
        case OperatorInstruction::OR:
            switch (instruction->left.type) {
                case Variant::CONST_BOOL:
                    return make_immediate(block, instruction, instruction->left._bool || instruction->right._bool);
                default:
                    assert(false);
            }
            break;
        case OperatorInstruction::TIMES:
            switch (instruction->left.type) {
                case Variant::CONST_INT:
                    return make_immediate(block, instruction, instruction->left._int * instruction->right._int);
                case Variant::CONST_REAL:
                    return make_immediate(block, instruction, instruction->left._float * instruction->right._float);
                default:
                    assert(false);
            }
            break;
        case OperatorInstruction::DIVIDE:
            switch (instruction->left.type) {
                case Variant::CONST_INT:
                    return make_immediate(block, instruction, instruction->left._int / instruction->right._int);
                case Variant::CONST_REAL:
                    return make_immediate(block, instruction, instruction->left._float / instruction->right._float);
                default:
                    assert(false);
            }
            break;
        case OperatorInstruction::MOD:
            switch (instruction->left.type) {
                case Variant::CONST_INT:
                    return make_immediate(block, instruction, instruction->left._int % instruction->right._int);
                default:
                    assert(false);
            }
            break;
        case OperatorInstruction::AND:
            switch (instruction->left.type) {
                case Variant::CONST_BOOL:
                    return make_immediate(block, instruction, instruction->left._bool && instruction->right._bool);
                default:
                    assert(false);
            }
            break;
    }
    return NULL;
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
           (instruction->right.type == Variant::CONST_REAL && instruction->right._float == (float)constant) ||
           (instruction->right.type == Variant::CONST_BOOL && instruction->right._bool == (bool)constant);
}

CodeGenerator::Variant CodeGenerator::typed_constant(Variant _register, int constant) {
    switch (m_register_type[_register._int]) {
        case REAL:
            return Variant((float) constant);
        case INTEGER:
            return Variant((int) constant, Variant::CONST_INT);
        case BOOL:
            return Variant((bool) constant);
        default:
            assert(false);
            return Variant();
    }
}

CodeGenerator::CopyInstruction * CodeGenerator::make_immediate(BasicBlock *block, UnaryInstruction *unary_instruction, int constant) {
    CopyInstruction * copy_instruction = new CopyInstruction(unary_instruction->dest, typed_constant(unary_instruction->dest, constant));
    delete unary_instruction;
    block->value_numbers.associate(copy_instruction->dest._int, get_value_number(block, copy_instruction->source));
    return copy_instruction;
}

CodeGenerator::CopyInstruction * CodeGenerator::make_immediate(BasicBlock * block, OperatorInstruction * operator_instruction, int constant) {
    CopyInstruction * copy_instruction = new CopyInstruction(operator_instruction->dest, typed_constant(operator_instruction->dest, constant));
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
        {
            // d := c and c;
            // a and false -> false
            if (operands_same(instruction))
                return make_copy(block, instruction);
            else if (constant_is(instruction, false))
                return make_immediate(block, instruction, false);
            break;
        }
        case OperatorInstruction::OR:
        {
            // d := c or c;
            // a or true -> true
            if (operands_same(instruction))
                return make_copy(block, instruction);
            else if (constant_is(instruction, true))
                return make_immediate(block, instruction, true);
            break;
        }
        default:
            break;
    }
    return instruction;
}

std::string CodeGenerator::hash_operand(BasicBlock * block, Variant operand) {
    if (operand.type == Variant::REGISTER) {
        Variant value = block->value_numbers.get(operand._int);
        if (value.type == Variant::VALUE_NUMBER) {
            std::stringstream ss;
            ss << "(" << value.str() << ")";
            return ss.str();
        } else {
            return value.str();
        }
    } else {
        return operand.str();
    }
}

std::string CodeGenerator::hash_operator_instruction(BasicBlock * block, OperatorInstruction * instruction) {
    std::stringstream ss;
    ss << hash_operand(block, instruction->left);
    ss << " " << instruction->operator_str() << " ";
    ss << hash_operand(block, instruction->right);

    return ss.str();
}

std::string CodeGenerator::next_unique_value() {
    std::stringstream ss;
    ss << "?" << m_unique_value_count++;
    return ss.str();
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

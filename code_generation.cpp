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

int g_next_unique_label = 0;
int getNextUniqueLabel() {
    return g_next_unique_label++;
}
int get_class_size_in_bytes(std::string class_name, SymbolTable *symbol_table);

class MethodGenerator {
public:
    MethodGenerator(std::string class_name, FunctionDeclaration * function_declaration, SymbolTable * symbol_table) :
        m_register_count(0),
        m_class_name(class_name),
        m_function_declaration(function_declaration),
        m_symbol_table(symbol_table) {}
    void generate();
    void build_basic_blocks();
    void value_numbering();
    void calculate_mangle_sets();
    void dependency_management();
    void block_deletion();
    void compute_addresses();
    void compress_registers();

    void print_basic_blocks(std::ostream & out);
    void print_control_flow_graph(std::ostream & out);
    void print_assembly(std::ostream & out);

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
            METHOD_CALL,
            NON_VOID_METHOD_CALL,
            ALLOCATE_OBJECT,
            WRITE_POINTER,
            READ_POINTER,
            ALLOCATE_ARRAY,
        };
        Type type;

        Instruction(Type type) : type(type) {}
        // insert the indexes registers you read (rvalues) in this instruction
        virtual void insertReadRegisters(std::set<int> & used_list) = 0;
        // insert the indexes registers you mangle (lvalues) in this instruction
        virtual void insertMangledRegisters(std::set<int> & mangled_list) = 0;
        // remap the register indexes used to a new value based on a vector lookup
        virtual void remapRegisters(std::vector<int> & map) = 0;

        virtual void print(std::ostream & out) = 0;
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
                    ss.setf(std::ios::showpoint);
                    ss << _float;
                    ss.unsetf(std::ios::showpoint);
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

    struct MethodCallInstruction : public Instruction {
        std::string class_name;
        std::string method_name;
        std::vector<Variant> parameters;
        MethodCallInstruction(std::string class_name, std::string method_name)
            : Instruction(METHOD_CALL), class_name(class_name), method_name(method_name) {}

        void insertReadRegisters(std::set<int> & used_list) {
            for (int i = 0; i < (int)parameters.size(); i++)
                if (parameters[i].type == Variant::REGISTER)
                    used_list.insert(parameters[i]._int);
        }

        virtual void insertMangledRegisters(std::set<int> &mangled_list) {}

        virtual void remapRegisters(std::vector<int> & map) {
            for (int i = 0; i < (int)parameters.size(); i++)
                if (parameters[i].type == Variant::REGISTER)
                    parameters[i]._int = map[parameters[i]._int];
        }
        virtual void print(std::ostream &out) {
            out << class_name << "::" << method_name << "(";
            out << parameters[0].str();
            for (int i=1; i<(int)parameters.size(); ++i) {
                out << ", " << parameters[i].str();
            }
            out << ")";
        }
    };
    struct NonVoidMethodCallInstruction : public MethodCallInstruction {
        Variant dest;
        NonVoidMethodCallInstruction(std::string class_name, std::string method_name)
            : MethodCallInstruction(class_name, method_name)
        {
            this->type = NON_VOID_METHOD_CALL;
        }

        void remapRegisters(std::vector<int> &map) {
            dest._int = map[dest._int];
            MethodCallInstruction::remapRegisters(map);
        }
        void insertMangledRegisters(std::set<int> &mangled_list) {
            if (dest.type == Variant::REGISTER)
                mangled_list.insert(dest._int);
        }
        void print(std::ostream &out) {
            out << dest.str() << " = ";
            MethodCallInstruction::print(out);
        }
    };

    struct CopyInstruction : public Instruction {
        Variant dest; // register number
        Variant source; // register number
        CopyInstruction(Variant dest, Variant source) : Instruction(COPY), dest(dest), source(source) {}

        void insertReadRegisters(std::set<int> & used_list) {
            if (source.type == Variant::REGISTER)
                used_list.insert(source._int);
        }

        void insertMangledRegisters(std::set<int> & mangled_list) {
            if (dest.type == Variant::REGISTER)
                mangled_list.insert(dest._int);
        }

        void remapRegisters(std::vector<int> & map) {
            if (dest.type == Variant::REGISTER)
                dest._int = map[dest._int];
            if (source.type == Variant::REGISTER)
                source._int = map[source._int];
        }
        void print(std::ostream &out) {
            out << dest.str() << " = " << source.str();
        }
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

        void insertReadRegisters(std::set<int> & used_list) {
            if (left.type == Variant::REGISTER)
                used_list.insert(left._int);
            if (right.type == Variant::REGISTER)
                used_list.insert(right._int);
        }

        void insertMangledRegisters(std::set<int> & mangled_list) {
            if (dest.type == Variant::REGISTER)
                mangled_list.insert(dest._int);
        }

        void remapRegisters(std::vector<int> & map) {
            if (dest.type == Variant::REGISTER)
                dest._int = map[dest._int];
            if (left.type == Variant::REGISTER)
                left._int = map[left._int];
            if (right.type == Variant::REGISTER)
                right._int = map[right._int];
        }
        void print(std::ostream &out) {
            out << str();
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


        void insertReadRegisters(std::set<int> & used_list) {
            if (source.type == Variant::REGISTER)
                used_list.insert(source._int);
        }

        void insertMangledRegisters(std::set<int> & mangled_list) {
            if (dest.type == Variant::REGISTER)
                mangled_list.insert(dest._int);
        }


        void remapRegisters(std::vector<int> & map) {
            if (dest.type == Variant::REGISTER)
                dest._int = map[dest._int];
            if (source.type == Variant::REGISTER)
                source._int = map[source._int];
        }
        void print(std::ostream &out) {
            out << dest.str() << " = ";
            if (_operator == UnaryInstruction::NEGATE)
                out << "-";
            else if (_operator == UnaryInstruction::NOT)
                out << "!";
            else
                assert(false);
            out << source.str();
        }
    };

    struct IfInstruction : public Instruction {
        Variant condition;
        int goto_index;

        IfInstruction(Variant condition, int goto_index) : Instruction(IF), condition(condition), goto_index(goto_index) {}

        void insertReadRegisters(std::set<int> & used_list) {
            if (condition.type == Variant::REGISTER)
                used_list.insert(condition._int);
        }

        void insertMangledRegisters(std::set<int> & mangled_list) {}

        void remapRegisters(std::vector<int> & map) {
            if (condition.type == Variant::REGISTER)
                condition._int = map[condition._int];
        }
        void print(std::ostream &out) {
            out << "if !" << condition.str() << " goto " << goto_index;
        }
    };

    struct GotoInstruction : public Instruction {
        int goto_index;

        GotoInstruction(int goto_index) : Instruction(GOTO), goto_index(goto_index) {}

        void insertReadRegisters(std::set<int> & used_list) {}
        void insertMangledRegisters(std::set<int> & mangled_list) {}
        void remapRegisters(std::vector<int> & map) {}
        void print(std::ostream &out) {
            out << "goto " << goto_index;
        }
    };

    struct ReturnInstruction : public Instruction {
        ReturnInstruction() : Instruction(RETURN) {}

        void insertReadRegisters(std::set<int> & used_list) {}
        void insertMangledRegisters(std::set<int> & mangled_list) {}
        void remapRegisters(std::vector<int> & map) {}
        void print(std::ostream &out) {
            out << "return";
        }
    };

    struct PrintInstruction : public Instruction {
        Variant value;
        PrintInstruction(Variant value) : Instruction(PRINT), value(value) {}

        void insertReadRegisters(std::set<int> & used_list) {
            if (value.type == Variant::REGISTER)
                used_list.insert(value._int);
        }

        void insertMangledRegisters(std::set<int> & mangled_list) {}

        void remapRegisters(std::vector<int> & map) {
            if (value.type == Variant::REGISTER)
                value._int = map[value._int];
        }
        void print(std::ostream &out) {
            out << "print " << value.str();
        }
    };

    struct AllocateObjectInstruction : public Instruction {
        Variant dest;
        std::string class_name;
        AllocateObjectInstruction(Variant dest, std::string class_name) : Instruction(ALLOCATE_OBJECT), dest(dest), class_name(class_name) {}

        void insertReadRegisters(std::set<int> & used_list) {}

        void insertMangledRegisters(std::set<int> & mangled_list) {
            if (dest.type == Variant::REGISTER)
                mangled_list.insert(dest._int);
        }

        void remapRegisters(std::vector<int> & map) {
            if (dest.type == Variant::REGISTER)
                dest._int = map[dest._int];
        }
        void print(std::ostream &out) {
            out << dest.str() << " = new " << class_name;
        }
    };

    struct WritePointerInstruction : public Instruction {
        Variant pointer; // register number that holds pointer to write to
        Variant source; // register number
        WritePointerInstruction(Variant pointer, Variant source) : Instruction(WRITE_POINTER), pointer(pointer), source(source) {}

        void insertReadRegisters(std::set<int> & used_list) {
            if (source.type == Variant::REGISTER)
                used_list.insert(source._int);
            if (pointer.type == Variant::REGISTER)
                used_list.insert(pointer._int);
        }

        void insertMangledRegisters(std::set<int> & mangled_list) {}

        void remapRegisters(std::vector<int> & map) {
            if (pointer.type == Variant::REGISTER)
                pointer._int = map[pointer._int];
            if (source.type == Variant::REGISTER)
                source._int = map[source._int];
        }
        void print(std::ostream &out) {
            out << "*" << pointer.str() << " = " << source.str();
        }
    };

    struct ReadPointerInstruction : public Instruction {
        Variant dest; // register number
        Variant source_pointer; // register number of pointer to read from
        ReadPointerInstruction(Variant dest, Variant source_pointer) : Instruction(READ_POINTER), dest(dest), source_pointer(source_pointer) {}

        void insertReadRegisters(std::set<int> & used_list) {
            if (source_pointer.type == Variant::REGISTER)
                used_list.insert(source_pointer._int);
        }

        void insertMangledRegisters(std::set<int> & mangled_list) {
            if (dest.type == Variant::REGISTER)
                mangled_list.insert(dest._int);
        }

        void remapRegisters(std::vector<int> & map) {
            if (dest.type == Variant::REGISTER)
                dest._int = map[dest._int];
            if (source_pointer.type == Variant::REGISTER)
                source_pointer._int = map[source_pointer._int];
        }
        void print(std::ostream &out) {
            out << dest.str() << " = *" << source_pointer.str();
        }
    };


    struct AllocateArrayInstruction : public Instruction {
        Variant dest;
        int size; // bytes
        AllocateArrayInstruction(Variant dest, int size) : Instruction(ALLOCATE_ARRAY), dest(dest), size(size) {}

        void insertReadRegisters(std::set<int> & used_list) {}

        void insertMangledRegisters(std::set<int> & mangled_list) {
            if (dest.type == Variant::REGISTER)
                mangled_list.insert(dest._int);
        }

        void remapRegisters(std::vector<int> & map) {
            if (dest.type == Variant::REGISTER)
                dest._int = map[dest._int];
        }
        void print(std::ostream &out) {
            out << dest.str() << " = new byte[" << size << "]";
        }
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

        bool deleted;
        // maps registers to value number/ constant
        TwoWayMap<int, Variant> value_numbers;

        BasicBlock(int start, int end) : start(start), end(end), is_destination(false), is_source(false), deleted(false) {}
    };

    enum RegisterType {
        INTEGER,
        REAL,
        BOOL,
        POINTER,
    };

    typedef std::list<Instruction *> InstructionList;

    std::vector<Instruction *> m_instructions;
    OrderedInsensitiveMap<Variant> m_variable_numbers;
    int m_register_count;
    int m_unique_value_count;
    std::vector<BasicBlock *> m_basic_blocks;
    std::string m_class_name;
    FunctionDeclaration * m_function_declaration;

    std::vector<RegisterType> m_register_type;
    SymbolTable * m_symbol_table;

private:
    Variant next_available_register(RegisterType type);

    void gen_statement_list(StatementList * statement_list);
    void gen_statement(Statement * statement);
    // returns whether it put anything in the dest
    bool gen_method_designator(MethodDesignator * method_designator, Variant & out_dest);
    Variant gen_expression(Expression * expression);
    Variant gen_additive_expression(AdditiveExpression * additive_expression);
    Variant gen_multiplicitive_expression(MultiplicativeExpression * multiplicative_expression);
    Variant gen_negatable_expression(NegatableExpression * negatable_expression);
    Variant gen_primary_expression(PrimaryExpression * primary_expression);
    Variant gen_variable_access(VariableAccess * variable);
    Variant gen_initialize_array(TypeDenoter * type);

    void gen_assignment(VariableAccess * variable, Variant source);
    void link_parent_and_child(int parent_index, int jump_child, int fallthrough_child);

    void print_instruction(std::ostream & out, int address, Instruction * instruction);

    Variant get_value_number(BasicBlock * block, Variant register_or_const);
    Variant inline_value(BasicBlock * block, Variant register_or_const);
    Instruction * constant_folded(BasicBlock * block, OperatorInstruction * instruction);

    CopyInstruction * make_copy(BasicBlock * block, OperatorInstruction * operator_instruction);
    bool operands_same(OperatorInstruction * instruction);
    bool right_constant_is(OperatorInstruction * instruction, int constant);
    bool left_constant_is(OperatorInstruction * instruction, int constant);
    CopyInstruction * make_immediate(BasicBlock * block, OperatorInstruction * operator_instruction, int constant);
    CopyInstruction * make_immediate(BasicBlock * block, OperatorInstruction * operator_instruction, float constant);
    CopyInstruction * make_immediate(BasicBlock * block, OperatorInstruction * operator_instruction, bool constant);
    CopyInstruction * make_immediate(BasicBlock * block, UnaryInstruction * operator_instruction, int constant);
    CopyInstruction * make_immediate(BasicBlock * block, UnaryInstruction * operator_instruction, float constant);
    CopyInstruction * make_immediate(BasicBlock * block, UnaryInstruction * operator_instruction, bool constant);
    CopyInstruction * constant_expression_evaluated(BasicBlock * block, OperatorInstruction * operator_instruction);
    CopyInstruction * constant_expression_evaluated(BasicBlock * block, UnaryInstruction * operator_instruction);
    std::string next_unique_value();
    std::string hash_operator_instruction(BasicBlock * block, OperatorInstruction * instruction);
    void basic_block_value_numbering(BasicBlock * block);
    std::string hash_operand(BasicBlock * block, Variant operand);
    RegisterType type_denoter_to_register_type(TypeDenoter * type);
    void calculate_mangle_set(int block_index);
    void calculate_downward_mangle_set(int block_index);
    void calculate_upward_mangle_set(int block_index);
    void delete_block(int index);
    void loadValue(std::ostream & out, Variant source_value, std::string dest_register);
    void storeRegister(std::ostream & out, int dest_register_number, std::string source_register);
    int get_stack_space();

    TypeDenoter * get_class_type(VariableAccess * variable_access);
    std::string get_class_name(TypeDenoter * type);
    int get_field_offset_in_bytes(std::string class_name, std::string field_name);
    Variant gen_attribute_pointer(AttributeDesignator * attribute);
    Variant gen_array_pointer(IndexedVariable * indexed_variable, ArrayType * type);
    TypeDenoter * variable_access_type(VariableAccess * variable_access);
    int get_stack_variable_offset_in_bytes(int variable_number);

};

MethodGenerator::Variant MethodGenerator::next_available_register(RegisterType type) {
    m_register_type.push_back(type);
    return Variant(m_register_count++, Variant::REGISTER);
}

void generate_code(Program * program, SymbolTable * symbol_table, bool debug, bool disable_optimization, bool skip_lame_stuff) {
    std::stringstream debug_out;
    std::stringstream asm_out;

    // mips header and main program
    asm_out << ".data" << std::endl;
    asm_out << "true_text: .asciiz \"true\"" << std::endl;
    asm_out << "false_text: .asciiz \"false\"" << std::endl;
    asm_out << "heap_start: .word 0" << std::endl;

    asm_out << ".text" << std::endl;
    asm_out << "main:" << std::endl;
    asm_out << "la $fp, heap_start" << std::endl;

    asm_out << "jal _entrypoint__entrypoint" << std::endl;

    asm_out << std::endl << "# quit" << std::endl;
    asm_out << "li $v0, 10" << std::endl;
    asm_out << "syscall" << std::endl;
    for (ClassList * class_list_node = program->class_list; class_list_node != NULL; class_list_node = class_list_node->next) {
        ClassDeclaration * class_declaration = class_list_node->item;
        for (FunctionDeclarationList * function_list_node = class_declaration->class_block->function_list; function_list_node != NULL; function_list_node = function_list_node->next) {
            FunctionDeclaration * function_declaration = function_list_node->item;

            debug_out << "Method " << class_declaration->identifier->text << "." << function_declaration->identifier->text << std::endl;
            debug_out << "--------------------------" << std::endl;

            MethodGenerator generator(Utils::to_lower(class_declaration->identifier->text), function_declaration, symbol_table);
            generator.generate();
            generator.build_basic_blocks();

            if (!skip_lame_stuff) {
                debug_out << "3 Address Code" << std::endl;
                debug_out << "--------------------------" << std::endl;
                generator.print_basic_blocks(debug_out);
                debug_out << "--------------------------" << std::endl;

                debug_out << "Control Flow Graph" << std::endl;
                debug_out << "--------------------------" << std::endl;
                generator.print_control_flow_graph(debug_out);
                debug_out << "--------------------------" << std::endl;
            }

            if (! disable_optimization) {
                generator.calculate_mangle_sets();
                generator.value_numbering();
                generator.compress_registers();

                if (!skip_lame_stuff) {
                    debug_out << "3 Address Code After Value Numbering" << std::endl;
                    debug_out << "--------------------------" << std::endl;
                    generator.print_basic_blocks(debug_out);
                    debug_out << "--------------------------" << std::endl;
                }

                generator.dependency_management();
                generator.compute_addresses();
                generator.compress_registers();

                if (!skip_lame_stuff) {
                    debug_out << "3 Address Code After Dependency Management" << std::endl;
                    debug_out << "--------------------------" << std::endl;
                    generator.print_basic_blocks(debug_out);
                    debug_out << "--------------------------" << std::endl;
                }

                generator.block_deletion();
                generator.compute_addresses();
                generator.compress_registers();
                debug_out << "3 Address Code After Block Deletion" << std::endl;
                debug_out << "--------------------------" << std::endl;
                generator.print_basic_blocks(debug_out);
                debug_out << "--------------------------" << std::endl;
            }

            generator.print_assembly(asm_out);
        }
    }

    if (debug)
        std::cout << debug_out.str();

    std::cout << asm_out.str();
}

int MethodGenerator::get_stack_variable_offset_in_bytes(int variable_number)
{
    return get_stack_space() - variable_number * 4 - 4;
}

void MethodGenerator::loadValue(std::ostream & out, Variant source_value, std::string dest_register)
{
    if (source_value.type == Variant::CONST_BOOL) {
        out << "li " << dest_register << ", " << (source_value._bool ? 1 : 0) << std::endl;
    } else if (source_value.type == Variant::CONST_INT) {
        out << "li " << dest_register << ", " << source_value._int << std::endl;
    } else if (source_value.type == Variant::REGISTER) {
        out << "lw " << dest_register << ", " << get_stack_variable_offset_in_bytes(source_value._int) << "($sp)" << std::endl;
    } else {
        assert(false);
    }
}

void MethodGenerator::storeRegister(std::ostream & out, int dest_register_number, std::string source_register)
{
    out << "sw " << source_register << ", " << get_stack_variable_offset_in_bytes(dest_register_number) << "($sp)" << std::endl;
}

int MethodGenerator::get_stack_space()
{
    return
        // a slot for each register (all types are the same size: 4 bytes)
        m_register_count * 4 +
        // a slot for return address (4 bytes)
        1 * 4;
}

void MethodGenerator::print_assembly(std::ostream & out)
{
    std::string method_name = Utils::to_lower(m_function_declaration->identifier->text);
    out << m_class_name << "_" << method_name << ":" << std::endl;

    // allocate stack space for locals
    out << "addi $sp, $sp, -" << get_stack_space() << std::endl;
    out << "sw $ra, 0($sp)" << std::endl;

    for (unsigned int b = 0; b < m_basic_blocks.size(); b++) {
        BasicBlock * block = m_basic_blocks[b];
        if (block->deleted)
            continue;
        out << m_class_name << "_" << method_name << "_" << b << ":" << std::endl;

        int i = block->start;
        for (InstructionList::iterator it = block->instructions.begin(); it != block->instructions.end(); ++it, ++i) {
            Instruction * instruction = *it;
            out << std::endl << "# ";
            instruction->print(out);
            out << std::endl;
            switch (instruction->type) {
                case Instruction::COPY:
                {
                    CopyInstruction * copy_instruction = (CopyInstruction *) instruction;
                    loadValue(out, copy_instruction->source, "$t0");
                    storeRegister(out, copy_instruction->dest._int, "$t0");
                    break;
                }
                case Instruction::OPERATOR:
                {
                    OperatorInstruction * operator_instruction = (OperatorInstruction *) instruction;
                    loadValue(out, operator_instruction->left, "$t0");
                    loadValue(out, operator_instruction->right, "$t1");
                    switch (operator_instruction->_operator) {
                        case OperatorInstruction::EQUAL:
                        {
                            int skip_label = getNextUniqueLabel();
                            out << "li $t2, 1" << std::endl;
                            out << "beq $t0, $t1, l" << skip_label << std::endl;
                            out << "li $t2, 0" << std::endl;
                            out << "l" << skip_label << ":" << std::endl;
                            out << "move $t0, $t2" << std::endl;
                            break;
                        }
                        case OperatorInstruction::NOT_EQUAL:
                        {
                            int skip_label = getNextUniqueLabel();
                            out << "li $t2, 1" << std::endl;
                            out << "bne $t0, $t1, l" << skip_label << std::endl;
                            out << "li $t2, 0" << std::endl;
                            out << "l" << skip_label << ":" << std::endl;
                            out << "move $t0, $t2" << std::endl;
                            break;
                        }
                        case OperatorInstruction::LESS:
                            out << "slt $t0, $t0, $t1" << std::endl;
                            break;
                        case OperatorInstruction::GREATER:
                            out << "slt $t0, $t1, t0" << std::endl;
                            break;
                        case OperatorInstruction::LESS_EQUAL:
                            out << "slt $t0, $t1, $t0" << std::endl;
                            out << "xori $t0, 1" << std::endl;
                            break;
                        case OperatorInstruction::GREATER_EQUAL:
                            out << "slt $t0, $t0, $t1" << std::endl;
                            out << "xori $t0, 1" << std::endl;
                            break;
                        case OperatorInstruction::PLUS:
                            out << "add $t0, $t0, $t1" << std::endl;
                            break;
                        case OperatorInstruction::MINUS:
                            out << "sub $t0, $t0, $t1" << std::endl;
                            break;
                        case OperatorInstruction::OR:
                            out << "or $t0, $t0, $t1" << std::endl;
                            break;
                        case OperatorInstruction::TIMES:
                            out << "mul $t0, $t0, $t1" << std::endl;
                            break;
                        case OperatorInstruction::DIVIDE:
                            out << "div $t0, $t1" << std::endl;
                            out << "mflo $t0" << std::endl;
                            break;
                        case OperatorInstruction::MOD:
                            out << "div $t0, $t1" << std::endl;
                            out << "mfhi $t0" << std::endl;
                            break;
                        case OperatorInstruction::AND:
                            out << "and $t0, $t0, $t1" << std::endl;
                            break;
                    }
                    storeRegister(out, operator_instruction->dest._int, "$t0");
                    break;
                }
                case Instruction::UNARY:
                {
                    UnaryInstruction * unary_instruction = (UnaryInstruction *) instruction;
                    loadValue(out, unary_instruction->source, "$t0");
                    if (unary_instruction->_operator == UnaryInstruction::NOT) {
                        out << "xori $t0, $t0, 1" << std::endl;
                    } else if (unary_instruction->_operator == UnaryInstruction::NEGATE) {
                        out << "sub $t0, $zero, $t0" << std::endl;
                    } else {
                        assert(false);
                    }
                    storeRegister(out, unary_instruction->dest._int, "$t0");
                    break;
                }
                case Instruction::IF:
                {
                    IfInstruction * if_instruction = (IfInstruction *) instruction;
                    loadValue(out, if_instruction->condition, "$t0");
                    out << "beq $t0, $0, " << m_class_name << "_" << method_name << "_" << block->jump_child << std::endl;
                    break;
                }
                case Instruction::GOTO:
                    out << "j " << m_class_name << "_" << method_name << "_" << block->jump_child << std::endl;
                    break;
                case Instruction::RETURN:
                    if (m_function_declaration->type != NULL) {
                        // put the result in $v0
                        loadValue(out, m_variable_numbers.get(m_function_declaration->identifier->text), "$v0");
                    }
                    // deallocate stack
                    out << "lw $ra, 0($sp)" << std::endl;
                    out << "addi $sp, $sp, " << get_stack_space() << std::endl;
                    out << "jr $ra" << std::endl;
                    return;
                case Instruction::PRINT:
                {
                    PrintInstruction * print_instruction = (PrintInstruction *) instruction;
                    bool is_bool = false;
                    if (print_instruction->value.type == Variant::REGISTER) {
                        if (m_register_type.at(print_instruction->value._int) == BOOL) {
                            is_bool = true;
                            loadValue(out, print_instruction->value, "$t0");
                            int skip_label = getNextUniqueLabel();
                            out << "la $a0, true_text" << std::endl;
                            out << "bne $t0, $0, l" << skip_label << std::endl;
                            out << "la $a0, false_text" << std::endl;
                            out << "l" << skip_label << ":" << std::endl;
                            out << "li $v0, 4" << std::endl;
                            out << "syscall" << std::endl;
                        }
                    } else {
                        if (print_instruction->value.type == Variant::CONST_BOOL) {
                            is_bool = true;
                            out << "la $a0, " << (print_instruction->value._bool ? "true_text" : "false_text") << std::endl;
                            out << "li $v0, 4" << std::endl;
                            out << "syscall" << std::endl;
                        }
                    }
                    if (! is_bool) {
                        loadValue(out, print_instruction->value, "$a0");
                        out << "li $v0, 1" << std::endl;
                        out << "syscall" << std::endl;
                    }
                    // newline
                    out << "li $a0, 10" << std::endl;
                    out << "li $v0, 11" << std::endl;
                    out << "syscall" << std::endl;
                    break;
                }
                case Instruction::NON_VOID_METHOD_CALL:
                case Instruction::METHOD_CALL:
                {
                    MethodCallInstruction * method_call_instruction = (MethodCallInstruction *) instruction;
                    for (int i = 0; i < (int)method_call_instruction->parameters.size(); i++) {
                        loadValue(out, method_call_instruction->parameters[i], "$t0");
                        out << "sw $t0, " << (-i * 4 - 4) << "($sp)" << std::endl;
                    }
                    out << "jal " << Utils::to_lower(method_call_instruction->class_name) << "_" << Utils::to_lower(method_call_instruction->method_name) << std::endl;
                    if (instruction->type == Instruction::NON_VOID_METHOD_CALL)
                        storeRegister(out, ((NonVoidMethodCallInstruction *)method_call_instruction)->dest._int, "$v0");
                    break;
                }
                case Instruction::ALLOCATE_OBJECT:
                {
                    AllocateObjectInstruction * allocate_instruction = (AllocateObjectInstruction *) instruction;
                    storeRegister(out, allocate_instruction->dest._int, "$fp");
                    int size = get_class_size_in_bytes(allocate_instruction->class_name, m_symbol_table);
                    out << "addi $fp, $fp, " << size << std::endl;
                    break;
                }
                case Instruction::ALLOCATE_ARRAY:
                {
                    AllocateArrayInstruction * allocate_instruction = (AllocateArrayInstruction *) instruction;
                    storeRegister(out, allocate_instruction->dest._int, "$fp");
                    out << "addi $fp, $fp, " << allocate_instruction->size << std::endl;
                    break;
                }
                case Instruction::WRITE_POINTER:
                {
                    WritePointerInstruction * write_pointer_instruction = (WritePointerInstruction *) instruction;
                    loadValue(out, write_pointer_instruction->source, "$t0");
                    loadValue(out, write_pointer_instruction->pointer, "$t1");
                    out << "sw $t0, 0($t1)" << std::endl;
                    break;
                }
                case Instruction::READ_POINTER:
                {
                    ReadPointerInstruction * read_pointer_instruction = (ReadPointerInstruction *) instruction;
                    loadValue(out, read_pointer_instruction->source_pointer, "$t0");
                    out << "lw $t0, 0($t0)" << std::endl;
                    storeRegister(out, read_pointer_instruction->dest._int, "$t0");
                    break;
                }
            }
        }
    }
    // should have been a return statement
    assert(false);
}

int get_class_size_in_bytes(std::string class_name, SymbolTable * symbol_table)
{
    ClassSymbolTable * class_symbols = symbol_table->get(class_name);
    return class_symbols->variables->count() * 4;
}

void MethodGenerator::print_instruction(std::ostream & out, int address, Instruction * instruction) {
    out << address << ":\t";
    instruction->print(out);
    out << ";" << std::endl;
}

void MethodGenerator::print_basic_blocks(std::ostream & out) {
    int block_count = 0;
    for (unsigned int b = 0; b < m_basic_blocks.size(); b++) {
        BasicBlock * block = m_basic_blocks[b];
        if (! block->deleted)
            out << "block_" << block_count++ << ":" << std::endl;
        int i = block->start;
        for (InstructionList::iterator it = block->instructions.begin(); it != block->instructions.end(); ++it, ++i)
            print_instruction(out, i, *it);
    }
}

void MethodGenerator::print_control_flow_graph(std::ostream & out) {
    for (int parent = 0; parent < (int)m_basic_blocks.size(); parent++) {
        BasicBlock * parent_block = m_basic_blocks[parent];
        for (int child = 0; child < (int)m_basic_blocks.size(); child++) {
            if (parent == child) {
                // for self, print the number (ugly if more than 1 digit)
                out << parent;
            } else if (parent_block->jump_child == child || parent_block->fallthrough_child == child) {
                // child is a child of parent
                out << (child < parent ? "^" : "v");
            } else {
                // nothing to see here
                out << " ";
            }
            out << " ";
        }
        out << std::endl;
    }
}

MethodGenerator::RegisterType MethodGenerator::type_denoter_to_register_type(TypeDenoter * type) {
    switch (type->type) {
        case TypeDenoter::BOOLEAN:
            return BOOL;
        case TypeDenoter::INTEGER:
            return INTEGER;
        case TypeDenoter::REAL:
            return REAL;
        case TypeDenoter::CLASS:
            return POINTER;
        case TypeDenoter::ARRAY:
            return POINTER;
        default:
            assert(false);
            return BOOL; // BOOL is COOL
    }
}

void MethodGenerator::generate() {
    m_variable_numbers.put("this", next_available_register(POINTER));
    for (VariableDeclarationList * variable_list = m_function_declaration->parameter_list; variable_list != NULL; variable_list = variable_list->next) {
        for (IdentifierList * id_list = variable_list->item->id_list; id_list != NULL; id_list = id_list->next)
            m_variable_numbers.put(id_list->item->text, next_available_register(type_denoter_to_register_type(variable_list->item->type)));
    }
    if (m_function_declaration->type != NULL) {
        // make a special return value variable
        m_variable_numbers.put(m_function_declaration->identifier->text, next_available_register(type_denoter_to_register_type(m_function_declaration->type)));
    }
    for (VariableDeclarationList * variable_list = m_function_declaration->block->variable_list; variable_list != NULL; variable_list = variable_list->next) {
        for (IdentifierList * id_list = variable_list->item->id_list; id_list != NULL; id_list = id_list->next)
            m_variable_numbers.put(id_list->item->text, next_available_register(type_denoter_to_register_type(variable_list->item->type)));
    }

    // allocate local arrays
    ClassSymbolTable * class_symbols = m_symbol_table->get(m_class_name);
    FunctionSymbolTable * function_symbols = class_symbols->function_symbols->get(m_function_declaration->identifier->text);
    for (int i = 0; i < function_symbols->variables->count(); ++i) {
        VariableData * variable = function_symbols->variables->get(i);
        if (variable->type == NULL || variable->type->type != TypeDenoter::ARRAY)
            continue;

        Variant value = gen_initialize_array(variable->type);
        m_instructions.push_back(new CopyInstruction(m_variable_numbers.get(variable->name), value));
    }

    gen_statement_list(m_function_declaration->block->statement_list);

    m_instructions.push_back(new ReturnInstruction());
}

void MethodGenerator::gen_statement_list(StatementList * statement_list) {
    for (StatementList * statement_list_node = statement_list; statement_list_node != NULL; statement_list_node = statement_list_node->next) {
        if (statement_list_node->item != NULL)
            gen_statement(statement_list_node->item);
    }
}

void MethodGenerator::gen_statement(Statement * statement) {
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
        case Statement::METHOD:
        {
            Variant ignored;
            gen_method_designator(statement->method, ignored);
            break;
        }
        default:
            assert(false);
    }
}

bool MethodGenerator::gen_method_designator(MethodDesignator * method_designator, Variant & out_dest) {
    std::string class_name = get_class_name(get_class_type(method_designator->owner));
    std::string method_name = method_designator->function->identifier->text;
    class_name = get_declaring_class(m_symbol_table, class_name, method_name);
    FunctionDeclaration * declaration = get_method(m_symbol_table, class_name, method_name);
    bool non_void = declaration->type != NULL;
    MethodCallInstruction * instruction = non_void ?
                                          new NonVoidMethodCallInstruction(class_name, method_name) :
                                          new MethodCallInstruction(class_name, method_name);
    instruction->parameters.push_back(gen_variable_access(method_designator->owner));
    for (ExpressionList * parameter_list = method_designator->function->parameter_list; parameter_list != NULL; parameter_list = parameter_list->next) {
        Expression * expression = parameter_list->item;
        Variant parameter = gen_expression(expression);
        instruction->parameters.push_back(parameter);
    }
    if (non_void) {
        // assign the result somewhere
        NonVoidMethodCallInstruction * non_void_instruction = (NonVoidMethodCallInstruction *) instruction;
        non_void_instruction->dest = next_available_register(type_denoter_to_register_type(declaration->type));
        out_dest = non_void_instruction->dest;
    }
    m_instructions.push_back(instruction);
    return non_void;
}


MethodGenerator::Variant MethodGenerator::gen_expression(Expression * expression) {
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

MethodGenerator::Variant MethodGenerator::gen_additive_expression(AdditiveExpression * additive_expression) {
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

MethodGenerator::Variant MethodGenerator::gen_multiplicitive_expression(MultiplicativeExpression * multiplicative_expression) {
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

MethodGenerator::Variant MethodGenerator::gen_negatable_expression(NegatableExpression * negatable_expression) {
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

MethodGenerator::Variant MethodGenerator::gen_initialize_array(TypeDenoter * type) {
    assert(type->type == TypeDenoter::ARRAY);

    int count = type->array_type->max->value - type->array_type->min->value + 1;
    Variant base_array_pointer = next_available_register(POINTER);
    m_instructions.push_back(new AllocateArrayInstruction(base_array_pointer, count * 4));
    if (type->array_type->type->type == TypeDenoter::ARRAY) {
        for (int i = 0; i < count; ++i) {
            Variant entry_pointer = next_available_register(POINTER);
            m_instructions.push_back(new OperatorInstruction(entry_pointer, base_array_pointer, OperatorInstruction::PLUS, Variant(i*4, Variant::CONST_INT)));
            Variant value = gen_initialize_array(type->array_type->type);
            m_instructions.push_back(new WritePointerInstruction(entry_pointer, value));
        }
    }
    return base_array_pointer;
}

MethodGenerator::Variant MethodGenerator::gen_primary_expression(PrimaryExpression * primary_expression) {
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
        case PrimaryExpression::OBJECT_INSTANTIATION:
        {
            Variant new_object_pointer = next_available_register(POINTER);
            // make the instance
            std::string class_name = primary_expression->object_instantiation->class_identifier->text;
            m_instructions.push_back(new AllocateObjectInstruction(new_object_pointer, class_name));
            ClassSymbolTable * class_symbols = m_symbol_table->get(class_name);

            // allocate its arrays
            for (int i = 0; i < class_symbols->variables->count(); ++i) {
                VariableData * variable = class_symbols->variables->get(i);
                if (variable->type->type != TypeDenoter::ARRAY)
                    continue;
                int field_offset = get_field_offset_in_bytes(class_name, variable->name);
                Variant field_pointer = next_available_register(POINTER);
                m_instructions.push_back(new OperatorInstruction(field_pointer, new_object_pointer, OperatorInstruction::PLUS, Variant(field_offset, Variant::CONST_INT)));
                Variant value = gen_initialize_array(variable->type);
                m_instructions.push_back(new WritePointerInstruction(field_pointer, value));
            }

            bool has_constructor = class_symbols->function_symbols->has_key(class_name);
            if (has_constructor) {
                MethodCallInstruction * method_call = new MethodCallInstruction(class_name, class_name);
                method_call->parameters.push_back(new_object_pointer);
                for (ExpressionList * expression_list = primary_expression->object_instantiation->parameter_list; expression_list != NULL; expression_list = expression_list->next) {
                    Expression * expression = expression_list->item;
                    Variant value = gen_expression(expression);
                    method_call->parameters.push_back(value);
                }
                m_instructions.push_back(method_call);
            }
            return new_object_pointer;
        }
        case PrimaryExpression::METHOD:
        {
            Variant result;
            assert(gen_method_designator(primary_expression->method, result));
            return result;
        }
        default:
            assert(false);
    }
}

MethodGenerator::Variant MethodGenerator::gen_attribute_pointer(AttributeDesignator * attribute)
{
    Variant owner_class_ref = gen_variable_access(attribute->owner);
    std::string owner_class_name = get_class_name(get_class_type(attribute->owner));
    int offset = get_field_offset_in_bytes(owner_class_name, attribute->identifier->text);
    Variant pointer_register = next_available_register(POINTER);
    m_instructions.push_back(new OperatorInstruction(pointer_register, owner_class_ref, OperatorInstruction::PLUS, Variant(offset, Variant::CONST_INT)));
    return pointer_register;
}

MethodGenerator::Variant MethodGenerator::gen_array_pointer(IndexedVariable * indexed_variable, ArrayType * array_type)
{
    Variant array_ref = gen_variable_access(indexed_variable->variable);
    for (ExpressionList * expression_list = indexed_variable->expression_list; expression_list != NULL; expression_list = expression_list->next) {
        Expression * expression = expression_list->item;
        Variant index = gen_expression(expression);
        if (array_type->min != 0) {
            Variant corrected_index = next_available_register(INTEGER);
            m_instructions.push_back(new OperatorInstruction(corrected_index, index, OperatorInstruction::MINUS, Variant(array_type->min->value, Variant::CONST_INT)));
            index = corrected_index;
        }
        Variant bytes_offset = next_available_register(INTEGER);
        m_instructions.push_back(new OperatorInstruction(bytes_offset, index, OperatorInstruction::TIMES, Variant(4, Variant::CONST_INT)));
        Variant array_pointer = next_available_register(POINTER);
        m_instructions.push_back(new OperatorInstruction(array_pointer, array_ref, OperatorInstruction::PLUS, bytes_offset));

        if (expression_list->next == NULL)
            return array_pointer;

        // dereference array_pointer
        array_ref = next_available_register(POINTER);
        m_instructions.push_back(new ReadPointerInstruction(array_ref, array_pointer));
    }
    assert(false);
    return array_ref;
}

TypeDenoter * MethodGenerator::variable_access_type(VariableAccess * variable_access)
{
    switch (variable_access->type) {
        case VariableAccess::IDENTIFIER:
        {
            ClassSymbolTable * class_symbols = m_symbol_table->get(m_class_name);
            FunctionSymbolTable * function_symbols = class_symbols->function_symbols->get(m_function_declaration->identifier->text);
            VariableData * variable = function_symbols->variables->get(variable_access->identifier->text);
            return variable->type;
        }
        case VariableAccess::ATTRIBUTE:
        {
            VariableData * variable = get_field(m_symbol_table, get_class_name(get_class_type(variable_access->attribute->owner)), variable_access->attribute->identifier->text);
            return variable->type;
        }
        case VariableAccess::INDEXED_VARIABLE:
        {
            int iterations_to_here = 0;
            while (true) {
                if (variable_access->type == VariableAccess::INDEXED_VARIABLE) {
                    iterations_to_here++;
                    variable_access = variable_access->indexed_variable->variable;
                } else {
                    break;
                }
            }
            TypeDenoter * type = variable_access_type(variable_access);
            for (int i=0; i<iterations_to_here; ++i) {
                assert(type->type == TypeDenoter::ARRAY);
                type = type->array_type->type;
            }
            return type;
        }
        case VariableAccess::THIS:
            return new TypeDenoter(new Identifier(m_class_name, -1));
        default:
            assert(false);
    }
}


MethodGenerator::Variant MethodGenerator::gen_variable_access(VariableAccess * variable) {
    switch (variable->type) {
        case VariableAccess::IDENTIFIER:
            return m_variable_numbers.get(variable->identifier->text);
        case VariableAccess::THIS:
            return m_variable_numbers.get("this");
        case VariableAccess::ATTRIBUTE:
        {
            Variant dest = next_available_register(type_denoter_to_register_type(get_field(m_symbol_table, get_class_name(get_class_type(variable->attribute->owner)), variable->attribute->identifier->text)->type));
            m_instructions.push_back(new ReadPointerInstruction(dest, gen_attribute_pointer(variable->attribute)));
            return dest;
        }
        case VariableAccess::INDEXED_VARIABLE:
        {
            TypeDenoter * type = variable_access_type(variable->indexed_variable->variable);
            assert(type->type == TypeDenoter::ARRAY);
            Variant dest = next_available_register(type_denoter_to_register_type(type));
            m_instructions.push_back(new ReadPointerInstruction(dest, gen_array_pointer(variable->indexed_variable, type->array_type)));
            return dest;
        }
        default:
            assert(false);
    }
}

TypeDenoter * MethodGenerator::get_class_type(VariableAccess * variable_access)
{
    switch (variable_access->type) {
        case VariableAccess::IDENTIFIER:
        {
            // look for the identifier in function symbols
            ClassSymbolTable * class_symbols = m_symbol_table->get(m_class_name);
            FunctionSymbolTable * function_symbols = class_symbols->function_symbols->get(m_function_declaration->identifier->text);
            VariableData * variable =
                    function_symbols->variables->has_key(variable_access->identifier->text) ?
                    function_symbols->variables->get(variable_access->identifier->text) :
                    class_symbols->variables->get(variable_access->identifier->text);
            return variable->type;
        }
        case VariableAccess::INDEXED_VARIABLE:
        {
            TypeDenoter * type = get_class_type(variable_access->indexed_variable->variable);
            assert(type->type == TypeDenoter::ARRAY);
            return type->array_type->type;
        }
        case VariableAccess::ATTRIBUTE:
        {
            std::string owner_class_name = get_class_name(get_class_type(variable_access->attribute->owner));
            ClassSymbolTable * class_symbols = m_symbol_table->get(owner_class_name);
            VariableData * variable = class_symbols->variables->get(variable_access->attribute->identifier->text);
            return variable->type;
        }
        case VariableAccess::THIS:
            return new TypeDenoter(new Identifier(m_class_name, -1));
        default:
            assert(false);
    }
}

std::string MethodGenerator::get_class_name(TypeDenoter * type_denoter)
{
    assert(type_denoter->type == TypeDenoter::CLASS);
    return type_denoter->class_identifier->text;
}

int MethodGenerator::get_field_offset_in_bytes(std::string class_name, std::string field_name)
{
    while (true) {
        ClassSymbolTable * class_symbols = m_symbol_table->get(class_name);
        int sum = 0;
        for (int i = 0; i < class_symbols->variables->count(); i++) {
            VariableData * field = class_symbols->variables->get(i);
            if (Utils::to_lower(field_name).compare(Utils::to_lower(field->name)) == 0) {
                // found it
                int parent_size = 0;
                if (class_symbols->class_declaration->parent_identifier != NULL)
                    parent_size = get_class_size_in_bytes(class_symbols->class_declaration->parent_identifier->text, m_symbol_table);
                return parent_size + sum;
            }
            sum += 4;
        }
        if (class_symbols->class_declaration->parent_identifier == NULL) {
            // couldn't find the field
            assert(false);
        }
        // check the next type up
        class_name = class_symbols->class_declaration->parent_identifier->text;
    }
}

void MethodGenerator::gen_assignment(VariableAccess * variable, Variant source) {
    switch (variable->type) {
        case VariableAccess::IDENTIFIER:
            m_instructions.push_back(new CopyInstruction(m_variable_numbers.get(variable->identifier->text), source));
            break;
        case VariableAccess::ATTRIBUTE:
            m_instructions.push_back(new WritePointerInstruction(gen_attribute_pointer(variable->attribute), source));
            break;
        case VariableAccess::INDEXED_VARIABLE:
        {
            TypeDenoter * type = variable_access_type(variable->indexed_variable->variable);
            assert(type->type == TypeDenoter::ARRAY);
            m_instructions.push_back(new WritePointerInstruction(gen_array_pointer(variable->indexed_variable, type->array_type), source));
            break;
        }
        default:
            assert(false);
    }
}

void MethodGenerator::link_parent_and_child(int parent_index, int jump_child, int fallthrough_child) {
    m_basic_blocks[parent_index]->jump_child = jump_child;
    m_basic_blocks[parent_index]->fallthrough_child = fallthrough_child;

    if (jump_child >= 0)
        m_basic_blocks[jump_child]->parents.insert(parent_index);
    if (fallthrough_child >= 0)
        m_basic_blocks[fallthrough_child]->parents.insert(parent_index);
}

void MethodGenerator::build_basic_blocks() {
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

void MethodGenerator::basic_block_value_numbering(BasicBlock * block) {
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
                switch (operator_instruction->_operator) {
                    case OperatorInstruction::MINUS:
                    case OperatorInstruction::DIVIDE:
                    case OperatorInstruction::MOD:
                        swap = false;
                    default:
                        break;
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
                std::list<int> * registers = block->value_numbers.keys(hash);
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
            case Instruction::NON_VOID_METHOD_CALL:
            case Instruction::METHOD_CALL:
            {
                MethodCallInstruction * method_call_instruction = (MethodCallInstruction *) instruction;
                for (int i = 0; i < (int)method_call_instruction->parameters.size(); i++)
                    method_call_instruction->parameters[i] = inline_value(block, method_call_instruction->parameters[i]);
                // leave result value as unknonw
                break;
            }
            case Instruction::ALLOCATE_OBJECT:
                break;
            case Instruction::ALLOCATE_ARRAY:
                break;
            case Instruction::WRITE_POINTER:
            {
                WritePointerInstruction * write_pointer_instruction = (WritePointerInstruction *) instruction;
                write_pointer_instruction->source = inline_value(block, write_pointer_instruction->source);
                write_pointer_instruction->pointer = inline_value(block, write_pointer_instruction->pointer);
                break;
            }
            case Instruction::READ_POINTER:
            {
                ReadPointerInstruction * read_pointer_instruction = (ReadPointerInstruction *) instruction;
                read_pointer_instruction->source_pointer = inline_value(block, read_pointer_instruction->source_pointer);
                break;
            }
        }
    }
}

void MethodGenerator::value_numbering() {

    for (int i = 0; i < (int)m_basic_blocks.size(); i++) {
        BasicBlock * block = m_basic_blocks[i];
        basic_block_value_numbering(block);
    }
}

void MethodGenerator::calculate_mangle_sets() {
    for (int i = 0; i < (int)m_basic_blocks.size(); i++)
        calculate_mangle_set(i);
}

void MethodGenerator::calculate_mangle_set(int block_index) {
    BasicBlock * node = m_basic_blocks[block_index];
    for (std::set<int>::iterator it = node->parents.begin(); it != node->parents.end(); it++) {
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
                    for (InstructionList::iterator it = node->instructions.begin(); it != node->instructions.end(); ++it) {
                        Instruction * instruction = *it;
                        instruction->insertMangledRegisters(end_node->mangled_registers);
                    }
                }
                // reset state for next go around.
                node->is_destination = false;
                node->is_source = false;
            }
        }
    }
}

void MethodGenerator::calculate_downward_mangle_set(int block_index) {
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

void MethodGenerator::calculate_upward_mangle_set(int block_index) {
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

void MethodGenerator::compute_addresses() {
    int address = 0;
    for (int i = 0; i < (int)m_basic_blocks.size(); ++i) {
        BasicBlock * block = m_basic_blocks[i];
        if (block->deleted)
            continue;
        block->start = address;
        address += block->instructions.size();
        block->end = address;
    }
    for (int i = 0; i < (int)m_basic_blocks.size(); ++i) {
        BasicBlock * block = m_basic_blocks[i];
        if (block->deleted)
            continue;
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

void MethodGenerator::compress_registers()
{
    // start out, assume not using any
    std::set<int> used_registers;

    // be sure not to compress parameters
    used_registers.insert(0);
    for (VariableDeclarationList * variable_list = m_function_declaration->parameter_list; variable_list != NULL; variable_list = variable_list->next) {
        used_registers.insert((int)used_registers.size());
    }

    // go through program and mark the ones we do use
    for (int b = 0; b < (int)m_basic_blocks.size(); ++b) {
        BasicBlock * block = m_basic_blocks[b];
        if (block->deleted)
            continue;
        for (InstructionList::iterator it = block->instructions.begin(); it != block->instructions.end(); ++it) {
            Instruction * instruction = *it;
            instruction->insertReadRegisters(used_registers);
            instruction->insertMangledRegisters(used_registers);
        }
    }

    // create a mapping between current registers and new ones.
    std::vector<int> new_number;
    std::vector<RegisterType> new_type;
    new_number.resize(m_register_count);
    new_type.resize(m_register_count);


    int new_register_count = 0;
    for (std::set<int>::iterator it = used_registers.begin(); it != used_registers.end(); ++it) {
        int used_register_number = *it;
        new_number[used_register_number] = new_register_count;
        new_type[new_register_count] = m_register_type[used_register_number];
        ++new_register_count;
    }

    // apply the mapping to existing code
    for (int b = 0; b < (int)m_basic_blocks.size(); ++b) {
        BasicBlock * block = m_basic_blocks[b];
        if (block->deleted)
            continue;
        for (InstructionList::iterator it = block->instructions.begin(); it != block->instructions.end(); ++it) {
            Instruction * instruction = *it;
            instruction->remapRegisters(new_number);
        }
    }

    m_register_count = new_register_count;

    // apply to register types
    for (int i=0; i<m_register_count; ++i) {
        m_register_type[i] = new_type[i];
    }
    m_register_type.resize(m_register_count);

}

void MethodGenerator::delete_block(int index) {
    BasicBlock * block = m_basic_blocks[index];

    assert((block->jump_child == -1) != (block->fallthrough_child == -1));
    int child = block->jump_child != -1 ? block->jump_child : block->fallthrough_child;

    for (std::set<int>::iterator it = block->parents.begin(); it != block->parents.end(); ++it) {
        BasicBlock * parent = m_basic_blocks[*it];
        if (parent->jump_child == index) {
            parent->jump_child = child;
        } else if (parent->fallthrough_child == index) {
            parent->fallthrough_child = child;
        }
    }
    block->deleted = true;
}

void MethodGenerator::block_deletion() {
    for (int i = m_basic_blocks.size() - 1; i >= 0; --i) {
        BasicBlock * block = m_basic_blocks[i];

        if (block->instructions.size() != 0) {
            // if last instruction is if statement
            Instruction * last_instruction = *(block->instructions.rbegin());
            bool fallthrough = false;

            if (last_instruction->type == Instruction::IF) {
                fallthrough = block->jump_child == block->fallthrough_child;
            } else if (last_instruction->type == Instruction::GOTO) {
                for (int j = i+1; j < (int)m_basic_blocks.size(); ++j) {
                    BasicBlock * next_block = m_basic_blocks[j];
                    if (next_block->deleted)
                        continue;
                    fallthrough = block->jump_child == j;
                    block->fallthrough_child = j;
                    break;
                }
            }
            if (fallthrough) {
                InstructionList::iterator last = block->instructions.end();
                --last;
                block->instructions.erase(last);
                block->jump_child = -1;
                delete last_instruction;
            }
        }

        // delete empty blocks
        if (block->instructions.size() == 0)
            delete_block(i);
    }
}

void MethodGenerator::dependency_management() {
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
                        for (InstructionList::iterator it = block->instructions.begin(); it != block->instructions.end(); ++it) {
                            Instruction * instruction = *it;
                            instruction->insertReadRegisters(end_node->used_registers);
                        }
                    }
                    // reset state for next go around.
                    node->is_destination = false;
                    node->is_source = false;
                }
            }
        }

    }

    if (m_function_declaration->type != NULL) {
        // mark the return value as required
        m_basic_blocks[m_basic_blocks.size() - 1]->used_registers.insert(m_variable_numbers.get(m_function_declaration->identifier->text)._int);
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

                    // see if instruction is unecessary
                    if (copy_instruction->source.type == Variant::REGISTER && copy_instruction->source._int == dest_register) {
                        bool beg = (block->instructions.begin() == it);
                        block->instructions.erase(it);
                        if (beg)
                            it = block->instructions.begin();

                        delete instruction;
                        instruction = NULL;
                        break;
                    } else if (! block->used_registers.count(dest_register)) {
                        // delete because nothing depends on it
                        bool beg = (block->instructions.begin() == it);
                        block->instructions.erase(it);
                        if (beg)
                            it = block->instructions.begin();

                        delete instruction;
                        instruction = NULL;
                    } else {
                        // add the source to used set
                        copy_instruction->insertReadRegisters(block->used_registers);
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
                        operator_instruction->insertReadRegisters(block->used_registers);
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
                        unary_instruction->insertReadRegisters(block->used_registers);
                    }

                    // delete dest from used set
                    block->used_registers.erase(dest_register);


                    break;
                }
                case Instruction::PRINT:
                {
                    PrintInstruction * print_instruction = (PrintInstruction *) instruction;
                    print_instruction->insertReadRegisters(block->used_registers);
                    break;
                }
                case Instruction::IF:
                {
                    IfInstruction * if_instruction = (IfInstruction *) instruction;
                    if_instruction->insertReadRegisters(block->used_registers);
                    break;
                }
                case Instruction::GOTO:
                    break;
                case Instruction::RETURN:
                    break;
                case Instruction::NON_VOID_METHOD_CALL:
                case Instruction::METHOD_CALL:
                {
                    MethodCallInstruction * method_call_instruction = (MethodCallInstruction *) instruction;
                    method_call_instruction->insertReadRegisters(block->used_registers);
                    break;
                }
                case Instruction::ALLOCATE_OBJECT:
                    break;
                case Instruction::ALLOCATE_ARRAY:
                    break;
                case Instruction::WRITE_POINTER:
                {
                    WritePointerInstruction * write_pointer_instruction = (WritePointerInstruction *) instruction;
                    write_pointer_instruction->insertReadRegisters(block->used_registers);
                    break;
                }
                case Instruction::READ_POINTER:
                {
                    ReadPointerInstruction * read_pointer_instruction = (ReadPointerInstruction *) instruction;
                    read_pointer_instruction->insertReadRegisters(block->used_registers);
                    break;
                }
            }
        }
    }

}

MethodGenerator::CopyInstruction * MethodGenerator::constant_expression_evaluated(BasicBlock * block, UnaryInstruction * instruction) {
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


MethodGenerator::CopyInstruction * MethodGenerator::constant_expression_evaluated(BasicBlock * block, OperatorInstruction * instruction) {
    if (instruction->left.type == Variant::REGISTER || instruction->right.type == Variant::REGISTER)
        return NULL;

    assert(instruction->left.type == instruction->right.type);

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

MethodGenerator::CopyInstruction * MethodGenerator::make_copy(BasicBlock * block, OperatorInstruction * operator_instruction) {
    CopyInstruction * copy_instruction = new CopyInstruction(operator_instruction->dest, operator_instruction->left);
    delete operator_instruction;
    block->value_numbers.associate(copy_instruction->dest._int, get_value_number(block, copy_instruction->source));
    return copy_instruction;
}

bool MethodGenerator::operands_same(OperatorInstruction * instruction) {
    return (instruction->left.type == Variant::REGISTER && instruction->right.type == Variant::REGISTER &&
            instruction->left._int == instruction->right._int);
}

bool MethodGenerator::right_constant_is(OperatorInstruction * instruction, int constant) {
    return (instruction->right.type == Variant::CONST_INT && instruction->right._int == constant) ||
           (instruction->right.type == Variant::CONST_REAL && instruction->right._float == (float)constant) ||
           (instruction->right.type == Variant::CONST_BOOL && instruction->right._bool == (bool)constant);
}

bool MethodGenerator::left_constant_is(OperatorInstruction * instruction, int constant) {
    return (instruction->left.type == Variant::CONST_INT && instruction->left._int == constant) ||
           (instruction->left.type == Variant::CONST_REAL && instruction->left._float == (float)constant) ||
           (instruction->left.type == Variant::CONST_BOOL && instruction->left._bool == (bool)constant);
}

MethodGenerator::CopyInstruction * MethodGenerator::make_immediate(BasicBlock *block, UnaryInstruction *unary_instruction, int constant) {
    CopyInstruction * copy_instruction = new CopyInstruction(unary_instruction->dest, Variant(constant, Variant::CONST_INT));
    delete unary_instruction;
    block->value_numbers.associate(copy_instruction->dest._int, get_value_number(block, copy_instruction->source));
    return copy_instruction;
}

MethodGenerator::CopyInstruction * MethodGenerator::make_immediate(BasicBlock *block, UnaryInstruction *unary_instruction, float constant) {
    CopyInstruction * copy_instruction = new CopyInstruction(unary_instruction->dest, Variant(constant));
    delete unary_instruction;
    block->value_numbers.associate(copy_instruction->dest._int, get_value_number(block, copy_instruction->source));
    return copy_instruction;
}

MethodGenerator::CopyInstruction * MethodGenerator::make_immediate(BasicBlock *block, UnaryInstruction *unary_instruction, bool constant) {
    CopyInstruction * copy_instruction = new CopyInstruction(unary_instruction->dest, Variant(constant));
    delete unary_instruction;
    block->value_numbers.associate(copy_instruction->dest._int, get_value_number(block, copy_instruction->source));
    return copy_instruction;
}

MethodGenerator::CopyInstruction * MethodGenerator::make_immediate(BasicBlock * block, OperatorInstruction * operator_instruction, int constant) {
    CopyInstruction * copy_instruction = new CopyInstruction(operator_instruction->dest, Variant(constant, Variant::CONST_INT));
    delete operator_instruction;
    block->value_numbers.associate(copy_instruction->dest._int, get_value_number(block, copy_instruction->source));
    return copy_instruction;
}

MethodGenerator::CopyInstruction * MethodGenerator::make_immediate(BasicBlock * block, OperatorInstruction * operator_instruction, float constant) {
    CopyInstruction * copy_instruction = new CopyInstruction(operator_instruction->dest, Variant(constant));
    delete operator_instruction;
    block->value_numbers.associate(copy_instruction->dest._int, get_value_number(block, copy_instruction->source));
    return copy_instruction;
}

MethodGenerator::CopyInstruction * MethodGenerator::make_immediate(BasicBlock * block, OperatorInstruction * operator_instruction, bool constant) {
    CopyInstruction * copy_instruction = new CopyInstruction(operator_instruction->dest, Variant(constant));
    delete operator_instruction;
    block->value_numbers.associate(copy_instruction->dest._int, get_value_number(block, copy_instruction->source));
    return copy_instruction;
}

MethodGenerator::Instruction * MethodGenerator::constant_folded(BasicBlock * block, OperatorInstruction * instruction) {
    // we know that constants are on the right
    switch (instruction->_operator) {
        case OperatorInstruction::PLUS:
        {
            // a + 0 = a
            if (right_constant_is(instruction, 0))
                return make_copy(block, instruction);
            break;
        }
        case OperatorInstruction::MINUS:
        {
            // b := a - 0;
            // b := a - a;
            // 0 - a = -a
            if (right_constant_is(instruction, 0))
                return make_copy(block, instruction);
            else if (operands_same(instruction))
                return make_immediate(block, instruction, 0);
            else if (left_constant_is(instruction, 0)) {
                UnaryInstruction * unary_instruction = new UnaryInstruction(instruction->dest, UnaryInstruction::NEGATE, instruction->right);
                delete instruction;
                return unary_instruction;
            }

            break;
        }
        case OperatorInstruction::TIMES:
        {
            // a * 1 = a
            // b := a * 0;
            // b := a * 2;
            if (right_constant_is(instruction, 1))
                return make_copy(block, instruction);
            else if (right_constant_is(instruction, 0))
                return make_immediate(block, instruction, 0);
            else if (right_constant_is(instruction, 2)) {
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
            // 0 / a = 0
            if (right_constant_is(instruction, 1))
                return make_copy(block, instruction);
            else if (operands_same(instruction))
                return make_immediate(block, instruction, 1);
            else if (left_constant_is(instruction, 0))
                return make_immediate(block, instruction, 0);
            break;
        }
        case OperatorInstruction::AND:
        {
            // d := c and c;
            // a and false -> false
            if (operands_same(instruction))
                return make_copy(block, instruction);
            else if (right_constant_is(instruction, false))
                return make_immediate(block, instruction, false);
            break;
        }
        case OperatorInstruction::OR:
        {
            // d := c or c;
            // a or true -> true
            if (operands_same(instruction))
                return make_copy(block, instruction);
            else if (right_constant_is(instruction, true))
                return make_immediate(block, instruction, true);
            break;
        }
        case OperatorInstruction::MOD:
        {
            // 0 % a = 0
            // a % a = 0
            if (left_constant_is(instruction, 0))
                return make_immediate(block, instruction, 0);
            else if (operands_same(instruction))
                return make_immediate(block, instruction, 0);
        }
        default:
            break;
    }
    return instruction;
}

std::string MethodGenerator::hash_operand(BasicBlock * block, Variant operand) {
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

std::string MethodGenerator::hash_operator_instruction(BasicBlock * block, OperatorInstruction * instruction) {
    std::stringstream ss;
    ss << hash_operand(block, instruction->left);
    ss << " " << instruction->operator_str() << " ";
    ss << hash_operand(block, instruction->right);

    return ss.str();
}

std::string MethodGenerator::next_unique_value() {
    std::stringstream ss;
    ss << "?" << m_unique_value_count++;
    return ss.str();
}

MethodGenerator::Variant MethodGenerator::inline_value(BasicBlock * block, Variant register_or_const) {
    if (register_or_const.type == Variant::REGISTER) {
        Variant value = block->value_numbers.get(register_or_const._int);
        if (value.type == Variant::VALUE_NUMBER) {
            std::list<int> * registers = block->value_numbers.keys(value);
            int lowest = *(registers->begin());
            return Variant(lowest, Variant::REGISTER);
        } else {
            return value;
        }
    } else {
        return register_or_const;
    }
}

MethodGenerator::Variant MethodGenerator::get_value_number(BasicBlock * block, Variant register_or_const) {
    if (register_or_const.type == Variant::REGISTER)
        return block->value_numbers.get(register_or_const._int);
    else
        return register_or_const;
}

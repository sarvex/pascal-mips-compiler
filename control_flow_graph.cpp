#include "control_flow_graph.h"

class ControlFlowGraphBuilder {
private:
    FunctionDeclaration * m_function_declaration;
    BasicBlock * m_enter;
    BasicBlock * m_exit;

    void scan_statement_list(StatementList * _statement_list, BasicBlock * previous, BasicBlock * next);
public:
    ControlFlowGraphBuilder(FunctionDeclaration * function_declaration);
    ControlFlowGraph * build();
};

std::vector<ControlFlowGraph *> * build_control_flow_graphs(Program * program) {
    std::vector<ControlFlowGraph *> * graphs = new std::vector<ControlFlowGraph *>();
    for (ClassList * class_list = program->class_list; class_list != NULL; class_list = class_list->next) {
        ClassDeclaration * class_declaration = class_list->item;
        for (FunctionDeclarationList * function_list = class_declaration->class_block->function_list; function_list != NULL; function_list = function_list->next) {
            FunctionDeclaration * function_declaration = function_list->item;
            ControlFlowGraphBuilder builder(function_declaration);
            graphs->push_back(builder.build());
        }
    }

    return graphs;
}

ControlFlowGraphBuilder::ControlFlowGraphBuilder(FunctionDeclaration * function_declaration) :
    m_function_declaration(function_declaration),
    m_enter(new BasicBlock()),
    m_exit(new BasicBlock())
{
}

ControlFlowGraph * ControlFlowGraphBuilder::build() {
    scan_statement_list(m_function_declaration->block->statement_list, m_enter, m_exit);
    return NULL;
}

void ControlFlowGraphBuilder::scan_statement_list(StatementList * _statement_list, BasicBlock * previous, BasicBlock * next) {
    BasicBlock * between = previous;
    for (StatementList * statement_list = _statement_list; statement_list != NULL; statement_list = statement_list->next) {
        // TODO
    }
}


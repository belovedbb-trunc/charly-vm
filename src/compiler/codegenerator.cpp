/*
 * This file is part of the Charly Virtual Machine (https://github.com/KCreate/charly-vm)
 *
 * MIT License
 *
 * Copyright (c) 2017 Leonard Schütz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cmath>
#include <iostream>

#include "codegenerator.h"

namespace Charly::Compilation {

InstructionBlock* CodeGenerator::compile(AST::AbstractNode* node) {
  this->visit_node(node);
  this->assembler->write_halt();

  // Codegen all blocks
  while (this->queued_blocks.size() > 0) {
    QueuedBlock& queued_block = this->queued_blocks.front();

    this->assembler->place_label(queued_block.label);
    this->visit_node(queued_block.block);

    this->queued_blocks.pop_front();
  }
  this->assembler->resolve_unresolved_label_references();
  return new InstructionBlock(*static_cast<InstructionBlock*>(this->assembler));
}

void CodeGenerator::reset() {
  this->assembler->reset();
  this->break_stack.clear();
  this->continue_stack.clear();
}

AST::AbstractNode* CodeGenerator::visit_block(AST::Block* node, VisitContinue cont) {
  (void)cont;

  for (auto& node : node->statements) {
    this->visit_node(node);

    // If the statement produces an expression, pop it off the stack now
    if (node->yields_value()) {
      this->assembler->write_pop();
    }
  }

  return node;
}

AST::AbstractNode* CodeGenerator::visit_if(AST::If* node, VisitContinue cont) {
  (void)cont;

  // Codegen the condition
  this->visit_node(node->condition);

  // Skip over the block if the condition was false
  Label end_block_label = this->assembler->reserve_label();
  this->assembler->write_branchunless_to_label(end_block_label);
  this->visit_node(node->then_block);
  this->assembler->place_label(end_block_label);

  return node;
}

AST::AbstractNode* CodeGenerator::visit_ifelse(AST::IfElse* node, VisitContinue cont) {
  (void)cont;

  // Codegen the condition
  this->visit_node(node->condition);

  // Skip over the block if the condition was false
  Label else_block_label = this->assembler->reserve_label();
  Label end_block_label = this->assembler->reserve_label();
  this->assembler->write_branchunless_to_label(else_block_label);
  this->visit_node(node->then_block);
  this->assembler->write_branch_to_label(end_block_label);
  this->assembler->place_label(else_block_label);
  this->visit_node(node->else_block);
  this->assembler->place_label(end_block_label);

  return node;
}

AST::AbstractNode* CodeGenerator::visit_unless(AST::Unless* node, VisitContinue cont) {
  (void)cont;

  // Codegen the condition
  this->visit_node(node->condition);

  // Skip over the block if the condition was false
  Label end_block_label = this->assembler->reserve_label();
  this->assembler->write_branchif_to_label(end_block_label);
  this->visit_node(node->then_block);
  this->assembler->place_label(end_block_label);

  return node;
}

AST::AbstractNode* CodeGenerator::visit_unlesselse(AST::UnlessElse* node, VisitContinue cont) {
  (void)cont;

  // Codegen the condition
  this->visit_node(node->condition);

  // Skip over the block if the condition was false
  Label else_block_label = this->assembler->reserve_label();
  Label end_block_label = this->assembler->reserve_label();
  this->assembler->write_branchunless_to_label(else_block_label);
  this->visit_node(node->then_block);
  this->assembler->write_branch_to_label(end_block_label);
  this->assembler->place_label(else_block_label);
  this->visit_node(node->else_block);
  this->assembler->place_label(end_block_label);

  return node;
}

AST::AbstractNode* CodeGenerator::visit_guard(AST::Guard* node, VisitContinue cont) {
  (void)cont;

  // Codegen the condition
  this->visit_node(node->condition);

  // Skip over the block if the condition was false
  Label end_block_label = this->assembler->reserve_label();
  this->assembler->write_branchif_to_label(end_block_label);
  this->visit_node(node->block);
  this->assembler->place_label(end_block_label);

  return node;
}

AST::AbstractNode* CodeGenerator::visit_while(AST::While* node, VisitContinue cont) {
  (void)cont;

  // Setup labels
  Label condition_label = this->assembler->place_label();
  Label break_label = this->assembler->reserve_label();
  this->break_stack.push_back(break_label);
  this->continue_stack.push_back(condition_label);

  // Condition codegen
  this->visit_node(node->condition);
  this->assembler->write_branchunless_to_label(break_label);

  // Block codegen
  this->visit_node(node->block);
  this->assembler->write_branch_to_label(condition_label);
  this->assembler->place_label(break_label);

  // Remove the break and continue labels from the stack again
  this->break_stack.pop_back();
  this->continue_stack.pop_back();

  return node;
}

AST::AbstractNode* CodeGenerator::visit_until(AST::Until* node, VisitContinue cont) {
  (void)cont;

  // Setup labels
  Label condition_label = this->assembler->place_label();
  Label break_label = this->assembler->reserve_label();
  this->break_stack.push_back(break_label);
  this->continue_stack.push_back(condition_label);

  // Condition codegen
  this->visit_node(node->condition);
  this->assembler->write_branchif_to_label(break_label);

  // Block codegen
  this->visit_node(node->block);
  this->assembler->write_branch_to_label(condition_label);
  this->assembler->place_label(break_label);

  // Remove the break and continue labels from the stack again
  this->break_stack.pop_back();
  this->continue_stack.pop_back();

  return node;
}

AST::AbstractNode* CodeGenerator::visit_loop(AST::Loop* node, VisitContinue cont) {
  (void)cont;

  // Setup labels
  Label block_label = this->assembler->place_label();
  Label break_label = this->assembler->reserve_label();
  this->break_stack.push_back(break_label);
  this->continue_stack.push_back(block_label);

  // Block codegen
  this->visit_node(node->block);
  this->assembler->write_branch_to_label(block_label);
  this->assembler->place_label(break_label);

  // Remove the break and continue labels from the stack again
  this->break_stack.pop_back();
  this->continue_stack.pop_back();

  return node;
}

AST::AbstractNode* CodeGenerator::visit_unary(AST::Unary* node, VisitContinue cont) {
  (void)cont;

  // Codegen expression
  this->visit_node(node->expression);
  this->assembler->write_operator(kOperatorOpcodeMapping[node->operator_type]);

  return node;
}

AST::AbstractNode* CodeGenerator::visit_binary(AST::Binary* node, VisitContinue cont) {
  (void)cont;

  // Codegen expression
  this->visit_node(node->left);
  this->visit_node(node->right);
  this->assembler->write_operator(kOperatorOpcodeMapping[node->operator_type]);

  return node;
}

AST::AbstractNode* CodeGenerator::visit_switch(AST::Switch* node, VisitContinue cont) {
  (void)cont;

  // Setup labels
  Label end_label = this->assembler->reserve_label();
  Label default_block = this->assembler->reserve_label();
  this->break_stack.push_back(end_label);

  // Codegen switch condition
  this->visit_node(node->condition);

  // Codegen each switch node
  for (auto n : node->cases->children) {
    // Check if this is a switchnode (it should be)
    if (n->type() != AST::kTypeSwitchNode) {
      this->fatal_error(n, "Expected node to be a SwitchNode");
    }

    AST::SwitchNode* snode = AST::cast<AST::SwitchNode>(n);

    // Label to go check for the next condition
    Label next_condition_label = this->assembler->reserve_label();

    // Label of the block which runs if this node is selected
    Label node_block = this->assembler->reserve_label();

    // Codegen each condition
    for (auto c : snode->conditions->children) {
      this->assembler->write_dup();
      this->visit_node(c);
      this->assembler->write_operator(Opcode::Eq);
      this->assembler->write_branchif_to_label(node_block);
    }

    // Jump to the next node
    this->assembler->write_branch_to_label(next_condition_label);

    // Codegen this node's block
    this->assembler->place_label(node_block);

    // Pop the condition off the stack
    this->assembler->write_pop();
    this->visit_node(snode->block);
    this->assembler->write_branch_to_label(end_label);

    // Beginning of the next condition, the default block of the end of the statement
    this->assembler->place_label(next_condition_label);
  }

  // Codegen default block if there is one
  this->assembler->place_label(default_block);
  if (node->default_block->type() != AST::kTypeEmpty) {
    this->visit_node(node->default_block);
  } else {
    this->assembler->write_pop();
  }
  this->assembler->place_label(end_label);

  this->break_stack.pop_back();

  return node;
}

AST::AbstractNode* CodeGenerator::visit_and(AST::And* node, VisitContinue cont) {
  (void)cont;

  // Label setup
  Label end_and_label = this->assembler->reserve_label();

  // Codegen expressions
  this->visit_node(node->left);
  this->assembler->write_dup();
  this->assembler->write_branchunless_to_label(end_and_label);
  this->assembler->write_pop();
  this->visit_node(node->right);

  this->assembler->place_label(end_and_label);

  return node;
}

AST::AbstractNode* CodeGenerator::visit_or(AST::Or* node, VisitContinue cont) {
  (void)cont;

  // Label setup
  Label end_or_label = this->assembler->reserve_label();

  // Codegen expressions
  this->visit_node(node->left);
  this->assembler->write_dup();
  this->assembler->write_branchif_to_label(end_or_label);
  this->assembler->write_pop();
  this->visit_node(node->right);

  this->assembler->place_label(end_or_label);

  return node;
}

AST::AbstractNode* CodeGenerator::visit_typeof(AST::Typeof* node, VisitContinue cont) {
  cont();
  this->assembler->write_typeof();
  return node;
}

AST::AbstractNode* CodeGenerator::visit_assignment(AST::Assignment* node, VisitContinue cont) {
  // Check if we have the offset info for this identifier
  if (node->offset_info == nullptr) {
    this->fatal_error(node, "Missing offset info for assignment codegen");
  }

  // Codegen assignment
  cont();
  this->assembler->write_setlocal(node->offset_info->level, node->offset_info->index);

  return node;
}

AST::AbstractNode* CodeGenerator::visit_memberassignment(AST::MemberAssignment* node, VisitContinue cont) {
  (void)cont;

  // Codegen assignment
  this->visit_node(node->target);
  this->visit_node(node->expression);
  this->assembler->write_setmembersymbol(this->symtable(node->member));

  return node;
}

AST::AbstractNode* CodeGenerator::visit_andmemberassignment(AST::ANDMemberAssignment* node, VisitContinue cont) {
  (void)cont;

  // Codegen assignment
  this->visit_node(node->target);
  this->assembler->write_dup();
  this->assembler->write_readmembersymbol(this->symtable(node->member));
  this->visit_node(node->expression);
  this->assembler->write_operator(kOperatorOpcodeMapping[node->operator_type]);
  this->assembler->write_setmembersymbol(this->symtable(node->member));

  return node;
}

AST::AbstractNode* CodeGenerator::visit_indexassignment(AST::IndexAssignment* node, VisitContinue cont) {
  (void)cont;

  // Codegen assignment
  this->visit_node(node->target);
  this->visit_node(node->index);
  this->visit_node(node->expression);
  this->assembler->write_setmembervalue();

  return node;
}

AST::AbstractNode* CodeGenerator::visit_andindexassignment(AST::ANDIndexAssignment* node, VisitContinue cont) {
  (void)cont;

  this->visit_node(node->target);
  this->visit_node(node->index);
  this->assembler->write_topn(1);
  this->assembler->write_topn(1);
  this->visit_node(node->expression);
  this->assembler->write_operator(kOperatorOpcodeMapping[node->operator_type]);
  this->assembler->write_setmembervalue();

  return node;
}

AST::AbstractNode* CodeGenerator::visit_call(AST::Call* node, VisitContinue cont) {
  (void)cont;

  // Codegen target
  this->visit_node(node->target);

  // Codegen arguments
  for (auto arg : node->arguments->children) {
    this->visit_node(arg);
  }

  this->assembler->write_call(node->arguments->children.size());

  return node;
}

AST::AbstractNode* CodeGenerator::visit_callmember(AST::CallMember* node, VisitContinue cont) {
  (void)cont;

  // Codegen target
  this->visit_node(node->context);

  // Codegen function
  this->assembler->write_dup();
  this->assembler->write_readmembersymbol(this->symtable(node->symbol));

  // Codegen arguments
  for (auto arg : node->arguments->children) {
    this->visit_node(arg);
  }

  this->assembler->write_callmember(node->arguments->children.size());

  return node;
}

AST::AbstractNode* CodeGenerator::visit_callindex(AST::CallIndex* node, VisitContinue cont) {
  (void)cont;

  // Codegen target
  this->visit_node(node->context);

  // Codegen function
  this->assembler->write_dup();
  this->visit_node(node->index);
  this->assembler->write_readmembervalue();

  // Codegen arguments
  for (auto arg : node->arguments->children) {
    this->visit_node(arg);
  }

  this->assembler->write_callmember(node->arguments->children.size());

  return node;
}

AST::AbstractNode* CodeGenerator::visit_identifier(AST::Identifier* node, VisitContinue cont) {
  (void)cont;

  // Check if we have the offset info for this identifier
  if (node->offset_info == nullptr) {
    this->fatal_error(node, "Missing offset info for identifier codegen");
  }

  this->assembler->write_readlocal(node->offset_info->level, node->offset_info->index);

  return node;
}

AST::AbstractNode* CodeGenerator::visit_indexintoarguments(AST::IndexIntoArguments* node, VisitContinue cont) {
  (void)cont;
  this->assembler->write_readlocal(0, 0);
  this->assembler->write_readarrayindex(node->index);
  return node;
}

AST::AbstractNode* CodeGenerator::visit_self(AST::Self* node, VisitContinue cont) {
  (void)cont;
  this->assembler->write_putself();
  return node;
}

AST::AbstractNode* CodeGenerator::visit_member(AST::Member* node, VisitContinue cont) {
  (void)cont;

  // Codegen target
  this->visit_node(node->target);
  this->assembler->write_readmembersymbol(this->symtable(node->symbol));

  return node;
}

AST::AbstractNode* CodeGenerator::visit_index(AST::Index* node, VisitContinue cont) {
  (void)cont;

  // Codegen target
  this->visit_node(node->target);
  this->visit_node(node->argument);
  this->assembler->write_readmembervalue();

  return node;
}

AST::AbstractNode* CodeGenerator::visit_null(AST::Null* node, VisitContinue cont) {
  (void)cont;
  this->assembler->write_putvalue(kNull);
  return node;
}

AST::AbstractNode* CodeGenerator::visit_nan(AST::Nan* node, VisitContinue cont) {
  (void)cont;
  this->assembler->write_putfloat(NAN);
  return node;
}

AST::AbstractNode* CodeGenerator::visit_string(AST::String* node, VisitContinue cont) {
  (void)cont;
  this->assembler->write_putstring(node->value);
  return node;
}

AST::AbstractNode* CodeGenerator::visit_integer(AST::Integer* node, VisitContinue cont) {
  (void)cont;
  this->assembler->write_putvalue(VALUE_ENCODE_INTEGER(node->value));
  return node;
}

AST::AbstractNode* CodeGenerator::visit_float(AST::Float* node, VisitContinue cont) {
  (void)cont;
  this->assembler->write_putfloat(node->value);
  return node;
}

AST::AbstractNode* CodeGenerator::visit_boolean(AST::Boolean* node, VisitContinue cont) {
  (void)cont;
  this->assembler->write_putvalue(node->value ? kTrue : kFalse);
  return node;
}

AST::AbstractNode* CodeGenerator::visit_array(AST::Array* node, VisitContinue cont) {
  (void)cont;

  // Codegen array expressions
  for (auto child : node->expressions->children) {
    this->visit_node(child);
  }
  this->assembler->write_putarray(node->expressions->children.size());
  return node;
}

AST::AbstractNode* CodeGenerator::visit_hash(AST::Hash* node, VisitContinue cont) {
  (void)cont;

  // Codegen hash key and values expressions
  for (auto& pair : node->pairs) {
    this->assembler->write_putvalue(this->symtable(pair.first));
    this->visit_node(pair.second);
  }
  this->assembler->write_puthash(node->pairs.size());
  return node;
}

AST::AbstractNode* CodeGenerator::visit_function(AST::Function* node, VisitContinue cont) {
  (void)cont;

  // Label setup
  Label function_block_label = this->assembler->reserve_label();

  this->assembler->write_putfunction_to_label(this->symtable(node->name), function_block_label, node->anonymous,
                                              node->parameters.size(), node->lvar_count);

  // Codegen the block
  this->queued_blocks.push_back(QueuedBlock({function_block_label, node->body}));

  return node;
}

AST::AbstractNode* CodeGenerator::visit_class(AST::Class* node, VisitContinue cont) {
  (void)cont;

  // Codegen all regular and static members
  for (auto& n : node->member_properties) {
    this->assembler->write_putvalue(this->symtable(n));
  }
  for (auto& n : node->static_properties) {
    this->assembler->write_putvalue(this->symtable(n));
  }
  for (auto n : node->member_functions->children) {
    this->visit_node(n);
  }
  for (auto n : node->static_functions->children) {
    this->visit_node(n);
  }
  for (auto n : node->parents->children) {
    this->visit_node(n);
  }
  if (node->constructor->type() != AST::kTypeEmpty) {
    this->visit_node(node->constructor);
  }

  this->assembler->write_putclass(this->symtable(node->name), node->member_properties.size(),
                                  node->static_properties.size(), node->member_functions->children.size(),
                                  node->member_functions->children.size(), node->parents->children.size(),
                                  node->constructor->type() != AST::kTypeEmpty);

  return node;
}

AST::AbstractNode* CodeGenerator::visit_return(AST::Return* node, VisitContinue cont) {
  cont();
  this->assembler->write_return();
  return node;
}

AST::AbstractNode* CodeGenerator::visit_throw(AST::Throw* node, VisitContinue cont) {
  cont();
  this->assembler->write_throw();
  return node;
}

AST::AbstractNode* CodeGenerator::visit_break(AST::Break* node, VisitContinue cont) {
  (void)cont;

  // Check if there is a label for the break instruction
  if (this->break_stack.size() == 0) {
    this->fatal_error(node, "Break has no jump target.");
  }

  this->assembler->write_branch_to_label(this->break_stack.back());
  return node;
}

AST::AbstractNode* CodeGenerator::visit_continue(AST::Continue* node, VisitContinue cont) {
  (void)cont;

  // Check if there is a label for the continue instruction
  if (this->continue_stack.size() == 0) {
    this->fatal_error(node, "Continue has no jump target.");
  }

  this->assembler->write_branch_to_label(this->continue_stack.back());
  return node;
}

AST::AbstractNode* CodeGenerator::visit_trycatch(AST::TryCatch* node, VisitContinue cont) {
  (void)cont;

  // Implementation of this method was inspired by:
  // http://lists.llvm.org/pipermail/llvm-dev/2008-April/013978.html

  // Check if we have the offset_info for the exception name
  if (node->offset_info == nullptr) {
    this->fatal_error(node, "Missing offset info for exception identifier");
  }

  // Label setup
  Label end_statement_label = this->assembler->reserve_label();
  Label handler_label = this->assembler->reserve_label();
  Label finally_label = this->assembler->reserve_label();

  // Codegen try block
  this->assembler->write_registercatchtable_to_label(handler_label);
  this->visit_node(node->block);
  this->assembler->write_branch_to_label(finally_label);

  // Codegen handler block
  // If we don't have a handler block, we treat this try catch statement
  // as a cleanup landing pad and rethrow the exception after executing the finally block
  this->assembler->place_label(handler_label);
  if (node->handler_block->type() != AST::kTypeEmpty) {
    this->visit_node(node->handler_block);

    // We don't emit a branch here because the end statement and finally block labels
    // would we generated after this node anyway.
  } else {
    if (node->finally_block->type() == AST::kTypeEmpty) {
      this->fatal_error(node, "Can't codegen try/catch statement with neither a handler nor finally block");
    }

    // Store the exception
    this->assembler->write_setlocal(node->offset_info->level, node->offset_info->index);
    this->visit_node(node->finally_block);
    this->assembler->write_readlocal(node->offset_info->level, node->offset_info->index);
    this->assembler->write_throw();
  }

  // Codegen finally block
  this->assembler->place_label(finally_label);
  if (node->finally_block->type() != AST::kTypeEmpty) {
    this->visit_node(node->finally_block);
  }
  this->assembler->place_label(end_statement_label);

  return node;
}

}  // namespace Charly::Compilation

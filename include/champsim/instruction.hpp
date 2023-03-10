#ifndef CHAMPSIM_INSTRUCTION_HPP_
#define CHAMPSIM_INSTRUCTION_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <vector>

namespace champsim {

// Instruction format
constexpr int NUM_INSTR_DESTINATIONS_SPARC = 4;
constexpr int NUM_INSTR_DESTINATIONS = 2;
constexpr int NUM_INSTR_SOURCES = 4;

// Special registers that help us identify branches
constexpr uint8_t REG_STACK_POINTER = 6;
constexpr uint8_t REG_FLAGS = 25;
constexpr uint8_t REG_INSTRUCTION_POINTER = 26;

// Branch Types
constexpr int NOT_BRANCH = 0;
constexpr int BRANCH_DIRECT_JUMP = 1;
constexpr int BRANCH_INDIRECT = 2;
constexpr int BRANCH_CONDITIONAL = 3;
constexpr int BRANCH_DIRECT_CALL = 4;
constexpr int BRANCH_INDIRECT_CALL = 5;
constexpr int BRANCH_RETURN = 6;
constexpr int BRANCH_OTHER = 7;

struct input_instr {
  uint64_t ip = 0;
  uint8_t is_branch = 0;
  uint8_t branch_taken = 0;
  uint8_t destination_registers[NUM_INSTR_DESTINATIONS] = {};
  uint8_t source_registers[NUM_INSTR_SOURCES] = {};
  uint64_t destination_memory[NUM_INSTR_DESTINATIONS] = {};
  uint64_t source_memory[NUM_INSTR_SOURCES] = {};
};

struct cloudsuite_instr {
  uint64_t ip = 0;
  uint8_t is_branch = 0;
  uint8_t branch_taken = 0;
  uint8_t destination_registers[NUM_INSTR_DESTINATIONS_SPARC] = {};
  uint8_t source_registers[NUM_INSTR_SOURCES] = {};
  uint64_t destination_memory[NUM_INSTR_DESTINATIONS_SPARC] = {};
  uint64_t source_memory[NUM_INSTR_SOURCES] = {};
  uint8_t asid[2] = {std::numeric_limits<uint8_t>::max(),
                     std::numeric_limits<uint8_t>::max()};
};

struct instruction {
  uint64_t ip;
  uint64_t branch_target;
  bool is_branch;
  bool branch_taken;
  uint8_t branch_type;
  std::vector<uint8_t> destination_registers = {};
  std::vector<uint8_t> source_registers = {};
};

template <typename T>
instruction parse_instruction(T trace_instr) {
  instruction instr{.ip = trace_instr.ip,
              .branch_target = 0,
              .is_branch = !!trace_instr.is_branch,
              .branch_taken = !!trace_instr.branch_taken,
              .branch_type = NOT_BRANCH,
              .destination_registers = {},
              .source_registers = {}};
  std::remove_copy(std::begin(trace_instr.destination_registers),
                   std::end(trace_instr.destination_registers),
                   std::back_inserter(instr.destination_registers), 0);
  std::remove_copy(std::begin(trace_instr.source_registers),
                   std::end(trace_instr.source_registers),
                   std::back_inserter(instr.source_registers), 0);
  return instr;
}

}  // namespace champsim

#endif  // CHAMPSIM_INSTRUCTION_HPP_

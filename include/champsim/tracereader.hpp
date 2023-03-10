#ifndef CHAMPSIM_TRACEREADER_HPP_
#define CHAMPSIM_TRACEREADER_HPP_

#include <cstdio>
#include <optional>
#include <string>

#include "instruction.hpp"

namespace champsim {

class tracereader {
 public:
  tracereader(const tracereader& other) = delete;
  tracereader(std::string trace_file);
  ~tracereader();

  void open(const std::string& trace_string);
  void close();
  bool eof() const;

  template <typename T>
  instruction read_single_instr();

  virtual instruction get() = 0;

 protected:
  FILE* trace_file = NULL;
  std::string cmd_fmtstr;
  std::string decomp_program;
  std::string trace_string;
};

class cloudsuite_tracereader : public tracereader {
  std::optional<instruction> last_instr;

 public:
  cloudsuite_tracereader(std::string _tn) : tracereader(_tn) {}

  instruction read_single_instr() {
    cloudsuite_instr trace_read_instr;
    if (!fread(&trace_read_instr, sizeof(cloudsuite_instr), 1, trace_file)) {
      return last_instr.value();
    }
    return parse_instruction(trace_read_instr);
  }

  instruction get() {
    instruction trace_read_instr = read_single_instr();
    if (!last_instr.has_value()) {
      last_instr = trace_read_instr;
      trace_read_instr = read_single_instr();
    }
    last_instr->branch_target = trace_read_instr.ip;
    instruction retval = last_instr.value();
    last_instr = trace_read_instr;
    return retval;
  }
};

class input_tracereader : public tracereader {
  std::optional<instruction> last_instr;

 public:
  input_tracereader(std::string _tn) : tracereader(_tn) {}

  instruction read_single_instr() {
    input_instr trace_read_instr;
    if (!fread(&trace_read_instr, sizeof(input_instr), 1, trace_file)) {
      return last_instr.value();
    }
    return parse_instruction(trace_read_instr);
  }

  instruction get() {
    instruction trace_read_instr = read_single_instr();
    if (!last_instr.has_value()) {
      last_instr = trace_read_instr;
      trace_read_instr = read_single_instr();
    }
    last_instr->branch_target = trace_read_instr.ip;
    instruction retval = last_instr.value();
    last_instr = trace_read_instr;
    return retval;
  }
};

}  // namespace champsim

#endif  // CHAMPSIM_TRACEREADER_HPP_

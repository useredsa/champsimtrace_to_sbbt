#include <cassert>
#include <iomanip>
#include <iostream>

#include "champsim/tracereader.hpp"
#include "mbp/sim/sbbt_writer.hpp"

using std::cerr;
using std::dec;
using std::endl;
using std::hex;
using std::setfill;
using std::setw;
using std::stoull;
using std::string;
constexpr auto JUMP = mbp::SbbtWriter::JUMP;
constexpr auto CALL = mbp::SbbtWriter::CALL;
constexpr auto RET = mbp::SbbtWriter::RET;
constexpr auto TYPE = mbp::SbbtWriter::TYPE;
constexpr auto CND = mbp::SbbtWriter::CND;
constexpr auto IND = mbp::SbbtWriter::IND;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <input_trace> <output_trace>\n";
    exit(1);
  }
  const string input_trace = argv[1];
  const string outfile = std::string(argv[2]);

  // Open input file and output file
  champsim::input_tracereader cstraceReader(input_trace);
  mbp::SbbtWriter sbbtWriter(outfile);

  // Process body
  uint64_t instrNum = 0;
  uint64_t numBranchOther = 0;
  for (; !cstraceReader.eof(); instrNum += 1) {
    champsim::instruction cstraceInstr = cstraceReader.get();
    bool writesIp = false;
    bool writesSp = false;
    for (uint8_t reg : cstraceInstr.destination_registers) {
      if (reg == champsim::REG_INSTRUCTION_POINTER) {
        writesIp = true;
      } else if (reg == champsim::REG_STACK_POINTER) {
        writesSp = true;
      }
    }
    if (!writesIp) {
      assert(!cstraceInstr.is_branch);
      assert(cstraceInstr.branch_type == champsim::NOT_BRANCH);
      continue;
    }
    bool readsIp = false;
    bool readsSp = false;
    bool readsFlags = false;
    bool readsOther = false;
    for (uint8_t reg : cstraceInstr.source_registers) {
      if (reg == champsim::REG_INSTRUCTION_POINTER) {
        readsIp = true;
      } else if (reg == champsim::REG_STACK_POINTER) {
        readsSp = true;
      } else if (reg == champsim::REG_FLAGS) {
        readsFlags = true;
      } else if (reg != 0) {
        readsOther = true;
      }
    }

    unsigned opcode;
    if (!readsSp && !writesSp) {
      opcode = JUMP;
    } else if (readsIp) {
      opcode = CALL;
    } else {
      opcode = RET | IND;
    }
    if (readsFlags) opcode |= CND;
    else cstraceInstr.branch_taken = 1;
    if (readsOther) opcode |= IND;
    if (!cstraceInstr.branch_taken) cstraceInstr.branch_target = 0;

    if ((opcode & (CND | IND)) == (CND | IND) || opcode == (CALL | CND)) {
      numBranchOther += 1;
    }

    sbbtWriter.addBranch(instrNum, cstraceInstr.ip, cstraceInstr.branch_target,
                         cstraceInstr.branch_taken, opcode);

    if (sbbtWriter.numBranches() <= 20) {
      cerr << setw(11) << setfill(' ') << instrNum << " 0x" << setw(16)
           << setfill('0') << hex << cstraceInstr.ip << " 0x" << setw(16)
           << setfill('0') << cstraceInstr.branch_target << ' ' << dec
           << (opcode & IND ? "IND " : "DIR ")
           << (opcode & CND ? "CND " : "UCD ");
      switch (opcode & TYPE) {
        case JUMP:
          cerr << "JUMP";
          break;
        case CALL:
          cerr << "CALL";
          break;
        case RET:
          cerr << " RET";
          break;
        default:
          assert(false);
          break;
      }
      cerr << (cstraceInstr.branch_taken ? " TAKEN\n" : " NOT_TAKEN\n");
    }
  }
  cerr << "\n\nFinished translation.\n";
  cerr << "Trace: " << argv[1] << '\n';
  cerr << "Num. Instr.: " << instrNum << '\n';
  cerr << "Num. Br.: " << sbbtWriter.numBranches() << '\n';
  cerr << "Num. Branch Other.: " << numBranchOther << '\n';
  cerr << "Recompressing trace..." << endl;
  sbbtWriter.close(instrNum);
  cerr << "Translation Complete!" << endl;
}

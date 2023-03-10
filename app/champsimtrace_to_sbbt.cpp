#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <champsim/tracereader.hpp>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>

#ifdef __GNUC__
static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__,
              "System must be little endian\n");
#elif __clang__
static_assert(__LITTLE_ENDIAN__, "System must be little endian\n");
#else
#pragma message(                                 \
    "Cannot determine endianess of the system. " \
    "Check that the system is little endian.")
#endif

using std::cerr;
using std::dec;
using std::endl;
using std::hex;
using std::setfill;
using std::setw;
using std::stoull;
using std::string;

enum OpCode {
  // Number of possible opcodes.
  NUMBER = 0b10000,
  // Bitmask corresponding to the base type.
  TYPE = 0b1100,
  // Bit for conditional branches.
  CND = 0b0001,
  // Bit for indirect branches.
  IND = 0b0010,
  // JUMP base type.
  JUMP = 0b0000,
  // RET (return from function) base type.
  RET = 0b0100,
  // CALL (function) base type.
  CALL = 0b1000,
};
struct SbbtHeader {
  char s = 'S';
  char b0 = 'B';
  char b1 = 'B';
  char t = 'T';
  char nl = '\n';
  uint8_t vmajor = 1;
  uint8_t vminor = 0;
  uint8_t vpatch = 0;
  uint64_t numInstructions;
  uint64_t numBranches;
};
struct SbbtBranch {
  unsigned opcode : 4;
  unsigned padding : 7;
  unsigned outcome : 1;
  unsigned long long ip : 52;
  unsigned ninstr : 12;
  unsigned long long target : 52;
};
static_assert(sizeof(SbbtHeader) == 24);
static_assert(sizeof(SbbtBranch) == 16);

static uint64_t sign_unextend_ip(uint64_t ip) {
  constexpr uint64_t lastAndMask = ((1ULL << 13) - 1) << 51;
  constexpr uint64_t mask = ((1ULL << 12) - 1) << 52;
  assert((ip & lastAndMask) == lastAndMask || (ip & lastAndMask) == 0);
  return ip & ~mask;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <input_trace> <output_file_basename>\n";
    exit(1);
  }
  const string input_trace = argv[1];
  const string outfile = std::string(argv[2]) + ".sbbt.zst";
  const string tmpfile = outfile + ".tmp";

  // Open input file and output file
  champsim::input_tracereader cstraceReader(input_trace);
  string cmd = "zstd --ultra -22 -o " + tmpfile;
  FILE* pipe = popen(cmd.c_str(), "w");

  // Process body
  uint64_t instrNum = 0;
  uint64_t lastBranch = 0;
  uint64_t numBranches = 0;
  uint64_t numBranchOther = 0;
  for (; !cstraceReader.eof(); instrNum += 1) {
    champsim::instruction cst_instr = cstraceReader.get();
    bool writesIp = false;
    bool writesSp = false;
    for (uint8_t reg : cst_instr.destination_registers) {
      if (reg == champsim::REG_INSTRUCTION_POINTER) {
        writesIp = true;
      } else if (reg == champsim::REG_STACK_POINTER) {
        writesSp = true;
      }
    }
    if (!writesIp) {
      assert(!cst_instr.is_branch);
      assert(cst_instr.branch_type == champsim::NOT_BRANCH);
      continue;
    }
    numBranches += 1;
    bool readsIp = false;
    bool readsSp = false;
    bool readsFlags = false;
    bool readsOther = false;
    for (uint8_t reg : cst_instr.source_registers) {
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

    OpCode opcode = static_cast<OpCode>(0);
    if (!readsSp && !writesSp) {
      opcode = JUMP;
    } else if (readsIp) {
      opcode = CALL;
    } else {
      opcode = static_cast<OpCode>(RET | IND);
    }
    if (readsFlags) opcode = static_cast<OpCode>(opcode | CND);
    if (readsOther) opcode = static_cast<OpCode>(opcode | IND);
    if (!cst_instr.branch_taken) cst_instr.branch_target = 0;

    if ((opcode & (CND | IND)) == (CND | IND) ||
        opcode == static_cast<OpCode>(CALL | CND)) {
      numBranchOther += 1;
    }

    SbbtBranch binarybranch;
    binarybranch.outcome = cst_instr.branch_taken;
    binarybranch.padding = 0;
    binarybranch.opcode = opcode;
    binarybranch.ip = sign_unextend_ip(cst_instr.ip);
    binarybranch.ninstr = instrNum - lastBranch;
    binarybranch.target = sign_unextend_ip(cst_instr.branch_target);
    fwrite(&binarybranch, sizeof(binarybranch), 1, pipe);
    if (std::ferror(pipe)) {
      throw std::runtime_error(std::strerror(errno));
    }
    lastBranch = instrNum;

    if (numBranches <= 20) {
      cerr << setw(10) << setfill(' ') << instrNum << " 0x" << setw(8)
           << setfill('0') << hex << cst_instr.ip << " 0x" << setw(8)
           << setfill('0') << cst_instr.branch_target << ' ' << dec
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
      cerr << (cst_instr.branch_taken ? " TAKEN\n" : " NOT_TAKEN\n");
    }
  }
  cerr << "Trace: " << argv[1] << endl;
  cerr << "Num. Instr.: " << instrNum << '\n';
  cerr << "Num. Br.: " << numBranches << '\n';
  cerr << "Num. Branch Other.: " << numBranchOther << '\n';
  pclose(pipe);

  cmd = "zstd --decompress --stdout " + tmpfile;
  FILE* tmpfilePipe = popen(cmd.c_str(), "r");
  cmd = "zstd --ultra -22 -o " + outfile;
  FILE* outfilePipe = popen(cmd.c_str(), "w");
  SbbtHeader sbbtHeader;
  sbbtHeader.numInstructions = instrNum;
  sbbtHeader.numBranches = numBranches;
  fwrite(&sbbtHeader, sizeof(sbbtHeader), 1, outfilePipe);
  if (std::ferror(outfilePipe)) {
    throw std::runtime_error(std::strerror(errno));
  }

  static constexpr size_t READ_SIZE = 1 << 16;
  std::array<char, READ_SIZE> buffer;
  size_t read = 0;
  while ((read = fread(buffer.data(), 1, READ_SIZE, tmpfilePipe)) != 0) {
    fwrite(buffer.data(), 1, read, outfilePipe);
    if (std::ferror(tmpfilePipe) || std::ferror(outfilePipe)) {
      throw std::runtime_error(std::strerror(errno));
    }
  }
  if (std::ferror(tmpfilePipe) || std::ferror(outfilePipe)) {
    throw std::runtime_error(std::strerror(errno));
  }
  pclose(tmpfilePipe);
  pclose(outfilePipe);
  unlink(tmpfile.c_str());
}

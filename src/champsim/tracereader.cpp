#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>

#include "champsim/tracereader.hpp"

namespace champsim {

tracereader::tracereader(std::string _ts) : trace_string(_ts) {
  std::string last_dot = trace_string.substr(trace_string.find_last_of("."));
  if (trace_string.substr(0, 4) == "http") {
    // Check file exists
    char testfile_command[4096];
    sprintf(testfile_command, "wget -q --spider %s", trace_string.c_str());
    FILE* testfile = popen(testfile_command, "r");
    if (pclose(testfile)) {
      std::cerr << "TRACE FILE NOT FOUND" << std::endl;
      assert(0);
    }
    cmd_fmtstr = "wget -qO- -o /dev/null %2$s | %1$s -dc";
  } else {
    std::ifstream testfile(trace_string);
    if (!testfile.good()) {
      std::cerr << "TRACE FILE NOT FOUND" << std::endl;
      assert(0);
    }
    cmd_fmtstr = "%1$s -dc %2$s";
  }

  if (last_dot[1] == 'g')  // gzip format
    decomp_program = "gzip";
  else if (last_dot[1] == 'x')  // xz
    decomp_program = "xz";
  else {
    std::cout
        << "ChampSim does not support traces other than gz or xz compression!"
        << std::endl;
    assert(0);
  }

  open(trace_string);
}

tracereader::~tracereader() { close(); }

bool tracereader::eof() const { return feof(trace_file); }

void tracereader::open(const std::string& trace_string) {
  char gunzip_command[4096];
  sprintf(gunzip_command, cmd_fmtstr.c_str(), decomp_program.c_str(),
          trace_string.c_str());
  trace_file = popen(gunzip_command, "r");
  if (trace_file == NULL) {
    std::cerr << std::endl
              << "*** CANNOT OPEN TRACE FILE: " << trace_string << " ***"
              << std::endl;
    assert(0);
  }
}

void tracereader::close() {
  if (trace_file != NULL) {
    pclose(trace_file);
  }
}

}  // namespace champsim

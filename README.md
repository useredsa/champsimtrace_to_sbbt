<h1 align="center">
  ChampSim Trace to SBBT (<a href="https://github.com/useredsa/MBPlib">MBPlib's</a> format) Translator
</h1>

This repository contains the source code of the utility `champsim_trace_to_sbbt`, capable of translating program traces of type ChampSimTrace (the type of traces that the [ChampSim] simulator uses) to the format Simple Binary Branch Trace (SBBT) version 1.0.0 (used by [MBPlib]).

[MBPlib]: https://github.com/useredsa/MBPlib
[ChampSim]: https://github.com/ChampSim/ChampSim

## Compiling

This software should compile and run on any Unix platform. It has been tested under Arch Linux and Ubuntu 18.04. To compile the software you need to install [CMake] and use the following commands.
```sh
mkdir build && cd build
cmake ..
cmake --build .
```
The binary will be created in `/build/app/champsimtrace_to_sbbt`.

Alternatively (since it is only necessary to compile and link two files) you can use a C++ compiler yourself. The compilation flags can be taken from the `CMakeFiles.txt` files in each directory.

[CMake]: https://cmake.org/

## Running

The program receives two arguments, the original trace and the filename (with `.sbbt.zst` extension) of the new SBBT trace.
```sh
champsim_trace_to_sbbt <input_trace> <output_trace>
```

The translator uses [`zstd`] to compress the traces, which is the compression tool that has proven to give the best compression ratio and decompression speed for MBPlib's traces. Thus, you need to have [`zstd`] installed.

[`zstd`]: https://en.wikipedia.org/wiki/Zstd

## Considerations

### Consider Keeping ChampSim Traces

ChampSim's traces contain more information than SBBT traces. If you erase them after translation, you will not be able to recover them from the SBBT files.

### Branch Targets

In ChampSim the target of a conditional branch is 0 if the branch was not taken, even for direct branches, whose target is encoded in the instruction (and thus ould be known after the decode phase in a real processor). The same happens when you translate ChampSim traces using this utility: the resulting trace will have null targets for not taken direct branches.

### Branch Type Translation

This is the equivalence between ChampSim's branch types and SBBT opcodes.
| ChampSim's Branch Type | SBBT OpCode      |
| ---------------------- | ---------------- |
| BRANCH_DIRECT_JUMP     | JUMP             |
| BRANCH_INDIRECT        | INDIRECT JUMP    |
| BRANCH_CONDITIONAL     | CONDITIONAL JUMP |
| BRANCH_DIRECT_CALL     | CALL             |
| BRANCH_INDIRECT_CALL   | INDIRECT CALL    |
| BRANCH_RETURN          | RET              |
| BRANCH_OTHER           | Others, depending on registers accessed. For instance, CONDITIONAL INDIRECT JUMP |

## License

This software is based on ChampSim's trace reader, which has an Apache 2.0 license (at the moment of writing). To make things simple, this repository is also licensed under the [Apache 2.0 license].

[Apache 2.0 license]: /LICENSE

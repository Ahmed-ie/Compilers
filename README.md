# MiniCCompiler

Ahmed Elmi

This project implements a MiniC compiler in four stages:

1. `frontend/`: lexing, parsing, AST construction, and semantic analysis
2. `ir_builder/`: AST to LLVM IR
3. `optimizations/`: LLVM IR optimization passes
4. `backend/`: LLVM IR to x86 32-bit assembly

## Requirements

- `g++`
- `flex`
- `bison`
- LLVM development tools for `ir_builder`, `optimizations`, and `backend`
- `clang`
- Linux with `clang -m32` for the full backend runtime tests

The LLVM-based Makefiles try these automatically:

1. `/opt/homebrew/opt/llvm/bin/llvm-config`
2. `llvm-config`
3. `llvm-config-18`
4. `llvm-config-17`

## Build

From the project root:

```sh
make
```

This builds:

- `frontend/parser`
- `ir_builder/ir_builder`
- `optimizations/optimizer`
- `backend/backend`

## Stage Tests

From the project root:

```sh
make tests
```

or:

```sh
make test-all
```

to run the full suite, or run the stage tests individually:

```sh
make test-parser
make test-semantic
make test-builder
make test-optimizations
make test-backend
```

Notes:

- `test-backend` runs full runtime comparisons only on Linux with `clang -m32`
- On unsupported systems such as macOS, `test-backend` prints a skip message instead of failing

## Manual Pipeline

### Frontend

```sh
./frontend/parser path/to/program.c
```

### IR builder

```sh
./ir_builder/ir_builder ./tests/builder_tests/p1.c /tmp/p1.ll
```

### Optimizer

```sh
./optimizations/optimizer /tmp/p1.ll /tmp/p1_opt.ll
```

Local-only optimization mode:

```sh
./optimizations/optimizer --local ./tests/optimizations/cfold_add.ll /tmp/cfold_add_opt.ll
```

### Backend

```sh
./backend/backend /tmp/p1_opt.ll /tmp/p1.s
```

The backend input is LLVM IR (`.ll`). The output is AT&T-style x86 32-bit assembly (`.s`).

## Full Example

```sh
./ir_builder/ir_builder ./tests/assembly_gen_tests/square.c /tmp/square.ll
./optimizations/optimizer /tmp/square.ll /tmp/square_opt.ll
./backend/backend /tmp/square_opt.ll /tmp/square.s
```

On Linux, you can assemble and run it with:

```sh
clang -m32 /tmp/square.s ./tests/assembly_gen_tests/main.c
./a.out
```

## Project Layout

- `frontend/`: lexer, parser, AST library, semantic analysis
- `ir_builder/`: LLVM IR generation from the MiniC AST
- `optimizations/`: local and global LLVM IR optimization passes
- `backend/`: handwritten x86 code generation with local register allocation
- `tests/`: parser, semantic, builder, optimization, and assembly-generation tests

## Folder README Files

Each stage folder has its own README with:

- what that stage does
- how to build it directly
- how to run it manually
- how to test it with `make`

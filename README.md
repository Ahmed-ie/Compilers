

# MiniCCompiler
## Ahmed Elmi 

This project now contains the MiniC compiler pipeline through backend assembly generation.

## Requirements
- `g++`
- `flex`
- `bison`

## Build
Build everything from the project root:

```sh
make
```

This produces:
- `frontend/parser`
- `ir_builder/ir_builder`
- `optimizations/optimizer`
- `backend/backend`

## Run

Frontend parse + semantic analysis:
```sh
./frontend/parser path/to/file.c
```

LLVM IR generation:
```sh
./ir_builder/ir_builder ./tests/builder_tests/p1.c /tmp/p1.ll
```

Optimization:
```sh
./optimizations/optimizer /tmp/p1.ll /tmp/p1_opt.ll
```

Backend assembly generation:
```sh
./backend/backend /tmp/p1_opt.ll /tmp/p1.s
```

The backend consumes LLVM IR (`.ll`) and emits AT&T-style x86 32-bit assembly (`.s`).
It uses a custom basic-block-local register allocator over `ebx`, `ecx`, and `edx`, with
`eax` reserved for returns, call results, and temporary emission as required by the assignment.

## Tests
From the `frontend` directory:

```sh
make parser-tests
make semantic-tests
```

## Project structure
- `frontend/` core lexer, parser, AST, and semantic analysis
- `ir_builder/` AST-to-LLVM IR generation
- `optimizations/` LLVM IR optimization passes
- `backend/` LLVM IR to x86 assembly generation
- `tests/` sample parser and semantic test programs

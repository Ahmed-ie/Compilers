# Backend

This folder contains the MiniC backend.

The backend reads LLVM IR and emits handwritten x86 32-bit assembly.

Main responsibilities:

- assign stack offsets to values used during code generation
- compute local liveness information per basic block
- perform local linear-scan register allocation
- emit AT&T-style x86 assembly

## Build

From this folder:

```sh
make
```

Or from the project root:

```sh
make
```

## Run

From the project root:

```sh
./backend/backend ./tests/optimizations/cfold_add.ll /tmp/cfold_add.s
```

From this folder:

```sh
./backend ../tests/optimizations/cfold_add.ll /tmp/cfold_add.s
```

Input:

- LLVM IR (`.ll`)

Output:

- x86 32-bit assembly (`.s`)

## Test With Make

From the project root:

```sh
make test-backend
```

From this folder:

```sh
make test
```

This uses [testing.sh](/Users/ahmedelmi/Downloads/CS57 lectures/MiniCCompiler/backend/testing.sh).

Behavior:

- on Linux with `clang -m32`, it runs full backend runtime tests
- on unsupported systems, it prints a skip message and exits cleanly

## Manual Test Runs

Direct IR to assembly:

```sh
./backend ../tests/optimizations/p4_const_prop.ll /tmp/p4_const_prop.s
```

Full pipeline from MiniC source:

```sh
../ir_builder/ir_builder ../tests/assembly_gen_tests/square.c /tmp/square.ll
../optimizations/optimizer /tmp/square.ll /tmp/square_opt.ll
./backend /tmp/square_opt.ll /tmp/square.s
```

On Linux, assemble and run with:

```sh
clang -m32 /tmp/square.s ../tests/assembly_gen_tests/main.c
./a.out
```

## Notes

- The backend uses `%ebx`, `%ecx`, and `%edx` as allocatable registers
- `%eax` is used for return values and call results
- runtime backend tests are Linux-only because they rely on `clang -m32`

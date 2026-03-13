# IR Builder

This folder contains the LLVM IR builder for MiniC.

It takes the AST produced by the frontend and emits LLVM IR for the MiniC function.

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
./ir_builder/ir_builder ./tests/builder_tests/p1.c /tmp/p1.ll
```

From this folder:

```sh
./ir_builder ../tests/builder_tests/p1.c /tmp/p1.ll
```

Input:

- MiniC source file

Output:

- LLVM IR file (`.ll`)

## Test With Make

From the project root:

```sh
make test-builder
```

From this folder:

```sh
make test-builder
```

This uses [testing.sh](/Users/ahmedelmi/Downloads/CS57 lectures/MiniCCompiler/ir_builder/testing.sh) and checks that all builder test files produce non-empty `.ll` output.

## Manual Test Runs

Generate IR:

```sh
./ir_builder ../tests/builder_tests/p2.c /tmp/p2.ll
```

Inspect output:

```sh
sed -n '1,200p' /tmp/p2.ll
```

## Notes

- The builder emits declarations for `read` and `print`
- The MiniC function is emitted as `@func`
- Control flow is lowered into LLVM basic blocks and branches
- Local variables are represented with stack allocations and `load`/`store`


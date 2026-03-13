# Optimizations

This folder contains the LLVM IR optimizer for the MiniC pipeline.

The optimizer works on `.ll` files and supports:

- local common subexpression elimination
- constant folding
- dead code elimination
- global constant propagation

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
./optimizations/optimizer ./tests/optimizations/p4_const_prop.ll /tmp/p4_const_prop_opt.ll
```

From this folder:

```sh
./optimizer ../tests/optimizations/p4_const_prop.ll /tmp/p4_const_prop_opt.ll
```

If no output file is given, the optimized IR is written to stdout:

```sh
./optimizer ../tests/optimizations/p4_const_prop.ll
```

Local-only mode:

```sh
./optimizer --local ../tests/optimizations/cfold_add.ll /tmp/cfold_add_opt.ll
```

CLI:

```sh
./optimizer [--local] <input.ll> [output.ll]
```

## Test With Make

From the project root:

```sh
make test-optimizations
```

From this folder:

```sh
make test
```

This uses [testing.sh](/Users/ahmedelmi/Downloads/CS57 lectures/MiniCCompiler/optimizations/testing.sh) and compares optimizer output against the expected `*_opt.ll` files in [tests/optimizations](/Users/ahmedelmi/Downloads/CS57 lectures/MiniCCompiler/tests/optimizations).

## Manual Test Runs

Local tests:

```sh
./optimizer --local ../tests/optimizations/cfold_mul.ll /tmp/cfold_mul_opt.ll
```

Global tests:

```sh
./optimizer ../tests/optimizations/p5_const_prop.ll /tmp/p5_const_prop_opt.ll
```

## Notes

- `--local` skips global constant propagation
- the test script ignores expected LLVM metadata differences such as `ModuleID` and `PIC Level`
- the optimizer can print to stdout or write to a file directly


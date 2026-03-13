# Frontend

This folder contains the MiniC frontend:

- lexer (`lex.l`)
- parser (`yac.y`)
- AST code (`ast.c`, `ast.h`)
- semantic analysis (`semantic_analysis.c`)
- frontend driver (`main.c`)

The frontend takes a MiniC source file, builds an AST, and runs semantic analysis.

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
./frontend/parser ./tests/parser/p1.c
```

From this folder:

```sh
./parser ../tests/parser/p1.c
```

The parser driver:

- prints `Parsing successful.` when parsing succeeds
- prints semantic analysis results after that
- returns a nonzero exit code for parser or semantic errors

## Test With Make

From the project root:

```sh
make test-parser
make test-semantic
```

From this folder:

```sh
make parser-tests
make semantic-tests
```

These use [testing.sh](/Users/ahmedelmi/Downloads/CS57 lectures/MiniCCompiler/frontend/testing.sh) and print `PASS`/`FAIL` for each test file.

## Manual Test Runs

Parser examples:

```sh
./parser ../tests/parser/p1.c
./parser ../tests/parser/p_bad.c
```

Semantic examples:

```sh
./parser ../tests/semantic/p1_good.c
./parser ../tests/semantic/p1_bad.c
```

## Notes

- The MiniC grammar stays close to the assignment spec.
- Semantic analysis checks:
  - variables are declared before use
  - variables are not redeclared in the same scope


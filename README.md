

# MiniCCompiler
## Ahmed Elmi 

This here contains the first part of MiniC Compiler Project - a frontend that parses MiniC programs and runs semantic analysis.

## Requirements
- `g++`
- `flex`
- `bison`

## Build
From the `frontend` directory:

```sh
make
```

This produces the `parser` binary in `frontend/`.

## Run
From the `frontend` directory:

```sh
./parser path/to/file.c
```

Expected output:
- `Parsing successful.` on a valid parse
- `Semantic analysis successful.` when semantic checks pass

Non-zero exit codes indicate an error:
- `1`: input file could not be opened
- `2`: parsing failed
- `3`: semantic analysis failed

## Tests
From the `frontend` directory:

```sh
make parser-tests
make semantic-tests
```

## Project structure
- `frontend/` core lexer, parser, AST, and semantic analysis
- `tests/` sample parser and semantic test programs


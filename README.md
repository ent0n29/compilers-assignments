# Compilers Backend Assignments

Various optimizations passes leveraging the LLVM IR.

This repository contains a standalone version of the optimization passes, tested for LLVM 17.0.6. A full version of LLVM's source code, with our passes plugged-in, can be found [here](https://github.com/mc-cat-tty/llvm-17.0.6-compilers).

## Primo assignment

Implementare tre passi LLVM (dentro lo stesso passo LocalOpts già scritto durante il LAB 2) che realizzano le seguenti ottimizzazioni locali:
 
 1. Algebraic Identity
    - $x + 0 = 0 + x \Rightarrow x$
    - $x \times 1 = 1 \times x \Rightarrow x$

 2. Strength Reduction (più avanzato)
    - $15 \times x = x \times 15 \Rightarrow (x << 4) – x$
    - $y = x / 8 ⇒ y = x >> 3$

 3. Multi-Instruction Optimization	
    - $a = b + 1, c = a − 1 ⇒ a = b + 1, c = b$

## Testing Logic

### Directories
The `testing` directory tree looks like this:

```bash
testing
├── expected
│   ├── advanced_pow2_sr_expected.ll
│   ├── algebraic_id_expected.ll
│   ├── basic_pow2_sr_expected.ll
│   ├── edge_cases_expected.ll
│   └── multi_instr_expected.ll
├── optimized
│   ├── advanced_pow2_sr_optimized.ll
│   ├── algebraic_id_optimized.ll
│   ├── basic_pow2_sr_optimized.ll
│   ├── edge_cases_optimized.ll
│   └── multi_instr_optimized.ll
└── tests
    ├── advanced_pow2_sr.ll
    ├── algebraic_id.ll
    ├── basic_pow2_sr.ll
    ├── edge_cases.ll
    └── multi_instr.ll
```

Where:
- tests -> contains some samples of IR code to check if our opt passes work
- expected -> contains the `expected outcome`, our desired results that our opt passes should reach
- optimized -> contains the IR code generated with our opt passes
  
### Automated testing
the bash script `testing.sh` automatically generates all the `optimized.ll` and put them in `optimized` files for every test in `tests`.

It also checks if every `expected.ll` and `optimized.ll` are equal, and if it's true, the test is passed.

## Coding Style
### Naming Conventions
 - Type definitions (classes, structs, and so on) -> `PascalCase`
 - Methods -> `camelCase`
 - Variables -> `camelCase`

### Indentation
Tabs must be expanded as **2 spaces**.

## Contributors
 - Christofer Fanò [@ch-fano]
 - Francesco Mecatti [@mc-cat-tty]
 - Antonio Stano [@ent0n29]

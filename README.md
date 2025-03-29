
# Project structure:


- bin: main.cpp - sandbox
- lib: contains parser
- test/sources: contain test files in `.ct` extension

# Tests

### Compile (from root)

```
cmake .
```

### Testing (from root)

run main.cpp:
```
cmake --build . && ./bin/crypt
```

run tests:
```
cmake --build . --target parser_test && ctest -V
```

# Syntax and Language Rules

## General

1. Program execution starts with the `main` function. Programs without `main` are invalid.  
2. Program files must use UTF-8 encoding.  
3. Every statement must end with a semicolon (`;`).  

## Variables

1. Variable declaration follows the structure:  
   ```Type name = value;```  
2. Variable names may contain:  
   - English letters (A-Z, a-z)  
   - Digits (0-9)  
   - Symbols: `_`  
3. Names cannot start with a digit
4. register independent:`int var` and `int Var` is a different variables 

## Type: `int`

1. Size: 32 bits (4 bytes).  
2. Supported arithmetic operations: `+`, `-`, `*`, `/`.  

## Type: `array`

1. *To be implemented*.  

## Functions

1. Structure:  
   ```fn name(Type1 arg1, ...) >>> RetType { ... }```  
2. Call syntax:  
   ```name(args);```  
3. Recursion is supported.
4. Return from function with a keyword `return`
   
# Compilers Backend

Various optimizations passes leveraging the LLVM IR.

## Primo assignment

Implementare tre passi LLVM (dentro lo stesso passo LocalOpts già scritto durante il LAB 2) che realizzano le seguenti ottimizzazioni locali:
 
 1. Algebraic Identity

$x + 0 = 0 + x \Rightarrow x$
  
$x \times 1 = 1 \times x \Rightarrow x$

 2. Strength Reduction (più avanzato)
	
$15 \times x = x \times 15 \Rightarrow (x << 4) – x$

$y = x / 8 ⇒ y = x >> 3$

 3. Multi-Instruction Optimization
	
$a = b + 1, c = a − 1 ⇒ a = b + 1, c = b$

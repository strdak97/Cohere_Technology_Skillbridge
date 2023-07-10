# Lab1 DataLab

Do not spend more than 1 day on this lab. Check in with your instructors if you are stuck. You can skip the float problems.

When working on WSL on Windows, I need to remove the `-m32` flag
in the `Makefile` in order to compile. In addition, on WSL, running
`./btest` gives me full mark and running `./dlc bits.c` gives no
warning or violation, but `./driver.pl` gives me zero mark. This might
be because of the removal of the `-m32` flag in `Makefile`.

### Difficult problems
1. `conditional`  
    Reference: https://stackoverflow.com/questions/10034470/conditional-statement-using-bitwise-operators/10034623#10034623  
    Most important technique:
    
    1. Use !! to convert into either 0 or 1; This is actually the way to
    check if a value is zero or non-zero, and get a one-bit result.
    
    2. Then negate the above to make all bits 0 or 1.  
    `x == 0` -> `!!(x) == 0` -> `~!!(x) == 0 == 0x00000000`;  
    `x != 0` -> `!!(x) == 1` -> `~!!(x) == -1 == 0xffffffff`.  
  

2. `isLessOrEqual`  
    When subtracting two numbers, `x` and `y`. The result `x - y` could overflow. 
    The only case when `x - y` are guaranteed to not overflow is when `x` and `y`
    are of the same sign. Thus, sign checking is very important in this problem.  

3. `logicalNegate`   
    Use the technique called `bit smearing`. You do not need to fully 
    understand how it works, just use the effect.  
    Check the links:   
    1. https://stackoverflow.com/questions/53161/find-the-highest-order-bit-in-c  
    2. https://stackoverflow.com/questions/10096599/bitwise-operations-equivalent-of-greater-than-operator/10097167#10097167

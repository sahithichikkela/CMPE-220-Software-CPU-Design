; Fibonacci Sequence Generator
; Calculates and stores first N Fibonacci numbers
; Demonstrates loops, arithmetic, and memory operations

start:
    LOAD #10           ; N = 10 (calculate first 10 Fibonacci numbers)
    MOV A C            ; Store N in C register
    
    LOAD #0            ; First Fibonacci number F(0) = 0
    MOV A D            ; Store in D register (previous)
    
    LOAD #1            ; Second Fibonacci number F(1) = 1  
    MOV A B            ; Store in B register (current)
    
    LOAD #result       ; Load address where results are stored
    PUSH A             ; Save result address on stack
    
    ; Store F(0) and F(1)
    POP B              ; Get result address
    LOAD D             ; Get F(0)
    STORE B            ; Store F(0) at result[0]
    
    LOAD B             ; Increment address
    ADD #2
    MOV A B
    PUSH B             ; Save address
    
    LOAD #1            ; Get F(1) again
    POP B              ; Get address
    STORE B            ; Store F(1) at result[1]
    
    LOAD B             ; Increment address for next storage
    ADD #2
    PUSH A             ; Save next storage address
    
    ; Initialize counter (already calculated 2 numbers)
    LOAD #2
    MOV A C            ; C = counter = 2
    
fib_loop:
    ; Check if we've calculated N numbers
    LOAD C             ; Load counter
    CMP #10            ; Compare with N
    JZ print_results   ; If counter == N, done
    
    ; Calculate next Fibonacci: next = current + previous
    LOAD B             ; Load current value
    PUSH A             ; Save it
    ADD D              ; Add previous (A = current + previous)
    
    ; Store result
    POP D              ; D = old current (now becomes previous)
    MOV A B            ; B = new current (the sum we just calculated)
    
    ; Store new value to memory
    POP C              ; Get storage address
    PUSH B             ; Save current value
    LOAD B
    STORE C            ; Store at memory address
    POP B              ; Restore current value
    
    ; Increment storage address
    LOAD C
    ADD #2
    PUSH A             ; Save updated address
    
    ; Increment counter
    LOAD C
    ADD #1
    MOV A C
    
    JMP fib_loop       ; Continue loop

print_results:
    ; Print a message indicating completion
    LOAD #msg
    OUT #1
    
    HALT               ; Stop execution

result:
    ; Reserve space for 10 Fibonacci numbers (20 bytes)
    
msg:
    ; "Done" message would go here

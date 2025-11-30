; Timer/Counter Example
; Demonstrates the Fetch-Decode-Execute cycle clearly
; Counts down from a value and displays the count

start:
    LOAD #100          ; FETCH: PC=0, reads instruction byte + operand
                       ; DECODE: LOAD immediate mode, operand=100
                       ; EXECUTE: A = 100, update flags
                       ; STORE: A register contains 100
    
    MOV A B            ; FETCH: Read MOV instruction
                       ; DECODE: MOV register mode, source=A, dest=B
                       ; EXECUTE: Copy A to B
                       ; STORE: B register now contains 100

countdown:
    ; Display current count
    LOAD B             ; FETCH: Read LOAD instruction at 'countdown' label
                       ; DECODE: LOAD register mode, source=B
                       ; EXECUTE: A = B (load counter value)
                       ; STORE: A contains current count
    
    OUT #1             ; FETCH: Read OUT instruction
                       ; DECODE: OUT immediate mode, port=1
                       ; EXECUTE: Write A to I/O port 1 (memory-mapped 0xFF01)
                       ; STORE: Character written to output
    
    ; Decrement counter
    LOAD B             ; FETCH: Read LOAD instruction
                       ; DECODE: LOAD register mode
                       ; EXECUTE: A = B
                       ; STORE: A has counter value
    
    SUB #1             ; FETCH: Read SUB instruction and operand
                       ; DECODE: SUB immediate mode, operand=1
                       ; EXECUTE: A = A - 1, set flags (Z, C, N)
                       ; STORE: A has decremented value, FLAGS updated
    
    MOV A B            ; FETCH: Read MOV instruction
                       ; DECODE: MOV register mode
                       ; EXECUTE: Copy A to B (save decremented value)
                       ; STORE: B updated with new count
    
    CMP #0             ; FETCH: Read CMP instruction
                       ; DECODE: CMP immediate mode, operand=0
                       ; EXECUTE: Compare A with 0, update flags
                       ; STORE: FLAGS register updated (Z flag set if A==0)
    
    JNZ countdown      ; FETCH: Read JNZ instruction and target address
                       ; DECODE: JNZ immediate mode, target='countdown' address
                       ; EXECUTE: If Z flag is NOT set, PC = target address
                       ;          If Z flag IS set, PC continues to next instruction
                       ; STORE: PC updated (either to countdown or next instruction)

    ; Counter reached zero
    LOAD #done_msg     ; FETCH: Read LOAD instruction
                       ; DECODE: LOAD immediate mode
                       ; EXECUTE: A = address of done_msg
                       ; STORE: A contains message address
    
    OUT #1             ; FETCH: Read OUT instruction
                       ; DECODE: OUT immediate mode
                       ; EXECUTE: Output value in A
                       ; STORE: Output performed
    
    HALT               ; FETCH: Read HALT instruction
                       ; DECODE: HALT (no operands)
                       ; EXECUTE: Set HALT flag in FLAGS register
                       ; STORE: CPU stops executing, program ends

done_msg:
    ; "Done!" message data

; CYCLE SUMMARY FOR EACH INSTRUCTION:
; 
; 1. FETCH CYCLE:
;    - PC points to next instruction in memory
;    - Instruction byte is read from memory[PC]
;    - PC is incremented
;    - Any operand bytes are fetched and PC incremented accordingly
;
; 2. DECODE CYCLE:
;    - Instruction byte is split into opcode (upper 6 bits) and mode (lower 2 bits)
;    - Operand is interpreted based on addressing mode:
;      * IMMEDIATE: value is the literal operand
;      * DIRECT: operand is memory address to read from
;      * REGISTER: operand specifies which register to use
;      * INDIRECT: operand specifies register containing address
;
; 3. EXECUTE CYCLE:
;    - ALU performs operation (arithmetic, logic, comparison)
;    - Control unit handles jumps, calls, memory operations
;    - Flags are updated based on results
;
; 4. STORE CYCLE:
;    - Results are written back to registers or memory
;    - PC is updated for jumps/calls
;    - Stack pointer updated for push/pop operations

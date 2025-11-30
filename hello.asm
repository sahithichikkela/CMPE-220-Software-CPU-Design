; Hello World Program
; Demonstrates basic output to memory-mapped I/O

start:
    LOAD #hello_msg    ; Load address of message
    MOV A B            ; Copy to B register for indexing
    
print_loop:
    LOAD [B]           ; Load character at address in B
    CMP #0             ; Check if null terminator
    JZ end             ; If zero, end program
    
    OUT #1             ; Output character to console (port 1)
    
    LOAD B             ; Load current address
    ADD #1             ; Increment address
    MOV A B            ; Store back to B
    
    JMP print_loop     ; Continue loop
    
end:
    HALT               ; Stop execution

hello_msg:
    ; String data stored as bytes
    ; In a real implementation, this would be: "Hello, World!\n\0"

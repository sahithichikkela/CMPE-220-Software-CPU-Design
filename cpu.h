#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

// Memory configuration
#define MEMORY_SIZE 65536  // 64KB of memory
#define IO_START 0xFF00    // Memory-mapped I/O starts here
#define STACK_START 0xFEFF // Stack grows downward from here
#define TIMER_ADDR 0xFF03  // Hardware timer (returns milliseconds since init)

// Register definitions
typedef struct {
    uint16_t PC;    // Program Counter
    uint16_t SP;    // Stack Pointer
    uint16_t A;     // Accumulator
    uint16_t B;     // General purpose register
    uint16_t C;     // General purpose register
    uint16_t D;     // General purpose register
    uint8_t FLAGS;  // Status flags
} Registers;

// Flag bit positions
#define FLAG_ZERO     0x01  // Zero flag
#define FLAG_CARRY    0x02  // Carry flag
#define FLAG_NEGATIVE 0x04  // Negative flag
#define FLAG_OVERFLOW 0x08  // Overflow flag
#define FLAG_HALT     0x80  // Halt flag

// Instruction format: [OPCODE:6 bits][MODE:2 bits] [OPERAND: 0-16 bits]
// Opcodes (6 bits = 64 possible instructions)
typedef enum {
    // Data movement (0-5)
    OP_NOP = 0,         // No operation
    OP_LOAD = 1,        // Load to A
    OP_STORE = 2,       // Store from A
    OP_MOV = 3,         // Move between registers
    OP_PUSH = 4,        // Push to stack
    OP_POP = 5,         // Pop from stack

    // Arithmetic (6-11)
    OP_ADD = 6,         // Add to A
    OP_SUB = 7,         // Subtract from A
    OP_INC = 8,         // Increment
    OP_DEC = 9,         // Decrement
    OP_MUL = 10,        // Multiply
    OP_DIV = 11,        // Divide

    // Logical (12-17)
    OP_AND = 12,        // Bitwise AND
    OP_OR = 13,         // Bitwise OR
    OP_XOR = 14,        // Bitwise XOR
    OP_NOT = 15,        // Bitwise NOT
    OP_SHL = 16,        // Shift left
    OP_SHR = 17,        // Shift right

    // Comparison (18-19)
    OP_CMP = 18,        // Compare
    OP_TEST = 19,       // Test (AND without storing)

    // Control flow (20-26)
    OP_JMP = 20,        // Unconditional jump
    OP_JZ = 21,         // Jump if zero
    OP_JNZ = 22,        // Jump if not zero
    OP_JC = 23,         // Jump if carry
    OP_JNC = 24,        // Jump if not carry
    OP_CALL = 25,       // Call subroutine
    OP_RET = 26,        // Return from subroutine

    // System (27-29)
    OP_HALT = 27,       // Halt execution
    OP_IN = 28,         // Input from I/O port
    OP_OUT = 29,        // Output to I/O port
} Opcode;

// Addressing modes (2 bits = 4 modes)
typedef enum {
    MODE_IMMEDIATE = 0,  // Immediate value
    MODE_DIRECT = 1,     // Direct memory address
    MODE_REGISTER = 2,   // Register
    MODE_INDIRECT = 3,   // Indirect through register
} AddressingMode;

// CPU structure
typedef struct {
    Registers regs;
    uint8_t memory[MEMORY_SIZE];
    bool running;
    uint64_t cycles;
    uint64_t timer_start_ms;  // Timer initialization timestamp
} CPU;

// Function declarations
void cpu_init(CPU *cpu);
void cpu_reset(CPU *cpu);
void cpu_load_program(CPU *cpu, const uint8_t *program, uint16_t size, uint16_t start_addr);
void cpu_step(CPU *cpu);
void cpu_run(CPU *cpu);
void cpu_dump_registers(const CPU *cpu);
void cpu_dump_memory(const CPU *cpu, uint16_t start, uint16_t length);
const char* get_opcode_name(uint8_t opcode);
const char* get_instruction_name(uint8_t opcode, uint8_t mode);

// Memory operations
uint8_t mem_read8(CPU *cpu, uint16_t addr);
uint16_t mem_read16(CPU *cpu, uint16_t addr);
void mem_write8(CPU *cpu, uint16_t addr, uint8_t value);
void mem_write16(CPU *cpu, uint16_t addr, uint16_t value);

// Stack operations
void stack_push8(CPU *cpu, uint8_t value);
void stack_push16(CPU *cpu, uint16_t value);
uint8_t stack_pop8(CPU *cpu);
uint16_t stack_pop16(CPU *cpu);

// Flag operations
void set_flag(CPU *cpu, uint8_t flag);
void clear_flag(CPU *cpu, uint8_t flag);
bool get_flag(const CPU *cpu, uint8_t flag);
void update_flags(CPU *cpu, uint16_t result);

#endif // CPU_H

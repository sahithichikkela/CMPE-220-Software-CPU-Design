#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// Helper function to get current time in milliseconds
static uint64_t get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

// Initialize CPU
void cpu_init(CPU *cpu) {
    memset(cpu, 0, sizeof(CPU));
    cpu->regs.SP = STACK_START;
    cpu->running = false;
    cpu->cycles = 0;
    cpu->timer_start_ms = get_time_ms();
}

// Reset CPU to initial state
void cpu_reset(CPU *cpu) {
    cpu->regs.PC = 0;
    cpu->regs.SP = STACK_START;
    cpu->regs.A = 0;
    cpu->regs.B = 0;
    cpu->regs.C = 0;
    cpu->regs.D = 0;
    cpu->regs.FLAGS = 0;
    cpu->running = false;
    cpu->cycles = 0;
}

// Load program into memory
void cpu_load_program(CPU *cpu, const uint8_t *program, uint16_t size, uint16_t start_addr) {
    if (start_addr + size > MEMORY_SIZE) {
        fprintf(stderr, "Error: Program too large for memory\n");
        return;
    }
    memcpy(&cpu->memory[start_addr], program, size);
    cpu->regs.PC = start_addr;
}

// Memory read operations
uint8_t mem_read8(CPU *cpu, uint16_t addr) {
    // Handle memory-mapped I/O
    if (addr >= IO_START) {
        // Simple console input at 0xFF00
        if (addr == 0xFF00) {
            return getchar();
        }
        return 0;
    }
    return cpu->memory[addr];
}

uint16_t mem_read16(CPU *cpu, uint16_t addr) {
    // Handle hardware timer at 0xFF03
    if (addr == TIMER_ADDR) {
        uint64_t elapsed_ms = get_time_ms() - cpu->timer_start_ms;
        // Return lower 16 bits (wraps around every ~65 seconds)
        return (uint16_t)(elapsed_ms & 0xFFFF);
    }

    uint8_t low = mem_read8(cpu, addr);
    uint8_t high = mem_read8(cpu, addr + 1);
    return (high << 8) | low;
}

// Memory write operations
void mem_write8(CPU *cpu, uint16_t addr, uint8_t value) {
    // Handle memory-mapped I/O
    if (addr >= IO_START) {
        // Simple console output at 0xFF01
        if (addr == 0xFF01) {
            putchar(value);
            fflush(stdout);
        }
        return;
    }
    cpu->memory[addr] = value;
}

void mem_write16(CPU *cpu, uint16_t addr, uint16_t value) {
    mem_write8(cpu, addr, value & 0xFF);
    mem_write8(cpu, addr + 1, (value >> 8) & 0xFF);
}

// Stack operations
void stack_push8(CPU *cpu, uint8_t value) {
    mem_write8(cpu, cpu->regs.SP, value);
    cpu->regs.SP--;
}

void stack_push16(CPU *cpu, uint16_t value) {
    stack_push8(cpu, (value >> 8) & 0xFF);
    stack_push8(cpu, value & 0xFF);
}

uint8_t stack_pop8(CPU *cpu) {
    cpu->regs.SP++;
    return mem_read8(cpu, cpu->regs.SP);
}

uint16_t stack_pop16(CPU *cpu) {
    uint8_t low = stack_pop8(cpu);
    uint8_t high = stack_pop8(cpu);
    return (high << 8) | low;
}

// Flag operations
void set_flag(CPU *cpu, uint8_t flag) {
    cpu->regs.FLAGS |= flag;
}

void clear_flag(CPU *cpu, uint8_t flag) {
    cpu->regs.FLAGS &= ~flag;
}

bool get_flag(const CPU *cpu, uint8_t flag) {
    return (cpu->regs.FLAGS & flag) != 0;
}

void update_flags(CPU *cpu, uint16_t result) {
    // Zero flag - check full 16-bit value
    if (result == 0) {
        set_flag(cpu, FLAG_ZERO);
    } else {
        clear_flag(cpu, FLAG_ZERO);
    }

    // Negative flag (bit 15 set for 16-bit, or bit 7 for 8-bit)
    if (result & 0x8000) {
        set_flag(cpu, FLAG_NEGATIVE);
    } else {
        clear_flag(cpu, FLAG_NEGATIVE);
    }
}

// Get register value by number
uint16_t* get_register(CPU *cpu, uint8_t reg_num) {
    switch (reg_num) {
        case 0: return &cpu->regs.A;
        case 1: return &cpu->regs.B;
        case 2: return &cpu->regs.C;
        case 3: return &cpu->regs.D;
        default: return &cpu->regs.A;
    }
}

// Execute one instruction (Fetch-Decode-Execute cycle)
void cpu_step(CPU *cpu) {
    if (!cpu->running || get_flag(cpu, FLAG_HALT)) {
        return;
    }
    
    // FETCH
    uint8_t instruction = mem_read8(cpu, cpu->regs.PC++);
    uint8_t opcode = (instruction >> 2) & 0x3F;  // Upper 6 bits
    uint8_t mode = instruction & 0x03;            // Lower 2 bits
    
    cpu->cycles++;
    
    // DECODE & EXECUTE
    uint16_t operand = 0;
    uint16_t address = 0;
    uint16_t *reg_ptr = NULL;

    // Only fetch operands for instructions that need them
    // NOP, HALT, RET, NOT, POP don't need operands
    bool needs_operand = true;
    if (opcode == OP_NOP || opcode == OP_HALT || opcode == OP_RET ||
        opcode == OP_NOT || (opcode == OP_POP && mode == MODE_IMMEDIATE)) {
        needs_operand = false;
    }

    // Get operand based on addressing mode
    if (needs_operand) {
        switch (mode) {
            case MODE_IMMEDIATE:
                operand = mem_read16(cpu, cpu->regs.PC);
                cpu->regs.PC += 2;
                break;

            case MODE_DIRECT:
                address = mem_read16(cpu, cpu->regs.PC);
                cpu->regs.PC += 2;
                operand = mem_read16(cpu, address);
                break;

            case MODE_REGISTER:
                {
                    uint8_t reg_num = mem_read8(cpu, cpu->regs.PC++);
                    reg_ptr = get_register(cpu, reg_num);
                    operand = *reg_ptr;
                }
                break;

            case MODE_INDIRECT:
                {
                    uint8_t reg_num = mem_read8(cpu, cpu->regs.PC++);
                    address = *get_register(cpu, reg_num);
                    operand = mem_read16(cpu, address);
                }
                break;
        }
    }
    
    // Execute instruction
    switch (opcode) {
        case OP_NOP:
            break;
            
        case OP_LOAD:
            cpu->regs.A = operand;
            update_flags(cpu, cpu->regs.A);
            break;
            
        case OP_STORE:
            if (mode == MODE_DIRECT) {
                mem_write16(cpu, address, cpu->regs.A);
            } else if (mode == MODE_INDIRECT) {
                mem_write16(cpu, address, cpu->regs.A);
            }
            break;
            
        case OP_MOV:
            if (mode == MODE_REGISTER) {
                uint8_t dest_reg = mem_read8(cpu, cpu->regs.PC++);
                uint16_t *dest = get_register(cpu, dest_reg);
                *dest = operand;
                update_flags(cpu, *dest);
            }
            break;
            
        case OP_PUSH:
            stack_push16(cpu, operand);
            break;
            
        case OP_POP:
            cpu->regs.A = stack_pop16(cpu);
            update_flags(cpu, cpu->regs.A);
            break;
            
        case OP_ADD:
            {
                uint32_t result = cpu->regs.A + operand;
                if (result > 0xFFFF) {
                    set_flag(cpu, FLAG_CARRY);
                } else {
                    clear_flag(cpu, FLAG_CARRY);
                }
                cpu->regs.A = result & 0xFFFF;
                update_flags(cpu, cpu->regs.A);
            }
            break;
            
        case OP_SUB:
            {
                int32_t result = cpu->regs.A - operand;
                if (result < 0) {
                    set_flag(cpu, FLAG_CARRY);
                } else {
                    clear_flag(cpu, FLAG_CARRY);
                }
                cpu->regs.A = result & 0xFFFF;
                update_flags(cpu, cpu->regs.A);
            }
            break;
            
        case OP_INC:
            if (mode == MODE_REGISTER && reg_ptr) {
                (*reg_ptr)++;
                update_flags(cpu, *reg_ptr);
            } else {
                cpu->regs.A++;
                update_flags(cpu, cpu->regs.A);
            }
            break;
            
        case OP_DEC:
            if (mode == MODE_REGISTER && reg_ptr) {
                (*reg_ptr)--;
                update_flags(cpu, *reg_ptr);
            } else {
                cpu->regs.A--;
                update_flags(cpu, cpu->regs.A);
            }
            break;
            
        case OP_MUL:
            cpu->regs.A = (cpu->regs.A * operand) & 0xFFFF;
            update_flags(cpu, cpu->regs.A);
            break;
            
        case OP_DIV:
            if (operand != 0) {
                cpu->regs.A = cpu->regs.A / operand;
                update_flags(cpu, cpu->regs.A);
            }
            break;
            
        case OP_AND:
            cpu->regs.A &= operand;
            update_flags(cpu, cpu->regs.A);
            break;
            
        case OP_OR:
            cpu->regs.A |= operand;
            update_flags(cpu, cpu->regs.A);
            break;
            
        case OP_XOR:
            cpu->regs.A ^= operand;
            update_flags(cpu, cpu->regs.A);
            break;
            
        case OP_NOT:
            cpu->regs.A = ~cpu->regs.A;
            update_flags(cpu, cpu->regs.A);
            break;
            
        case OP_SHL:
            cpu->regs.A <<= operand;
            update_flags(cpu, cpu->regs.A);
            break;
            
        case OP_SHR:
            cpu->regs.A >>= operand;
            update_flags(cpu, cpu->regs.A);
            break;
            
        case OP_CMP:
            {
                int32_t result = cpu->regs.A - operand;
                if (result < 0) {
                    set_flag(cpu, FLAG_CARRY);
                } else {
                    clear_flag(cpu, FLAG_CARRY);
                }
                update_flags(cpu, result & 0xFFFF);
            }
            break;
            
        case OP_TEST:
            {
                uint16_t result = cpu->regs.A & operand;
                update_flags(cpu, result);
            }
            break;
            
        case OP_JMP:
            cpu->regs.PC = operand;
            break;
            
        case OP_JZ:
            if (get_flag(cpu, FLAG_ZERO)) {
                cpu->regs.PC = operand;
            }
            break;
            
        case OP_JNZ:
            if (!get_flag(cpu, FLAG_ZERO)) {
                cpu->regs.PC = operand;
            }
            break;
            
        case OP_JC:
            if (get_flag(cpu, FLAG_CARRY)) {
                cpu->regs.PC = operand;
            }
            break;
            
        case OP_JNC:
            if (!get_flag(cpu, FLAG_CARRY)) {
                cpu->regs.PC = operand;
            }
            break;
            
        case OP_CALL:
            stack_push16(cpu, cpu->regs.PC);
            cpu->regs.PC = operand;
            break;
            
        case OP_RET:
            cpu->regs.PC = stack_pop16(cpu);
            break;
            
        case OP_HALT:
            set_flag(cpu, FLAG_HALT);
            cpu->running = false;
            printf("\n[CPU HALTED after %llu cycles]\n", 
                   (unsigned long long)cpu->cycles);
            break;
            
        case OP_IN:
            cpu->regs.A = mem_read8(cpu, IO_START + operand);
            update_flags(cpu, cpu->regs.A);
            break;
            
        case OP_OUT:
            mem_write8(cpu, IO_START + 1, cpu->regs.A & 0xFF);
            break;
            
        default:
            fprintf(stderr, "Unknown opcode: 0x%02X at PC=0x%04X\n", 
                    opcode, cpu->regs.PC - 1);
            cpu->running = false;
            break;
    }
}

// Run CPU until halt
void cpu_run(CPU *cpu) {
    cpu->running = true;
    while (cpu->running && !get_flag(cpu, FLAG_HALT)) {
        cpu_step(cpu);
    }
}

// Dump register contents
void cpu_dump_registers(const CPU *cpu) {
    printf("\n=== CPU Registers ===\n");
    printf("PC: 0x%04X   SP: 0x%04X\n", cpu->regs.PC, cpu->regs.SP);
    printf("A:  0x%04X   B:  0x%04X\n", cpu->regs.A, cpu->regs.B);
    printf("C:  0x%04X   D:  0x%04X\n", cpu->regs.C, cpu->regs.D);
    printf("FLAGS: 0x%02X [", cpu->regs.FLAGS);
    if (get_flag(cpu, FLAG_ZERO)) printf("Z");
    if (get_flag(cpu, FLAG_CARRY)) printf("C");
    if (get_flag(cpu, FLAG_NEGATIVE)) printf("N");
    if (get_flag(cpu, FLAG_OVERFLOW)) printf("O");
    if (get_flag(cpu, FLAG_HALT)) printf("H");
    printf("]\n");
    printf("Cycles: %llu\n", (unsigned long long)cpu->cycles);
}

// Dump memory contents with ASCII representation
void cpu_dump_memory(const CPU *cpu, uint16_t start, uint16_t length) {
    printf("\n--- Memory Dump (%04X - %04X [Hex]) ---\n", start, start + length - 1);
    printf("Addr | 00 01 02 03 04 05 06 07 | ASCII\n");
    printf("------------------------------------------------\n");

    for (uint16_t i = 0; i < length; i += 8) {
        printf("%04X | ", start + i);

        // Print hex values
        for (uint16_t j = 0; j < 8 && (i + j) < length; j++) {
            printf("%02X ", cpu->memory[start + i + j]);
        }

        // Padding if less than 8 bytes
        for (uint16_t j = (i + 8 > length) ? (length - i) : 8; j < 8; j++) {
            printf("   ");
        }

        printf("| ");

        // Print ASCII representation
        for (uint16_t j = 0; j < 8 && (i + j) < length; j++) {
            uint8_t byte = cpu->memory[start + i + j];
            if (byte >= 32 && byte <= 126) {
                printf("%c", byte);
            } else {
                printf(".");
            }
        }

        printf("\n");
    }

    printf("------------------------------------------------\n");
}

// Get opcode name as string
const char* get_opcode_name(uint8_t opcode) {
    switch(opcode) {
        case OP_NOP: return "NOP";
        case OP_LOAD: return "LOAD";
        case OP_STORE: return "STORE";
        case OP_MOV: return "MOV";
        case OP_PUSH: return "PUSH";
        case OP_POP: return "POP";
        case OP_ADD: return "ADD";
        case OP_SUB: return "SUB";
        case OP_INC: return "INC";
        case OP_DEC: return "DEC";
        case OP_MUL: return "MUL";
        case OP_DIV: return "DIV";
        case OP_AND: return "AND";
        case OP_OR: return "OR";
        case OP_XOR: return "XOR";
        case OP_NOT: return "NOT";
        case OP_SHL: return "SHL";
        case OP_SHR: return "SHR";
        case OP_CMP: return "CMP";
        case OP_TEST: return "TEST";
        case OP_JMP: return "JMP";
        case OP_JZ: return "JZ";
        case OP_JNZ: return "JNZ";
        case OP_JC: return "JC";
        case OP_JNC: return "JNC";
        case OP_CALL: return "CALL";
        case OP_RET: return "RET";
        case OP_HALT: return "HALT";
        case OP_IN: return "IN";
        case OP_OUT: return "OUT";
        default: return "UNKNOWN";
    }
}

// Get instruction name with mode suffix (e.g., "LOADI", "JUMPEQ")
const char* get_instruction_name(uint8_t opcode, uint8_t mode) {
    static char buffer[16];
    const char* base_name = get_opcode_name(opcode);

    // For immediate mode, add "I" suffix for certain instructions
    if (mode == MODE_IMMEDIATE) {
        // Instructions that commonly use "I" suffix
        if (opcode == OP_LOAD || opcode == OP_ADD || opcode == OP_SUB ||
            opcode == OP_AND || opcode == OP_OR || opcode == OP_XOR ||
            opcode == OP_CMP || opcode == OP_STORE) {
            snprintf(buffer, sizeof(buffer), "%sI", base_name);
            return buffer;
        }
    }

    // For jumps, use descriptive names
    if (opcode == OP_JZ) {
        return "JUMPEQ";
    } else if (opcode == OP_JNZ) {
        return "JUMPNEQ";
    } else if (opcode == OP_JMP) {
        return "JUMP";
    } else if (opcode == OP_JC) {
        return "JUMPC";
    } else if (opcode == OP_JNC) {
        return "JUMPNC";
    }

    // Default: return base name
    return base_name;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "assembler.h"

void print_usage(const char *prog_name) {
    printf("Usage:\n");
    printf("  %s assemble <input.asm> <output.bin>  - Assemble program\n", prog_name);
    printf("  %s run <program.bin>                  - Run binary program\n", prog_name);
    printf("  %s demo <fibonacci|hello|timer>       - Run demo program\n", prog_name);
    printf("\n");
}

// Create Fibonacci program in memory
void create_fibonacci_demo(CPU *cpu) {
    printf("Creating Fibonacci demo program...\n");

    // Simpler Fibonacci: calculate F(5) = 5
    // Strategy: B=prev, D=current, iterate 4 times (from F1 to F5)
    uint8_t program[] = {
        // Initialize
        encode_instruction(OP_LOAD, MODE_IMMEDIATE), 0x00, 0x00,   // 0: A = 0
        encode_instruction(OP_MOV, MODE_REGISTER), 0x00, 0x01,     // 3: MOV A B (B = 0)

        encode_instruction(OP_LOAD, MODE_IMMEDIATE), 0x01, 0x00,   // 6: A = 1
        encode_instruction(OP_MOV, MODE_REGISTER), 0x00, 0x03,     // 9: MOV A D (D = 1)

        encode_instruction(OP_LOAD, MODE_IMMEDIATE), 0x04, 0x00,   // 12: A = 4
        encode_instruction(OP_MOV, MODE_REGISTER), 0x00, 0x02,     // 15: MOV A C (C = 4)

        // Loop: calculate next = current + previous
        // Address 18 (0x12)
        encode_instruction(OP_LOAD, MODE_REGISTER), 0x03,          // 18: A = D (current)
        encode_instruction(OP_ADD, MODE_REGISTER), 0x01,           // 20: A = D + B
        encode_instruction(OP_PUSH, MODE_REGISTER), 0x00,          // 22: PUSH A (save next)

        encode_instruction(OP_LOAD, MODE_REGISTER), 0x03,          // 24: A = D
        encode_instruction(OP_MOV, MODE_REGISTER), 0x00, 0x01,     // 26: MOV A B (B = old D)

        encode_instruction(OP_POP, MODE_IMMEDIATE), 0x00, 0x00,    // 29: POP (A = next)
        encode_instruction(OP_MOV, MODE_REGISTER), 0x00, 0x03,     // 32: MOV A D (D = next)

        // Decrement and loop
        encode_instruction(OP_LOAD, MODE_REGISTER), 0x02,          // 35: A = C
        encode_instruction(OP_SUB, MODE_IMMEDIATE), 0x01, 0x00,    // 37: A = C - 1
        encode_instruction(OP_MOV, MODE_REGISTER), 0x00, 0x02,     // 40: MOV A C (C = A)
        encode_instruction(OP_CMP, MODE_IMMEDIATE), 0x00, 0x00,    // 43: CMP A, 0
        encode_instruction(OP_JNZ, MODE_IMMEDIATE), 0x12, 0x00,    // 46: JNZ loop (jump to 18)

        // End at 49
        encode_instruction(OP_HALT, MODE_IMMEDIATE), 0x00, 0x00,   // 49: HALT
    };

    cpu_load_program(cpu, program, sizeof(program), 0);
    printf("Program loaded. Computing F(5) = 5\n");
    printf("Algorithm: Start with F(0)=0, F(1)=1, iterate 4 times\n");
}

// Create Hello World demo
void create_hello_demo(CPU *cpu) {
    printf("Creating Hello World demo program...\n");
    
    // Simple hello program that outputs characters
    char *message = "Hello, World!\n";
    int msg_len = strlen(message);
    
    // Program starts at 0x0000
    // Message stored at 0x0100
    uint16_t msg_addr = 0x0100;
    
    // Store message in memory
    for (int i = 0; i < msg_len; i++) {
        cpu->memory[msg_addr + i] = message[i];
    }
    cpu->memory[msg_addr + msg_len] = 0; // Null terminator
    
    uint8_t program[] = {
        // Load message address (bytes 0-5)
        encode_instruction(OP_LOAD, MODE_IMMEDIATE), 0x00, 0x01,   // 0-2: A = 0x0100
        encode_instruction(OP_MOV, MODE_REGISTER), 0x00, 0x01,     // 3-5: MOV A B (B = A, pointer)

        // Loop start (address 6)
        encode_instruction(OP_LOAD, MODE_INDIRECT), 0x01,          // 6-7: A = [B]
        encode_instruction(OP_CMP, MODE_IMMEDIATE), 0x00, 0x00,    // 8-10: Compare with 0
        encode_instruction(OP_JZ, MODE_IMMEDIATE), 0x1B, 0x00,     // 11-13: If zero, jump to HALT at 27 (0x1B)

        encode_instruction(OP_OUT, MODE_REGISTER), 0x00,           // 14-15: Output character in A

        encode_instruction(OP_LOAD, MODE_REGISTER), 0x01,          // 16-17: A = B
        encode_instruction(OP_ADD, MODE_IMMEDIATE), 0x01, 0x00,    // 18-20: A = A + 1
        encode_instruction(OP_MOV, MODE_REGISTER), 0x00, 0x01,     // 21-23: MOV A B (B = A)

        encode_instruction(OP_JMP, MODE_IMMEDIATE), 0x06, 0x00,    // 24-26: Jump to loop at 6

        // End (address 27 = 0x1B)
        encode_instruction(OP_HALT, MODE_REGISTER), 0x00,          // 27-28: HALT
    };
    
    cpu_load_program(cpu, program, sizeof(program), 0);
    printf("Program loaded.\n");
}

// Create timer demo with detailed cycle tracking
void create_timer_demo(CPU *cpu) {
    printf("Creating Timer/Counter demo program...\n");
    printf("This program demonstrates Fetch-Decode-Execute cycles.\n\n");
    
    uint8_t program[] = {
        // Start: count from 5 down to 0
        encode_instruction(OP_LOAD, MODE_IMMEDIATE), 0x05, 0x00,   // 0-2: A = 5
        encode_instruction(OP_MOV, MODE_REGISTER), 0x00, 0x01,     // 3-5: MOV A B (B = 5)

        // Loop start (address 6)
        encode_instruction(OP_LOAD, MODE_REGISTER), 0x01,          // 6-7: A = B
        encode_instruction(OP_CMP, MODE_IMMEDIATE), 0x00, 0x00,    // 8-10: Compare A with 0
        encode_instruction(OP_JZ, MODE_IMMEDIATE), 0x19, 0x00,     // 11-13: If zero, jump to HALT at 25 (0x19)

        // Decrement
        encode_instruction(OP_LOAD, MODE_REGISTER), 0x01,          // 14-15: A = B
        encode_instruction(OP_SUB, MODE_IMMEDIATE), 0x01, 0x00,    // 16-18: A = A - 1
        encode_instruction(OP_MOV, MODE_REGISTER), 0x00, 0x01,     // 19-21: MOV A B (B = A)

        encode_instruction(OP_JMP, MODE_IMMEDIATE), 0x06, 0x00,    // 22-24: Loop back to address 6

        // End (address 25 = 0x19)
        encode_instruction(OP_HALT, MODE_IMMEDIATE), 0x00, 0x00,   // 25-27: HALT
    };
    
    cpu_load_program(cpu, program, sizeof(program), 0);
    
    printf("\n=== Executing with Cycle Tracking ===\n");
    printf("--- CPU Running ---\n");
    cpu->running = true;

    int step = 0;
    while (cpu->running && !get_flag(cpu, FLAG_HALT)) {
        uint16_t pc_before = cpu->regs.PC;
        uint8_t instruction = cpu->memory[pc_before];
        uint8_t opcode = (instruction >> 2) & 0x3F;
        uint8_t mode = instruction & 0x03;

        cpu_step(cpu);

        // Compact format matching example: [PC=0x0000] OPCODE | R0=0x0000 R1=0x0000 SP=0xFEFF FP=0x0000 ZN=00
        uint8_t zn_flags = 0;
        if (get_flag(cpu, FLAG_ZERO)) zn_flags |= 0x10;
        if (get_flag(cpu, FLAG_NEGATIVE)) zn_flags |= 0x01;

        printf("[PC=0x%04X] %-7s | R0=0x%04X R1=0x%04X R2=0x%04X R3=0x%04X SP=0x%04X ZN=%02X\n",
               pc_before, get_instruction_name(opcode, mode),
               cpu->regs.A, cpu->regs.B, cpu->regs.C, cpu->regs.D,
               cpu->regs.SP, zn_flags);

        if (++step > 100) {
            printf("Safety limit reached\n");
            break;
        }
    }

    printf("\n[CPU HALTED after %d cycles]\n", step);
    printf("\n=== Execution Complete ===\n");
    cpu_dump_registers(cpu);

    // Add memory dump
    printf("\n=== Memory Dump (Program Area) ===\n");
    cpu_dump_memory(cpu, 0x0000, 0x0030);

    return;
}

int main(int argc, char *argv[]) {
    printf("=== Software CPU Emulator ===\n\n");
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "assemble") == 0) {
        if (argc != 4) {
            printf("Usage: %s assemble <input.asm> <output.bin>\n", argv[0]);
            return 1;
        }
        
        if (assemble_file(argv[2], argv[3])) {
            printf("Assembly successful!\n");
            return 0;
        } else {
            printf("Assembly failed!\n");
            return 1;
        }
    }
    else if (strcmp(argv[1], "run") == 0) {
        if (argc != 3) {
            printf("Usage: %s run <program.bin>\n", argv[0]);
            return 1;
        }
        
        FILE *f = fopen(argv[2], "rb");
        if (!f) {
            fprintf(stderr, "Error: Cannot open file '%s'\n", argv[2]);
            return 1;
        }
        
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        uint8_t *program = malloc(size);
        fread(program, 1, size, f);
        fclose(f);
        
        CPU cpu;
        cpu_init(&cpu);
        cpu_load_program(&cpu, program, size, 0);
        
        printf("Running program '%s' (%ld bytes)...\n\n", argv[2], size);
        cpu_run(&cpu);
        
        printf("\n");
        cpu_dump_registers(&cpu);
        
        free(program);
        return 0;
    }
    else if (strcmp(argv[1], "demo") == 0) {
        if (argc != 3) {
            printf("Usage: %s demo <fibonacci|hello|timer>\n", argv[0]);
            return 1;
        }
        
        CPU cpu;
        cpu_init(&cpu);
        
        if (strcmp(argv[2], "fibonacci") == 0) {
            create_fibonacci_demo(&cpu);

            printf("\n--- CPU Running ---\n");
            cpu.running = true;
            int max_cycles = 150;
            int cycle = 0;

            while (cpu.running && !get_flag(&cpu, FLAG_HALT) && cycle < max_cycles) {
                uint16_t pc_before = cpu.regs.PC;
                uint8_t instruction = cpu.memory[pc_before];
                uint8_t opcode = (instruction >> 2) & 0x3F;
                uint8_t mode = instruction & 0x03;

                cpu_step(&cpu);

                // Show opcode trace
                uint8_t zn_flags = 0;
                if (get_flag(&cpu, FLAG_ZERO)) zn_flags |= 0x10;
                if (get_flag(&cpu, FLAG_NEGATIVE)) zn_flags |= 0x01;

                printf("[PC=0x%04X] %-7s | R0=0x%04X R1=0x%04X R2=0x%04X R3=0x%04X SP=0x%04X ZN=%02X\n",
                       pc_before, get_instruction_name(opcode, mode),
                       cpu.regs.A, cpu.regs.B, cpu.regs.C, cpu.regs.D,
                       cpu.regs.SP, zn_flags);

                cycle++;
            }

            if (cycle >= max_cycles) {
                printf("\n[Safety limit reached after %d cycles]\n", max_cycles);
            }

            printf("\n[CPU HALTED after %d cycles]\n", cycle);
            printf("\n--- CPU Halted ---\n");
            cpu_dump_registers(&cpu);
            printf("\nFibonacci sequence calculated!\n");
            printf("F(5) result in register D: %d (expected: 5)\n", cpu.regs.D);
            cpu_dump_memory(&cpu, 0x0000, 0x0040);
        }
        else if (strcmp(argv[2], "hello") == 0) {
            create_hello_demo(&cpu);

            printf("\n--- CPU Running ---\n");
            printf("Output:\n");

            cpu.running = true;
            int cycle = 0;
            int max_cycles = 200;

            while (cpu.running && !get_flag(&cpu, FLAG_HALT) && cycle < max_cycles) {
                uint16_t pc_before = cpu.regs.PC;
                uint8_t instruction = cpu.memory[pc_before];
                uint8_t opcode = (instruction >> 2) & 0x3F;
                uint8_t mode = instruction & 0x03;

                cpu_step(&cpu);

                // Show opcode trace
                uint8_t zn_flags = 0;
                if (get_flag(&cpu, FLAG_ZERO)) zn_flags |= 0x10;
                if (get_flag(&cpu, FLAG_NEGATIVE)) zn_flags |= 0x01;

                printf("[PC=0x%04X] %-7s | R0=0x%04X R1=0x%04X R2=0x%04X R3=0x%04X SP=0x%04X ZN=%02X\n",
                       pc_before, get_instruction_name(opcode, mode),
                       cpu.regs.A, cpu.regs.B, cpu.regs.C, cpu.regs.D,
                       cpu.regs.SP, zn_flags);

                cycle++;
            }

            printf("\n[CPU HALTED after %d cycles]\n", cycle);
            printf("\n--- CPU Halted ---\n");
            cpu_dump_registers(&cpu);
            cpu_dump_memory(&cpu, 0x0000, 0x0030);
            cpu_dump_memory(&cpu, 0x0100, 0x0010);
        }
        else if (strcmp(argv[2], "timer") == 0) {
            create_timer_demo(&cpu);
        }
        else {
            printf("Unknown demo: %s\n", argv[2]);
            printf("Available demos: fibonacci, hello, timer\n");
            return 1;
        }
        
        return 0;
    }
    else {
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}

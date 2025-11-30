#define _POSIX_C_SOURCE 200809L
#include "assembler.h"
#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Initialize assembler
void assembler_init(Assembler *as) {
    memset(as, 0, sizeof(Assembler));
    as->output = malloc(MEMORY_SIZE);
    as->output_size = 0;
    as->current_address = 0;
    as->line_number = 0;
}

// Free assembler resources
void assembler_free(Assembler *as) {
    if (as->output) {
        free(as->output);
        as->output = NULL;
    }
}

// Find label by name
int find_label(Assembler *as, const char *name) {
    for (int i = 0; i < as->label_count; i++) {
        if (strcmp(as->labels[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Add label
void add_label(Assembler *as, const char *name, uint16_t address) {
    if (as->label_count >= MAX_LABELS) {
        fprintf(stderr, "Error: Too many labels\n");
        return;
    }
    strncpy(as->labels[as->label_count].name, name, MAX_TOKEN_LENGTH - 1);
    as->labels[as->label_count].address = address;
    as->label_count++;
}

// Emit byte to output
void emit_byte(Assembler *as, uint8_t byte) {
    as->output[as->output_size++] = byte;
    as->current_address++;
}

// Emit word to output
void emit_word(Assembler *as, uint16_t word) {
    emit_byte(as, word & 0xFF);
    emit_byte(as, (word >> 8) & 0xFF);
}

// Encode instruction byte
uint8_t encode_instruction(uint8_t opcode, uint8_t mode) {
    return (opcode << 2) | (mode & 0x03);
}

// Parse addressing mode and operand
bool parse_operand(Assembler *as, char *operand, uint8_t *mode, 
                   uint16_t *value, bool first_pass) {
    // Remove whitespace
    while (*operand && isspace(*operand)) operand++;
    
    if (*operand == '\0') {
        *mode = MODE_IMMEDIATE;
        *value = 0;
        return true;
    }
    
    // Immediate: #value or #label
    if (*operand == '#') {
        *mode = MODE_IMMEDIATE;
        operand++;
        if (isdigit(*operand) || *operand == '-') {
            *value = strtol(operand, NULL, 0);
        } else {
            // Label reference
            if (first_pass) {
                *value = 0;
            } else {
                int label_idx = find_label(as, operand);
                if (label_idx >= 0) {
                    *value = as->labels[label_idx].address;
                } else {
                    fprintf(stderr, "Error line %d: Undefined label '%s'\n", 
                            as->line_number, operand);
                    return false;
                }
            }
        }
        return true;
    }
    
    // Register: A, B, C, D
    if ((operand[0] == 'A' || operand[0] == 'B' || 
         operand[0] == 'C' || operand[0] == 'D') && 
        (operand[1] == '\0' || isspace(operand[1]))) {
        *mode = MODE_REGISTER;
        *value = operand[0] - 'A';
        return true;
    }
    
    // Indirect: [register]
    if (*operand == '[') {
        *mode = MODE_INDIRECT;
        operand++;
        if (*operand == 'A' || *operand == 'B' || 
            *operand == 'C' || *operand == 'D') {
            *value = *operand - 'A';
        }
        return true;
    }
    
    // Direct: address or label
    *mode = MODE_DIRECT;
    if (isdigit(*operand) || *operand == '-') {
        *value = strtol(operand, NULL, 0);
    } else {
        // Label reference
        if (first_pass) {
            *value = 0;
        } else {
            int label_idx = find_label(as, operand);
            if (label_idx >= 0) {
                *value = as->labels[label_idx].address;
            } else {
                fprintf(stderr, "Error line %d: Undefined label '%s'\n", 
                        as->line_number, operand);
                return false;
            }
        }
    }
    return true;
}

// Get opcode from mnemonic
int get_opcode(const char *mnemonic) {
    if (strcmp(mnemonic, "NOP") == 0) return OP_NOP;
    if (strcmp(mnemonic, "LOAD") == 0) return OP_LOAD;
    if (strcmp(mnemonic, "STORE") == 0) return OP_STORE;
    if (strcmp(mnemonic, "MOV") == 0) return OP_MOV;
    if (strcmp(mnemonic, "PUSH") == 0) return OP_PUSH;
    if (strcmp(mnemonic, "POP") == 0) return OP_POP;
    if (strcmp(mnemonic, "ADD") == 0) return OP_ADD;
    if (strcmp(mnemonic, "SUB") == 0) return OP_SUB;
    if (strcmp(mnemonic, "INC") == 0) return OP_INC;
    if (strcmp(mnemonic, "DEC") == 0) return OP_DEC;
    if (strcmp(mnemonic, "MUL") == 0) return OP_MUL;
    if (strcmp(mnemonic, "DIV") == 0) return OP_DIV;
    if (strcmp(mnemonic, "AND") == 0) return OP_AND;
    if (strcmp(mnemonic, "OR") == 0) return OP_OR;
    if (strcmp(mnemonic, "XOR") == 0) return OP_XOR;
    if (strcmp(mnemonic, "NOT") == 0) return OP_NOT;
    if (strcmp(mnemonic, "SHL") == 0) return OP_SHL;
    if (strcmp(mnemonic, "SHR") == 0) return OP_SHR;
    if (strcmp(mnemonic, "CMP") == 0) return OP_CMP;
    if (strcmp(mnemonic, "TEST") == 0) return OP_TEST;
    if (strcmp(mnemonic, "JMP") == 0) return OP_JMP;
    if (strcmp(mnemonic, "JZ") == 0) return OP_JZ;
    if (strcmp(mnemonic, "JNZ") == 0) return OP_JNZ;
    if (strcmp(mnemonic, "JC") == 0) return OP_JC;
    if (strcmp(mnemonic, "JNC") == 0) return OP_JNC;
    if (strcmp(mnemonic, "CALL") == 0) return OP_CALL;
    if (strcmp(mnemonic, "RET") == 0) return OP_RET;
    if (strcmp(mnemonic, "HALT") == 0) return OP_HALT;
    if (strcmp(mnemonic, "IN") == 0) return OP_IN;
    if (strcmp(mnemonic, "OUT") == 0) return OP_OUT;
    return -1;
}

// Parse a single line
bool parse_line(Assembler *as, char *line, bool first_pass) {
    // Remove comments
    char *comment = strchr(line, ';');
    if (comment) *comment = '\0';
    
    // Skip empty lines
    char *ptr = line;
    while (*ptr && isspace(*ptr)) ptr++;
    if (*ptr == '\0') return true;
    
    // Check for label
    char *colon = strchr(line, ':');
    if (colon) {
        *colon = '\0';
        char label[MAX_TOKEN_LENGTH];
        sscanf(line, "%s", label);
        
        if (first_pass) {
            add_label(as, label, as->current_address);
        }
        
        // Move past label
        line = colon + 1;
        while (*line && isspace(*line)) line++;
        if (*line == '\0') return true;
    }
    
    // Parse mnemonic
    char mnemonic[MAX_TOKEN_LENGTH];
    char operand_str[MAX_LINE_LENGTH];
    operand_str[0] = '\0';
    
    sscanf(line, "%s %[^\n]", mnemonic, operand_str);
    
    // Convert to uppercase
    for (char *p = mnemonic; *p; p++) *p = toupper(*p);
    
    int opcode = get_opcode(mnemonic);
    if (opcode < 0) {
        fprintf(stderr, "Error line %d: Unknown instruction '%s'\n", 
                as->line_number, mnemonic);
        return false;
    }
    
    // Parse operand
    uint8_t mode = MODE_IMMEDIATE;
    uint16_t value = 0;
    
    if (operand_str[0] != '\0') {
        if (!parse_operand(as, operand_str, &mode, &value, first_pass)) {
            return false;
        }
    }
    
    // Emit instruction (only in second pass)
    if (!first_pass) {
        emit_byte(as, encode_instruction(opcode, mode));
        
        // Emit operand based on mode
        if (mode == MODE_IMMEDIATE || mode == MODE_DIRECT) {
            emit_word(as, value);
        } else if (mode == MODE_REGISTER || mode == MODE_INDIRECT) {
            emit_byte(as, value);
        }
    } else {
        // Calculate instruction size for first pass
        as->current_address += 1; // instruction byte
        if (mode == MODE_IMMEDIATE || mode == MODE_DIRECT) {
            as->current_address += 2;
        } else if (mode == MODE_REGISTER || mode == MODE_INDIRECT) {
            as->current_address += 1;
        }
    }
    
    return true;
}

// First pass: collect labels
bool assembler_first_pass(Assembler *as, const char *source) {
    as->current_address = 0;
    as->line_number = 0;
    
    char *source_copy = strdup(source);
    char *line = strtok(source_copy, "\n");
    
    while (line) {
        as->line_number++;
        if (!parse_line(as, line, true)) {
            free(source_copy);
            return false;
        }
        line = strtok(NULL, "\n");
    }
    
    free(source_copy);
    return true;
}

// Second pass: generate code
bool assembler_second_pass(Assembler *as, const char *source) {
    as->current_address = 0;
    as->output_size = 0;
    as->line_number = 0;
    
    char *source_copy = strdup(source);
    char *line = strtok(source_copy, "\n");
    
    while (line) {
        as->line_number++;
        if (!parse_line(as, line, false)) {
            free(source_copy);
            return false;
        }
        line = strtok(NULL, "\n");
    }
    
    free(source_copy);
    return true;
}

// Assemble file
bool assemble_file(const char *input_file, const char *output_file) {
    FILE *f = fopen(input_file, "r");
    if (!f) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", input_file);
        return false;
    }
    
    // Read source file
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *source = malloc(size + 1);
    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);
    
    // Initialize assembler
    Assembler as;
    assembler_init(&as);
    
    printf("Assembling '%s'...\n", input_file);
    
    // First pass
    if (!assembler_first_pass(&as, source)) {
        fprintf(stderr, "Assembly failed during first pass\n");
        free(source);
        assembler_free(&as);
        return false;
    }
    
    printf("First pass complete. Found %d labels.\n", as.label_count);
    
    // Second pass
    if (!assembler_second_pass(&as, source)) {
        fprintf(stderr, "Assembly failed during second pass\n");
        free(source);
        assembler_free(&as);
        return false;
    }
    
    printf("Second pass complete. Generated %d bytes.\n", as.output_size);
    
    // Write output file
    FILE *out = fopen(output_file, "wb");
    if (!out) {
        fprintf(stderr, "Error: Cannot create output file '%s'\n", output_file);
        free(source);
        assembler_free(&as);
        return false;
    }
    
    fwrite(as.output, 1, as.output_size, out);
    fclose(out);
    
    printf("Output written to '%s'\n", output_file);
    
    free(source);
    assembler_free(&as);
    return true;
}

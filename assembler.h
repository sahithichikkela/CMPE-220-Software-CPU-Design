#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_LABELS 256
#define MAX_LINE_LENGTH 256
#define MAX_TOKEN_LENGTH 64

// Label structure
typedef struct {
    char name[MAX_TOKEN_LENGTH];
    uint16_t address;
} Label;

// Assembler state
typedef struct {
    Label labels[MAX_LABELS];
    int label_count;
    uint8_t *output;
    uint16_t output_size;
    uint16_t current_address;
    int line_number;
} Assembler;

// Function declarations
bool assemble_file(const char *input_file, const char *output_file);
void assembler_init(Assembler *as);
void assembler_free(Assembler *as);
bool assembler_first_pass(Assembler *as, const char *source);
bool assembler_second_pass(Assembler *as, const char *source);
int find_label(Assembler *as, const char *name);
void add_label(Assembler *as, const char *name, uint16_t address);
void emit_byte(Assembler *as, uint8_t byte);
void emit_word(Assembler *as, uint16_t word);
uint8_t encode_instruction(uint8_t opcode, uint8_t mode);
bool parse_line(Assembler *as, char *line, bool first_pass);

#endif // ASSEMBLER_H

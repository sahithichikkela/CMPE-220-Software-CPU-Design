# Software CPU Emulator - CPU Design FA25

A complete software-based 16-bit CPU implementation in C, featuring a custom instruction set architecture (ISA), assembler, and emulator.

## Team Members & Contributions

**Team Member Contributions:**
- **[Vinuta Patil]** - CPU Architecture & ISA Design
- **[Sahithi Chikkela]** - CPU Core & ALU Implementation
- **[Vineela Mukkamala]** - Memory Management & Control Flow
- **[Abhinav Sriharsha Anumanchi]** - Demo Programs, Testing & Documentation

## Architecture Overview

### CPU Schematic

```
┌─────────────────────────────────────────────────────────────┐
│                        CPU CORE                              │
│                                                              │
│  ┌──────────────┐         ┌──────────────┐                 │
│  │   Registers  │         │      ALU     │                 │
│  │              │         │              │                 │
│  │  PC  SP      │◄────────┤  Add  Sub    │                 │
│  │  A   B       │         │  Mul  Div    │                 │
│  │  C   D       │────────►│  And  Or     │                 │
│  │  FLAGS       │         │  Xor  Shift  │                 │
│  └──────────────┘         └──────────────┘                 │
│         │                         │                         │
│         ▼                         ▼                         │
│  ┌──────────────────────────────────────┐                  │
│  │         Control Unit                 │                  │
│  │  ┌────────────┐    ┌──────────────┐ │                  │
│  │  │   Fetch    │───►│    Decode    │ │                  │
│  │  └────────────┘    └──────────────┘ │                  │
│  │         │                  │         │                  │
│  │         ▼                  ▼         │                  │
│  │  ┌────────────────────────────────┐ │                  │
│  │  │         Execute                │ │                  │
│  │  └────────────────────────────────┘ │                  │
│  └──────────────────────────────────────┘                  │
│                      │                                      │
└──────────────────────┼──────────────────────────────────────┘
                       │
                       ▼
        ┌──────────────────────────────┐
        │         System Bus           │
        └──────────────────────────────┘
                       │
        ┌──────────────┴──────────────┐
        │                             │
        ▼                             ▼
┌───────────────┐           ┌─────────────────┐
│    Memory     │           │  Memory-Mapped  │
│    (64KB)     │           │      I/O        │
│               │           │                 │
│  0x0000-0xFEFF│           │  0xFF00: Input  │
│               │           │  0xFF01: Output │
│               │           │  0xFF03: Timer  │
└───────────────┘           └─────────────────┘
```

## Instruction Set Architecture (ISA)

### Instruction Format

```
┌─────────────┬──────────────┬─────────────────────┐
│   Opcode    │     Mode     │      Operand        │
│   (6 bits)  │   (2 bits)   │   (0, 8, or 16 bits)│
└─────────────┴──────────────┴─────────────────────┘
     Byte 0          Byte 0        Byte 1-2 or 1
```

### Addressing Modes

| Mode | Code | Format | Example | Description |
|------|------|--------|---------|-------------|
| Immediate | 00 | #value | `LOAD #42` | Use literal value |
| Direct | 01 | address | `LOAD 0x1000` | Load from memory address |
| Register | 10 | reg | `LOAD B` | Use register value |
| Indirect | 11 | [reg] | `LOAD [B]` | Use address in register |

### Registers

- **PC** (16-bit): Program Counter - points to next instruction
- **SP** (16-bit): Stack Pointer - points to top of stack
- **A** (16-bit): Accumulator - primary register for arithmetic
- **B** (16-bit): General purpose register
- **C** (16-bit): General purpose register
- **D** (16-bit): General purpose register
- **FLAGS** (8-bit): Status flags

### Status Flags

| Bit | Flag | Description |
|-----|------|-------------|
| 0 | Z | Zero - set when result is zero |
| 1 | C | Carry - set on arithmetic overflow/underflow |
| 2 | N | Negative - set when bit 7 of result is 1 |
| 3 | O | Overflow - set on signed overflow |
| 7 | H | Halt - set when CPU halts |

### Instruction Set

#### Data Movement (0x00-0x0F)
- `NOP` - No operation
- `LOAD <operand>` - Load value into A
- `STORE <address>` - Store A to memory
- `MOV <src> <dest>` - Move between registers
- `PUSH <operand>` - Push to stack
- `POP` - Pop from stack to A

#### Arithmetic (0x10-0x1F)
- `ADD <operand>` - Add to A
- `SUB <operand>` - Subtract from A
- `INC <register>` - Increment register
- `DEC <register>` - Decrement register
- `MUL <operand>` - Multiply A by operand
- `DIV <operand>` - Divide A by operand

#### Logical (0x20-0x2F)
- `AND <operand>` - Bitwise AND with A
- `OR <operand>` - Bitwise OR with A
- `XOR <operand>` - Bitwise XOR with A
- `NOT` - Bitwise NOT of A
- `SHL <operand>` - Shift A left
- `SHR <operand>` - Shift A right

#### Comparison (0x30-0x3F)
- `CMP <operand>` - Compare A with operand
- `TEST <operand>` - Test (AND without storing)

#### Control Flow (0x40-0x4F)
- `JMP <address>` - Unconditional jump
- `JZ <address>` - Jump if zero
- `JNZ <address>` - Jump if not zero
- `JC <address>` - Jump if carry
- `JNC <address>` - Jump if not carry
- `CALL <address>` - Call subroutine
- `RET` - Return from subroutine

#### System (0x50-0x5F)
- `HALT` - Stop execution
- `IN <port>` - Input from I/O port
- `OUT <port>` - Output to I/O port

### Instruction Encoding Examples

```
LOAD #42:
  [00000101][00101010][00000000]
   LOAD+IMM   Low byte  High byte

ADD B:
  [01000010][00000001]
   ADD+REG    Reg B (1)

JMP loop:
  [10000001][addr_low][addr_high]
   JMP+IMM    Target address
```

### Memory Map

```
0x0000 - 0xFEFF : General Memory (65,280 bytes)
  0x0000 - 0x00FF : Interrupt vectors (future use)
  0x0100 - 0xFDFF : User program space
  0xFE00 - 0xFEFF : Stack space (256 bytes)

0xFF00 - 0xFFFF : Memory-Mapped I/O (256 bytes)
  0xFF00 : Console input
  0xFF01 : Console output
  0xFF02+ : Reserved for future devices
```

## Fetch-Decode-Execute Cycle

Each instruction executes in these phases:

### 1. FETCH
- Read instruction byte at PC
- Read operand bytes if needed
- Increment PC

### 2. DECODE
- Extract opcode (upper 6 bits)
- Extract addressing mode (lower 2 bits)
- Determine operand location

### 3. EXECUTE
- Perform operation via ALU or Control Unit
- Update flags as needed

### 4. STORE
- Write results to registers or memory
- Update PC for jumps/branches

## Quick Start Guide - How to Run

### Prerequisites
- GCC compiler (any recent version)
- Make utility
- macOS, Linux, or Unix-like system

### Step 1: Build the Project
```bash
cd "Team 15- Software CPU Emulator"
make
```

**Expected output:**
```
gcc -Wall -Wextra -std=c11 -g -c main.c
gcc -Wall -Wextra -std=c11 -g -c cpu.c
gcc -Wall -Wextra -std=c11 -g -c assembler.c
gcc -Wall -Wextra -std=c11 -g -o cpu_emulator main.o cpu.o assembler.o
```

### Step 2: Run Demo Programs

**Option 1: Run all three demos at once**
```bash
make test
```

**Option 2: Run individual demos**

**Timer Demo** (counts 0 to 5 with 1-second hardware timer delays):
```bash
./cpu_emulator demo timer
```
*Expected: Shows Fetch-Decode-Execute cycles for each second, prints 0-5 with real 1-second delays using hardware timer at 0xFF03, completes in ~345 million cycles*

**Fibonacci Demo** (calculates F(5) = 5):
```bash
./cpu_emulator demo fibonacci
```
*Expected: Shows opcode trace, halts after 63 cycles with D register = 5*

**Hello World Demo** (prints "Hello, World!"):
```bash
./cpu_emulator demo hello
```
*Expected: Prints "Hello, World!" to console, halts after 118 cycles*

### Troubleshooting

**If you get "Permission denied":**
```bash
chmod +x cpu_emulator
```

**If you get "exec format error":**
```bash
make clean
make
```

**To clean and rebuild:**
```bash
make clean
make
```

### Assembling Programs
```bash
./cpu_emulator assemble fibonacci.asm fibonacci.bin
```

### Running Binary Programs
```bash
./cpu_emulator run fibonacci.bin
```

## Hardware Features

### Memory-Mapped Hardware Timer

The CPU includes a hardware timer at memory address **0xFF03** that returns milliseconds elapsed since CPU initialization.

**Features:**
- 16-bit timer value (wraps around every ~65 seconds)
- Returns milliseconds since `cpu_init()` was called
- Useful for implementing real-time delays without sleep()
- Demonstrates busy-wait polling in the Timer demo

**Usage Example:**
```assembly
; Read start time
LOAD 0xFF03      ; R0 = current time in milliseconds
MOV R0, R2       ; Save start time in R2

delay_loop:
    LOAD 0xFF03  ; R0 = current time
    SUB R2       ; R0 = elapsed time
    CMP #1000    ; Compare with 1000ms (1 second)
    JC delay_loop ; Loop if elapsed < 1000ms
; Continue after 1 second has elapsed
```

**Timer Demo Implementation:**
The timer demo program uses this hardware timer to create real 1-second delays between each count from 0 to 5, requiring approximately **53 million CPU cycles per second** of busy-waiting.

## Assembly Language Syntax

### Comments
```assembly
; This is a comment
```

### Labels
```assembly
loop:
    LOAD A
    JMP loop
```

### Immediate Values
```assembly
LOAD #42        ; Decimal
LOAD #0x2A      ; Hexadecimal
```

### Register Operations
```assembly
LOAD A          ; Load from register A
MOV A B         ; Copy A to B
INC C           ; Increment register C
```

### Memory Operations
```assembly
LOAD 0x1000     ; Load from address 0x1000
STORE 0x2000    ; Store A to address 0x2000
```

### Indirect Addressing
```assembly
LOAD [B]        ; Load from address stored in B
STORE [C]       ; Store to address stored in C
```

## Example Programs

### 1. Fibonacci Sequence
Calculates first N Fibonacci numbers using iterative approach with registers.

### 2. Hello World
Demonstrates string output using memory-mapped I/O and indirect addressing.

### 3. Timer/Counter
Shows detailed fetch-decode-execute cycle with countdown loop.

## Project Structure

```
.
├── cpu.h              # CPU architecture definitions
├── cpu.c              # CPU emulator implementation
├── assembler.h        # Assembler interface
├── assembler.c        # Two-pass assembler
├── main.c             # Main program and demos
├── Makefile           # Build configuration
├── README.md          # This file
├── fibonacci.asm      # Fibonacci example
├── hello.asm          # Hello World example
└── timer.asm          # Timer example with cycle details
```

## Features

✓ Complete 16-bit CPU architecture
✓ 64KB addressable memory
✓ 6 registers + program counter + stack pointer
✓ 30+ instructions
✓ 4 addressing modes
✓ Status flags (Zero, Carry, Negative, Overflow, Halt)
✓ Stack operations
✓ Memory-mapped I/O
✓ Two-pass assembler with label support
✓ Detailed cycle tracking
✓ Example programs demonstrating features

## Educational Value

This project demonstrates:
- Computer architecture fundamentals
- Instruction set design
- Fetch-decode-execute cycle
- Assembly language programming
- Two-pass assembly process
- Memory hierarchy
- I/O handling
- Stack-based operations

## Future Enhancements

Possible additions:
- Interrupt handling
- More addressing modes
- Floating-point operations
- Graphics/video output
- Debugging features (breakpoints, step-through)
- Performance optimization
- More complex example programs

## License

This is an educational project. Feel free to use and modify for learning purposes.

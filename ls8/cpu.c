#include "cpu.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DATA_LEN 6

/**
 * cpu-ram-read/write functions
 */

unsigned char cpu_ram_read(struct cpu *cpu, unsigned char address){
	return cpu->ram[address];
}

void cpu_ram_write(struct cpu *cpu, unsigned char address, unsigned char value){
	cpu->ram[address] = value;
}

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */

void cpu_load(struct cpu *cpu, char *filename)
{
	FILE *fp;
	char line[1024];
	int address = 0;

	fp = fopen(filename, "r");

	if (fp == NULL) {
		fprintf(stderr, "file not found\n");
		exit(1);
	}

	while (fgets(line, 1024, fp) != NULL) { // not EOF
		char *endptr;

		unsigned char v = strtoul(line, &endptr, 2);

		if (endptr == line) {
			//printf("Ignoring line: %s\n", line);
			continue;
		}

		//printf("%s", line);
		//printf("%u\n", v);
		cpu_ram_write(cpu, address, v);
		address++;
	}

	fclose(fp);

	// TODO: Replace this with something less hard-coded
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
	switch (op) {
	case ALU_MUL:
		cpu->registers[regA] = cpu->registers[regA] * cpu->registers[regB];
		break;

		// TODO: implement more ALU ops
	case ALU_ADD:
		cpu->registers[regA] = cpu->registers[regA] + cpu->registers[regB];
		break;

	case ALU_CMP:
		if(cpu->registers[regA] > cpu->registers[regB])
			cpu->flags = 0b00000100;
		else if(cpu->registers[regA] < cpu->registers[regB])
			cpu->flags = 0b00000010;
		else if(cpu->registers[regA] == cpu->registers[regB])
			cpu->flags = 0b00000001;
		break;
	}
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
	int running = 1; // True until we get a HLT instruction

	while (running) {
		// TODO
		// 1. Get the value of the current instruction (in address PC).
		// 2. Figure out how many operands this next instruction requires
		// 3. Get the appropriate value(s) of the operands following this instruction
		// 4. switch() over it to decide on a course of action.
		// 5. Do whatever the instruction should do according to the spec.
		// 6. Move the PC to the next instruction.
		unsigned char instruction = cpu_ram_read(cpu, cpu->program_counter);
		int num_operands = instruction >> 6;
		int setsPC = instruction >> 4 & 0b0001;
		int L = cpu->flags & 0b00000100 >> 2;
		int G = cpu->flags & 0b00000010 >> 1;
		int E = cpu->flags & 0b00000001;


		//	  unsigned char op_arr[num_operands];

		//	  for(int i; i >= num_operands; i++){
		//		  op_arr[i] = cpu_ram_read(cpu, cpu->program_counter + i);
		//	  }
		unsigned char op1 = cpu_ram_read(cpu, cpu->program_counter + 1);
		unsigned char op2 = cpu_ram_read(cpu, cpu->program_counter + 2);
		//printf("Sets instruction pointer: %d \n",setsPC);
		switch(instruction){
		case CALL:
			cpu->registers[7]--;
			cpu_ram_write(cpu, cpu->registers[7], cpu->program_counter + 2);

			cpu->program_counter = cpu->registers[op1];
			break;

		case RET:
			cpu->program_counter = cpu_ram_read(cpu, cpu->registers[7]);
			cpu->registers[7]++;
			break;

		case PUSH:
			cpu->registers[7]--;
			cpu_ram_write(cpu, cpu->registers[7], cpu->registers[op1]);
			break;

		case POP:
			cpu->registers[op1] = cpu_ram_read(cpu, cpu->registers[7]);
			cpu->registers[7]++;
			break;

		case LDI:
			cpu->registers[op1] = op2;
			break;

		case MUL:
			alu(cpu, ALU_MUL, op1, op2);
			break;

		case ADD:
			alu(cpu, ALU_ADD, op1, op2);
			break;

		case PRN:
			printf("%d\n", cpu->registers[op1]);
			break;

		case HLT:
			running = 0;
			break;

		case JEQ:
			if (E)
				cpu->program_counter = cpu->registers[op1];
			else
				cpu->program_counter += 2;
			break;

		case JGE:
			if (G||E)
				cpu->program_counter = cpu->registers[op1];
			else
				cpu->program_counter += 2;
			break;

		case JGT:
			if (G)
				cpu->program_counter = cpu->registers[op1];
			else
				cpu->program_counter += 2;
			break;

		case JLE:
			if (L||E)
				cpu->program_counter = cpu->registers[op1];
			else
				cpu->program_counter += 2;
			break;

		case JLT:
			if (L)
				cpu->program_counter = cpu->registers[op1];
			else
				cpu->program_counter += 2;
			break;

		case JNE:
			if (!E)
				cpu->program_counter = cpu->registers[op1];
			else
				cpu->program_counter += 2;
			break;

		case JMP:
			cpu->program_counter = cpu->registers[op1];

			break;

		case CMP:
			alu(cpu, ALU_CMP, op1, op2);
			break;
		}

		if(!setsPC)
			cpu->program_counter = cpu->program_counter + num_operands + 1;
	}
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
	// TODO: Initialize the PC and other special registers

	cpu->program_counter = 0;
	cpu->flags = 0b00000000;
	memset(cpu->ram, 0, sizeof cpu->ram);
	memset(cpu->registers, 0, sizeof cpu->registers);
	cpu->registers[7] = 0xF4;
}

//Noah Redden
//7841009
//COMP 3430, Assignment 1
//CPU simulator

#include <stdio.h>
#include <fstream>
#include <string>

using namespace std;

///////////////////////////////////////////////
// constants and structures

// constants for our processor definition (sizes are in words)
#define WORD_SIZE     2
#define DATA_SIZE     1024
#define CODE_SIZE     1024
#define REGISTERS			16

//upper limit for loops, used to detect infinite loops
#define MAX_LOOPS	100

// our opcodes are nicely incremental
enum OPCODES
{
  ADD_OPCODE,
  SUB_OPCODE,
  AND_OPCODE,
  OR_OPCODE,
  XOR_OPCODE,
  MOVE_OPCODE,
  SHIFT_OPCODE,
  BRANCH_OPCODE,
  NUM_OPCODES
};

typedef enum OPCODES Opcode;

// We have specific phases that we use to execute each instruction.
// We use this to run through a simple state machine that always advances to the
// next state and then cycles back to the beginning.
enum PHASES
{
  FETCH_INSTR,
  DECODE_INSTR,
  CALCULATE_EA,
  FETCH_OPERANDS,
  EXECUTE_INSTR,
  WRITE_BACK,
  NUM_PHASES,
  // the following are error return codes that the state machine may return
  ILLEGAL_OPCODE,    // indicates that we can't execute anymore instructions
  INFINITE_LOOP,     // indicates that we think we have an infinite loop
  ILLEGAL_ADDRESS,   // inidates that we have an memory location that's out of range
};

typedef enum PHASES Phase;

// standard function pointer to run our control unit state machine
typedef Phase (*process_phase)(void);


///////////////////////////////////////////////
// prototypes
Phase fetch_instr();
Phase decode_instr();
Phase calculate_ea();
Phase fetch_operands();
Phase execute_instr();
Phase write_back();

void clear_registers();

void clear_code();
void clear_data();

////////////////////////////////////////////////
// local variables


// memory for our code and data, using our word size for a second dimension to make accessing bytes easier
static unsigned char code[CODE_SIZE][WORD_SIZE];
static unsigned char data[DATA_SIZE][WORD_SIZE];


// A list of handlers to process each state. Provides for a nice simple
// state machine loop and is easily extended without using a huge
// switch statement.
static process_phase control_unit[NUM_PHASES] =
{
  fetch_instr, decode_instr, calculate_ea, fetch_operands, execute_instr, write_back
};

//cpu struct
struct cpuStruct{
	int r[REGISTERS];
	
	//program counter
	int pc;

	//mar and mdr
	int mar;
	int mdr;

	//ir
	int ir;

};

static cpuStruct cpu;

//alu struct
struct aluStruct{
	int a;
	int b;
	int y;
	int result;
};

static aluStruct alu;

//counts how many loops have occured to prevent infinite loops
static int loopCount;

/////////////////////////////////////////////////
// state processing routines 



/////////////////////////////////////////////////
// general routines

void initialize_system()
{
	//sets registers to 0
	clear_registers();

	clear_code();
	clear_data();

	cpu.pc = 0;

	cpu.mar = 0;
	cpu.mdr = 0;

	loopCount = 0;
}

void print_memory()
{

	for(int i = 0; i < DATA_SIZE; i++){
		//newline every 16 words
		if(i%8==0)
			printf("\n\n%08x   ",(i*2));

		printf("%02x %02x ",data[i][0],data[i][1]);
		
	}
	printf("\n\n");

}

void print_registers(){
	for(int i = 0; i < REGISTERS; i++)
		printf("%i\n", cpu.r[i]);
}

//clears registers, sets all values to FF
void clear_registers(){
	for(int i = 0; i < REGISTERS; i++)
		cpu.r[i] = 0xFF;
}

//clears code memory, sets all values to FF
void clear_code(){
	for(int i = 0; i < CODE_SIZE; i++){
		for(int j = 0; j < WORD_SIZE; j++){
			code[i][j]=0xFF;
		}
	}

}

//clears data memory, sets all values to FF
void clear_data(){
	for(int i = 0; i < DATA_SIZE; i++){
		for(int j = 0; j < WORD_SIZE; j++){
			data[i][j]=0xFF;
		}
	}

}

// converts the passed string into binary form and inserts it into our data area
// assumes an even number of words!!!
void insert_data( string line )
{
  static int    data_index = 0;
  unsigned int  i;
  char          ascii_data[5];
  unsigned char byte1;
  unsigned char byte2;
  
  ascii_data[4] = '\0';
  
  for ( i=0 ; i<line.length() ; i+=4 )
  {
    ascii_data[0] = line[i];
    ascii_data[1] = line[i+1];
    ascii_data[2] = line[i+2];
    ascii_data[3] = line[i+3];
    if ( data_index < DATA_SIZE*WORD_SIZE )
    {
      sscanf( ascii_data, "%02hhx%02hhx", &byte1, &byte2 );
      data[data_index][0] = byte1;
      data[data_index++][1] = byte2;
    }
  }
}


// reads in the file data and returns true is our code and data areas are ready for processing
bool load_files( const char *code_filename, const char *data_filename )
{
  FILE           *code_file = NULL;
  std::ifstream  data_file( data_filename );
  string         line;           // used to read in a line of text
  bool           rc = false;
  
  // using RAW C here since I want to have straight binary access to the data
  code_file = fopen( code_filename, "r" );
  
  // since we're allowing anything to be specified, make sure it's a file...
  if ( code_file )
  {
    // put the code into the code area
    
fread( code, 1, CODE_SIZE*WORD_SIZE, code_file );
    
    fclose( code_file );
    
    // since we're allowing anything to be specified, make sure it's a file...
    if ( data_file.is_open() )
    {
      // read the data into our data area
      getline( data_file, line );
      while ( !data_file.eof() )
      {
        // put the data into the data area
        insert_data( line );
        
        getline( data_file, line );
      }
      data_file.close();
      
      // both files were read so we can continue processing
      rc = true;
    }
  }
  
  return rc;
}


// runs our simulation after initializing our memory
int main (int argc, const char * argv[])
{
  Phase current_phase = FETCH_INSTR;  // we always start if an instruction fetch
  
  initialize_system();
  
  // read in our code and data
  if ( load_files( argv[1], argv[2] ) )
  {

    // run our simulator
    while ( current_phase < NUM_PHASES )
      current_phase = control_unit[current_phase]();
    
	

    // output what stopped the simulator
    switch( current_phase )
    {
      case ILLEGAL_OPCODE:
        printf( "Illegal instruction %02x%02x detected at address %04x\n\n",
               code[cpu.pc][0],code[cpu.pc][1],cpu.pc);
        break;
        
      case INFINITE_LOOP:
        printf( "Possible infinite loop detected with instruction %02x%02x at address %04x\n\n",
		code[cpu.pc][0],code[cpu.pc][1],cpu.pc );
        break;
        
      case ILLEGAL_ADDRESS:
        printf( "Illegal address %04x detected with instruction %02x%02x at address %04x\n\n",
               cpu.mar,code[cpu.pc][0],code[cpu.pc][1],cpu.pc );
        break;
        
      default:
        break;
    }
    
    // print out the data area
    print_memory();
  }
}

Phase fetch_instr(){
	//fetch instruction, store in ir
	cpu.mar = cpu.pc;
	if(cpu.mar>CODE_SIZE||cpu.mar<0) return ILLEGAL_ADDRESS;//make sure pc is within bounds of code memory

	cpu.mdr = code[cpu.mar][0] << 8;
	cpu.mdr = cpu.mdr | code[cpu.mar][1];

	if(cpu.mdr>65535||cpu.mdr<0) return ILLEGAL_OPCODE;//make sure instruction is actually with 16 bits

	cpu.ir = cpu.mdr;

	return DECODE_INSTR;
}

Phase decode_instr(){
	//checks opcodes for validity
	int num = cpu.ir;

	//check first 3 bits
	int test = num & 57344;
	test = test >> 13;

	if(test>7) return ILLEGAL_OPCODE; //check if valid code

	//if test is greater than 7, then it is not valid
	if(test>7) return ILLEGAL_OPCODE;

	//check bits 4-6
	int test2 = num & 7168;
	test2 = test2 >> 10;

	if(test==5){
		//move code
		if(test2>7) return ILLEGAL_OPCODE; //check if valid move code
		return CALCULATE_EA;
	}else if(test==7){
		//branch code
		if(test2>6) return ILLEGAL_OPCODE; //check if valid branch code
		return FETCH_OPERANDS;
	}else{
		//other opcodes
		if(test2>1) return ILLEGAL_OPCODE; //check if any other codes are valid
		return FETCH_OPERANDS;
	}
}

Phase calculate_ea(){
	//sets up MAR for operations involving memory
	int address = cpu.ir & 63;

	int operation = cpu.ir & 7168;
	operation = operation >> 10;
	if(operation==1||operation==5)address = address>>2;//certain operations require removing last two bits

	//which op is it?
	if(operation==0){
		//literal to register
		//nothing needs to be calculated here
	}else if(operation==1){
		//memory to register
		
	}else if(operation==2||operation==3){
		//register to register
				
	}else if(operation==4){
		//literal to memory
		
	}else if(operation==5){
		//register to memory
		//address variable corresponds to a register with the data address
		cpu.mar = cpu.r[address];
	}else{
		//operation == 6 or 7
		//memory to memory
	}

	return FETCH_OPERANDS;
}

Phase fetch_operands(){
	//fetches operands from register, literal memory, puts them into ALU

	//what are we fetching?
	//checks opcodes
	int num = cpu.ir;

	//check first 3 bits
	int test = num & 57344;
	test = test >> 13;

	//check bits 4-6
	int test2 = num & 7168;
	test2 = test2 >> 10;

	//check bits 7-10
	int test3 = num & 960;
	test3 = test3 >> 6;

	//check bits 11-16
	int test4 = num & 63;

	//bits 7-10 should always be a register
	alu.a = cpu.r[test3];
	if(test==5){
		//move
		if(test2==0){
			//literal to register
			alu.result = test4;
		}else if(test2==1){
			//memory to register
			alu.result = data[cpu.r[test4>>2]][0];
			alu.result = alu.result << 8;
			alu.result = alu.result|data[cpu.r[test4>>2]][1];
		}else if(test2==4){
			//literal to memory
			alu.result = test4;
		}else if(test2==5){
			//register to memory
			cpu.mar = cpu.r[test4>>2];
			alu.result = cpu.mar;

		}
		return WRITE_BACK;
	}else if(test==7){
		//branch
		alu.a = cpu.r[0];
		alu.b = cpu.r[test3];
		alu.y = test;

		int offset = test4&31;
		if((test4&32)==32)
			offset -= 32;

		//use alu here to check conditions
		if(test2==0){
                        //JR
			alu.a = offset;
                }else if(test2==1){
                        //BEQ
			if(alu.a==alu.b)
				alu.a = offset;
			else
				alu.a = 0;

                }else if(test2==2){
                        //BNE
			if(alu.a!=alu.b)
				alu.a = offset;
			else
				alu.a = 0;

                }else if(test2==3){
                        //BLT
			if(alu.a>alu.b)
				alu.a = offset;
			else
				alu.a = 0;
                }else if(test2==4){
                        //BGT
			if(alu.a<alu.b)
				alu.a = offset;
			else
				alu.a = 0;

                }else if(test2==5){
                        //BLE
			if(alu.a>=alu.b)
				alu.a = offset;
			else
				alu.a = 0;

                }else{
                        //BGE
			if(alu.a<=alu.b)
				alu.a = offset;
			else
				alu.a = 0;

                }
		if(cpu.pc+offset<0||cpu.pc+offset>CODE_SIZE) return ILLEGAL_OPCODE;//should not jump out of bounds of code memory
		return EXECUTE_INSTR;
	}else if(test==6){
		//shift
		if(test2==0){
			//right >>
		}else{
			//left <<
		}
		

	}else{
		//other ops
		//literal or register?
		if(test2==1){
			//register
			alu.b = cpu.r[test4>>2];
		}else{
			//literal
			alu.b = test4;
		}
		alu.y = test;
		return EXECUTE_INSTR;
	}
	return ILLEGAL_OPCODE;

}

Phase execute_instr(){
	//figure out which opcode is on y
	if(alu.y==0){
		//add
		alu.result = alu.a + alu.b;
	}else if(alu.y==1){
		//sub
		alu.result = alu.a - alu.b;
	}else if(alu.y==2){
		//and
		alu.result = alu.a & alu.b;
	}else if(alu.y==3){
		//or
		alu.result = alu.a | alu.b;
	}else if(alu.y==4){
		//xor
		alu.result = alu.a ^ alu.b;
	}else if(alu.y==6){
		//shift
		
	}else if(alu.y==7){
		//branch
		//if the branch condition is true, then jump
		//should figure that out here, along with target
		if(alu.a!=0){
			cpu.pc += alu.a;
			cpu.pc--;

			loopCount++;
		}
	}

	//check for infinite loop
	if(loopCount > MAX_LOOPS)return INFINITE_LOOP;	
	return WRITE_BACK;
}

Phase write_back(){
	//check bits 7-10
	int destination = cpu.ir & 960;
	destination = destination >> 6;

	//is destination a register or memory?
	//check bits 4-6
	int opCode1 = cpu.ir & 7168;
	opCode1 = opCode1 >> 10;

	//which op?
	//check first 3 bits
	int opCode2 = cpu.ir & 57344;
	opCode2 = opCode2 >> 13;

		
	if(opCode1>=4&&opCode2==5){
		//memory destination for move
		//converts alu's result
		data[cpu.r[destination]][0] = (alu.result&65280)>>8;
		data[cpu.r[destination]][1] = alu.result&255;
	}else if(opCode2!=7){
		//register destination, should not be branch
		cpu.r[destination] = alu.result;
	}

	cpu.pc++;

	return FETCH_INSTR;
}

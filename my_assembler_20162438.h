/*
 *my_assembler �Լ��� ���� ���� ���� �� ��ũ�θ� ��� �ִ� ��� �����̴�.
 *
 */
#define MAX_INST 256
#define MAX_LINES 5000
#define MAX_OPERAND 3

/*
 *instruction ��� ���Ϸ� ���� ������ �޾ƿͼ� �����ϴ� ����ü �����̴�.
 *���� ���� �ϳ��� instruction�� �����Ѵ�.
 */
struct inst_unit
{
	char str [ 10 ] ;
	unsigned char op ;
	int format ;
	int ops ;
} ;
typedef struct inst_unit inst ;
inst * inst_table [ MAX_INST ] ;
int inst_index ;

/*
 *����� �� �ҽ��ڵ带 �Է¹޴� ���̺��̴�. ���� ������ ������ �� �ִ�.
 */
char * input_data [ MAX_LINES ] ;
static int line_num ;

/*
 *����� �� �ҽ��ڵ带 ��ū������ �����ϱ� ���� ����ü �����̴�.
 *operator�� renaming�� ����Ѵ�.
 */
struct token_unit
{
	char*label ;
	char*operator_ ;
	char*operand [ MAX_OPERAND ] ;
	char comment [ 100 ] ;
	char nixbpe ;
} ;

typedef struct token_unit token ;
token * token_table [ MAX_LINES ] ;
static int token_line ;

/*
 *�ɺ��� �����ϴ� ����ü�̴�.
 *�ɺ� ���̺��� �ɺ� �̸�, �ɺ��� ��ġ�� �����ȴ�.
 */
struct symbol_unit
{
	char symbol [ 10 ] ;
	int addr ;
} ;

/*
 *���ͷ��� �����ϴ� ����ü�̴�.
 *���ͷ� ���̺��� ���ͷ��� �̸�, ���ͷ��� ��ġ�� �����ȴ�.
 */
struct literal_unit {
	char*literal ;
	int addr ;
} ;

typedef struct symbol_unit symbol ;
symbol sym_table [ MAX_LINES ] ;
static int sym_line ;							// User defined variable. To count symbol table line

typedef struct literal_unit literal ;
literal literal_table [ MAX_LINES ] ;
static int literal_line ;						// User defined variable. To count literal table line

static int locctr ;

static int g_irgProgramLength [ 100	] ;			// User defined variable. Max 100 routine, and [ n ] means program length of n+1 th routine
static int g_irgLiteralCount [ 100 ] ;			// User defined variable. Max 100 routine, and [ n ] means literal count of n+1 th routine

static char g_cdrgRegiter [ 9 ] [ 3 ] = { "A" , "X" , "L" , "B" , "S" , "T" , "F" , "PC" , "SW" } ;
												// User defined variable. Check string to check if register

struct extref_unit								// User defined struct. Save where EXTREF is used, with what byte
{
	char * literal ;
	int addr ;
	int half_byte ;
	char cSign ;
} ;
typedef struct extref_unit extref ;
extref extref_table [ 100 ] ;					// User defined variable. Save EXTREF is used at current section
static int extref_count ;						// User defined variable. Count EXTREF is used at current section
char * cprgEXTREFList [ 10 ] ;					// User defined variable. Save string of EXTREF at current section
static int g_iExtrefPointCount ;				// User defined variable. Count EXTREF at current section

//--------------

static char * input_file ;
static char * output_file ;
int init_my_assembler ( void ) ;
int init_inst_file ( char * inst_file ) ;
int init_input_file ( char * input_file ) ;
int token_parsing ( char * str ) ;
int search_opcode ( char * str ) ;
static int assem_pass1 ( void ) ;
void make_opcode_output ( char * file_name ) ;

void make_symtab_output ( char * file_name ) ;
void make_literaltab_output ( char * file_name ) ;
static int assem_pass2 ( void ) ;
void make_objectcode_output ( char * file_name ) ;

int iStringToHex ( char * str ) ;				// Change string to hex
char cHexToChar ( const int ciNum ) ;			// Change hex to char
void clearMemory () ;							// Free all the memory
int iGetOperandByte ( const int ciOpcode ) ;	// Get operand size using opcode
int iGetInstOperandNum ( const int ciOpcode ) ;	// Get number of instruction operand using opcode
void setInformationAfterPass1 () ;				// After pass 1, set informations
int iGetSymLocation ( char * str , int iSection ) ;	// Get symbol location using str
int iGetLitLocation ( char * str ) ;			// Get literal location using str
void printToFileOrConsole ( FILE * file , char * cpString ) ;		// Print string to file or console
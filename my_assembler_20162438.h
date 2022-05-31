/*
 *my_assembler 함수를 위한 변수 선언 및 매크로를 담고 있는 헤더 파일이다.
 *
 */
#define MAX_INST 256
#define MAX_LINES 5000
#define MAX_OPERAND 3

/*
 *instruction 목록 파일로 부터 정보를 받아와서 생성하는 구조체 변수이다.
 *라인 별로 하나의 instruction을 저장한다.
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
 *어셈블리 할 소스코드를 입력받는 테이블이다. 라인 단위로 관리할 수 있다.
 */
char * input_data [ MAX_LINES ] ;
static int line_num ;

/*
 *어셈블리 할 소스코드를 토큰단위로 관리하기 위한 구조체 변수이다.
 *operator는 renaming을 허용한다.
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
 *심볼을 관리하는 구조체이다.
 *심볼 테이블은 심볼 이름, 심볼의 위치로 구성된다.
 */
struct symbol_unit
{
	char symbol [ 10 ] ;
	int addr ;
} ;

/*
 *리터럴을 관리하는 구조체이다.
 *리터럴 테이블은 리터럴의 이름, 리터럴의 위치로 구성된다.
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
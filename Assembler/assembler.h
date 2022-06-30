#define MAX_INST 256
#define MAX_LINES 5000
#define MAX_OPERAND 3
#define MAX_SECTION 50

/*
 * Instruction info
 */
typedef struct inst_unit
{
	char * m_cpOperation ;
	unsigned char m_uiOpcode ;
	int m_iFormat ;
	int m_iOperands ;
} inst ;
inst * g_pInst_table [ MAX_INST ] ;
int g_iInst_count ;

/*
 * Assembly line info
 */
char * g_cpInput_data [ MAX_LINES ] ;
static int g_siLine_count ;

/*
 * Convert assembly line to token
 */
typedef struct token_unit
{
	char * m_cpLabel ;
	char * m_cpOperator ;
	char * m_cpOperand [ MAX_OPERAND ] ;
	char m_cNixbpe ;
} token ;
token * g_pToken_table [ MAX_LINES ] ;
static int g_iToken_count ;

/*
 * Symbol info
 */
typedef struct symbol_unit
{
	char * m_cpSymbol ;
	int m_iAddr ;
} symbol ;
symbol * g_Symbol_table [ MAX_LINES ] ;
static int g_iSymbol_count ;
					// Symbol�� Literal�� ������ ���� ���� ������ ����ϱ� �ؼ� ���� ����� ����
					// �ٵ� �̰Ÿ� ��ġ�� �ΰ� ó���� �� �� �ϳ��� üũ�ؾ� �ϴ°� �� ���� ������
					// �׷��� �̰Ÿ� ��ġ�°� ������? �ƴϸ� ����ó�� �и��ϴ°� �´°ǰ�?
					// ���� ������Ʈ �� �ƴ϶�, �ǹ������� ��ó�� A�� B�� ���빰�� ������ �ᱹ ���� ó������ ���� �ٸ� ���
					// A�� B�� ���ļ� ó���ϴ°� ������? ���� ó���ϴ°� ������? �̷� ���鿡�� �ٶ󺸾��� �� ���� �������� �𸣰���
					// MVC ������ �����´� ġ�� Model ���鿡���� ��ġ�� bool�̳� string ������ �̰� A�� B, Ȥ�� �ٸ��͵��� ���� �ǹ��ϴ��� ���� �ϳ��� �߰��ϴ� ���� �� ���� ���̴µ�
					// Control ���鿡���� ���� �ٸ��ǵ� �̰� ���ĳ��� �Ź� ó���� �� ������ A�� �´��� üũ�ؾ� �ϰ� �̷� ���鿡�� ������ ����⵵ �ϰ�
					// �̰Ŵ� �����ڵ����� ����� �پ��� �ǰ��� ���°� ���� �� ����.
					// ���� �����ڵ��� ����� "��Ȳ�� ���� �޶��" �� ������ �� �� ����......
/*
 * Literal info
 */
typedef struct literal_unit {
	char * m_cpLiteral ;
	int m_iAddr ;
} USER_Literal ;		// literal keyword exist
USER_Literal * g_Literal_table [ MAX_LINES ] ;
static int g_iLiteral_count ;

static int g_iLocctr ;

static int g_irgProgramLengthforEach [ MAX_SECTION	] ;	// Program length of each section | routine
static int g_irgLiteralCountforEach [ MAX_SECTION ] ;	// Literal counts of each section | routine

static char g_cdrgRegiter [ 9 ] [ 3 ] = { "A" , "X" , "L" , "B" , "S" , "T" , "F" , "PC" , "SW" } ;
												// Register name to check

/*
 * External reference info
 */
typedef struct extref_unit
{
	char * m_cpLiteral ;
	int m_iAddr ;
	int m_iHalf_byte ;
	char m_cSign ;
} extref ;
extref * g_Extref_table [ 100 ] ;
static int g_iExtref_count ;
// char * g_cprgExtrefList [ 10 ] ;				// User defined variable. Save string of EXTREF at current section
// static int g_iExtrefPointCount ;				// User defined variable. Count EXTREF at current section

int init_assembler ( void ) ;
int init_inst_table ( char * cpInst_file ) ;
int init_input_file ( char * cpInput_file ) ;
int token_parsing ( char * cpStr ) ;
int search_opcode ( char * cpStr ) ;
int assem_pass1 () ;
int iSetSymbolLiteralInfo () ;
int iSetAddrNixbpeInfo () ;
void make_objectcode_output ( char * cpFile_name ) ;

int iStringToHex ( char * cpStr ) ;				// Change string to hex
char cHexToChar ( const int ciNum ) ;			// Change hex to char
void clearMemory () ;							// Free all the memory
int iGetOperandByte ( const int ciOpcode ) ;	// Get operand size using opcode
int iGetInstOperandNum ( const int ciOpcode ) ;	// Get number of instruction operand using opcode
int iGetSymLocation ( char * cpStr ) ;
int iGetLitLocation ( char * cpStr ) ;
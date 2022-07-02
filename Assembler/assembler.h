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
int g_iLine_count ;

/*
 * Convert assembly line to token
 * Add byte and displacement variable to separate process
 * At former version, need to process to save into file
 * So add these variable, and separate process and saving
 * If connect to GUI simulator, then can connect and send JSON format
 * Then make connect & JSON conversion function instead saving function
 */
typedef struct token_unit
{
	char * m_cpLabel ;
	char * m_cpOperator ;
	char * m_cpOperand [ MAX_OPERAND ] ;
	char m_cNixbpe ;
	int m_iByte ;
	int m_iDisplacement ;		// Base relative, then 0 <= disp <= 4095
								// PC relative, then -2048 <= disp <= 2047
} token ;
token * g_pToken_table [ MAX_LINES ] ;
int g_iToken_count ;

/*
 * Symbol info
 */
typedef struct symbol_unit
{
	char * m_cpSymbol ;
	int m_iAddr ;
	int m_iByte ;
} symbol ;
symbol * g_Symbol_table [ MAX_LINES ] ;
int g_iSymbol_count ;
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
	int m_iByte ;
} USER_Literal ;		// literal keyword exist
USER_Literal * g_Literal_table [ MAX_LINES ] ;
int g_iLiteral_count ;

int g_iLocctr ;

int g_irgProgramLengthforEach [ MAX_SECTION ] ;	// Program length of each section | routine
int g_irgLiteralCountforEach [ MAX_SECTION ] ;	// Literal counts of each section | routine

char g_cdrgRegiter [ 9 ] [ 3 ] = { "A" , "X" , "L" , "B" , "S" , "T" , "F" , "PC" , "SW" } ;
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
int g_iExtref_count ;


int init_assembler ( void ) ;
int init_inst_table ( char * cpInst_file ) ;
int init_input_file ( char * cpInput_file ) ;
int token_parsing ( char * cpStr ) ;
int search_opcode ( char * cpStr ) ;
int assem_pass1 () ;
int iSetByteOfToken () ;
int iSetSymbolLiteralInfo () ;
int iSetAddrNixbpeInfo () ;
void tempSetSomething () ;
int iPrintObjectCode ( char * cpFile_name ) ;

int iStringToHex ( char * cpStr ) ;				// Change string to hex
char cHexToChar ( const int ciNum ) ;			// Change hex to char
void clearMemory () ;							// Free all the memory
int iGetOperandByte ( const int ciOpcode ) ;	// Get operand size using opcode
int iGetInstOperandNum ( const int ciOpcode ) ;	// Get number of instruction operand using opcode
int iGetSymLocation ( char * cpStr ) ;
int iGetLitLocation ( char * cpStr ) ;
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
					// Symbol과 Literal이 가지는 것은 같고 개념이 비슷하긴 해서 묶는 방법도 있음
					// 근데 이거를 합치면 두개 처리를 할 때 하나씩 체크해야 하는게 좀 많이 귀찮음
					// 그래서 이거를 합치는게 맞을까? 아니면 지금처럼 분리하는게 맞는건가?
					// 현재 프로젝트 뿐 아니라, 실무에서도 이처럼 A와 B의 내용물은 같지만 결국 서로 처리해줄 것이 다를 경우
					// A와 B를 합쳐서 처리하는게 나은가? 따로 처리하는게 나은가? 이런 측면에서 바라보았을 때 뭐가 정답일지 모르겠음
					// MVC 패턴을 가져온다 치면 Model 측면에서는 합치고 bool이나 string 등으로 이게 A와 B, 혹은 다른것들중 뭐를 의미하는지 변수 하나를 추가하는 것이 더 나아 보이는데
					// Control 측면에서는 서로 다른건데 이걸 합쳐놔서 매번 처리를 할 때마다 A가 맞는지 체크해야 하고 이런 측면에서 문제가 생기기도 하고
					// 이거는 현직자들한테 물어보고 다양한 의견을 들어보는게 나을 것 같다.
					// 물론 현직자들의 대답은 "상황에 따라 달라요" 로 통일이 될 것 같다......
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
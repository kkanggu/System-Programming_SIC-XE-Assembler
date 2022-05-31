#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#pragma warning ( disable : 4996 )

#include "my_assembler_20162438.h"

/* ------------------------------------------------------------
 * 설명 : 사용자로 부터 어셈블리 파일을 받아서 명령어의 OPCODE를 찾아 출력한다.
 * 매계 : 실행 파일 , 어셈블리 파일 
 * 반환 : 성공 = 0 , 실패 = < 0 
 * 주의 : 현재 어셈블리 프로그램의 리스트 파일을 생성하는 루틴은 만들지 않았다. 
 * 		 또한 중간파일을 생성하지 않는다. 
 * ------------------------------------------------------------ */
int main ( int args , char * arg [] )
{
	if ( init_my_assembler () < 0 )
	{
		printf ( "init_my_assembler: 프로그램 초기화에 실패 했습니다.\n" ) ;
		return -1 ;
	}

	if ( assem_pass1 () < 0 )
	{
		printf ( "assem_pass1: 패스1 과정에서 실패하였습니다. \n" ) ;
		return -1 ;
	}

	setInformationAfterPass1 () ;

//	make_opcode_output ( "output_20162438" ) ;

	make_symtab_output ( "symtab_20162438" ) ;
	make_literaltab_output ( "literaltab_20162438" ) ;
 	if ( assem_pass2 () < 0 )
	{
		printf ( " assem_pass2: 패스2 과정에서 실패하였습니다. \n" ) ;
		return -1 ;
	}

	make_objectcode_output ( "output_20162438" ) ;

	clearMemory () ;


	return 0 ;
}

/* ------------------------------------------------------------ * 설명 : 프로그램 초기화를 위한 자료구조 생성 및 파일을 읽는 함수이다. 
 * 매계 : 없음
 * 반환 : 정상종료 = 0 , 에러 발생 = -1
 * 주의 : 각각의 명령어 테이블을 내부에 선언하지 않고 관리를 용이하게 하기 위해서
 * 	 파일 단위로 관리하여 프로그램 초기화를 통해 정보를 읽어 올 수 있도록
 * 	 구현하였다. 
 * ------------------------------------------------------------ */
int init_my_assembler ( void )
{
	int result ;

	if ( ( result = init_inst_file ( "inst.data" ) ) < 0 )
		return -1 ;
	if ( ( result = init_input_file ( "input.txt" ) ) < 0 )
		return -1 ;
	return result ;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 머신을 위한 기계 코드목록 파일을 읽어 기계어 목록 테이블 ( inst_table )을 
 * 생성하는 함수이다. 
 * 매계 : 기계어 목록 파일
 * 반환 : 정상종료 = 0 , 에러 < 0 
 * 주의 : 기계어 목록파일 형식은 자유롭게 구현한다. 예시는 다음과 같다.
 * 	
 * 	===============================================================================
 * 		 | 이름 | 형식 | 기계어 코드 | 오퍼랜드의 갯수 | NULL|
 * 	===============================================================================	 
 * 		
 * ----------------------------------------------------------------------------------
 */
int init_inst_file ( char * inst_file )
{
	FILE * file ;
	int errno ;
	char str [ 10 ] = "\0 , " ;



	file = fopen ( inst_file , "r" ) ;

	if ( file == NULL )
	{
		errno = -1 ;
	}
	else
	{
		while ( EOF != fscanf ( file , "%s" , str ) )				// Read operator name
		{
			inst_table [ inst_index ] = malloc ( sizeof ( inst ) ) ;
			strcpy ( inst_table [ inst_index ] -> str , str ) ;
			fscanf ( file , "%s" , str ) ;							// Read operation code
			inst_table [ inst_index ] -> op = iStringToHex ( str ) ;
			fscanf ( file , "%s" , str ) ;							// Read operation format
			inst_table [ inst_index ] -> format = atoi ( str ) ;		// 1, 2, 3, 4 -> 1, 2, 4, 8
			fscanf ( file , "%s" , str ) ;							// Read operand number
			inst_table [ inst_index ] -> ops = atoi ( str ) ;

			++ inst_index ;
		}

		if ( inst_index != MAX_INST - 1 )
		{
			inst_table [ inst_index ] = NULL ;
		}

		fclose ( file ) ;
	}


	return errno ;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 어셈블리 할 소스코드를 읽어 소스코드 테이블 ( input_data )를 생성하는 함수이다. 
 * 매계 : 어셈블리할 소스파일명
 * 반환 : 정상종료 = 0 , 에러 < 0 
 * 주의 : 라인단위로 저장한다.
 * 		
 * ----------------------------------------------------------------------------------
 */
int init_input_file ( char * input_file )
{
	FILE * file ;
	int errno ;
	int iCount = 0 ;
	char str [ MAX_INST ] = "\0 , " ;



	file = fopen ( input_file , "r" ) ;

	if ( file == NULL )
	{
		errno = -1 ;
	}
	else
	{
		while ( fgets ( str , MAX_INST , file )	)					// Read operator name
		{
			input_data [ iCount ] = malloc ( strlen ( str ) + 1 ) ;
			strcpy ( input_data [ iCount ] , str ) ;

			++ iCount ;
		}

		if ( iCount != MAX_LINES - 1 )
		{
			input_data [ iCount ] = NULL ;
		}

		fclose ( file ) ;
	}


	return errno ;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 소스 코드를 읽어와 토큰단위로 분석하고 토큰 테이블을 작성하는 함수이다. 
 * 패스 1로 부터 호출된다. 
 * 매계 : 파싱을 원하는 문자열 
 * 반환 : 정상종료 = 0 , 에러 < 0 
 * 주의 : my_assembler 프로그램에서는 라인단위로 토큰 및 오브젝트 관리를 하고 있다. 
 * ----------------------------------------------------------------------------------
 */
int token_parsing ( char * str )
{
	char * cpTemp ;
	int iOperand = 0 ;
	char crgTemp [ 100 ] = { 0 , } ;



	if ( NULL == str )														// EOF
	{
		return 0 ;
	}

	token_table [ token_line ] = malloc ( sizeof ( token ) ) ;

	token_table [ token_line ] -> nixbpe = 0 ;
	token_table [ token_line ] -> label = NULL ;								// Initialize for free
	token_table [ token_line ] -> operator_ = NULL ;
	token_table [ token_line ] -> operand [ 0 ] = NULL ;
	token_table [ token_line ] -> comment [ 0 ] = '\0' ;


	if ( '.' == str [ 0 ] )													// Annotation
	{
		token_table [ token_line ] -> label = malloc ( 2 * sizeof ( char ) ) ;
		token_table [ token_line ] -> label [ 0 ] = '.' ;						// This function read only comments. . is ignored
		token_table [ token_line ] -> label [ 1 ] = '\0' ;

	
		cpTemp = strtok ( str , ".\n" ) ;


		if ( NULL != cpTemp )												// Comment copy if exist
		{
			strcpy ( token_table [ token_line ] -> comment , cpTemp ) ;
		}
	}
	else
	{
		if ( '\t' != str [ 0 ] )											// Label found
		{
			cpTemp = strtok ( str , "\t \n" ) ;

			token_table [ token_line ] -> label = malloc ( strlen ( cpTemp ) + 1 ) ;
			strcpy ( token_table [ token_line ] -> label , cpTemp ) ;

			cpTemp = strtok ( NULL , "\t \n" ) ;
		}
		else																		// Label not found, initialize
		{
			cpTemp = strtok ( str , "\t \n" ) ;
		}

		token_table [ token_line ] -> operator_ = malloc ( strlen ( cpTemp ) + 1 ) ;
		strcpy ( token_table [ token_line ] -> operator_ , cpTemp ) ;


		if ( ( 0 != strcmp ( cpTemp , "RSUB" ) ) && ( 0 != strcmp ( cpTemp , "LTORG" ) )
			&& ( 0 != strcmp ( cpTemp , "CSECT" ) ) )						// If true, get operand
		{
			cpTemp = strtok ( NULL , "\t \n" ) ;										// Full of operand

			strcpy ( crgTemp , cpTemp ) ;
		}
		
		cpTemp = strtok ( NULL , "\t \n" ) ;									// Process comment first

		if ( cpTemp != NULL )												// Comment exist
		{
			strcpy ( token_table [ token_line ] -> comment , cpTemp ) ;
		}

		if ( '\0' != crgTemp [ 0 ] )										// If operand exist, cpOprandTemp not null
		{
			cpTemp = strtok ( crgTemp , "," ) ;										// Split using ','

			token_table [ token_line ] -> operand [ 1 ] = NULL ;
			token_table [ token_line ] -> operand [ 2 ] = NULL ;

			while ( NULL != cpTemp )												// Operand remain
			{
				token_table [ token_line ] -> operand [ iOperand ] = malloc ( strlen ( cpTemp ) + 1 ) ;
				strcpy ( token_table [ token_line ] -> operand [ iOperand ++ ] , cpTemp ) ;
				cpTemp = strtok ( NULL , "," ) ;
			}
		}
	}


	return 0 ;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 입력 문자열이 기계어 코드인지를 검사하는 함수이다. 
 * 매계 : 토큰 단위로 구분된 문자열 
 * 반환 : 정상종료 = 기계어 테이블 인덱스 , 에러 < 0 
 * 주의 : 
 * 		
 * ----------------------------------------------------------------------------------
 */
int search_opcode ( char * str )
{
	int iCount = 0 ;
	char cTemp [ 10 ] ;



	strcpy ( cTemp , str ) ;

	if ( '+' == str [ 0 ] )
	{
		for ( iCount = 0 ; iCount < strlen ( str ) ; ++ iCount )
		{
			cTemp [ iCount ] = str [ iCount + 1 ] ;
		}
	}

	while ( ( iCount < MAX_INST ) && ( NULL != inst_table [ iCount ] ) )
	{
		if ( 0 == strcmp ( cTemp , inst_table [ iCount ] ) )
		{
			return inst_table [ iCount ] -> op ;
		}

		++ iCount ;
	}

	return -1 ;								// Can't find opcode
}

/* ----------------------------------------------------------------------------------
 * 설명 : 어셈블리 코드를 위한 패스1과정을 수행하는 함수이다.
 * 		 패스1에서는..
 * 		 1. 프로그램 소스를 스캔하여 해당하는 토큰단위로 분리하여 프로그램 라인별 토큰
 * 		 테이블을 생성한다.
 * 
 * 매계 : 없음
 * 반환 : 정상 종료 = 0 , 에러 = < 0
 * 주의 : 현재 초기 버전에서는 에러에 대한 검사를 하지 않고 넘어간 상태이다.
 * 	 따라서 에러에 대한 검사 루틴을 추가해야 한다.
 * 
 * -----------------------------------------------------------------------------------
 */
static int assem_pass1 ( void )
{
	token_line = 0 ;



	while ( ( token_line < MAX_LINES ) && ( NULL != input_data [ token_line ] ) )			// While input left
	{
		if ( 0 > token_parsing ( input_data [ token_line ] ) )		// Error
		{
			printf ( "Parsing failed.\n" ) ;
			
			return -1 ;
		}

		++ token_line ;
	}


	return 0 ;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
 * 여기서 출력되는 내용은 명령어 옆에 OPCODE가 기록된 표 ( 과제 3번 )이다.
 * 매계 : 생성할 오브젝트 파일명
 * 반환 : 없음
 * 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
 * 화면에 출력해준다.
 * 또한 과제 3번에서만 쓰이는 함수이므로 이후의 프로젝트에서는 사용되지 않는다.
 * -----------------------------------------------------------------------------------
 */
void make_opcode_output ( char * file_name )
{
	FILE * file ;
	int iCount = 0 ;
	int iTemp = 0 ;



	file = fopen ( file_name , "w+" ) ;

	for ( iCount = 0 ; iCount < line_num ; ++ iCount )
	{
		if ( ( NULL != token_table [ iCount ] -> label ) && ( '.' == token_table [ iCount ] -> label [ 0 ] ) )
		{
			if ( NULL != file )
				fprintf ( file , "%s\t%s\n" , token_table [ iCount ] -> label , token_table [ iCount ] -> comment ) ;
			else
				printf ( "%s\t%s\n" , token_table [ iCount ] -> label , token_table [ iCount ] -> comment ) ;
		}
		else
		{
			if ( NULL != token_table [ iCount ] -> label )							// Print label
			{
				if ( NULL != file )
					fprintf ( file , "%s" , token_table [ iCount ] -> label ) ;
				else
					printf ( "%s" , token_table [ iCount ] -> label ) ;
			}
			if ( NULL != file )
				fprintf ( file , "\t" ) ;
			else
				printf ( "\t" ) ;

			if ( NULL != token_table [ iCount ] -> operator_ )						// Print operator
			{
				if ( NULL != file )
					fprintf ( file , "%s" , token_table [ iCount ] -> operator_ ) ;
				else
					printf ( "%s" , token_table [ iCount ] -> operator_ ) ;
			}
			if ( NULL != file )
				fprintf ( file , "\t" ) ;
			else
				printf ( "\t" ) ;

			if ( NULL != token_table [ iCount ] -> operand [ 0 ] )					// Print operand
			{
				if ( NULL != file )
					fprintf ( file , "%s" , token_table [ iCount ] -> operand [ 0 ] ) ;
				else
					printf ( "%s" , token_table [ iCount ] -> operand [ 0 ] ) ;

				for ( iTemp = 1 ; ( iTemp < MAX_OPERAND ) && ( NULL != token_table [ iCount ] -> operand [ iTemp ] ) ; ++ iTemp )
				{
					if ( NULL != file )
						fprintf ( file , ",%s" , token_table [ iCount ] -> operand [ iTemp ] ) ;
					else
						printf ( ",%s" , token_table [ iCount ] -> operand [ iTemp ] ) ;
				}
			}
			if ( NULL != file )
				fprintf ( file , "\t" ) ;
			else
				printf ( "\t" ) ;


			iTemp = search_opcode ( token_table [ iCount ] -> operator_ ) ;

			if ( iTemp >= 0 )
			{
				if ( NULL != file )
					fprintf ( file , "%02X" , iTemp ) ;
				else
					printf ( "%02X" , iTemp ) ;
			}

			if ( NULL != file )
				fprintf ( file , "\n" ) ;
			else
				printf ( "\n" ) ;
		}
	}

	if ( NULL != file )
		fclose ( file ) ;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
 * 여기서 출력되는 내용은 SYMBOL별 주소값이 저장된 TABLE이다.
 * 매계 : 생성할 오브젝트 파일명
 * 반환 : 없음
 * 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
 * 화면에 출력해준다.
 * 
 * -----------------------------------------------------------------------------------
 */
void make_symtab_output ( char * file_name )
{
	FILE * file ;
	int i = 0 ;



	file = fopen ( file_name , "w+" ) ;

	for ( i = 0 ; i < sym_line ; ++i )
	{
		if ( '\0' != sym_table [ i ].symbol [ 0 ] )
		{
			if ( NULL != file )
				fprintf ( file , "%s\t\t\t%+6X" , sym_table [ i ].symbol , sym_table [ i ].addr ) ;
			else
				printf ( "%s\t\t\t%+6X" , sym_table [ i ].symbol , sym_table [ i ].addr ) ;
		}

		if ( NULL != file )
			fprintf ( file , "\n" ) ;
		else
			printf ( "\n" ) ;
	}

	if ( NULL != file )
		fclose ( file ) ;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 LITERAL별 주소값이 저장된 TABLE이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*
* -----------------------------------------------------------------------------------
*/
void make_literaltab_output(char* file_name)
{
	FILE * file ;
	int i = 0 ;
	int iTemp = 0 ;
	char crgPrint [ 50 ] = { 0 , } ;
	char crgTemp [ 20 ] = { 0 , } ;
	char * cpTemp ;



	file = fopen ( file_name , "w+" ) ;

	for ( i = 0 ; i < literal_line ; ++i )
	{
		cpTemp = literal_table [ i ].literal ;


		if ( '\0' != cpTemp [ 0 ] )
		{
			if ( 'X' == cpTemp [ 1 ] )
			{
				strncpy ( crgPrint , cpTemp + 3 , 2 ) ;
				crgPrint [ 2 ] = '\0' ;
			}
			else
			{
				strncpy ( crgPrint , cpTemp + 3 , strlen ( cpTemp ) - 4 ) ;
				crgPrint [ strlen ( cpTemp ) - 4 ] = '\0' ;
			}

			sprintf ( crgTemp , "\t\t\t%+6x" , literal_table [ i ].addr ) ;
			strcat ( crgPrint , crgTemp ) ;

			printToFileOrConsole ( file , crgPrint ) ;
		}
	}

	if ( NULL != file )
		fclose ( file ) ;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 어셈블리 코드를 기계어 코드로 바꾸기 위한 패스2 과정을 수행하는 함수이다.
 * 		 패스 2에서는 프로그램을 기계어로 바꾸는 작업은 라인 단위로 수행된다.
 * 		 다음과 같은 작업이 수행되어 진다.
 * 		 1. 실제로 해당 어셈블리 명령어를 기계어로 바꾸는 작업을 수행한다.
 * 매계 : 없음
 * 반환 : 정상종료 = 0 , 에러발생 = < 0
 * 주의 :
 * -----------------------------------------------------------------------------------
 */
static int assem_pass2 ( void )
{
	int i = 0 ;
	int iByte = 0 ;
	int iLocation = 0 ;
	int iSection = -1 ;
	int j = 0 ;
	int iTemp = 0 ;
	int iCurrentLiteralCount = 0 ;
	int iTotalLiteralCount = 0 ;
	locctr = 0 ;


	for ( ; i < token_line ; ++i )
	{
		if ( ( NULL != token_table [ i ] -> label ) && ( 0 == strcmp ( "." , token_table [ i ] -> label ) ) )
		{
			continue ;																		// Skip annotation
		}

		if ( ( 0 == strcmp ( "START" , token_table [ i ] -> operator_ ) )					// New section
			|| ( 0 == strcmp ( "CSECT" , token_table [ i ] -> operator_ ) ) )
		{
			if ( 0 <= iSection )
			{
				g_irgProgramLength [ iSection ] = locctr ;
				g_irgLiteralCount [ iSection ] = iCurrentLiteralCount ;
			}

			locctr = 0 ;
			++ iSection ;
			iCurrentLiteralCount = 0 ;
		}
		

		iByte = iGetOperandByte ( search_opcode ( token_table [ i ] -> operator_ ) ) ;

		if ( '+' == token_table [ i ] -> operator_ [ 0 ] )
		{
			locctr += 4 ;
			token_table [ i ] -> nixbpe = 0b110001 ;
		}
		else if ( iByte > 0 )
		{
			locctr += iByte ;
			token_table [ i ] -> nixbpe = 0b110000 ;
		}
		else if ( 0 == strcmp ( token_table [ i ] -> operator_ , "BYTE" ) )			// BYTE, WORD, RESB, RESW process for locctr
		{
			++ locctr ;
		}
		else if ( 0 == strcmp ( token_table [ i ] -> operator_ , "WORD" ) )
		{
			locctr += 3 ;
		}
		else if ( 0 == strcmp ( token_table [ i ] -> operator_ , "RESB" ) )
		{
			locctr += atoi ( token_table [ i ] -> operand [ 0 ] ) ;
		}
		else if ( 0 == strcmp ( token_table [ i ] -> operator_ , "RESW" ) )
		{
			locctr += atoi ( token_table [ i ] -> operand [ 0 ] ) * 3 ;
		}
		else if ( 0 == strcmp ( token_table [ i ] -> operator_ , "LTORG" ) )		// If LTORG found, assign memory to unassigned literals
		{
			for ( j = iTotalLiteralCount - iCurrentLiteralCount ; j < iTotalLiteralCount ; ++j )
			{
				if ( 'X' == literal_table [ j ].literal [ 1 ] )				// If 1 byte
				{
					iByte = 1 ;
				}
				else
				{
					iByte = strlen ( literal_table [ j ].literal ) - 4 ;
				}

				
				literal_table [ j ].addr = locctr ;
				locctr += iByte ;
			}
		}
		else																				// If operator don't need space, move to next line
		{
			continue ;
		}

		if ( NULL != token_table [ i ] -> operand [ 0 ] )									// If operand exist
		{
			iLocation = -1 ;

			
			if ( '=' == token_table [ i ] -> operand [ 0 ] [ 0 ] )							// This is literal
			{
				for ( j = 0 ; j < literal_line ; ++j )
				{
					if ( 0 == strcmp ( literal_table [ j ].literal , token_table [ i ] -> operand [ 0 ] ) )
					{
						iLocation = literal_table [ j ].addr ;

						if ( j == iTotalLiteralCount )
						{
							++ iCurrentLiteralCount ;
							++ iTotalLiteralCount ;
						}

						break ;
					}
				}

				if ( -1 != iLocation )
				{
					if ( ( -4097 < iLocation - locctr ) && ( iLocation - locctr < 0x1000 ) )		// If pc relative is possible
					{
						token_table [ i ] -> nixbpe |= 0b000010 ;
					}
					else																			// We already filtered format 4 extension
					{																				// So if exceed 0x1000, then base relative is the only way
						token_table [ i ] -> nixbpe |= 0b000100 ;
					}
				}
			}
			else																			// Need find at symbol table
			{
				iTemp = 0 ;
				for ( j = 0 ; j < sym_line ; ++j )													// Find first symbol of current section
				{
					if ( iTemp == iSection )
					{
						break ;
					}
					else if ( '\0' == sym_table [ j ].symbol [ 0 ] )
					{
						++ iTemp ;
						++ j ;
					}
				}
				for ( ; '\0' != sym_table [ j ].symbol [ 0 ] ; ++j )
				{
					if ( ( ( '@' == token_table [ i ] -> operand [ 0 ] [ 0 ] )				// # or @ exist then need to process
							|| ( '#' == token_table [ i ] -> operand [ 0 ] [ 0 ] ) )
						&& ( 0 == strcmp ( sym_table [ j ].symbol , token_table [ i ] -> operand [ 0 ] + 1 ) ) )
					{
						iLocation = sym_table [ j ].addr ;

						break ;
					}
					else if ( 0 == strcmp ( sym_table [ j ].symbol , token_table [ i ] -> operand [ 0 ] ) )
					{
						iLocation = sym_table [ j ].addr ;

						break ;
					}
				}

				if ( '#' == token_table [ i ] -> operand [ 0 ] [ 0 ] )								// Immediate addressing
				{
					token_table [ i ] -> nixbpe &= 0b011111 ;										// Disable n at nixbpe
				}
				else if ( '@' == token_table [ i ] -> operand [ 0 ] [ 0 ] )							// Indirect addressing check
				{
					token_table [ i ] -> nixbpe &= 0b101111 ;
				}

				if ( -1 != iLocation )
				{
					if ( ( -4097 < iLocation - locctr ) && ( iLocation - locctr < 0x1000 ) )		// If pc relative is possible
					{
						token_table [ i ] -> nixbpe |= 0b000010 ;
					}
					else																			// We already filtered format 4 extension
					{																				// So if exceed 0x1000, then base relative is the only way
						token_table [ i ] -> nixbpe |= 0b000100 ;
					}
				}
			}
		}
		
		if ( ( NULL != token_table [ i ] -> operand [ 0 ] )
			&& ( NULL != token_table [ i ] -> operand [ 1 ] )
			&& (  0 == strcmp ( "X" , token_table [ i ] -> operand [ 1 ] ) ) )
		{																					// Check for x
			token_table [ i ] -> nixbpe |= 0b001000 ;
		}
	}

	iTemp = 10 ;
	for ( i = 0 ; i < iSection + 1 ; ++i )
	{
		iTemp += g_irgLiteralCount [ i ] ;
	}
	if ( iTemp != iTotalLiteralCount )																// Literal address is not computed
	{
		iByte = 1 ;

		if ( 'C' == literal_table [ iTotalLiteralCount - 1 ].literal [ 1 ] )
		{
			iByte = strlen ( literal_table [ iTotalLiteralCount - 1 ].literal - 4 ) ;
		}

		locctr = literal_table [ iTotalLiteralCount - 1 ].addr + iByte ;
	}
	g_irgProgramLength [ iSection ] = locctr ;
	g_irgLiteralCount [ iSection ] = iCurrentLiteralCount ;

	return 0 ;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
 * 여기서 출력되는 내용은 object code ( 프로젝트 1번 )이다.
 * 매계 : 생성할 오브젝트 파일명
 * 반환 : 없음
 * 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
 * 화면에 출력해준다.
 * 
 * -----------------------------------------------------------------------------------
 */
void make_objectcode_output ( char * file_name )
{
	FILE * file ;
	int i = 0 ;
	int j = 0 ;
	int k = 0 ;
	int iTemp = 0 ;
	int iSection = -1 ;
	int iByte = 0 ;
	int iLineLength = 0 ;
	int iAddress = 0 ;
	int iImmediate = 0 ;
	char crgPrint [ 100 ] ;
	char crgTemp [ 20 ] = { 0 , } ;
	char * cpTemp ;
	char cSign = 0 ;
	int iWordLen = 0 ;
	int iIndicator = 0 ;																	// 0 Normal, 1 Byte, 2 Word, 3 LTORG
	int iLiteralProcessCount = 0 ;
	extref_count = 0 ;



	file = fopen ( file_name , "w+" ) ;

	for ( ; i < token_line ; ++i )
	{
		if ( ( NULL != token_table [ i ] -> label ) && ( 0 == strcmp ( "." , token_table [ i ] -> label ) ) )
		{
			continue ;																		// Skip annotation
		}

		if ( ( 0 == strcmp ( "START" , token_table [ i ] -> operator_ ) )					// New section
			|| ( 0 == strcmp ( "CSECT" , token_table [ i ] -> operator_ ) ) )
		{
			if ( 0 != extref_count )
			{
				if ( '\0' != crgPrint [ 0 ] )
				{
					printToFileOrConsole ( file , crgPrint ) ;
				}

				for ( j = 0 ; j < extref_count ; ++j )
				{
					iAddress = extref_table [ j ].addr ;
					iByte = extref_table [ j ].half_byte ;

					if ( 1 == ( iByte % 2 ) )
					{
						++ iAddress ;
					}

					sprintf ( crgPrint , "M%06X%02X" , iAddress , iByte ) ;

					if ( 1 == extref_table [ j ].cSign )
					{
						sprintf ( crgTemp , "+" ) ;
					}
					else
					{
						sprintf ( crgTemp , "-" ) ;
					}
					strcat ( crgPrint , crgTemp ) ;

					sprintf ( crgTemp , "%s" , extref_table [ j ].literal ) ;
					strcat ( crgPrint , crgTemp ) ;

					printToFileOrConsole ( file , crgPrint ) ;
				}
			}
			if ( -1 < iSection )
			{
				if ( 0 == iSection )														// Main routine
				{
					sprintf ( crgPrint , "E%06x\n", atoi ( token_table [ 0 ] -> operand [ 0 ] ) ) ;
				}
				else
				{
					sprintf ( crgPrint , "E\n" ) ;
				}

				printToFileOrConsole ( file , crgPrint ) ;
			}

			locctr = 0 ;
			++ iSection ;
			extref_count = 0 ;
			g_iExtrefPointCount = 0 ;

			sprintf ( crgPrint , "H%s " , token_table [ i ] -> label ) ;					// If label length is n, cnt of space is 6 - n. So fixed "H%-6s" -> "H%s "

			if ( NULL == token_table [ i ] -> operand [ 0 ] )								// If CSECT, starting address is 0
			{
				sprintf ( crgTemp , "%06X" , 0 ) ;
			}
			else
			{
				sprintf ( crgTemp , "%06X" , atoi ( token_table [ i ] -> operand [ 0 ] ) ) ;
			}

			strcat ( crgPrint , crgTemp ) ;

			sprintf ( crgTemp , "%06X" , g_irgProgramLength [ iSection ] ) ;
			strcat ( crgPrint , crgTemp ) ;

			printToFileOrConsole ( file , crgPrint ) ;
		}
		else if ( 0 == strcmp ( "EXTDEF" , token_table [ i ] -> operator_ ) )
		{
			sprintf ( crgTemp , "D" ) ;
			strcat ( crgPrint , crgTemp ) ;

			for ( j = 0 ; ( NULL != token_table [ i ] -> operand [ j ] ) && ( j < 3 ) ; ++j )
			{
				sprintf ( crgTemp , "%s%06X" , token_table [ i ] -> operand [ j ] , iGetSymLocation ( token_table [ i ] -> operand [ j ] , iSection ) ) ;
				strcat ( crgPrint , crgTemp ) ;
			}

			printToFileOrConsole ( file , crgPrint ) ;
		}
		else if ( 0 == strcmp ( "EXTREF" , token_table [ i ] -> operator_ ) )
		{
			sprintf ( crgTemp , "R" ) ;
			strcat ( crgPrint , crgTemp ) ;

			for ( j = 0 ; ( NULL != token_table [ i ] -> operand [ j ] ) && ( j < 3 ) ; ++j )
			{
				sprintf ( crgTemp , "%-6s" , token_table [ i ] -> operand [ j ] ) ;
				strcat ( crgPrint , crgTemp ) ;

				cprgEXTREFList [ g_iExtrefPointCount ++ ] = token_table [ i ] -> operand [ j ] ;
			}

			printToFileOrConsole ( file , crgPrint ) ;
		}
		else
		{
			iAddress = search_opcode ( token_table [ i ] -> operator_ ) ;
			iByte = iGetOperandByte ( iAddress ) ;

			if ( '+' == token_table [ i ] -> operator_ [ 0 ] )
			{
				iByte = 4 ;
				locctr += iByte ;
				iIndicator = 0 ;
			}
			else if ( iByte > 0 )
			{
				locctr += iByte ;
				iIndicator = 0 ;
			}
			else if ( 0 == strcmp ( "BYTE" , token_table [ i ] -> operator_ ) )
			{
				iByte = 1 ;
				locctr += iByte ;
				iIndicator = 1 ;
			}
			else if ( 0 == strcmp ( "WORD" , token_table [ i ] -> operator_ ) )
			{
				iByte = 3 ;
				locctr += iByte ;
				iIndicator = 2 ;
			}
			else if ( 0 == strcmp ( "LTORG" , token_table [ i ] -> operator_ ) )						// Assign literal
			{
				iByte = 0 ;

				if ( 0 != g_irgLiteralCount [ iSection ] )
				{
					iByte = 0 ;


					for ( j = iLiteralProcessCount ; j < iLiteralProcessCount + g_irgLiteralCount [ iSection ] ; ++j )
					{
						if ( 'X' == literal_table [ j ].literal [ 1 ] )									// Current literal is byte
						{
							++ iByte ;
						}
						else																			// Current literal is word
						{
							iByte += strlen ( literal_table [ j ].literal ) - 4 ;
						}
					}

					locctr += iByte ;
					iIndicator = 3 ;
				}
			}
			else if ( ( 0 == strcmp ( "RESW" , token_table [ i ] -> operator_ ) )
				|| ( 0 == strcmp ( "RESB" , token_table [ i ] -> operator_ ) ) )						// Assign space, don't generate object code
			{
				iByte = 1 ;

				if ( 0 == strcmp ( "RESW" , token_table [ i ] -> operator_ ) )
					iByte = 3 ;

				iByte *= atoi ( token_table [ i ] -> operand [ 0 ] ) ;


				locctr += iByte ;

				if ( '\0' != crgPrint [ 0 ] )
					printToFileOrConsole ( file , crgPrint ) ;

				iByte = 0 ;
			}

			if ( iByte > 0 )
			{
				if ( '\0' == crgPrint [ 0 ] )															// New line
				{
					sprintf ( crgTemp , "T%06X%02X" , locctr - iByte , 0 ) ;							// Line length keep updating
					strcat ( crgPrint , crgTemp ) ;
					iLineLength = 0 ;
				}
				else if ( ( 0x20 <= iLineLength + iByte ) || ( 3 == iIndicator ) )						// If line must end
				{
					printToFileOrConsole ( file , crgPrint ) ;

					iLineLength = 0 ;
					sprintf ( crgTemp , "T%06X%02X" , locctr - iByte , iLineLength ) ;					// Line length keep updating
					strcat ( crgPrint , crgTemp ) ;
				}

				iImmediate = 0 ;

				cpTemp = token_table [ i ] -> operand [ 0 ] ;

				if ( ( NULL != cpTemp )																	// For # and @
					&&		( ( '#' == cpTemp [ 0 ] )
							|| ( '@' == cpTemp [ 0 ] ) ) )
				{
					if ( '#' == cpTemp [ 0 ] )
						iImmediate = 1 ;
					++ cpTemp ;
				}


				if ( 1 == iIndicator )																	// BYTE instruction
				{
					iByte = 1 ;

					sprintf ( crgTemp , "%02X" , iStringToHex ( cpTemp + 2 ) ) ;
					strcat ( crgPrint , crgTemp ) ;
				}
				else if ( 2 == iIndicator )																// WORD instruction
				{
					iByte = 3 ;
					iWordLen = 0 ;
					cSign = 1 ;
					iAddress = 0 ;
					k = 0 ;
					iTemp = 0 ;

					for ( k = 0 ; k < iSection ; ++k )
					{
						iTemp += g_irgLiteralCount [ k ] ;
					}
					for ( j = 0 ; j < strlen ( cpTemp ) ; ++j )
					{
						if ( '-' == cpTemp [ j ] )
						{
							if ( 0 != j )
							{
								strncpy ( crgTemp , cpTemp + j - iWordLen , iWordLen ) ;
								crgTemp [ iWordLen ] = '\0' ;
																
								for ( k = iTemp ; k < iTemp + g_irgLiteralCount [ iSection ] ; ++k )	// Check if it reference variable of current section
								{
									if ( 0 == strcmp ( crgTemp , literal_table [ i ].literal ) )
									{
										iAddress += literal_table [ i ].addr * cSign ;
										
										break ;
									}
								}
							}

							cSign = -1 ;
							iWordLen = 0 ;
						}
						else if ( '+' == cpTemp [ j ] )
						{
							if ( 0 != j )
							{
								strncpy ( crgTemp , cpTemp + j - iWordLen , iWordLen ) ;
								crgTemp [ iWordLen ] = '\0' ;
																
								for ( k = iTemp ; k < iTemp + g_irgLiteralCount [ iSection ] ; ++k )
								{
									if ( 0 == strcmp ( crgTemp , literal_table [ i ].literal ) )
									{
										iAddress += literal_table [ i ].addr * cSign ;
										
										break ;
									}
								}
							}

							cSign = 1 ;
							iWordLen = 0 ;
						}
						else
						{
							++ iWordLen ;
						}
					}

					sprintf ( crgTemp , "%06X" , iAddress ) ;
					strcat ( crgPrint , crgTemp ) ;
				}
				else if ( 3 == iIndicator )																// LTORG instruction
				{
					for ( j = iLiteralProcessCount ; j < iLiteralProcessCount + g_irgLiteralCount [ iSection ] ; ++j )
					{
						cpTemp = literal_table [ j ].literal ;


						if ( 'X' == cpTemp [ 1 ] )														// Current literal is byte
						{
							iByte = 1 ;
						}
						else																			// Current literal is word
						{
							iByte = strlen ( cpTemp ) - 4 ;
						}
						if ( 1 == iByte )
						{
							sprintf ( crgTemp , "%02X" , iStringToHex ( cpTemp + 3 ) ) ;
							strcat ( crgPrint , crgTemp ) ;
						}
						else
						{
							for ( k = 0 ; k < iByte ; ++k )
							{
								sprintf ( crgTemp , "%02X" , cpTemp [ 3 + k ] ) ;
								strcat ( crgPrint , crgTemp ) ;
							}
						}
					}

					iLiteralProcessCount += g_irgLiteralCount [ iSection ] ;
				}
				else if ( 1 == iByte )
				{
					// Add here for 1 byte operand

					sprintf ( crgTemp , "%02X" , iAddress ) ;
					strcat ( crgPrint , crgTemp ) ;
				}
				else if ( 2 == iByte )
				{
					iAddress <<= 8 ;

					for ( j = 0 ; j < 9 ; ++j )
					{
						if ( 0 == strcmp ( g_cdrgRegiter [ j ] , cpTemp ) )
						{
							if ( j >= 7 )																// If j >= 7, then j + 1 is register number
							{
								++j ;
							}

							iAddress += ( j << 4 ) ;

							break ;
						}
					}
					if ( ( 2 == iGetInstOperandNum ( search_opcode ( token_table [ i ] -> operator_ ) ) )
						&& ( NULL != token_table [ i ] -> operand [ 1 ] ) )								// If multiple operand
					{
						for ( j = 0 ; j < 9 ; ++j )
						{
							if ( 0 == strcmp ( g_cdrgRegiter [ j ] , token_table [ i ] -> operand [ 1 ] ) )
							{
								if ( j >= 7 )															// If j >= 7, then j + 1 is register number
								{
									++j ;
								}

								iAddress += j ;

								break ;
							}
						}
					}

					sprintf ( crgTemp , "%04X" , iAddress ) ;
					strcat ( crgPrint , crgTemp ) ;
				}
				else if ( 3 == iByte )
				{
					iAddress <<= 16 ;
					iAddress += ( ( int ) ( token_table [ i ] -> nixbpe ) ) << 12 ;						// Cast to int and shift left

					if ( 1 == iImmediate )
					{
						iAddress += atoi ( cpTemp ) ;
					}
					else if ( 0 != ( iAddress & 0x2000 ) )												// If pc relative
					{
						iTemp = iGetSymLocation ( cpTemp , iSection ) ;
						if ( -1 == iTemp )
						{
							iTemp = iGetLitLocation ( cpTemp ) ;
						}
						iTemp -= locctr ;

						iAddress |= ( iTemp & 0xFFF ) ;
					}
					else if ( 0 != ( iAddress & 0x4000 ) )												// If base relative
					{
						// Add here for base relative
					}

					sprintf ( crgTemp , "%06X" , iAddress ) ;
					strcat ( crgPrint , crgTemp ) ;
				}
				else if ( 4 == iByte )
				{
					iAddress <<= 24 ;
					iAddress += ( ( int ) ( token_table [ i ] -> nixbpe ) ) << 20 ;						// Cast to int and shift left

					cpTemp = token_table [ i ] -> operand [ 0 ] ;

					sprintf ( crgTemp , "%08X" , iAddress ) ;
					strcat ( crgPrint , crgTemp ) ;
				}


				for ( j = 0 ; ( NULL != token_table [ i ] -> operand [ j ] ) && ( j < 3 ) ; ++j )		// Check if EXTREF is used
				{
					cpTemp = token_table [ i ] -> operand [ j ] ;

					if ( ( '#' == cpTemp [ 0 ] ) || ( '@' == cpTemp [ 0 ] ) )							// If #,@ exist, move pointer
						++ cpTemp ;

					for ( k = 0 ; k < g_iExtrefPointCount ; ++k )
					{
						if ( 0 == strcmp ( cpTemp , cprgEXTREFList [ k ] ) )
						{
							extref_table [ extref_count ].literal = cprgEXTREFList [ k ] ;
							extref_table [ extref_count ].addr = locctr - iByte ;

							if ( 2 == iIndicator )
							{
								extref_table [ extref_count ].half_byte = 6 ;
							}
							else
							{
								extref_table [ extref_count ].half_byte = 5 ;
							}

							extref_table [ extref_count ++ ].cSign = 1 ;

							break ;
						}
					}

					if ( ( NULL != strchr ( cpTemp , '+' ) ) || ( NULL != strchr ( cpTemp , '-' ) ) )	// WORD instruction, arithmetic
					{
						iWordLen = 0 ;
						cSign = 1 ;

						for ( iTemp = 0 ; iTemp < strlen ( cpTemp ) ; ++ iTemp )
						{
							if ( '-' == cpTemp [ iTemp ] )
							{
								if ( 0 != iTemp )
								{
									strncpy ( crgTemp , cpTemp + iTemp - iWordLen , iWordLen ) ;
									crgTemp [ iWordLen ] = '\0' ;
																
									for ( k = 0 ; k < g_iExtrefPointCount ; ++k )
									{
										if ( 0 == strcmp ( crgTemp , cprgEXTREFList [ k ] ) )
										{
											extref_table [ extref_count ].literal = cprgEXTREFList [ k ] ;
											extref_table [ extref_count ].addr = locctr - iByte ;
											extref_table [ extref_count ].half_byte = 6 ;
											extref_table [ extref_count ++ ].cSign = cSign ;

											break ;
										}
									}
								}

								cSign = -1 ;
								iWordLen = 0 ;
							}
							else if ( '+' == cpTemp [ iTemp ] )
							{
								if ( 0 != iTemp )
								{
									strncpy ( crgTemp , cpTemp + iTemp - iWordLen , iWordLen ) ;
									crgTemp [ iWordLen ] = '\0' ;
																
									for ( k = 0 ; k < g_iExtrefPointCount ; ++k )
									{
										if ( 0 == strcmp ( crgTemp , cprgEXTREFList [ k ] ) )
										{
											extref_table [ extref_count ].literal = cprgEXTREFList [ k ] ;
											extref_table [ extref_count ].addr = locctr - iByte ;
											extref_table [ extref_count ].half_byte = 6 ;
											extref_table [ extref_count ++ ].cSign = cSign ;

											break ;
										}
									}
								}

								cSign = 1 ;
								iWordLen = 0 ;
							}
							else
							{
								++ iWordLen ;
							}
						}

						strncpy ( crgTemp , cpTemp + iTemp - iWordLen , iWordLen ) ;
						crgTemp [ iWordLen ] = '\0' ;

						for ( k = 0 ; k < g_iExtrefPointCount ; ++k )
						{
							if ( 0 == strcmp ( crgTemp , cprgEXTREFList [ k ] ) )
							{
								extref_table [ extref_count ].literal = cprgEXTREFList [ k ] ;
								extref_table [ extref_count ].addr = locctr - iByte ;
								extref_table [ extref_count ].half_byte = 6 ;
								extref_table [ extref_count ++ ].cSign = cSign ;

								break ;
							}
						}
					}
				}
				
				iLineLength += iByte ;

				crgPrint [ 7 ] = cHexToChar ( iLineLength >> 4 ) ;
				crgPrint [ 8 ] = cHexToChar ( iLineLength & 0xF ) ;
			}
		}
	}

	if ( '\0' != crgPrint [ 0 ] )
	{
		for ( i = iLiteralProcessCount ; i < iLiteralProcessCount + g_irgLiteralCount [ iSection ] ; ++i )
		{																						// If literal not assigned
			cpTemp = literal_table [ i ].literal ;


			if ( 'X' == cpTemp [ 1 ] )																	// Current literal is byte
			{
				iByte = 1 ;
			}
			else																						// Current literal is word
			{
				iByte = strlen ( cpTemp ) - 4 ;
			}
			if ( 1 == iByte )
			{
				sprintf ( crgTemp , "%02X" , iStringToHex ( cpTemp + 3 ) ) ;
				strcat ( crgPrint , crgTemp ) ;
			}
			else
			{
				for ( k = 0 ; k < iByte ; ++k )
				{
					sprintf ( crgTemp , "%02X" , cpTemp [ 3 + k ] ) ;
					strcat ( crgPrint , crgTemp ) ;
				}
			}
		}

		printToFileOrConsole ( file , crgPrint ) ;
	}
	for ( i = 0 ; i < extref_count ; ++i )														// If EXTREF is not modified
	{
		iAddress = extref_table [ i ].addr ;
		iByte = extref_table [ i ].half_byte ;

		if ( 1 == ( iByte % 2 ) )
		{
			++ iAddress ;
		}

		sprintf ( crgPrint , "M%06X%02X" , iAddress , iByte ) ;

		if ( 1 == extref_table [ i ].cSign )
		{
			sprintf ( crgTemp , "+" ) ;
		}
		else
		{
			sprintf ( crgTemp , "-" ) ;
		}
		strcat ( crgPrint , crgTemp ) ;

		sprintf ( crgTemp , "%s" , extref_table [ i ].literal ) ;
		strcat ( crgPrint , crgTemp ) ;

		printToFileOrConsole ( file , crgPrint ) ;
	}



	sprintf ( crgPrint , "E" ) ;
	printToFileOrConsole ( file , crgPrint ) ;

	if ( NULL != file )
		fclose ( file ) ;
}

int iStringToHex ( char * str )					// Change string to hex
{
	int iHigh = 0 ;
	int iLow = 0 ;
	int i = 0 ;



	for ( i = 0 ; i < 1 ; ++i )					// Now 1 Byte. If need n byte transfer, change for condition
	{
		iHigh = ( str [ i * 2 ] > '9' ) ? str [ i * 2 ] - 'A' + 10 : str [ i * 2 ] - '0' ;
		iLow = ( str [ i * 2 + 1 ] > '9' ) ? str [ i * 2 + 1 ] - 'A' + 10 : str [ i * 2 + 1 ] - '0' ;
	}

	return ( iHigh << 4 ) | iLow ;
}

char cHexToChar ( const int ciNum )				// Change hex to char
{
	char cReturn = ciNum + '0' ;



	if ( ciNum > 9 )									// If ciNum is 10 to 15, add more
	{
		cReturn += 7 ;
	}


	return cReturn ;
}

void clearMemory ()								// Free all the memory
{
	int iCount = 0 ;



	while ( ( iCount < MAX_INST ) && ( NULL != inst_table [ iCount ] ) )
	{
		free ( inst_table [ iCount ++ ] ) ;
	}

	iCount = 0 ;

	while ( iCount < line_num )
	{
		free ( input_data [ iCount ++ ] ) ;
	}

	line_num = 0 ;
	iCount = 0 ;
	
	while ( ( line_num < MAX_LINES ) && ( NULL != token_table [ token_line ] ) )
	{
		if ( NULL != token_table [ token_line ] -> label )			// If label exist, free
		{
			free ( token_table [ token_line ] -> label ) ;
		}
		if ( NULL != token_table [ token_line ] -> operator_ )		// If operator exist, free
		{
			free ( token_table [ token_line ] -> operator_ ) ;
		}
		while ( ( iCount != MAX_OPERAND ) && ( NULL != token_table [ token_line ] -> operand [ iCount ] ) )			// If operand exist, free
		{
			free ( token_table [ token_line ] -> operand [ iCount ++ ] ) ;
		}

		free ( token_table [ line_num ++ ] ) ;

		iCount = 0 ;
	}
}

int iGetOperandByte ( const int ciOpcode )		// Get operand size using opcode
{
	int i = 0 ;
	int iReturn = 0 ;



	if ( -1 == ciOpcode )
	{
		return 0 ;
	}

	for ( ; i < inst_index ; ++i )
	{
		if ( ciOpcode == inst_table [ i ] -> op )
		{
			iReturn = 7 & inst_table [ i ] -> format ;
			
			break ;
		}
	}

	if ( 4 == iReturn )
		-- iReturn ;

	return iReturn ;
}

int iGetInstOperandNum ( const int ciOpcode )	// Get number of instruction operand using opcode
{
	int i = 0 ;
	int iReturn = 0 ;



	if ( -1 == ciOpcode )
	{
		return 0 ;
	}

	for ( ; i < inst_index ; ++i )
	{
		if ( ciOpcode == inst_table [ i ] -> op )
		{
			iReturn = inst_table [ i ] -> ops ;
			
			break ;
		}
	}

	return iReturn ;
}

void setInformationAfterPass1 ()				// After pass 1, set informations
{
	char * cpTemp ;
	int iOpcode = 0 ;
	int iByte = 0 ;
	int i = 0 ;
	char crgLable [ 20 ] = { 0 , } ;
	int j = 0 ;
	char cSign = 1 ;														// If -1, it means -
	int iLen = 0 ;



	for ( ; i < token_line ; ++i )
	{
		cpTemp = token_table [ i ] -> label ;

		if ( ( NULL != cpTemp ) && ( '.' != cpTemp [ 0 ] ) )				// Label exist, add to symbol table
		{
			strcpy ( sym_table [ sym_line ].symbol , cpTemp ) ;						// Copy label to symbol table
			sym_table [ sym_line ++ ].addr = locctr ;

			if ( ( 0 == strcmp ( token_table [ i ] -> operator_ , "EQU" ) )
				&& ( 0 != strcmp ( token_table [ i ] -> operand [ 0 ] , "*" ) ) )	// If EQU found, process
			{
				iLen = 0 ;

				sym_table [ sym_line - 1 ].addr = 0 ;

				cpTemp = token_table [ i ] -> operand [ 0 ] ;

				for ( j = 0 ; j < strlen ( cpTemp ) ; ++j )
				{
					if ( '-' == cpTemp [ j ] )
					{
						if ( 0 != j )
						{
							strncpy ( crgLable , cpTemp + j - iLen , iLen ) ;
							crgLable [ iLen ] = '\0' ;

							sym_table [ sym_line - 1 ].addr += iGetSymLocation ( crgLable , -1 ) * cSign ;
						}

						cSign = -1 ;
						iLen = 0 ;
					}
					else if ( '+' == cpTemp [ j ] )
					{
						if ( 0 != j )
						{
							strncpy ( crgLable , cpTemp + j - iLen , iLen ) ;
							crgLable [ iLen ] = '\0' ;

							sym_table [ sym_line - 1 ].addr += iGetSymLocation ( crgLable , -1 ) * cSign ;
						}

						cSign = 1 ;
						iLen = 0 ;
					}
					else
					{
						++ iLen ;
					}
				}

				strncpy ( crgLable , cpTemp + j - iLen , iLen ) ;
				crgLable [ iLen ] = '\0' ;

				sym_table [ sym_line - 1 ].addr += iGetSymLocation ( crgLable , -1 ) * cSign ;
			}
		}
		if ( ( NULL == cpTemp ) || ( '.' != cpTemp [ 0 ] ) )				// Exception of '.' comment
		{
			cpTemp = token_table [ i ] -> operator_ ;

			if ( '+' == cpTemp [ 0 ] )										// Get opcode
			{
				iOpcode = search_opcode ( cpTemp + 1 ) + 256 ;						// For format 4, add 0100(16)
			}
			else
			{
				iOpcode = search_opcode ( cpTemp ) ;
			}

			if ( iOpcode > 255 )													// Format 4 found
			{
				locctr += 4 ;
			}
			else
			{
				locctr += iGetOperandByte ( iOpcode ) ;								// If not format 4, use function to get byte
			}

			if ( 0 == strcmp ( token_table [ i ] -> operator_ , "BYTE" ) )	// BYTE, WORD, RESB, RESW process for locctr
			{
				++ locctr ;
			}
			else if ( 0 == strcmp ( token_table [ i ] -> operator_ , "WORD" ) )
			{
				locctr += 3 ;
			}
			else if ( 0 == strcmp ( token_table [ i ] -> operator_ , "RESB" ) )
			{
				locctr += atoi ( token_table [ i ] -> operand [ 0 ] ) ;
			}
			else if ( 0 == strcmp ( token_table [ i ] -> operator_ , "RESW" ) )
			{
				locctr += atoi ( token_table [ i ] -> operand [ 0 ] ) * 3 ;
			}


			if ( 0 == strcmp ( cpTemp , "CSECT" ) )							// If CSECT found, add 1 additional line to symbol table
			{
				for ( j = 0 ; j < literal_line ; ++j )								// If some literal is not assigned at former section
				{
					if ( -1 == literal_table [ j ].addr )
					{
						if ( 'X' == literal_table [ j ].literal [ 1 ] )				// If 1 byte
						{
							iByte = 1 ;
						}
						else
						{
							iByte = strlen ( literal_table [ j ].literal ) - 3 ;
						}

				
						literal_table [ j ].addr = locctr ;
						locctr += iByte ;
					}
				}

				strcpy ( sym_table [ sym_line ].symbol , sym_table [ sym_line - 1 ].symbol ) ;
				sym_table [ sym_line ].addr = 0 ;

				sym_table [ sym_line ++ - 1 ].symbol [ 0 ] = '\0' ;					// Make wasting symbol to NULL

				locctr = 0 ;														// New section, initialize locctr to 0
			}
			else if ( 0 == strcmp ( cpTemp , "LTORG" ) )					// If LTORG found, assign memory to unassigned literals
			{
				for ( j = 0 ; j < literal_line ; ++j )
				{
					if ( -1 == literal_table [ j ].addr )
					{
						if ( 'X' == literal_table [ j ].literal [ 1 ] )				// If 1 byte
						{
							iByte = 1 ;
						}
						else
						{
							iByte = strlen ( literal_table [ j ].literal ) - 4 ;
						}

				
						literal_table [ j ].addr = locctr ;
						locctr += iByte ;
					}
				}
			}

			cpTemp = token_table [ i ] -> operand [ 0 ] ;

			if ( ( NULL != cpTemp ) && ( '=' == cpTemp [ 0 ] ) )
			{
				for ( j = 0 ; j < literal_line ; ++j )
				{
					if ( 0 == strcmp ( cpTemp , literal_table [ j ].literal ) )
					{																// If literal already exist, exit
						break ;
					}
				}

				if ( j == literal_line )											// No overlapped literal exist
				{
					iLen = strlen ( cpTemp ) ;
					literal_table [ literal_line ].literal = malloc ( iLen + 1 ) ;
					strcpy ( literal_table [ literal_line ].literal , cpTemp ) ;
					literal_table [ literal_line ].literal [ iLen ] = '\0' ;

					literal_table [ literal_line ].addr = -1 ;
					++ literal_line ;
				}
			}
		}
	}
	for ( j = 0 ; j < literal_line ; ++j )									// If some literal is not assigned at last section
	{
		if ( -1 == literal_table [ j ].addr )
		{
			if ( 'X' == literal_table [ j ].literal [ 1 ] )				// If 1 byte
			{
				iByte = 1 ;
			}
			else
			{
				iByte = strlen ( literal_table [ j ].literal ) - 3 ;
			}

				
			literal_table [ j ].addr = locctr ;
			locctr += iByte ;
		}
	}
}

int iGetSymLocation ( char * str , int iSection )	// Get symbol location using str
{
	int i = 0 ;
	int iCount = 0 ;



	if ( -1 == iSection )
	{
		for ( ; i < sym_line ; ++i )
		{
			if ( 0 == strcmp ( str , sym_table [ i ].symbol ) )
			{
				return sym_table [ i ].addr ;
			}
		}
	}
	else
	{
		for ( ; iCount < iSection ; ++i )
		{
			if ( '\0' == sym_table [ i ].symbol [ 0 ] )
			{
				++ iCount ;
			}
		}

		++i ;

		for ( ; ( i < sym_line ) && ( '\0' != sym_table [ i ].symbol [ 0 ] ) ; ++i )
		{
			if ( 0 == strcmp ( str , sym_table [ i ].symbol ) )
			{
				return sym_table [ i ].addr ;
			}
		}
	}


	return -1 ;
}

int iGetLitLocation ( char * str )				// Get literal location using str
{
	int i = 0 ;



	for ( ; i < literal_line ; ++i )
	{
		if ( 0 == strcmp ( str , literal_table [ i ].literal ) )
		{
			return literal_table [ i ].addr ;
		}
	}


	return -1 ;
}

void printToFileOrConsole ( FILE * file , char * cpString )			// Print string to file or console
{
	if ( NULL != file )
		fprintf ( file , "%s\n" , cpString ) ;
	else
		printf ( "%s\n" , cpString ) ;

	cpString [ 0 ] = '\0' ;
}
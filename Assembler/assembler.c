#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#pragma warning ( disable : 4996 )

#include "assembler.h"

/*
 * Get assembly file and convert to object program
 * Return 0 if success, - if fail
 */
int main ( int args , char * arg [] )
{
	if ( init_assembler () < 0 )
	{
		printf ( "init_assembler : Failed to intialize Program.\n" ) ;
		return -1 ;
	}

	if ( assem_pass1 () < 0 )
	{
		printf ( "assem_pass1 : Failed to procedd pass 1.\n" ) ;
		return -1 ;
	}

	if ( iSetSymbolLiteralInfo () < 0 )
	{
		printf ( "iSetSymbolLiteralInfo : Failed to process symbol and literal info.\n" ) ;
		return -1 ;
	}

 	if ( iSetAddrNixbpeInfo () < 0 )
	{
		printf ( "assem_pass2 : Failed to procedd pass 1.\n" ) ;
		return -1 ;
	}

	make_objectcode_output ( "Object_Program" ) ;

	clearMemory () ;


	return 0 ;
}

/*
 * Read instruction list and assembly input
 * Return 0 if success, - if fail
 * Caution : Each instruction table is at file, so need to implement instruction table to use
 */
int init_assembler ( void )
{
	if ( ( init_inst_table ( "inst.data" ) ) < 0 )		// Read instruction file
		return -1 ;
	if ( ( init_input_file ( "input.txt" ) ) < 0 )	// Read assembly input
		return -1 ;

	return 0 ;
}

/* 
 * Read instruction list and implement instruction table
 * Return 0 if success, - if fail
 * Instruction format is below
 * ----------------------------------------------------------------------------------------
 * Instruction name | Operation code | Instruction format in binary | Count of max operands
 * ----------------------------------------------------------------------------------------
 */
int init_inst_table ( char * cpInst_file )
{
	FILE * fpFile = NULL ;
	char crgTemp [ 10 ] = { 0 , } ;



	fpFile = fopen ( cpInst_file , "r" ) ;

	if ( fpFile == NULL )					// If no instruction list, can't make object program
	{
		return -1 ;
	}

	while ( EOF != fscanf ( fpFile , "%s" , crgTemp ) )				// Read operator name
	{
		g_pInst_table [ g_iInst_count ] = malloc ( sizeof ( inst ) ) ;
		g_pInst_table [ g_iInst_count ] -> m_cpOperation = malloc ( strlen ( crgTemp ) + 1 ) ;
		strcpy ( g_pInst_table [ g_iInst_count ] -> m_cpOperation , crgTemp ) ;
		fscanf ( fpFile , "%s" , crgTemp ) ;							// Read operation code
		g_pInst_table [ g_iInst_count ] -> m_uiOpcode = iStringToHex ( crgTemp ) ;
		fscanf ( fpFile , "%s" , crgTemp ) ;							// Read operation format
		g_pInst_table [ g_iInst_count ] -> m_iFormat = atoi ( crgTemp ) ;		// 1, 2, 3, 4 -> 1, 2, 4, 8
		fscanf ( fpFile , "%s" , crgTemp ) ;							// Read operand number
		g_pInst_table [ g_iInst_count ] -> m_iOperands = atoi ( crgTemp ) ;

		++ g_iInst_count ;
	}


	if ( g_iInst_count != MAX_INST - 1 )		// If not full, set NULL to check
	{
		g_pInst_table [ g_iInst_count ] = NULL ;
	}

	fclose ( fpFile ) ;


	return 0 ;
}

/*
 * Read assembly source and convert into table
 * Return 0 if success, - if fail
 */
int init_input_file ( char * cpInput_file )
{
	FILE * file ;
	int iCount = 0 ;
	char str [ MAX_INST ] = "\0 , " ;



	file = fopen ( cpInput_file , "r" ) ;

	if ( file == NULL )						// If no source, can't make into object program
	{
		return -1 ;
	}

	while ( fgets ( str , MAX_INST , file )	)					// Read operator name
	{
		g_cpInput_data [ iCount ] = malloc ( strlen ( str ) + 1 ) ;
		strcpy ( g_cpInput_data [ iCount ] , str ) ;

		++ iCount ;
	}


	if ( iCount != MAX_LINES - 1 )
	{
		g_cpInput_data [ iCount ] = NULL ;
	}

	fclose ( file ) ;


	return 0 ;
}

/*
 * Proceed pass 1
 *  : Scan assembly source and split into token table
 * Return 0 if success, - if fail
 */
int assem_pass1 ()
{
	g_iToken_count = 0 ;



	while ( ( g_iToken_count < MAX_LINES ) && ( NULL != g_cpInput_data [ g_iToken_count ] ) )			// While input left
	{
		if ( token_parsing ( g_cpInput_data [ g_iToken_count ] ) < 0 )		// Proceed token parsing with checking error
		{
			printf ( "Parsing failed.\n" ) ;
			
			return -1 ;
		}

		++ g_iToken_count ;
	}


	return 0 ;
}

/*
 * Split source into token table, called by assem_pass1
 * Return 0 if success, - if fail
 */
int token_parsing ( char * cpStr )
{
	char * cpTemp ;
	int iOperand = 0 ;
	char crgTemp [ 100 ] = { 0 , } ;



	if ( NULL == cpStr )													// EOF
	{
		g_pToken_table [ g_iToken_count ] = NULL ;

		return 0 ;
	}
	if ( '.' == cpStr [ 0 ] )												// If annotation line, ignore
	{
		-- g_iToken_count ;

		return 0 ;
	}


	g_pToken_table [ g_iToken_count ] = malloc ( sizeof ( token ) ) ;
	g_pToken_table [ g_iToken_count ] -> m_cNixbpe = 0 ;
	g_pToken_table [ g_iToken_count ] -> m_cpLabel = NULL ;					// Initialize for free allocation
	g_pToken_table [ g_iToken_count ] -> m_cpOperator = NULL ;
	g_pToken_table [ g_iToken_count ] -> m_cpOperand [ 0 ] = NULL ;

	cpTemp = strtok ( cpStr , "\t \n" ) ;

	if ( '\t' != cpStr [ 0 ] )											// Label found
	{
		g_pToken_table [ g_iToken_count ] -> m_cpLabel = malloc ( strlen ( cpTemp ) + 1 ) ;
		strcpy ( g_pToken_table [ g_iToken_count ] -> m_cpLabel , cpTemp ) ;

		cpTemp = strtok ( NULL , "\t \n" ) ;
	}

	g_pToken_table [ g_iToken_count ] -> m_cpOperator = malloc ( strlen ( cpTemp ) + 1 ) ;		// Copy operator
	strcpy ( g_pToken_table [ g_iToken_count ] -> m_cpOperator , cpTemp ) ;


	if ( ( 0 != strcmp ( cpTemp , "RSUB" ) )			// If true, need to get operand.
		&& ( 0 != strcmp ( cpTemp , "LTORG" ) )
		&& ( 0 != strcmp ( cpTemp , "CSECT" ) ) )		// If true, get operand. These 3 operator have no operand
	{
		cpTemp = strtok ( NULL , "\t \n" ) ;

		strcpy ( crgTemp , cpTemp ) ;


		cpTemp = strtok ( crgTemp , "," ) ;				// Can be multiple operands, so need to split using ','

		g_pToken_table [ g_iToken_count ] -> m_cpOperand [ 1 ] = NULL ;
		g_pToken_table [ g_iToken_count ] -> m_cpOperand [ 2 ] = NULL ;

		while ( NULL != cpTemp )						// Operand remain
		{
			g_pToken_table [ g_iToken_count ] -> m_cpOperand [ iOperand ] = malloc ( strlen ( cpTemp ) + 1 ) ;
			strcpy ( g_pToken_table [ g_iToken_count ] -> m_cpOperand [ iOperand ++ ] , cpTemp ) ;
			cpTemp = strtok ( NULL , "," ) ;
		}
	}


	return 0 ;
}

/*
 * Proceed pass 2
 *  : Convert token which split from assembly source to object program
 * Return 0 if success, - if fail
 */
int assem_pass2 ()
{
	if ( ( iSetSymbolLiteralInfo () ) < 0 )		// Set symbol and literal info
		return -1 ;
	if ( ( iSetAddrNixbpeInfo () ) < 0 )		// Set addr and nixbpe of tokens
		return -1 ;

	return 0 ;
}

/*
 * Set symbol table and literal table
 * Return 0 if success, - if fail
 */
int iSetSymbolLiteralInfo ()
{
	char * cpTemp ;
	int iOpcode = 0 ;
	int iByte = 0 ;
	int i = 0 ;
	char crgLable [ 20 ] = { 0 , } ;
	int j = 0 ;
	char cSign = 1 ;		// If -1, it means -
	int iLen = 0 ;



	for ( ; i < g_iToken_count ; ++i )
	{
		cpTemp = g_pToken_table [ i ] -> m_cpLabel ;


		if ( NULL != cpTemp )					// Label exist, add to symbol table
		{
			g_Symbol_table [ g_iSymbol_count ] = malloc ( sizeof ( symbol ) ) ;
			g_Symbol_table [ g_iSymbol_count ] -> m_cpSymbol = NULL ;
			g_Symbol_table [ g_iSymbol_count ] -> m_iAddr = -1 ;

			g_Symbol_table [ g_iSymbol_count ] -> m_cpSymbol = malloc ( strlen ( cpTemp ) + 1 ) ;
			strcpy ( g_Symbol_table [ g_iSymbol_count ] -> m_cpSymbol , cpTemp ) ;						// Copy label to symbol table
			g_Symbol_table [ g_iSymbol_count ] -> m_iAddr = g_iLocctr ;

			if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "EQU" ) )							// If EQU found, process
			{
				iLen = 0 ;

				g_Symbol_table [ g_iSymbol_count ] -> m_iAddr = 0 ;

				cpTemp = g_pToken_table [ i ] -> m_cpOperand [ 0 ] ;

				for ( j = 0 ; j < strlen ( cpTemp ) ; ++j )
				{
					if ( ( '-' == cpTemp [ j ] ) || ( '+' == cpTemp [ j ] ) )		// Need to proceed former formula
					{
						if ( 0 != j )
						{
							strncpy ( crgLable , cpTemp + j - iLen , iLen ) ;
							crgLable [ iLen ] = '\0' ;

							g_Symbol_table [ g_iSymbol_count ] -> m_iAddr += iGetSymLocation ( crgLable , -1 ) * cSign ;
						}

						cSign = -1 ;
						iLen = 0 ;

						if ( '+' == cpTemp [ j ] )
							cSign = 1 ;
					}
					else
					{
						++ iLen ;
					}
				}

				strncpy ( crgLable , cpTemp + j - iLen , iLen ) ;			// Need to proceed last formula
				crgLable [ iLen ] = '\0' ;

				g_Symbol_table [ g_iSymbol_count ] -> m_iAddr += iGetSymLocation ( crgLable , -1 ) * cSign ;
			}

			++ g_iSymbol_count ;
		}
		

		// From here, process literal

		cpTemp = g_pToken_table [ i ] -> m_cpOperator ;

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
			g_iLocctr += 4 ;
		}
		else
		{
			g_iLocctr += iGetOperandByte ( iOpcode ) ;								// If not format 4, use function to get byte
		}

		if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "BYTE" ) )	// BYTE, WORD, RESB, RESW process for locctr
		{
			++ g_iLocctr ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "WORD" ) )
		{
			g_iLocctr += 3 ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "RESB" ) )
		{
			g_iLocctr += atoi ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "RESW" ) )
		{
			g_iLocctr += atoi ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) * 3 ;
		}

		if ( 0 == strcmp ( cpTemp , "CSECT" ) )			// New section, initialize locctr to 0
			g_iLocctr = 0 ;



		if ( ( 0 == strcmp ( cpTemp , "CSECT" ) ) || ( 0 == strcmp ( cpTemp , "LTORG" ) ) )		// If CSECT or LTORG found, assign memory to unassigned literals
		{
			for ( j = 0 ; j < g_iLiteral_count ; ++j )
			{
				if ( -1 == g_Literal_table [ j ] -> m_iAddr )
				{
					if ( 'X' == g_Literal_table [ j ] -> m_cpLiteral [ 1 ] )					// If 1 byte
					{
						iByte = 1 ;
					}
					else if ( '=' == g_Literal_table [ j ] -> m_cpLiteral [ 1 ] )				// =C'ASDF'
					{
						iByte = strlen ( g_Literal_table [ j ] -> m_cpLiteral ) - 4 ;
					}
					else																		// C'ASDF'
					{
						iByte = strlen ( g_Literal_table [ j ] -> m_cpLiteral ) - 3 ;
					}

				
					g_Literal_table [ j ] -> m_iAddr = g_iLocctr ;
					g_iLocctr += iByte ;
				}
			}
		}

		cpTemp = g_pToken_table [ i ] -> m_cpOperand [ 0 ] ;

		if ( ( NULL != cpTemp ) && ( '=' == cpTemp [ 0 ] ) )								// Assign operation exist. =X'F1'
		{
			for ( j = 0 ; j < g_iLiteral_count ; ++j )
			{
				if ( 0 == strcmp ( cpTemp , g_Literal_table [ j ] -> m_cpLiteral ) )		// If literal already exist, don't need to add new literal
				{
					break ;
				}
			}

			if ( j == g_iLiteral_count )													// No overlapped literal exist
			{
				iLen = strlen ( cpTemp ) ;
				g_Literal_table [ g_iLiteral_count ] = malloc ( sizeof ( USER_Literal ) ) ;
				g_Literal_table [ g_iLiteral_count ] -> m_cpLiteral = malloc ( iLen + 1 ) ;
				strcpy ( g_Literal_table [ g_iLiteral_count ] -> m_cpLiteral , cpTemp ) ;
				g_Literal_table [ g_iLiteral_count ] -> m_iAddr = -1 ;
				++ g_iLiteral_count ;
			}
		}
	}
	for ( j = 0 ; j < g_iLiteral_count ; ++j )									// If some literal is not assigned at last section
	{
		if ( -1 == g_Literal_table [ j ] -> m_iAddr )
		{
			if ( 'X' == g_Literal_table [ j ] -> m_cpLiteral [ 1 ] )					// If 1 byte
			{
				iByte = 1 ;
			}
			else if ( '=' == g_Literal_table [ j ] -> m_cpLiteral [ 1 ] )				// =C'ASDF'
			{
				iByte = strlen ( g_Literal_table [ j ] -> m_cpLiteral ) - 4 ;
			}
			else																		// C'ASDF'
			{
				iByte = strlen ( g_Literal_table [ j ] -> m_cpLiteral ) - 3 ;
			}

				
			g_Literal_table [ j ] -> m_iAddr = g_iLocctr ;
			g_iLocctr += iByte ;
		}
	}

	return 0 ;
}

/*
 * Set address and nixbpe of each tokens
 * Return 0 if success, - if fail
 */
int iSetAddrNixbpeInfo ()
{
	int i = 0 ;			// FROM HERE 여기서부터 다시 해라 제발 내일은 다 끝내라
	int iByte = 0 ;
	int iLocation = 0 ;
	int iSection = -1 ;
	int j = 0 ;
	int iTemp = 0 ;
	int iCurrentLiteralCount = 0 ;
	int iTotalLiteralCount = 0 ;
	g_iLocctr = 0 ;


	for ( ; i < g_iToken_count ; ++i )
	{
		if ( ( 0 == strcmp ( "START" , g_pToken_table [ i ] -> m_cpOperator ) )					// New section
			|| ( 0 == strcmp ( "CSECT" , g_pToken_table [ i ] -> m_cpOperator ) ) )
		{
			if ( 0 <= iSection )		// Save former section info
			{
				g_irgProgramLengthforEach [ iSection ] = g_iLocctr ;
				g_irgLiteralCountforEach [ iSection ] = iCurrentLiteralCount ;
			}

			g_iLocctr = 0 ;
			++ iSection ;
			iCurrentLiteralCount = 0 ;
		}
		

		iByte = iGetOperandByte ( search_opcode ( g_pToken_table [ i ] -> m_cpOperator ) ) ;

		if ( '+' == g_pToken_table [ i ] -> m_cpOperator [ 0 ] )
		{
			g_iLocctr += 4 ;
			g_pToken_table [ i ] -> m_cNixbpe = 0b110001 ;
		}
		else if ( iByte > 0 )
		{
			g_iLocctr += iByte ;
			g_pToken_table [ i ] -> m_cNixbpe = 0b110000 ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "BYTE" ) )		// BYTE, WORD, RESB, RESW process for locctr
		{
			++ g_iLocctr ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "WORD" ) )
		{
			g_iLocctr += 3 ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "RESB" ) )
		{
			g_iLocctr += atoi ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "RESW" ) )
		{
			g_iLocctr += atoi ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) * 3 ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "LTORG" ) )		// If LTORG found, assign memory to unassigned literals
		{
			for ( j = iTotalLiteralCount - iCurrentLiteralCount ; j < iTotalLiteralCount ; ++j )
			{
				if ( 'X' == g_Literal_table [ j ] -> m_cpLiteral [ 1 ] )				// If 1 byte
				{
					iByte = 1 ;
				}
				else
				{
					iByte = strlen ( g_Literal_table [ j ] -> m_cpLiteral ) - 4 ;
				}

				
				g_Literal_table [ j ] -> m_iAddr = g_iLocctr ;
				g_iLocctr += iByte ;
			}
		}
		else																				// If operator don't need space, move to next line
		{
			continue ;
		}

		if ( NULL != g_pToken_table [ i ] -> m_cpOperand [ 0 ] )									// If operand exist
		{
			iLocation = -1 ;

			
			if ( '=' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] )							// This is literal
			{
				for ( j = 0 ; j < g_iLiteral_count ; ++j )
				{
					if ( 0 == strcmp ( g_Literal_table [ j ] -> m_cpLiteral , g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) )
					{
						iLocation = g_Literal_table [ j ] -> m_iAddr ;

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
					if ( ( -4097 < iLocation - g_iLocctr ) && ( iLocation - g_iLocctr < 0x1000 ) )		// If pc relative is possible
					{
						g_pToken_table [ i ] -> m_cNixbpe |= 0b000010 ;
					}
					else																			// We already filtered format 4 extension
					{																				// So if exceed 0x1000, then base relative is the only way
						g_pToken_table [ i ] -> m_cNixbpe |= 0b000100 ;
					}
				}
			}
			else																			// Need find at symbol table
			{
				iTemp = 0 ;
				for ( j = 0 ; j < g_iSymbol_count ; ++j )													// Find first symbol of current section
				{
					if ( iTemp == iSection )
					{
						break ;
					}
					else if ( '\0' == g_Symbol_table [ j ].m_crgSymbol [ 0 ] )
					{
						++ iTemp ;
						++ j ;
					}
				}
				for ( ; '\0' != g_Symbol_table [ j ].m_crgSymbol [ 0 ] ; ++j )
				{
					if ( ( ( '@' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] )				// # or @ exist then need to process
							|| ( '#' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] ) )
						&& ( 0 == strcmp ( g_Symbol_table [ j ].m_crgSymbol , g_pToken_table [ i ] -> m_cpOperand [ 0 ] + 1 ) ) )
					{
						iLocation = g_Symbol_table [ j ].m_iAddr ;

						break ;
					}
					else if ( 0 == strcmp ( g_Symbol_table [ j ].m_crgSymbol , g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) )
					{
						iLocation = g_Symbol_table [ j ].m_iAddr ;

						break ;
					}
				}

				if ( '#' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] )								// Immediate addressing
				{
					g_pToken_table [ i ] -> m_cNixbpe &= 0b011111 ;										// Disable n at nixbpe
				}
				else if ( '@' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] )							// Indirect addressing check
				{
					g_pToken_table [ i ] -> m_cNixbpe &= 0b101111 ;
				}

				if ( -1 != iLocation )
				{
					if ( ( -4097 < iLocation - g_iLocctr ) && ( iLocation - g_iLocctr < 0x1000 ) )		// If pc relative is possible
					{
						g_pToken_table [ i ] -> m_cNixbpe |= 0b000010 ;
					}
					else																			// We already filtered format 4 extension
					{																				// So if exceed 0x1000, then base relative is the only way
						g_pToken_table [ i ] -> m_cNixbpe |= 0b000100 ;
					}
				}
			}
		}
		
		if ( ( NULL != g_pToken_table [ i ] -> m_cpOperand [ 0 ] )
			&& ( NULL != g_pToken_table [ i ] -> m_cpOperand [ 1 ] )
			&& (  0 == strcmp ( "X" , g_pToken_table [ i ] -> m_cpOperand [ 1 ] ) ) )
		{																					// Check for x
			g_pToken_table [ i ] -> m_cNixbpe |= 0b001000 ;
		}
	}

	iTemp = 10 ;
	for ( i = 0 ; i < iSection + 1 ; ++i )
	{
		iTemp += g_irgLiteralCountforEach [ i ] ;
	}
	if ( iTemp != iTotalLiteralCount )																// Literal address is not computed
	{
		iByte = 1 ;

		if ( 'C' == g_Literal_table [ iTotalLiteralCount - 1 ].m_cpLiteral [ 1 ] )
		{
			iByte = strlen ( g_Literal_table [ iTotalLiteralCount - 1 ].m_cpLiteral - 4 ) ;
		}

		g_iLocctr = g_Literal_table [ iTotalLiteralCount - 1 ].m_iAddr + iByte ;
	}
	g_irgProgramLengthforEach [ iSection ] = g_iLocctr ;
	g_irgLiteralCountforEach [ iSection ] = iCurrentLiteralCount ;

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
void make_objectcode_output ( char * cpFile_name )
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
	g_iExtref_count = 0 ;



	file = fopen ( file_name , "w+" ) ;

	for ( ; i < g_iToken_count ; ++i )
	{
		if ( ( NULL != g_pToken_table [ i ] -> m_cpLabel ) && ( 0 == strcmp ( "." , g_pToken_table [ i ] -> m_cpLabel ) ) )
		{
			continue ;																		// Skip annotation
		}

		if ( ( 0 == strcmp ( "START" , g_pToken_table [ i ] -> m_cpOperator ) )					// New section
			|| ( 0 == strcmp ( "CSECT" , g_pToken_table [ i ] -> m_cpOperator ) ) )
		{
			if ( 0 != g_iExtref_count )
			{
				if ( '\0' != crgPrint [ 0 ] )
				{
					printToFileOrConsole ( file , crgPrint ) ;
				}

				for ( j = 0 ; j < g_iExtref_count ; ++j )
				{
					iAddress = g_Extref_table [ j ].addr ;
					iByte = g_Extref_table [ j ].half_byte ;

					if ( 1 == ( iByte % 2 ) )
					{
						++ iAddress ;
					}

					sprintf ( crgPrint , "M%06X%02X" , iAddress , iByte ) ;

					if ( 1 == g_Extref_table [ j ].cSign )
					{
						sprintf ( crgTemp , "+" ) ;
					}
					else
					{
						sprintf ( crgTemp , "-" ) ;
					}
					strcat ( crgPrint , crgTemp ) ;

					sprintf ( crgTemp , "%s" , g_Extref_table [ j ].literal ) ;
					strcat ( crgPrint , crgTemp ) ;

					printToFileOrConsole ( file , crgPrint ) ;
				}
			}
			if ( -1 < iSection )
			{
				if ( 0 == iSection )														// Main routine
				{
					sprintf ( crgPrint , "E%06x\n", atoi ( g_pToken_table [ 0 ] -> m_cpOperand [ 0 ] ) ) ;
				}
				else
				{
					sprintf ( crgPrint , "E\n" ) ;
				}

				printToFileOrConsole ( file , crgPrint ) ;
			}

			g_iLocctr = 0 ;
			++ iSection ;
			g_iExtref_count = 0 ;
			g_iExtrefPointCount = 0 ;

			sprintf ( crgPrint , "H%s " , g_pToken_table [ i ] -> m_cpLabel ) ;					// If label length is n, cnt of space is 6 - n. So fixed "H%-6s" -> "H%s "

			if ( NULL == g_pToken_table [ i ] -> m_cpOperand [ 0 ] )								// If CSECT, starting address is 0
			{
				sprintf ( crgTemp , "%06X" , 0 ) ;
			}
			else
			{
				sprintf ( crgTemp , "%06X" , atoi ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) ) ;
			}

			strcat ( crgPrint , crgTemp ) ;

			sprintf ( crgTemp , "%06X" , g_irgProgramLengthforEach [ iSection ] ) ;
			strcat ( crgPrint , crgTemp ) ;

			printToFileOrConsole ( file , crgPrint ) ;
		}
		else if ( 0 == strcmp ( "EXTDEF" , g_pToken_table [ i ] -> m_cpOperator ) )
		{
			sprintf ( crgTemp , "D" ) ;
			strcat ( crgPrint , crgTemp ) ;

			for ( j = 0 ; ( NULL != g_pToken_table [ i ] -> m_cpOperand [ j ] ) && ( j < 3 ) ; ++j )
			{
				sprintf ( crgTemp , "%s%06X" , g_pToken_table [ i ] -> m_cpOperand [ j ] , iGetSymLocation ( g_pToken_table [ i ] -> m_cpOperand [ j ] , iSection ) ) ;
				strcat ( crgPrint , crgTemp ) ;
			}

			printToFileOrConsole ( file , crgPrint ) ;
		}
		else if ( 0 == strcmp ( "EXTREF" , g_pToken_table [ i ] -> m_cpOperator ) )
		{
			sprintf ( crgTemp , "R" ) ;
			strcat ( crgPrint , crgTemp ) ;

			for ( j = 0 ; ( NULL != g_pToken_table [ i ] -> m_cpOperand [ j ] ) && ( j < 3 ) ; ++j )
			{
				sprintf ( crgTemp , "%-6s" , g_pToken_table [ i ] -> m_cpOperand [ j ] ) ;
				strcat ( crgPrint , crgTemp ) ;

				g_cprgExtrefList [ g_iExtrefPointCount ++ ] = g_pToken_table [ i ] -> m_cpOperand [ j ] ;
			}

			printToFileOrConsole ( file , crgPrint ) ;
		}
		else
		{
			iAddress = search_opcode ( g_pToken_table [ i ] -> m_cpOperator ) ;
			iByte = iGetOperandByte ( iAddress ) ;

			if ( '+' == g_pToken_table [ i ] -> m_cpOperator [ 0 ] )
			{
				iByte = 4 ;
				g_iLocctr += iByte ;
				iIndicator = 0 ;
			}
			else if ( iByte > 0 )
			{
				g_iLocctr += iByte ;
				iIndicator = 0 ;
			}
			else if ( 0 == strcmp ( "BYTE" , g_pToken_table [ i ] -> m_cpOperator ) )
			{
				iByte = 1 ;
				g_iLocctr += iByte ;
				iIndicator = 1 ;
			}
			else if ( 0 == strcmp ( "WORD" , g_pToken_table [ i ] -> m_cpOperator ) )
			{
				iByte = 3 ;
				g_iLocctr += iByte ;
				iIndicator = 2 ;
			}
			else if ( 0 == strcmp ( "LTORG" , g_pToken_table [ i ] -> m_cpOperator ) )						// Assign literal
			{
				iByte = 0 ;

				if ( 0 != g_irgLiteralCountforEach [ iSection ] )
				{
					iByte = 0 ;


					for ( j = iLiteralProcessCount ; j < iLiteralProcessCount + g_irgLiteralCountforEach [ iSection ] ; ++j )
					{
						if ( 'X' == g_Literal_table [ j ] -> m_cpLiteral [ 1 ] )									// Current literal is byte
						{
							++ iByte ;
						}
						else																			// Current literal is word
						{
							iByte += strlen ( g_Literal_table [ j ] -> m_cpLiteral ) - 4 ;
						}
					}

					g_iLocctr += iByte ;
					iIndicator = 3 ;
				}
			}
			else if ( ( 0 == strcmp ( "RESW" , g_pToken_table [ i ] -> m_cpOperator ) )
				|| ( 0 == strcmp ( "RESB" , g_pToken_table [ i ] -> m_cpOperator ) ) )						// Assign space, don't generate object code
			{
				iByte = 1 ;

				if ( 0 == strcmp ( "RESW" , g_pToken_table [ i ] -> m_cpOperator ) )
					iByte = 3 ;

				iByte *= atoi ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) ;


				g_iLocctr += iByte ;

				if ( '\0' != crgPrint [ 0 ] )
					printToFileOrConsole ( file , crgPrint ) ;

				iByte = 0 ;
			}

			if ( iByte > 0 )
			{
				if ( '\0' == crgPrint [ 0 ] )															// New line
				{
					sprintf ( crgTemp , "T%06X%02X" , g_iLocctr - iByte , 0 ) ;							// Line length keep updating
					strcat ( crgPrint , crgTemp ) ;
					iLineLength = 0 ;
				}
				else if ( ( 0x20 <= iLineLength + iByte ) || ( 3 == iIndicator ) )						// If line must end
				{
					printToFileOrConsole ( file , crgPrint ) ;

					iLineLength = 0 ;
					sprintf ( crgTemp , "T%06X%02X" , g_iLocctr - iByte , iLineLength ) ;					// Line length keep updating
					strcat ( crgPrint , crgTemp ) ;
				}

				iImmediate = 0 ;

				cpTemp = g_pToken_table [ i ] -> m_cpOperand [ 0 ] ;

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
						iTemp += g_irgLiteralCountforEach [ k ] ;
					}
					for ( j = 0 ; j < strlen ( cpTemp ) ; ++j )
					{
						if ( '-' == cpTemp [ j ] )
						{
							if ( 0 != j )
							{
								strncpy ( crgTemp , cpTemp + j - iWordLen , iWordLen ) ;
								crgTemp [ iWordLen ] = '\0' ;
																
								for ( k = iTemp ; k < iTemp + g_irgLiteralCountforEach [ iSection ] ; ++k )	// Check if it reference variable of current section
								{
									if ( 0 == strcmp ( crgTemp , g_Literal_table [ i ].m_cpLiteral ) )
									{
										iAddress += g_Literal_table [ i ].m_iAddr * cSign ;
										
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
																
								for ( k = iTemp ; k < iTemp + g_irgLiteralCountforEach [ iSection ] ; ++k )
								{
									if ( 0 == strcmp ( crgTemp , g_Literal_table [ i ].m_cpLiteral ) )
									{
										iAddress += g_Literal_table [ i ].m_iAddr * cSign ;
										
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
					for ( j = iLiteralProcessCount ; j < iLiteralProcessCount + g_irgLiteralCountforEach [ iSection ] ; ++j )
					{
						cpTemp = g_Literal_table [ j ] -> m_cpLiteral ;


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

					iLiteralProcessCount += g_irgLiteralCountforEach [ iSection ] ;
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
					if ( ( 2 == iGetInstOperandNum ( search_opcode ( g_pToken_table [ i ] -> m_cpOperator ) ) )
						&& ( NULL != g_pToken_table [ i ] -> m_cpOperand [ 1 ] ) )								// If multiple operand
					{
						for ( j = 0 ; j < 9 ; ++j )
						{
							if ( 0 == strcmp ( g_cdrgRegiter [ j ] , g_pToken_table [ i ] -> m_cpOperand [ 1 ] ) )
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
					iAddress += ( ( int ) ( g_pToken_table [ i ] -> m_cNixbpe ) ) << 12 ;						// Cast to int and shift left

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
						iTemp -= g_iLocctr ;

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
					iAddress += ( ( int ) ( g_pToken_table [ i ] -> m_cNixbpe ) ) << 20 ;						// Cast to int and shift left

					cpTemp = g_pToken_table [ i ] -> m_cpOperand [ 0 ] ;

					sprintf ( crgTemp , "%08X" , iAddress ) ;
					strcat ( crgPrint , crgTemp ) ;
				}


				for ( j = 0 ; ( NULL != g_pToken_table [ i ] -> m_cpOperand [ j ] ) && ( j < 3 ) ; ++j )		// Check if EXTREF is used
				{
					cpTemp = g_pToken_table [ i ] -> m_cpOperand [ j ] ;

					if ( ( '#' == cpTemp [ 0 ] ) || ( '@' == cpTemp [ 0 ] ) )							// If #,@ exist, move pointer
						++ cpTemp ;

					for ( k = 0 ; k < g_iExtrefPointCount ; ++k )
					{
						if ( 0 == strcmp ( cpTemp , g_cprgExtrefList [ k ] ) )
						{
							g_Extref_table [ g_iExtref_count ].literal = g_cprgExtrefList [ k ] ;
							g_Extref_table [ g_iExtref_count ].addr = g_iLocctr - iByte ;

							if ( 2 == iIndicator )
							{
								g_Extref_table [ g_iExtref_count ].half_byte = 6 ;
							}
							else
							{
								g_Extref_table [ g_iExtref_count ].half_byte = 5 ;
							}

							g_Extref_table [ g_iExtref_count ++ ].cSign = 1 ;

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
										if ( 0 == strcmp ( crgTemp , g_cprgExtrefList [ k ] ) )
										{
											g_Extref_table [ g_iExtref_count ].literal = g_cprgExtrefList [ k ] ;
											g_Extref_table [ g_iExtref_count ].addr = g_iLocctr - iByte ;
											g_Extref_table [ g_iExtref_count ].half_byte = 6 ;
											g_Extref_table [ g_iExtref_count ++ ].cSign = cSign ;

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
										if ( 0 == strcmp ( crgTemp , g_cprgExtrefList [ k ] ) )
										{
											g_Extref_table [ g_iExtref_count ].literal = g_cprgExtrefList [ k ] ;
											g_Extref_table [ g_iExtref_count ].addr = g_iLocctr - iByte ;
											g_Extref_table [ g_iExtref_count ].half_byte = 6 ;
											g_Extref_table [ g_iExtref_count ++ ].cSign = cSign ;

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
							if ( 0 == strcmp ( crgTemp , g_cprgExtrefList [ k ] ) )
							{
								g_Extref_table [ g_iExtref_count ].literal = g_cprgExtrefList [ k ] ;
								g_Extref_table [ g_iExtref_count ].addr = g_iLocctr - iByte ;
								g_Extref_table [ g_iExtref_count ].half_byte = 6 ;
								g_Extref_table [ g_iExtref_count ++ ].cSign = cSign ;

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
		for ( i = iLiteralProcessCount ; i < iLiteralProcessCount + g_irgLiteralCountforEach [ iSection ] ; ++i )
		{																						// If literal not assigned
			cpTemp = g_Literal_table [ i ].m_cpLiteral ;


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
	for ( i = 0 ; i < g_iExtref_count ; ++i )														// If EXTREF is not modified
	{
		iAddress = g_Extref_table [ i ].addr ;
		iByte = g_Extref_table [ i ].half_byte ;

		if ( 1 == ( iByte % 2 ) )
		{
			++ iAddress ;
		}

		sprintf ( crgPrint , "M%06X%02X" , iAddress , iByte ) ;

		if ( 1 == g_Extref_table [ i ].cSign )
		{
			sprintf ( crgTemp , "+" ) ;
		}
		else
		{
			sprintf ( crgTemp , "-" ) ;
		}
		strcat ( crgPrint , crgTemp ) ;

		sprintf ( crgTemp , "%s" , g_Extref_table [ i ].literal ) ;
		strcat ( crgPrint , crgTemp ) ;

		printToFileOrConsole ( file , crgPrint ) ;
	}



	sprintf ( crgPrint , "E" ) ;
	printToFileOrConsole ( file , crgPrint ) ;

	if ( NULL != file )
		fclose ( file ) ;
}














/*
 * Check whether cpStr is operation or not
 * Return opcode, or - if fail
 */
int search_opcode ( char * cpStr )
{
	int iCount = 0 ;
	char * cpTemp = cpStr ;



	if ( '+' == cpStr [ 0 ] )			// For format 4 instruction, ignore '+'
	{
		++ cpTemp ;
	}

	while ( ( iCount < MAX_INST ) && ( NULL != g_pInst_table [ iCount ] ) )
	{
		if ( 0 == strcmp ( cpTemp , g_pInst_table [ iCount ] ) )
		{
			return g_pInst_table [ iCount ] -> m_uiOpcode ;
		}

		++ iCount ;
	}

	return -1 ;								// Can't find opcode
}

int iStringToHex ( char * cpStr )					// Change string to hex
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



	while ( ( iCount < MAX_INST ) && ( NULL != g_pInst_table [ iCount ] ) )
	{
		free ( g_pInst_table [ iCount ++ ] ) ;
	}

	iCount = 0 ;

	while ( iCount < g_siLine_count )
	{
		free ( g_cpInput_data [ iCount ++ ] ) ;
	}

	g_siLine_count = 0 ;
	iCount = 0 ;
	
	while ( ( g_siLine_count < MAX_LINES ) && ( NULL != g_pToken_table [ g_iToken_count ] ) )
	{
		if ( NULL != g_pToken_table [ g_iToken_count ] -> m_cpLabel )			// If label exist, free
		{
			free ( g_pToken_table [ g_iToken_count ] -> m_cpLabel ) ;
		}
		if ( NULL != g_pToken_table [ g_iToken_count ] -> m_cpOperator )		// If operator exist, free
		{
			free ( g_pToken_table [ g_iToken_count ] -> m_cpOperator ) ;
		}
		while ( ( iCount != MAX_OPERAND ) && ( NULL != g_pToken_table [ g_iToken_count ] -> m_cpOperand [ iCount ] ) )			// If operand exist, free
		{
			free ( g_pToken_table [ g_iToken_count ] -> m_cpOperand [ iCount ++ ] ) ;
		}

		free ( g_pToken_table [ g_siLine_count ++ ] ) ;

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

	for ( ; i < g_iInst_count ; ++i )
	{
		if ( ciOpcode == g_pInst_table [ i ] -> m_uiOpcode )
		{
			iReturn = 7 & g_pInst_table [ i ] -> m_iFormat ;
			
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

	for ( ; i < g_iInst_count ; ++i )
	{
		if ( ciOpcode == g_pInst_table [ i ] -> m_uiOpcode )
		{
			iReturn = g_pInst_table [ i ] -> m_iOperands ;
			
			break ;
		}
	}

	return iReturn ;
}

/*
 * Find symbol address using cpStr
 * Former version use section number, but it's unnecessary
 * There are no same symbol at different section, so don't need to check section
 * Return address of symbol, or - if not found
 */
int iGetSymLocation ( char * cpStr )
{
	int i = 0 ;



	for ( ; i < g_iSymbol_count ; ++i )
	{
		if ( 0 == strcmp ( cpStr , g_Symbol_table [ i ] -> m_cpSymbol ) )
		{
			return g_Symbol_table [ i ] -> m_iAddr ;
		}
	}
	

	return -1 ;
}

/*
 * Find literal address using cpStr
 * Return address of literal, or - if not found
 */
int iGetLitLocation ( char * cpStr )
{
	int i = 0 ;



	for ( ; i < g_iLiteral_count ; ++i )
	{
		if ( 0 == strcmp ( cpStr , g_Literal_table [ i ] -> m_cpLiteral ) )
		{
			return g_Literal_table [ i ] -> m_iAddr ;
		}
	}


	return -1 ;
}
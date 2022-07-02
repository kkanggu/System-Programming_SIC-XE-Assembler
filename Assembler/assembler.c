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

	if ( assem_pass2 () < 0 )
	{
		printf ( "assem_pass2 : Failed to procedd pass 2.\n" ) ;
		return -1 ;
	}

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
	g_iLine_count = 0 ;
	char str [ MAX_INST ] = "\0 , " ;



	file = fopen ( cpInput_file , "r" ) ;

	if ( file == NULL )						// If no source, can't make into object program
	{
		return -1 ;
	}

	while ( fgets ( str , MAX_INST , file )	)					// Read operator name
	{
		g_cpInput_data [ g_iLine_count ] = malloc ( strlen ( str ) + 1 ) ;
		strcpy ( g_cpInput_data [ g_iLine_count ] , str ) ;

		++ g_iLine_count ;
	}


	if ( g_iLine_count != MAX_LINES - 1 )
	{
		g_cpInput_data [ g_iLine_count ] = NULL ;
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
	int i = 0 ;
	g_iToken_count = 0 ;



	for ( i = 0 ; ( i < g_iLine_count ) && ( g_iToken_count < MAX_LINES ) ; ++i )
	{
		if ( token_parsing ( g_cpInput_data [ i ] ) < 0 )		// Proceed token parsing with checking error
		{
			printf ( "assem_pass1 : Parsing failed.\n" ) ;
			
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
	g_pToken_table [ g_iToken_count ] -> m_iByte = -1 ;
	g_pToken_table [ g_iToken_count ] -> m_iDisplacement = 0 ;

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
	if ( ( iSetByteOfToken () ) < 0 )		// Set byte of each tokens
	{
		printf ( "iSetByteOfToken : Setting byte failed.\n" ) ;
		return -1 ;
	}
	if ( ( iSetSymbolLiteralInfo () ) < 0 )	// Set symbol and literal info
	{
		printf ( "iSetSymbolLiteralInfo : Setting symbol and literal info failed.\n" ) ;
		return -1 ;
	}
	if ( ( iSetAddrNixbpeInfo () ) < 0 )	// Set addr and nixbpe of tokens
	{
		printf ( "iSetAddrNixbpeInfo : Setting address and nixbpe failed.\n" ) ;
		return -1 ;
	}

	// 2형식, 4형식 싹다 고쳐야 함
	// + RDREC에서 6번 add, LDT MAXTREN도 이상함
	// + 추가적으로 Literal들에 section 변수를 추가해줘야 LTORG 처리가 가능함
	// Literal assign은 그냥 지역변수로 카운트 하면서 그 카운트부터 LTORG addr까지 쭉 체크하는 방식으로 하면 될 듯

	tempSetSomething () ;

	return 0 ;
}

/*
 * Set symbol table and literal table
 * Return 0 if success, - if fail
 */
int iSetByteOfToken ()
{
	int i = 0 ;
	char * cpTemp ;
	int iOpcode = 0 ;



	for ( ; i < g_iToken_count ; ++i )
	{
		cpTemp = g_pToken_table [ i ] -> m_cpOperator ;
		iOpcode = search_opcode ( cpTemp ) ;						// Get opcode

		if ( '+' == cpTemp [ 0 ] )									// Format 4
		{
			g_pToken_table [ i ] -> m_iByte = 4 ;
		}
		else if ( 0 <= iOpcode )										// Format 1,2,3
		{
			g_pToken_table [ i ] -> m_iByte = iGetOperandByte ( iOpcode ) ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "BYTE" ) )	// BYTE, WORD, RESB, RESW process for locctr
		{
			g_pToken_table [ i ] -> m_iByte = 1 ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "WORD" ) )
		{
			g_pToken_table [ i ] -> m_iByte = 3 ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "RESB" ) )
		{
			g_pToken_table [ i ] -> m_iByte = atoi ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "RESW" ) )
		{
			g_pToken_table [ i ] -> m_iByte = atoi ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) * 3 ;
		}
		else														// If exception process is needed, then modify this else statement
		{
			g_pToken_table [ i ] -> m_iByte = 0 ;
		}
	}

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
		if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "CSECT" ) )			// New section, initialize locctr to 0
			g_iLocctr = 0 ;

		cpTemp = g_pToken_table [ i ] -> m_cpLabel ;

		if ( NULL != cpTemp )					// Label exist, add to symbol table
		{
			g_Symbol_table [ g_iSymbol_count ] = malloc ( sizeof ( symbol ) ) ;
			g_Symbol_table [ g_iSymbol_count ] -> m_cpSymbol = malloc ( strlen ( cpTemp ) + 1 ) ;
			strcpy ( g_Symbol_table [ g_iSymbol_count ] -> m_cpSymbol , cpTemp ) ;						// Copy label to symbol table
			g_Symbol_table [ g_iSymbol_count ] -> m_iAddr = g_iLocctr ;
			g_Symbol_table [ g_iSymbol_count ] -> m_iByte = g_pToken_table [ i ] -> m_iByte ;


			if ( ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "EQU" ) )
				&& ( 0 != strcmp ( "*" , g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) ) )				// If EQU found, need to process
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

							g_Symbol_table [ g_iSymbol_count ] -> m_iAddr += iGetSymLocation ( crgLable ) * cSign ;
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

				g_Symbol_table [ g_iSymbol_count ] -> m_iAddr += iGetSymLocation ( crgLable ) * cSign ;
			}

			++ g_iSymbol_count ;
		}
		

		// From here, process literal

		cpTemp = g_pToken_table [ i ] -> m_cpOperator ;

		g_iLocctr += g_pToken_table [ i ] -> m_iByte ;

		
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
					else if ( '=' == g_Literal_table [ j ] -> m_cpLiteral [ 0 ] )				// =C'ASDF'
					{
						iByte = strlen ( g_Literal_table [ j ] -> m_cpLiteral ) - 4 ;
					}
					else																		// C'ASDF'
					{
						iByte = strlen ( g_Literal_table [ j ] -> m_cpLiteral ) - 3 ;
					}

				
					g_Literal_table [ j ] -> m_iAddr = g_iLocctr ;
					g_Literal_table [ j ] -> m_iByte = iByte ;
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

			if ( j == g_iLiteral_count )													// If this literal is unique, add
			{
				iLen = strlen ( cpTemp ) ;
				g_Literal_table [ g_iLiteral_count ] = malloc ( sizeof ( USER_Literal ) ) ;
				g_Literal_table [ g_iLiteral_count ] -> m_cpLiteral = malloc ( iLen + 1 ) ;
				strcpy ( g_Literal_table [ g_iLiteral_count ] -> m_cpLiteral , cpTemp ) ;
				g_Literal_table [ g_iLiteral_count ] -> m_iAddr = -1 ;
				g_Literal_table [ g_iLiteral_count ] -> m_iByte = g_pToken_table [ i ] -> m_iByte ;
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
			g_Literal_table [ j ] -> m_iByte = iByte ;
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
	int i = 0 ;
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
		
		iByte = g_pToken_table [ i ] -> m_iByte ;

		if ( '+' == g_pToken_table [ i ] -> m_cpOperator [ 0 ] )						// Format 4
		{
			g_pToken_table [ i ] -> m_cNixbpe = 0b110001 ;
		}
		else if ( iByte > 0 )															// Format 1,2,3
		{
			g_pToken_table [ i ] -> m_cNixbpe = 0b110000 ;
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
			}
		}
		else																			// If operator don't need space(byte), move to next line
		{
			continue ;
		}

		g_iLocctr += iByte ;


		if ( NULL != g_pToken_table [ i ] -> m_cpOperand [ 0 ] )						// If operand exist, need to compute target memory(dispacement)
		{
			iLocation = -1 ;

			
			if ( '=' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] )				// If literal
			{
				for ( j = 0 ; j < g_iLiteral_count ; ++j )
				{
					if ( 0 == strcmp ( g_Literal_table [ j ] -> m_cpLiteral , g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) )
					{																	// Find literal
						iLocation = g_Literal_table [ j ] -> m_iAddr ;

						if ( j == iTotalLiteralCount )
						{
							++ iCurrentLiteralCount ;
							++ iTotalLiteralCount ;
						}

						break ;
					}
				}

				if ( -1 == iLocation )
				{
					printf ( "iSetAddrNixbpeInfo : Can't find literal\n" ) ;

					return -1 ;
				}
			}
			else																		// Need find at symbol table
			{
				for ( j = 0 ; j < g_iSymbol_count ; ++j )
				{
					if ( ( ( '@' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] )	// # or @ exist then need to process
							|| ( '#' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] ) )
						&& ( 0 == strcmp ( g_Symbol_table [ j ] -> m_cpSymbol , g_pToken_table [ i ] -> m_cpOperand [ 0 ] + 1 ) ) )
					{
						iLocation = g_Symbol_table [ j ] -> m_iAddr ;

						break ;
					}
					else if ( 0 == strcmp ( g_Symbol_table [ j ] -> m_cpSymbol , g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) )
					{
						iLocation = g_Symbol_table [ j ] -> m_iAddr ;

						break ;
					}
				}

				if ( -1 == iLocation )
				{
					iLocation = atoi ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] + 1 ) ;


					// printf ( "iSetAddrNixbpeInfo : Can't find symbol\n" ) ;
					// 
					// return -1 ;
				}
			}

			if ( '#' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] )			// Immediate addressing, n1 = 01
			{
				g_pToken_table [ i ] -> m_cNixbpe &= 0b011111 ;
			}
			else if ( '@' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] )		// Indirect addressing, n1 = 10
			{
				g_pToken_table [ i ] -> m_cNixbpe &= 0b101111 ;
			}

			if ( ( -2048 <= iLocation - g_iLocctr )
					   && ( iLocation - g_iLocctr <= 2047 ) )						// If pc relative is possible
			{
				g_pToken_table [ i ] -> m_cNixbpe |= 0b000010 ;
				g_pToken_table [ i ] -> m_iDisplacement = iLocation - g_iLocctr ;
			}
			else																	// We already filtered format 4 extension
			{																		// So if exceed condition failed, then base relative is the only way
				g_pToken_table [ i ] -> m_cNixbpe |= 0b000100 ;
					
				// Set base position and update dispacement
			}
		}

		for ( j = 0 ; j < MAX_OPERAND ; ++j )		// Max 3 operand, find x for loop
		{
			if ( ( NULL != g_pToken_table [ i ] -> m_cpOperand [ j ] )
				&& ( 0 == strcmp ( "X" , g_pToken_table [ i ] -> m_cpOperand [ j ] ) ) )
			{
				g_pToken_table [ i ] -> m_cNixbpe |= 0b001000 ;

				continue ;
			}

			break ;			// If find break, then NULL. Don't have to check
		}
	}

	iTemp = 0 ;
	for ( i = 0 ; i < iSection + 1 ; ++i )
	{
		iTemp += g_irgLiteralCountforEach [ i ] ;
	}
	for ( i = iTemp ; i < iTotalLiteralCount ; ++i )	// If literal address is not computed, then proceed for loop
	{
		iByte = 1 ;

		if ( 'C' == g_Literal_table [ i ] -> m_cpLiteral [ 1 ] )
		{
			iByte = strlen ( g_Literal_table [ i ] -> m_cpLiteral - 4 ) ;
		}

		g_iLocctr = g_Literal_table [ i ] -> m_iAddr + iByte ;
	}

	if ( -1 == iSection )
	{
		printf ( "iSetAddrNixbpeInfo : Section is -1." ) ;

		return -1 ;
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
void tempSetSomething ()
{
	int i = 0 ;
	int j = 0 ;
	int k = 0 ;
	int iTemp = 0 ;
	int iSection = -1 ;
	int iByte = 0 ;
	int iLineLength = 0 ;
	int iAddress = 0 ;
	int iImmediate = 0 ;
	char crgTemp [ 20 ] = { 0 , } ;
	char * cpTemp ;
	char cSign = 0 ;
	int iWordLen = 0 ;
	int iIndicator = 0 ;				// 0 Normal, 1 Byte, 2 Word, 3 LTORG
	int iLiteralProcessCount = 0 ;
	char * cprgExtrefList [ 10 ] ;		// Save string of EXTREF at current section
	int iExtrefPointCount ;				// Count EXTREF at current section
	g_iExtref_count = 0 ;



	for ( ; i < g_iToken_count ; ++i )
	{
		if ( ( 0 == strcmp ( "START" , g_pToken_table [ i ] -> m_cpOperator ) )				// New section
			|| ( 0 == strcmp ( "CSECT" , g_pToken_table [ i ] -> m_cpOperator ) ) )
		{
			g_iLocctr = 0 ;
			++ iSection ;
			g_iExtref_count = 0 ;
			iExtrefPointCount = 0 ;
		}
		else if ( ( 0 != strcmp ( "EXTDEF" , g_pToken_table [ i ] -> m_cpOperator ) ) 
			&& ( 0 == strcmp ( "EXTREF" , g_pToken_table [ i ] -> m_cpOperator ) ) )
		{
			iAddress = search_opcode ( g_pToken_table [ i ] -> m_cpOperator ) ;
			iByte = g_pToken_table [ i ] -> m_iByte ;
			g_iLocctr += iByte ;
			iIndicator = 0 ;

//			if ( iByte > 0 )
//			{
//				iIndicator = 0 ;
//			}
			if ( 0 == strcmp ( "BYTE" , g_pToken_table [ i ] -> m_cpOperator ) )
			{
				iIndicator = 1 ;
			}
			else if ( 0 == strcmp ( "WORD" , g_pToken_table [ i ] -> m_cpOperator ) )
			{
				iIndicator = 2 ;
			}
			else if ( 0 == strcmp ( "LTORG" , g_pToken_table [ i ] -> m_cpOperator ) )						// Assign literal
			{
//				iByte = 0 ;
//
//				if ( 0 != g_irgLiteralCountforEach [ iSection ] )
//				{
//					iByte = 0 ;
//
//
//					for ( j = iLiteralProcessCount ; j < iLiteralProcessCount + g_irgLiteralCountforEach [ iSection ] ; ++j )
//					{
//						if ( 'X' == g_Literal_table [ j ] -> m_cpLiteral [ 1 ] )									// Current literal is byte
//						{
//							++ iByte ;
//						}
//						else																			// Current literal is word
//						{
//							iByte += strlen ( g_Literal_table [ j ] -> m_cpLiteral ) - 4 ;
//						}
//					}
//
//					g_iLocctr += iByte ;
//					iIndicator = 3 ;
//				}
				iIndicator = 3 ;
			}
//			else if ( ( 0 == strcmp ( "RESW" , g_pToken_table [ i ] -> m_cpOperator ) )
//				|| ( 0 == strcmp ( "RESB" , g_pToken_table [ i ] -> m_cpOperator ) ) )					// Assign space, don't generate object code
//			{
//				iByte = 1 ;
//
//				if ( 0 == strcmp ( "RESW" , g_pToken_table [ i ] -> m_cpOperator ) )
//					iByte = 3 ;
//
//				iByte *= atoi ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) ;
//
//
//				g_iLocctr += iByte ;
//
//				iByte = 0 ;
//			}
			
			if ( 0 < iByte )
			{
				for ( j = 0 ; ( NULL != g_pToken_table [ i ] -> m_cpOperand [ j ] ) && ( j < 3 ) ; ++j )		// Check if EXTREF is used
				{
					cpTemp = g_pToken_table [ i ] -> m_cpOperand [ j ] ;

					if ( ( '#' == cpTemp [ 0 ] ) || ( '@' == cpTemp [ 0 ] ) )							// If #,@ exist, move pointer
						++ cpTemp ;

					for ( k = 0 ; k < iExtrefPointCount ; ++k )
					{
						if ( 0 == strcmp ( cpTemp , cprgExtrefList [ k ] ) )
						{
							g_Extref_table [ g_iExtref_count ] -> m_cpLiteral = cprgExtrefList [ k ] ;
							g_Extref_table [ g_iExtref_count ] -> m_iAddr = g_iLocctr - iByte ;

							if ( 2 == iIndicator )
							{
								g_Extref_table [ g_iExtref_count ] -> m_iHalf_byte = 6 ;
							}
							else
							{
								g_Extref_table [ g_iExtref_count ] -> m_iHalf_byte = 5 ;
							}

							g_Extref_table [ g_iExtref_count ++ ] -> m_cSign = 1 ;

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
																
									for ( k = 0 ; k < iExtrefPointCount ; ++k )
									{
										if ( 0 == strcmp ( crgTemp , cprgExtrefList [ k ] ) )
										{
											g_Extref_table [ g_iExtref_count ] -> m_cpLiteral = cprgExtrefList [ k ] ;
											g_Extref_table [ g_iExtref_count ] -> m_iAddr = g_iLocctr - iByte ;
											g_Extref_table [ g_iExtref_count ] -> m_iHalf_byte = 6 ;
											g_Extref_table [ g_iExtref_count ++ ] -> m_cSign = cSign ;

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
																
									for ( k = 0 ; k < iExtrefPointCount ; ++k )
									{
										if ( 0 == strcmp ( crgTemp , cprgExtrefList [ k ] ) )
										{
											g_Extref_table [ g_iExtref_count ] -> m_cpLiteral = cprgExtrefList [ k ] ;
											g_Extref_table [ g_iExtref_count ] -> m_iAddr = g_iLocctr - iByte ;
											g_Extref_table [ g_iExtref_count ] -> m_iHalf_byte = 6 ;
											g_Extref_table [ g_iExtref_count ++ ] -> m_cSign = cSign ;

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

						for ( k = 0 ; k < iExtrefPointCount ; ++k )
						{
							if ( 0 == strcmp ( crgTemp , cprgExtrefList [ k ] ) )
							{
								g_Extref_table [ g_iExtref_count ] -> m_cpLiteral = cprgExtrefList [ k ] ;
								g_Extref_table [ g_iExtref_count ] -> m_iAddr = g_iLocctr - iByte ;
								g_Extref_table [ g_iExtref_count ] -> m_iHalf_byte = 6 ;
								g_Extref_table [ g_iExtref_count ++ ] -> m_cSign = cSign ;

								break ;
							}
						}
					}
				}
			}
		}
	}
}

 /*
  * Convert source to object program using all the info
  * If parameter is NULL(don't save to file), then print to console
  * Return 0 if success, - if fail
  */
 int iPrintObjectCode ( char * cpFile_name )
 {
 


	 return 0 ;
 }











/*
 * Check whether cpStr is operation or not
 * Return opcode, or - if fail
 */
int search_opcode ( char * cpStr )
{
	int i = 0 ;
	char * cpTemp = cpStr ;



	if ( '+' == cpStr [ 0 ] )			// For format 4 instruction, ignore '+'
	{
		++ cpTemp ;
	}

	for ( int i = 0 ; i < g_iInst_count ; ++i )
	{
		if ( 0 == strcmp ( cpTemp , g_pInst_table [ i ] -> m_cpOperation ) )
		{
			return g_pInst_table [ i ] -> m_uiOpcode ;
		}
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
		iHigh = ( cpStr [ i * 2 ] > '9' ) ? cpStr [ i * 2 ] - 'A' + 10 : cpStr [ i * 2 ] - '0' ;
		iLow = ( cpStr [ i * 2 + 1 ] > '9' ) ? cpStr [ i * 2 + 1 ] - 'A' + 10 : cpStr [ i * 2 + 1 ] - '0' ;
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
	int i = 0 ;
	int j = 0 ;


	for ( i = 0 ; ( i < MAX_INST ) && ( NULL != g_pInst_table [ i ] ) ; ++i )
	{
		free ( g_pInst_table [ iCount ] -> m_cpOperation ) ;
		free ( g_pInst_table [ iCount ++ ] ) ;
	}

	for ( int i = 0 ; i < g_iLine_count ; ++i )
	{
		free ( g_cpInput_data [ iCount ++ ] ) ;
	}
	
	for ( i = 0 ; ( i < MAX_LINES ) && ( NULL != g_pToken_table [ i ] ) ; ++i )
	{
		if ( NULL != g_pToken_table [ i ] -> m_cpLabel )		// If label exist, free
		{
			free ( g_pToken_table [ i ] -> m_cpLabel ) ;
		}
		if ( NULL != g_pToken_table [ i ] -> m_cpOperator )		// If operator exist, free
		{
			free ( g_pToken_table [ i ] -> m_cpOperator ) ;
		}
		for ( j = 0 ; ( j < MAX_OPERAND ) && ( NULL != g_pToken_table [ i ] -> m_cpOperand [ j ] ) ; ++j )			// If operand exist, free
		{
			free ( g_pToken_table [ i ] -> m_cpOperand [ j ] ) ;
		}

		free ( g_pToken_table [ i ] ) ;

		iCount = 0 ;
	}

	for ( int i = 0 ; i < g_iSymbol_count ; ++i )
	{
		free ( g_Symbol_table [ i ] -> m_cpSymbol ) ;
		free ( g_Symbol_table [ i ] ) ;
	}

	for ( int i = 0 ; i < g_iLiteral_count ; ++i )
	{
		free ( g_Literal_table [ i ] -> m_cpLiteral ) ;
		free ( g_Literal_table [ i ] ) ;
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
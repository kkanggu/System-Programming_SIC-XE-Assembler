#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

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
	if ( ( init_input_file ( "input.txt" ) ) < 0 )		// Read assembly input
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
	char * cpFile = "object_code.obj" ;

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
	if ( ( iSetDisplacement () ) < 0 )		// Set displacement of tokens
	{
		printf ( "iSetDisplacement : Setting displacement failed.\n" ) ;
		return -1 ;
	}


	if ( ( iConvertToObjectCode () ) < 0 )			// Convert assembly to object code
	{
		printf ( "iConvertToObjectCode : Converting assembly to object code.\n" ) ;
		return -1 ;
	}

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
		iOpcode = iSearch_opcode ( cpTemp ) ;						// Get opcode

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
	int iSection = 0 ;



	for ( ; i < g_iToken_count ; ++i )
	{
		if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "CSECT" ) )			// New section, initialize locctr to 0
		{
			g_iLocctr = 0 ;
			++ iSection ;
		}

		cpTemp = g_pToken_table [ i ] -> m_cpLabel ;

		if ( NULL != cpTemp )					// Label exist, add to symbol table
		{
			g_Symbol_table [ g_iSymbol_count ] = malloc ( sizeof ( symbol ) ) ;
			g_Symbol_table [ g_iSymbol_count ] -> m_cpSymbol = malloc ( strlen ( cpTemp ) + 1 ) ;
			strcpy ( g_Symbol_table [ g_iSymbol_count ] -> m_cpSymbol , cpTemp ) ;						// Copy label to symbol table
			g_Symbol_table [ g_iSymbol_count ] -> m_iAddr = g_iLocctr ;
			g_Symbol_table [ g_iSymbol_count ] -> m_iByte = g_pToken_table [ i ] -> m_iByte ;
			g_Symbol_table [ g_iSymbol_count ] -> m_iSection = iSection ;


			if ( ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "EQU" ) )
				&& ( 0 != strcmp ( "*" , g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) ) )				// If EQU found, need to process
			{
				iLen = 0 ;

				g_Symbol_table [ g_iSymbol_count ] -> m_iAddr = 0 ;
				g_Symbol_table [ g_iSymbol_count ] -> m_iByte = -1 ;				// EQU is like define

				cpTemp = g_pToken_table [ i ] -> m_cpOperand [ 0 ] ;

				for ( j = 0 ; j < strlen ( cpTemp ) ; ++j )
				{
					if ( ( '-' == cpTemp [ j ] ) || ( '+' == cpTemp [ j ] ) )		// Need to proceed former formula
					{
						if ( 0 != j )
						{
							strncpy ( crgLable , cpTemp + j - iLen , iLen ) ;
							crgLable [ iLen ] = '\0' ;

							g_Symbol_table [ g_iSymbol_count ] -> m_iAddr += iGetSymLocation ( crgLable , iSection ) * cSign ;
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

				g_Symbol_table [ g_iSymbol_count ] -> m_iAddr += iGetSymLocation ( crgLable , iSection ) * cSign ;
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
				g_Literal_table [ g_iLiteral_count ] -> m_iSection = iSection ;
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
 * ++ Set displacement of format 3 operation
 * Return 0 if success, - if fail
 */
int iSetAddrNixbpeInfo ()
{
	int i = 0 ;
	int iByte = 0 ;
	int iLocation = 0 ;
	int iSection = 0 ;
	int j = 0 ;
	int iTemp = 0 ;
	int iCurrentLiteralCount = 0 ;
	int iTotalLiteralCount = 0 ;
	g_iLocctr = 0 ;



	for ( ; i < g_iToken_count ; ++i )
	{
		if ( 0 == strcmp ( "CSECT" , g_pToken_table [ i ] -> m_cpOperator ) )			// New section
		{
//			g_irgProgramLengthforEach [ iSection ] = g_iLocctr ;						// Save former section info
//			g_irgLiteralCountforEach [ iSection ] = iCurrentLiteralCount ;
			g_iLocctr = 0 ;
			++ iSection ;
			iCurrentLiteralCount = 0 ;
		}
		
		iByte = g_pToken_table [ i ] -> m_iByte ;

		if ( ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "BYTE" ) )			// BYTE, WORD then nixbpe is not needed
			|| ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "WORD" ) ) )
		{
			g_pToken_table [ i ] -> m_cNixbpe = -1 ;
			continue ;
		}
		else if ( ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "RESB" ) )		// RESB, RESW then no need to print
			|| ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "RESW" ) ) )
		{
			g_pToken_table [ i ] -> m_cNixbpe = -1 ;
			continue ;
		}
		else if ( '+' == g_pToken_table [ i ] -> m_cpOperator [ 0 ] )					// Format 4
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
				if ( ( '@' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] )			// # or @ exist then need to process
						|| ( '#' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] ) )
				{
					iLocation = iGetSymLocation ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] + 1 , iSection ) ;
				}
				else
				{
					iLocation = iGetSymLocation ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] , iSection ) ;
				}

				if ( -1 == iLocation )
				{
					iLocation = 0 ;


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
				g_pToken_table [ i ] -> m_iDisplacement = iLocation - g_iLocctr ;	// This is right for format 3
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


	for ( i = 0 ; i < iTotalLiteralCount ; ++i )
	{
		if ( iSection == g_Literal_table [ i ] -> m_iSection )
			break ;
	}
	for ( ; i < iTotalLiteralCount ; ++i )	// If literal address is not computed, then proceed for loop
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

//	iTemp = 0 ;
//	for ( i = 0 ; i < iSection + 1 ; ++i )
//	{
//		iTemp += g_irgLiteralCountforEach [ i ] ;
//	}
//	for ( i = iTemp ; i < iTotalLiteralCount ; ++i )	// If literal address is not computed, then proceed for loop
//	{
//		iByte = 1 ;
//
//		if ( 'C' == g_Literal_table [ i ] -> m_cpLiteral [ 1 ] )
//		{
//			iByte = strlen ( g_Literal_table [ i ] -> m_cpLiteral - 4 ) ;
//		}
//
//		g_iLocctr = g_Literal_table [ i ] -> m_iAddr + iByte ;
//	}
//
//	if ( -1 == iSection )
//	{
//		printf ( "iSetAddrNixbpeInfo : Section is -1." ) ;
//
//		return -1 ;
//	}
//
//	g_irgProgramLengthforEach [ iSection ] = g_iLocctr ;
//	g_irgLiteralCountforEach [ iSection ] = iCurrentLiteralCount ;

	


	return 0 ;
}

/*
 * Set displacement of operation except format 3
 * Return 0 if success, - if fail
 */
int iSetDisplacement ()
{
	int i = 0 ;
	int j = 0 ;
	int iByte = 0 ;
	int iLen = 0 ;
	int iTemp = 0 ;
	char crgTemp [ 50 ] = { 0 , } ;
	char * cpTemp ;
	int iSection = 0 ;
	char cSign = 1 ;



	for ( ; i < g_iToken_count ; ++i )
	{
		iByte = g_pToken_table [ i ] -> m_iByte ;

		if ( 0 == iByte )
			continue ;


		if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "BYTE" ) )		// BYTE operation
		{
			strncpy ( crgTemp , g_pToken_table [ i ] -> m_cpOperand [ 0 ] + 2 , 2 ) ;
			crgTemp [ 2 ] = '\0' ;
			g_pToken_table [ i ] -> m_iDisplacement = iStringToHex ( crgTemp ) ;
		}
		else if ( 0 == strcmp ( g_pToken_table [ i ] -> m_cpOperator , "WORD" ) )	// WORD operation
		{																			// Similar as EQU process, but process timing and target variable is different, so separate it
			iLen = 0 ;
			cpTemp = g_pToken_table [ i ] -> m_cpLabel ;			
			g_pToken_table [ i ] -> m_iDisplacement = 0 ;

			for ( j = 0 ; j < g_iSymbol_count ; ++j )
			{
				if ( ( 0 == strcmp ( cpTemp , g_Symbol_table [ j ] -> m_cpSymbol ) )
					&& ( -1 != g_Symbol_table [ j ] -> m_iByte ) )					// Except EQU
				{
					iSection = g_Symbol_table [ j ] -> m_iSection ;
					break ;
				}
			}

			cpTemp = g_pToken_table [ i ] -> m_cpOperand [ 0 ] ;			

			for ( j = 0 ; j < strlen ( cpTemp ) ; ++j )
			{
				if ( ( '-' == cpTemp [ j ] ) || ( '+' == cpTemp [ j ] ) )	// Need to proceed former formula
				{
					if ( 0 != j )
					{
						strncpy ( crgTemp , cpTemp + j - iLen , iLen ) ;
						crgTemp [ iLen ] = '\0' ;

						iTemp = iGetSymLocation ( crgTemp , iSection ) ;

						if ( -1 != iTemp )
							g_pToken_table [ i ] -> m_iDisplacement += iTemp * cSign ;
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

			strncpy ( crgTemp , cpTemp + j - iLen , iLen ) ;				// Need to proceed last formula
			crgTemp [ iLen ] = '\0' ;

			iTemp = iGetSymLocation ( crgTemp , iSection ) ;

			if ( -1 != iTemp )
				g_pToken_table [ i ] -> m_iDisplacement += iTemp * cSign;
		}
		else if ( ( NULL != g_pToken_table [ i ] -> m_cpOperand [ 0 ] )
			&& ( '#' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] )
			&& ( '+' == g_pToken_table [ i ] -> m_cpOperator [ 0 ] ) )		// Immediate addressing, not format 4
		{
			g_pToken_table [ i ] -> m_iDisplacement = atoi ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) ;
		}
		else if ( 2 == iByte )			// Format 2 operation
		{
			g_pToken_table [ i ] -> m_iDisplacement = iGetRegisterNum ( g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) << 4 ;

			if ( NULL != g_pToken_table [ i ] -> m_cpOperand [ 1 ] )
			{
				g_pToken_table [ i ] -> m_iDisplacement += iGetRegisterNum ( g_pToken_table [ i ] -> m_cpOperand [ 1 ] ) ;
			}
		}
		else if ( 4 == iByte )
		{
			g_pToken_table [ i ] -> m_iDisplacement = 0 ;
		}
	}


	return 0 ;
}

/*
 * Convert source to object program using all the info
 * Infos are saved at object code table
 * Return 0 if success, - if fail
 */
int iConvertToObjectCode ()
{
	int i = 0 ;
	int j = 0 ;
	int k = 0 ;
	int iByte = 0 ;
	int iSection = -1 ;
	int iLiteralFoundCount = 0 ;		// Count of found literal
	int iLiteralAddCount = 0 ;			// Count of added literal
	bool bHeadAdd = true ;
	char cTemp ;
	char * cpTemp ;
	g_iLocctr = 0 ;
	int iLen = 0 ;
	char crgLable [ 20 ] = { 0 , } ;
	char cSign = 1 ;


	for ( i = 0 ; i < g_iToken_count ; ++i )
	{
		iByte = g_pToken_table [ i ] -> m_iByte ;

		if ( ( NULL != g_pToken_table [ i ] -> m_cpOperand [ 0 ] )							// If LTORG, need to add literal
			&& ( 0 == strcmp ( "LTORG" , g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) ) )
		{
			for ( j = iLiteralAddCount ; j < iLiteralFoundCount ; ++j )
			{
				addObjectCodeTableMember ( 'T' , g_Literal_table [ j ] -> m_cpLiteral , iSection ) ;
				g_ObjectCode_table [ g_iObjectCode_count ] -> m_iCode = g_pToken_table [ i ] -> m_iDisplacement ;
				g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte = g_pToken_table [ i ] -> m_iByte ;
				g_ObjectCode_table [ g_iObjectCode_count ] -> m_iAddr = g_iLocctr ;

				++ iLiteralAddCount ;
				g_iLocctr += g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte ;
			}
		}
		else if ( ( bHeadAdd ) && ( NULL != g_pToken_table [ i ] -> m_cpLabel )				// Need to add Head record
			&& ( 0 == iGetSymLocation ( g_pToken_table [ i ] -> m_cpLabel , iSection ) ) )
		{
			if ( iLiteralFoundCount != iLiteralAddCount )									// Literals need to be assigned
			{
				for ( j = iLiteralAddCount ; j < iLiteralFoundCount ; ++j )
				{
					addObjectCodeTableMember ( 'T' , g_Literal_table [ j ] -> m_cpLiteral , iSection ) ;
					g_ObjectCode_table [ g_iObjectCode_count ] -> m_iCode = g_Literal_table [ j ] -> m_iAddr ;
					g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte = g_Literal_table [ j ] -> m_iByte ;
					g_ObjectCode_table [ g_iObjectCode_count ] -> m_iAddr = g_iLocctr ;
					
					++ iLiteralAddCount ;
					g_iLocctr += g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte ;
				}
			}
			if ( 0 != g_iExtref_count )														// Modification record
			{
				for ( j = 0 ; j < g_iExtref_count ; --j )
				{
					addObjectCodeTableMember ( 'M' , g_Extref_table [ j ] -> m_cpLabel , iSection ) ;
					g_ObjectCode_table [ g_iObjectCode_count ] -> m_iAddr = g_Extref_table [ j ] -> m_iAddr ;
					g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte = g_Extref_table [ j ] -> m_iByte ;
					g_ObjectCode_table [ g_iObjectCode_count ] -> m_cSign = g_Extref_table [ j ] -> m_cSign ;
					
					if ( g_Extref_table [ j ] -> m_bHalf_byte )								// If true, then half byte
						-- g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte ;
				}
				for ( j = 0 ; j < g_iExtref_count ; --j )
				{
					free ( g_Extref_table [ j ] -> m_cpLabel ) ;
					free ( g_Extref_table [ j ] ) ;
				}

				g_iExtref_count = 0 ;
			}
			if ( -1 != iSection )															// Not first program, then need to add End record
			{
				addObjectCodeTableMember ( 'E' , NULL , iSection ) ;
				g_irgProgramLengthforEach [ iSection ] = g_iLocctr ;
			}

			++ iSection ;
			g_iLocctr = 0 ;

			addObjectCodeTableMember ( 'H' , g_pToken_table [ i ] -> m_cpLabel , iSection ) ;

			bHeadAdd = false ;
		}
		else if ( ( NULL != g_pToken_table [ i ] -> m_cpOperator )							// Extdef && Extref
			&& ( 0 == strncmp ( "EXT" , g_pToken_table [ i ] -> m_cpOperator , 3 ) ) )
		{
			if ( 'D' == g_pToken_table [ i ] -> m_cpOperator [ 3 ] )
				cTemp = 'D' ;
			else
				cTemp = 'R' ;

			for ( j = 0 ; j < MAX_OPERAND ; ++j )
			{
				if ( NULL == g_pToken_table [ i ] -> m_cpOperand [ j ] )
					break ;
				
				addObjectCodeTableMember ( cTemp , g_pToken_table [ i ] -> m_cpOperand [ j ] , iSection ) ;

				if ( 'D' == cTemp )
				{
					g_ObjectCode_table [ g_iObjectCode_count ] -> m_iAddr = iGetSymLocation ( g_ObjectCode_table [ g_iObjectCode_count ] -> m_cpSymbol , iSection ) ;
				}
			}
		}
		else if ( 0 != iByte )
		{
			addObjectCodeTableMember ( 'T' , NULL , iSection ) ;
			g_ObjectCode_table [ g_iObjectCode_count ] -> m_iCode = iGetInstObjectCode ( g_pToken_table [ i ] -> m_iByte ,
				iSearch_opcode ( g_pToken_table [ i ] -> m_cpOperator ) , g_pToken_table [ i ] -> m_cNixbpe , g_pToken_table [ i ] -> m_iDisplacement ) ;
			g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte = g_pToken_table [ i ] -> m_iByte ;
			g_ObjectCode_table [ g_iObjectCode_count ] -> m_iAddr = g_iLocctr ;

			if ( ( NULL != g_pToken_table [ i ] -> m_cpOperand [ 0 ] ) && ( '=' == g_pToken_table [ i ] -> m_cpOperand [ 0 ] [ 0 ] ) )
			{
				++ iLiteralFoundCount ;
			}

			for ( j = 0 ; j < MAX_OPERAND ; ++j )			// Find external reference for processing modification record
			{
				cpTemp = g_pToken_table [ i ] -> m_cpOperand [ j ] ;
				cSign = 1 ;

				if ( NULL == cpTemp )
					break ;

				if ( ( '#' == cpTemp [ 0 ] ) || ( '@' == cpTemp [ 0 ] ) || ( '=' == cpTemp [ 0 ] ) )		// Except unnecessary symbol
				{
					if ( -1 == iGetSymLocation ( cpTemp + 1 , iSection ) )
					{
						addExtrefTableMember ( cpTemp + 1 , g_iLocctr , iByte , ( -1 != g_pToken_table [ i ] -> m_cNixbpe ) , cSign ) ;
					}
				}
				else																	// Separate formula and process
				{
					iLen = 0 ;

					for ( k = 0 ; k < strlen ( cpTemp ) ; ++k )
					{
						if ( ( '-' == cpTemp [ k ] ) || ( '+' == cpTemp [ k ] ) )		// Need to proceed former formula
						{
							if ( 0 != k )
							{
								strncpy ( crgLable , cpTemp + j - iLen , iLen ) ;
								crgLable [ iLen ] = '\0' ;

								addExtrefTableMember ( crgLable , g_iLocctr , iByte , ( -1 != g_pToken_table [ i ] -> m_cNixbpe ) , cSign ) ;
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

					addExtrefTableMember ( crgLable , g_iLocctr , iByte , ( -1 != g_pToken_table [ i ] -> m_cNixbpe ) , cSign ) ;
				}
			}

			bHeadAdd = true ;
			g_iLocctr += g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte ;
		}
	}

	if ( iLiteralFoundCount != g_iLiteral_count )	// If some literals are missing
	{
		printf ( "iConvertToObjectCode : Some literals are missing\n" ) ;

		return -1 ;
	}
	if ( iLiteralAddCount != g_iLiteral_count )		// Program end without adding last literal, so need to add literals
	{
		for ( j = iLiteralAddCount ; j < iLiteralFoundCount ; ++j )
		{
			addObjectCodeTableMember ( 'T' , g_Literal_table [ j ] -> m_cpLiteral , iSection ) ;
			g_ObjectCode_table [ g_iObjectCode_count ] -> m_iCode = g_Literal_table [ j ] -> m_iAddr ;
			g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte = g_Literal_table [ j ] -> m_iByte ;
			g_ObjectCode_table [ g_iObjectCode_count ] -> m_iAddr = g_iLocctr ;

			++ iLiteralAddCount ;
			g_iLocctr += g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte ;
		}
	}

	if ( 0 != g_iExtref_count )														// Modification record
	{
		for ( j = 0 ; j < g_iExtref_count ; --j )
		{
			addObjectCodeTableMember ( 'M' , g_Extref_table [ j ] -> m_cpLabel , iSection ) ;
			g_ObjectCode_table [ g_iObjectCode_count ] -> m_iAddr = g_Extref_table [ j ] -> m_iAddr ;
			g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte = g_Extref_table [ j ] -> m_iByte ;
			g_ObjectCode_table [ g_iObjectCode_count ] -> m_cSign = g_Extref_table [ j ] -> m_cSign ;
					
			if ( g_Extref_table [ j ] -> m_bHalf_byte )								// If true, then half byte
				-- g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte ;
		}
		for ( j = 0 ; j < g_iExtref_count ; --j )
		{
			free ( g_Extref_table [ j ] -> m_cpLabel ) ;
			free ( g_Extref_table [ j ] ) ;
		}

		g_iExtref_count = 0 ;
	}

	addObjectCodeTableMember ( 'E' , NULL , iSection ) ;

	
	return 0 ;
}

/*
 * Print object code to file
 * If parameter is NULL(don't save to file), then print to console
 * If network connection is needed, make another function instead of this
 * Return 0 if success, - if fail
 */
int iPrintObjectCode ( char * cpFile_name )
{
	int iByte = 0 ;
	char crgPrint [ 100 ] ;
	FILE * fpOut ;
	int i = 0 ;
	int iSection = -1 ;



	if ( NULL == fopen ( cpFile_name , "w+" ) )
	{
		printf ( "iPrintObjectCode : Can't open the file\n" ) ;

		return -1 ;
	}

	for ( ; i < g_iObjectCode_count ; ++i )
	{
		if ( ( '\0' != crgPrint [ 0 ] ) && ( crgPrint [ 0 ] != g_ObjectCode_table [ i ] -> m_cRecord ) )		// If former record remain at crgPrint, then print
		{
			if ( ' ' == crgPrint [ strlen ( crgPrint ) - 1 ] )
				crgPrint [ strlen ( crgPrint ) - 1 ] = '\0 ' ;

			fprintf ( fpOut , "%s\n" , crgPrint ) ;
			crgPrint [ 0 ] = '\0' ;
		}


		if ( 'H' == g_ObjectCode_table [ i ] -> m_cRecord )				// Head record
		{
			sprintf ( crgPrint , "H%s %06X %06X" , g_ObjectCode_table [ i ] -> m_cpSymbol , 0 , g_irgProgramLengthforEach [ ++ iSection ] ) ;

			fprintf ( fpOut , "%s\n" , crgPrint ) ;

			crgPrint [ 0 ] = '\0' ;
		}
		else if ( 'D' == g_ObjectCode_table [ i ] -> m_cRecord )		// Extdef record
		{
			if ( '\0' == crgPrint [ 0 ] )
				sprintf ( crgPrint , "D" ) ;


			sprintf ( crgPrint , "%s %06X " , g_ObjectCode_table [ i ] -> m_cpSymbol , g_ObjectCode_table [ i ] -> m_iAddr ) ;
		}
		else if ( 'R' == g_ObjectCode_table [ i ] -> m_cRecord )		// Extref record
		{
			if ( '\0' == crgPrint [ 0 ] )
				sprintf ( crgPrint , "R" ) ;
			

			sprintf ( crgPrint , "%s " , g_ObjectCode_table [ i ] -> m_cpSymbol ) ;
		}
		else if ( 'T' == g_ObjectCode_table [ i ] -> m_cRecord )		// Text record
		{
			if ( 32 <= iByte + g_ObjectCode_table [ i ] -> m_iByte )
			{
				crgPrint [ strlen ( crgPrint ) - 1 ] = '\0 ' ;

				fprintf ( fpOut , "%s\n" , crgPrint ) ;

				crgPrint [ 0 ] = '\0' ;
				iByte = 0 ;
			}
			if ( '\0' == crgPrint [ 0 ] )
				sprintf ( crgPrint , "T %06X %02X " , g_ObjectCode_table [ i ] -> m_iAddr , 0 ) ;
			

			if ( 1 == g_ObjectCode_table [ i ] -> m_iByte )
				sprintf ( crgPrint , "%02X " , g_ObjectCode_table [ i ] -> m_iCode ) ;
			else if ( 2 == g_ObjectCode_table [ i ] -> m_iByte )
				sprintf ( crgPrint , "%04X " , g_ObjectCode_table [ i ] -> m_iCode ) ;
			else if ( 3 == g_ObjectCode_table [ i ] -> m_iByte )
				sprintf ( crgPrint , "%06X " , g_ObjectCode_table [ i ] -> m_iCode ) ;
			else if ( 4 == g_ObjectCode_table [ i ] -> m_iByte )
				sprintf ( crgPrint , "%08X " , g_ObjectCode_table [ i ] -> m_iCode ) ;

			iByte += g_ObjectCode_table [ i ] -> m_iByte ;

			crgPrint [ 9 ] = cHexToChar ( iByte / 16 ) ;
			crgPrint [ 10 ] = cHexToChar ( iByte % 16 ) ;


			fprintf ( fpOut , "%s\n" , crgPrint ) ;
		}
		else if ( 'M' == g_ObjectCode_table [ i ] -> m_cRecord )		// Modification record
		{
			sprintf ( crgPrint , "M %06X %02X %c%s" , g_ObjectCode_table [ i ] -> m_iAddr , g_ObjectCode_table [ i ] -> m_iByte ,
				( 1 == g_ObjectCode_table [ i ] -> m_cSign ) ? '+' : '-' , g_ObjectCode_table [ i ] -> m_cpSymbol ) ;

			fprintf ( fpOut , "%s\n" , crgPrint ) ;
		}
		else if ( 'E' == g_ObjectCode_table [ i ] -> m_cRecord )		// End record
		{
			sprintf ( crgPrint , "E" ) ;

			if ( 0 == iSection )
			{
				sprintf ( crgPrint , " %06X" , 0 ) ;
			}

			fprintf ( fpOut , "%s\n" , crgPrint ) ;
		}
	}



	return 0 ;
}











/*
 * Check whether cpStr is operation or not
 * Return opcode, or - if fail
 */
int iSearch_opcode ( char * cpStr )
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

/*
 * Free allocated memory
 * If this program need too many memories, then split this function and use early
 */
void clearMemory ()								// Free all the memory
{
	int i = 0 ;
	int j = 0 ;


	for ( i = 0 ; ( i < MAX_INST ) && ( NULL != g_pInst_table [ i ] ) ; ++i )
	{
		free ( g_pInst_table [ i ] -> m_cpOperation ) ;
		free ( g_pInst_table [ i ] ) ;
	}

	for ( int i = 0 ; i < g_iLine_count ; ++i )
	{
		free ( g_cpInput_data [ i ] ) ;
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

	// Already free Extref table at iConvertToObjectCode

	for ( int i = 0 ; i < g_iObjectCode_count ; ++i )
	{
		free ( g_ObjectCode_table [ i ] -> m_cRecord ) ;
		free ( g_ObjectCode_table [ i ] -> m_cpSymbol ) ;
		free ( g_ObjectCode_table [ i ] ) ;
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
 * Need to use section number to compute symbol displacement
 * If iSection is -1, check all symbol
 * Return address of symbol, or - if not found
 */
int iGetSymLocation ( char * cpStr , int iSection )
{
	int i = 0 ;
	


	if ( -1 == iSection )
	{
		for ( ; i < g_iSymbol_count ; ++i )
		{
			if ( 0 == strcmp ( cpStr , g_Symbol_table [ i ] -> m_cpSymbol ) )
			{
				return g_Symbol_table [ i ] -> m_iAddr ;
			}
		}
	}
	else
	{
		for ( ; i < g_iSymbol_count ; ++i )
		{
			if ( iSection == g_Symbol_table [ i ] -> m_iSection )
			{
				break ;
			}
		}

		for ( ; ( i < g_iSymbol_count ) && ( iSection == g_Symbol_table [ i ] -> m_iSection ) ; ++i )
		{
			if ( 0 == strcmp ( cpStr , g_Symbol_table [ i ] -> m_cpSymbol ) )
			{
				return g_Symbol_table [ i ] -> m_iAddr ;
			}
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

/*
 * Find register number and return
 * Return number of register, or - if not found
 */
int iGetRegisterNum ( char * cpStr )
{
	char cdrgReg [ 9 ] [ 3 ] = { "A" , "X" , "L" , "B" , "S" , "T" , "F" , "PC" , "SW" } ;	// Register name to check



	for ( int i = 0 ; i < 9 ; ++i )
	{
		if ( 0 == strcmp ( cdrgReg [ i ] , cpStr ) )
		{
			if ( 7 <= i )
				return i + 1 ;

			return i ;
		}
	}

	return -1 ;
}

/*
 * Add one member to object code table
 */
void addObjectCodeTableMember ( char cRecord , char * cpSymbol , int iSection )
{
	g_ObjectCode_table [ g_iObjectCode_count ] = malloc ( sizeof ( object_code ) ) ;
	g_ObjectCode_table [ g_iObjectCode_count ] -> m_cRecord = cRecord ;
	g_ObjectCode_table [ g_iObjectCode_count ] -> m_iCode = 0 ;
	g_ObjectCode_table [ g_iObjectCode_count ] -> m_iByte = 0 ;
	g_ObjectCode_table [ g_iObjectCode_count ] -> m_cpSymbol = NULL ;
	g_ObjectCode_table [ g_iObjectCode_count ] -> m_cSign = 0 ;
	g_ObjectCode_table [ g_iObjectCode_count ] -> m_iSection = iSection ;
	g_ObjectCode_table [ g_iObjectCode_count ] -> m_iAddr = 0 ;



	if ( NULL != cpSymbol )
	{
		g_ObjectCode_table [ g_iObjectCode_count ] -> m_cpSymbol = malloc ( sizeof ( char ) * ( strlen ( cpSymbol ) + 1 ) ) ;
		strcpy ( g_ObjectCode_table [ g_iObjectCode_count ] -> m_cpSymbol , cpSymbol ) ;
	}

	++ g_iObjectCode_count ;
}

/*
 * Add one member to extref table
 */
void addExtrefTableMember ( char * cpLabel , int iAddr , int iByte , bool bIsHalf , char cSign )
{
	g_Extref_table [ g_iExtref_count ] = malloc ( sizeof ( extref ) ) ;
	g_Extref_table [ g_iExtref_count ] -> m_cpLabel = malloc ( strlen ( cpLabel ) + 1 ) ;
	strcpy ( g_Extref_table [ g_iExtref_count ] -> m_cpLabel , cpLabel ) ;
	g_Extref_table [ g_iExtref_count ] -> m_iAddr = iAddr ;
	g_Extref_table [ g_iExtref_count ] -> m_iByte = iByte ;
	g_Extref_table [ g_iExtref_count ] -> m_bHalf_byte = bIsHalf ;
	g_Extref_table [ g_iExtref_count ++ ] -> m_cSign = cSign ;
}

/*
 * Get object code of information
 * If cNixbpe == -1, then BYTE or WORD
 * Return object code of information, or - if wrong information
 */
int iGetInstObjectCode ( int iByte , int iOpcode , char cNixbpe , int iDisplacement )
{
	int iReturn = 0 ;

	
	
	if ( -1 == cNixbpe )
	{
		return iDisplacement ;
	}

	if ( 1 == iByte )
	{
		return iOpcode ;
	}
	if ( 2 == iByte )
	{
		return ( iOpcode << 8 ) + iDisplacement ;
	}
	if ( 3 == iByte )
	{
		return ( iOpcode << 16 ) + ( cNixbpe << 12 ) + ( iDisplacement & 0xFFF ) ;		// If displacement is -, then cut bit
	}
	if ( 4 == iByte )
	{
		return ( iOpcode << 24 ) + ( cNixbpe << 20 ) + ( iDisplacement & 0xFFFFF ) ;	// If displacement is -, then cut bit
	}

	return -1 ;
}
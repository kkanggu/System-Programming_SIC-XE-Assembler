package SP22_simulator;

// instruction에 따라 동작을 수행하는 메소드를 정의하는 클래스

import java.io.*;
import java.math.BigInteger;
import java.util.StringTokenizer;

class Instruction
{
    public String m_strInstruction ;
    public int m_iOpcode ;
    public int m_iFormat ;
    public int m_iOperand ;
}

public class InstLauncher
{
    ResourceManager rMgr;
    Instruction instList [] = new Instruction [ 59 ] ;
    boolean bIsImmediate ;          // Some instruction uses disp without memory reference even ni = 11.
                                    // For these instructions, use this variable to check immediate

    public InstLauncher ( ResourceManager resourceManager)
    {
        this.rMgr = resourceManager;

        BufferedReader br = null;
        try
        {
            br = new BufferedReader ( new FileReader ( "./inst.data" ) );

            StringTokenizer st ;

            for ( int i = 0 ; i < 59 ; ++i )
            {
                try
                {
                    st = new StringTokenizer ( br.readLine () ) ;
                    instList [ i ] = new Instruction () ;
                    instList [ i ].m_strInstruction = st.nextToken () ;
                    instList [ i ].m_iOpcode = Integer.parseInt ( st.nextToken () , 16 ) ;
                    instList [ i ].m_iFormat = Integer.parseInt ( st.nextToken () ) ;
                    instList [ i ].m_iOperand = Integer.parseInt ( st.nextToken () ) ;
                }
                catch ( IOException e )
                {
                    e.printStackTrace ();
                }

            }
        }
        catch ( FileNotFoundException e )
        {
            e.printStackTrace ();
        }
    }

    /**
     * Return class with Opcode
     *
     * @param iOpcode Opcode
     * @return class or null
     */
    public Instruction getOpcode ( int iOpcode )
    {
        for ( int i = 0 ; i < 59 ; ++i )
        {
            if ( iOpcode == instList [ i ].m_iOpcode )
                return instList [ i ] ;
        }


        return null ;
    }

    public void doInstruction ( int iObjectCode , int iFormat , int iOpcode )
    {
        int disp = 0 ;
        int nixbpe = ( iObjectCode & 0x03F000 ) >> 12 ;
        bIsImmediate = false ;

        rMgr.setRegister ( 8 , rMgr.getRegister ( 8 ) + iFormat ) ;

        if ( 1 == iFormat )             // Format 1, no disp
        {
            ;
        }
        else if ( 2 == iFormat )
        {
            disp = iObjectCode & 0xFF ;
        }
        else if ( 3 == iFormat )
        {
            disp = iObjectCode & 0xFFF ;

            if ( 0 != ( disp & 0x800 ) )
            {
                disp |= 0xFFFFF000 ;
            }
        }
        else if ( 4 == iFormat )
        {
            disp = iObjectCode & 0xFFFFF ;
            nixbpe = ( iObjectCode & 0x03F00000 ) >> 20 ;

            if ( 0 != ( disp & 0x80000 ) )
            {
                disp |= 0xFFF00000 ;
            }
        }

        if ( ( 3 <= iFormat ) && ( 0 != nixbpe ) )
        {
            if ( 0x02 == ( nixbpe & 0x02 ) )        // PC relative
            {
                disp += rMgr.getRegister ( 8 ) ;
            }

            if ( 0x20 == ( nixbpe & 0x30 ) )        // Indirect, Memory reference 2 times
            {
                disp = Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) ) ;
                // Mem ref 2
            }
            else if ( 0x10 == ( nixbpe & 0x30 ) )   // Immediate, no memory reference
            {
                bIsImmediate = true ;
            }

            if ( 0x08 == ( nixbpe & 0x08 ) )
            {
                disp += rMgr.getRegister ( 1 ) ;
            }
        }

        rMgr.iTargetAddress = disp ;


        // From here, call each instruction
        if ( 0x18 == iOpcode )              // ADD
        {
            instADD ( disp ) ;
        }
        else if ( 0x58 == iOpcode )         // ADDF
        {
            instADDF ( disp ) ;
        }
        else if ( 0x90 == iOpcode )         // ADDR
        {
            instADDR ( disp ) ;
        }
        else if ( 0x40 == iOpcode )         // AND
        {
            instAND ( disp ) ;
        }
        else if ( 0xB4 == iOpcode )         // CLEAR
        {
            instCLEAR ( disp ) ;
        }
        else if ( 0x28 == iOpcode )         // COMP
        {
            instCOMP ( disp ) ;
        }
        else if ( 0x88 == iOpcode )         // COMPF
        {
            instCOMPF ( disp ) ;
        }
        else if ( 0xA0 == iOpcode )         // COMPR
        {
            instCOMPR ( disp ) ;
        }
        else if ( 0x24 == iOpcode )         // DIV
        {
            instDIV ( disp ) ;
        }
        else if ( 0x64 == iOpcode )         // DIVF
        {
            instDIVF ( disp ) ;
        }
        else if ( 0x9C == iOpcode )         // DIVR
        {
            instDIVR ( disp ) ;
        }
        else if ( 0xC4 == iOpcode )         // FIX
        {
            instFIX () ;
        }
        else if ( 0xC0 == iOpcode )         // FLOAT
        {
            instFLOAT () ;
        }
        else if ( 0xF4 == iOpcode )         // HIO
        {
            instHIO () ;
        }
        else if ( 0x3C == iOpcode )         // J
        {
            instJ ( disp ) ;
        }
        else if ( 0x30 == iOpcode )         // JEQ
        {
            instJEQ ( disp ) ;
        }
        else if ( 0x34 == iOpcode )         // JGT
        {
            instJGT ( disp ) ;
        }
        else if ( 0x38 == iOpcode )         // JLT
        {
            instJLT ( disp ) ;
        }
        else if ( 0x48 == iOpcode )         // JSUB
        {
            instJSUB ( disp ) ;
        }
        else if ( 0x00 == iOpcode )         // LDA
        {
            instLDA ( disp ) ;
        }
        else if ( 0x68 == iOpcode )         // LDB
        {
            instLDB ( disp ) ;
        }
        else if ( 0x50 == iOpcode )         // LDCH
        {
            instLDCH ( disp ) ;
        }
        else if ( 0x70 == iOpcode )         // LDF
        {
            instLDF ( disp ) ;
        }
        else if ( 0x08 == iOpcode )         // LDL
        {
            instLDL ( disp ) ;
        }
        else if ( 0x6C == iOpcode )         // LDS
        {
            instLDS ( disp ) ;
        }
        else if ( 0x74 == iOpcode )         // LDT
        {
            instLDT ( disp ) ;
        }
        else if ( 0x04 == iOpcode )         // LDX
        {
            instLDX ( disp ) ;
        }
        else if ( 0xD0 == iOpcode )         // LPS
        {
            instLPS ( disp ) ;
        }
        else if ( 0x20 == iOpcode )         // MUL
        {
            instMUL ( disp ) ;
        }
        else if ( 0x60 == iOpcode )         // MULF
        {
            instMULF ( disp ) ;
        }
        else if ( 0x98 == iOpcode )         // MULR
        {
            instMULR ( disp ) ;
        }
        else if ( 0xC8 == iOpcode )         // NORM
        {
            instNORM () ;
        }
        else if ( 0x44 == iOpcode )         // OR
        {
            instOR ( disp ) ;
        }
        else if ( 0xD8 == iOpcode )         // RD
        {
            instRD ( disp ) ;
        }
        else if ( 0xAC == iOpcode )         // RMO
        {
            instRMO ( disp ) ;
        }
        else if ( 0x4C == iOpcode )         // RSUB
        {
            instRSUB () ;
        }
        else if ( 0xA4 == iOpcode )         // SHIFTL
        {
            instSHIFTL ( disp ) ;
        }
        else if ( 0xA8 == iOpcode )         // SHIFTR
        {
            instSHIFTR ( disp ) ;
        }
        else if ( 0xF0 == iOpcode )         // SIO
        {
            instSIO () ;
        }
        else if ( 0xEC == iOpcode )         // SSK
        {
            instSSK ( disp ) ;
        }
        else if ( 0x0C == iOpcode )         // STA
        {
            instSTA ( disp ) ;
        }
        else if ( 0x78 == iOpcode )         // STB
        {
            instSTB ( disp ) ;
        }
        else if ( 0x54 == iOpcode )         // STCH
        {
            instSTCH ( disp ) ;
        }
        else if ( 0x80 == iOpcode )         // STF
        {
            instSTF ( disp ) ;
        }
        else if ( 0xD4 == iOpcode )         // STI
        {
            instSTI ( disp ) ;
        }
        else if ( 0x14 == iOpcode )         // STL
        {
            instSTL ( disp ) ;
        }
        else if ( 0x7C == iOpcode )         // STS
        {
            instSTS ( disp ) ;
        }
        else if ( 0xE8 == iOpcode )         // STSW
        {
            instSTSW ( disp ) ;
        }
        else if ( 0x84 == iOpcode )         // STT
        {
            instSTT ( disp ) ;
        }
        else if ( 0x10 == iOpcode )         // STX
        {
            instSTX ( disp ) ;
        }
        else if ( 0x1C == iOpcode )         // SUB
        {
            instSUB ( disp ) ;
        }
        else if ( 0x5C == iOpcode )         // SUBF
        {
            instSUBF ( disp ) ;
        }
        else if ( 0x94 == iOpcode )         // SUBR
        {
            instSUBR ( disp ) ;
        }
        else if ( 0xB0 == iOpcode )         // SVC
        {
            instSVC ( disp ) ;
        }
        else if ( 0xE0 == iOpcode )         // TD
        {
            instTD ( disp ) ;
        }
        else if ( 0xF8 == iOpcode )         // TIO
        {
            instTIO () ;
        }
        else if ( 0x2C == iOpcode )         // TIX
        {
            instTIX ( disp ) ;
        }
        else if ( 0xB8 == iOpcode )         // TIXR
        {
            instTIXR ( disp ) ;
        }
        else if ( 0xDC == iOpcode )         // WD
        {
            instWD ( disp ) ;
        }
    }

    /**
     * Change string to double
     *
     * @param strDouble Hex String
     */
    public double getDouble ( String strDouble )
    {
        String strBinary = new BigInteger ( strDouble , 16 ).toString ( 2 ) ;

        return Double.longBitsToDouble ( new BigInteger ( strBinary , 2 ).longValue () ) ;
    }

    /**
     * ADD instruction, no memory reference
     *
     * @param disp value
     */
    public void instADD ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        rMgr.setRegister ( 0 , rMgr.getRegister ( 0 ) + iValue ) ;
    }

    /**
     * ADDF instruction, no memory reference
     *
     * @param disp value
     */
    public void instADDF ( int disp )
    {
        double dValue = bIsImmediate ? disp : this.getDouble ( String.valueOf ( rMgr.getMemory ( disp * 2 , 12 ) ) ) ;

        rMgr.register_F += dValue ;
    }

    /**
     * ADDR instruction, no memory reference
     *
     * @param disp value
     */
    public void instADDR ( int disp )
    {
        int r1 = ( disp & 0x00F0 ) >> 4 ;
        int r2 = disp & 0x000F ;

        rMgr.setRegister ( r2 , rMgr.getRegister ( r1 ) + rMgr.getRegister ( r2 ) );
    }

    /**
     * AND instruction, no memory reference
     *
     * @param disp value
     */
    public void instAND ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        rMgr.setRegister ( 0 , rMgr.getRegister ( 0 ) & iValue ) ;
    }

    /**
     * CLEAR instruction, no memory reference
     *
     * @param disp value
     */
    public void instCLEAR ( int disp )
    {
        int r1 = ( disp & 0x00F0 ) >> 4 ;

        rMgr.setRegister ( r1 , 0 ) ;
    }

    /**
     * COMP instruction, no memory reference
     *
     * @param disp value
     */
    public void instCOMP ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        int CC = 0 ;                                                // If same, SW = 0
        if ( rMgr.getRegister ( 0 ) > iValue )              // If origin > operand, SW = 1
            CC = 1 ;
        else if ( rMgr.getRegister ( 0 ) < iValue )         // If origin < operand, SW = -1
            CC = -1 ;

        rMgr.setRegister ( 9 , CC ) ;
    }

    /**
     * COMPF instruction, no memory reference
     *
     * @param disp value
     */
    public void instCOMPF ( int disp )
    {
        double dValue = bIsImmediate ? disp : this.getDouble ( String.valueOf ( rMgr.getMemory ( disp * 2 , 12 ) ) ) ;

        int CC = 0 ;                               // If same, SW = 0
        if ( rMgr.register_F > dValue )            // If origin > operand, SW = 1
            CC = 1 ;
        else if ( rMgr.register_F < dValue )       // If origin < operand, SW = -1
            CC = -1 ;

        rMgr.setRegister ( 9 , CC ) ;
    }

    /**
     * COMPR instruction, no memory reference
     *
     * @param disp value
     */
    public void instCOMPR ( int disp )
    {
        int r1 = ( disp & 0x00F0 ) >> 4 ;
        int r2 = disp & 0x000F ;

        int CC = 0 ;                                            // If same, SW = 0
        if ( rMgr.getRegister ( r1 ) > rMgr.getRegister ( r2 ) )            // If origin > operand, SW = 1
            CC = 1 ;
        else if ( rMgr.getRegister ( r1 ) < rMgr.getRegister ( r2 ) )       // If origin < operand, SW = -1
            CC = -1 ;

        rMgr.setRegister ( 9 , CC ) ;
    }

    /**
     * DIV instruction, no memory reference
     *
     * @param disp value
     */
    public void instDIV ( int disp )
    {
        // If 10 / 3, result is 3


        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        rMgr.setRegister ( 0 , rMgr.getRegister ( 0 ) / iValue );
    }

    /**
     * DIVF instruction, no memory reference
     *
     * @param disp value
     */
    public void instDIVF ( int disp )
    {
        // If 10.0 / 3.0, result is 3.333333...


        double dValue = bIsImmediate ? disp : this.getDouble ( String.valueOf ( rMgr.getMemory ( disp * 2 , 12 ) ) ) ;

        rMgr.register_F /= dValue ;
    }

    /**
     * DIVF instruction, no memory reference
     *
     * @param disp value
     */
    public void instDIVR ( int disp )
    {
        int r1 = ( disp & 0x00F0 ) >> 4 ;
        int r2 = disp & 0x000F ;

        rMgr.setRegister ( r2 , rMgr.getRegister ( r2 ) / rMgr.getRegister ( r1 ) );
    }

    /**
     * FIX instruction, no memory reference
     *
     */
    public void instFIX ()
    {
        rMgr.setRegister ( 0 , ( ( int ) rMgr.register_F ) );
    }

    /**
     * FLOAT instruction, no memory reference
     *
     */
    public void instFLOAT ()
    {
        rMgr.register_F = ( ( double ) rMgr.getRegister ( 0 ) ) ;
    }

    /**
     * HIO instruction, no memory reference
     *
     */
    public void instHIO ()
    {
        // Later
    }

    /**
     * J instruction, no memory reference
     *
     * @param disp value
     */
    public void instJ ( int disp )
    {
        rMgr.setRegister ( 8 , disp );
    }

    /**
     * JEQ instruction, no memory reference
     *
     * @param disp value
     */
    public void instJEQ ( int disp )
    {
        if ( 0 == rMgr.getRegister ( 9 ) )      // If CC set =, jump
        {
            rMgr.setRegister ( 8 , disp );
        }
    }

    /**
     * JGT instruction, no memory reference
     *
     * @param disp value
     */
    public void instJGT ( int disp )
    {
        if ( 1 == rMgr.getRegister ( 9 ) )      // If CC set >, jump
        {
            rMgr.setRegister ( 8 , disp );
        }
    }

    /**
     * JLT instruction, no memory reference
     *
     * @param disp value
     */
    public void instJLT ( int disp )
    {
        if ( -1 == rMgr.getRegister ( 9 ) )      // If CC set <, jump
        {
            rMgr.setRegister ( 8 , disp );
        }
    }

    /**
     * JSUB instruction, no memory reference
     *
     * @param disp value
     */
    public void instJSUB ( int disp )
    {
        rMgr.setRegister ( 2 , rMgr.getRegister ( 8 ) );
        rMgr.setRegister ( 8 , disp );

        for ( int i = 0 ; i < rMgr.progInfoList.size () ; ++i )
        {
            if ( disp == rMgr.progInfoList.elementAt ( i ).m_iStartAddress )
            {
                rMgr.iSection = i ;

                break ;
            }
        }
    }

    /**
     * LDA instruction, no memory reference
     *
     * @param disp value
     */
    public void instLDA ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        rMgr.setRegister ( 0 , iValue );
    }

    /**
     * LDB instruction, no memory reference
     *
     * @param disp value
     */
    public void instLDB ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        rMgr.setRegister ( 3 , iValue );
    }

    /**
     * LDCH instruction, no memory reference
     *
     * @param disp value
     */
    public void instLDCH ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 2 ) ) , 16 ) ;

        rMgr.setRegister ( 0 , iValue );
    }

    /**
     * LDF instruction, no memory reference
     *
     * @param disp value
     */
    public void instLDF ( int disp )
    {
        double dValue = bIsImmediate ? disp : this.getDouble ( String.valueOf ( rMgr.getMemory ( disp * 2 , 12 ) ) ) ;

        rMgr.register_F = dValue ;
    }

    /**
     * LDL instruction, no memory reference
     *
     * @param disp value
     */
    public void instLDL ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        rMgr.setRegister ( 2 , iValue );
    }

    /**
     * LDS instruction, no memory reference
     *
     * @param disp value
     */
    public void instLDS ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        rMgr.setRegister ( 4 , iValue );
    }

    /**
     * LDT instruction, no memory reference
     *
     * @param disp value
     */
    public void instLDT ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        rMgr.setRegister ( 5 , iValue );
    }

    /**
     * LDX instruction, no memory reference
     *
     * @param disp value
     */
    public void instLDX ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        rMgr.setRegister ( 1 , iValue );
    }

    /**
     * LPS instruction, no memory reference
     *
     * @param disp value
     */
    public void instLPS ( int disp )
    {
        // Later
    }

    /**
     * MUL instruction, no memory reference
     *
     * @param disp value
     */
    public void instMUL ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        rMgr.setRegister ( 0 , rMgr.getRegister ( 0 ) * iValue );
    }

    /**
     * MULF instruction, no memory reference
     *
     * @param disp value
     */
    public void instMULF ( int disp )
    {
        double dValue = bIsImmediate ? disp : this.getDouble ( String.valueOf ( rMgr.getMemory ( disp * 2 , 12 ) ) ) ;

        rMgr.register_F *= dValue ;
    }

    /**
     * MULR instruction, no memory reference
     *
     * @param disp value
     */
    public void instMULR ( int disp )
    {
        int r1 = ( disp & 0x00F0 ) >> 4 ;
        int r2 = disp & 0x00F0 ;

        rMgr.setRegister ( r2 , rMgr.getRegister ( r1 ) * rMgr.getRegister ( r2 ) );
    }

    /**
     * NORM instruction, no memory reference
     *
     */
    public void instNORM ()
    {
        // Floating point normalization
        // Later
    }

    /**
     * OR instruction, no memory reference
     *
     * @param disp value
     */
    public void instOR ( int disp )
    {
        int iValue = Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        rMgr.setRegister ( 0 , rMgr.getRegister ( 0 ) | iValue );
    }
    /**
     * RD instruction, no memory reference
     *
     * @param disp value
     */
    public void instRD ( int disp )
    {
        String strDevice = String.valueOf ( rMgr.getMemory ( disp * 2 , 2 ) ) ;

        char [] crgRead = rMgr.readDevice ( strDevice , 1 ) ;

        rMgr.setRegister ( 0 , ( rMgr.getRegister ( 0 ) & 0xFFFFFF00 ) | crgRead [ 0 ] ) ;
    }

    /**
     * RMO instruction, no memory reference
     *
     * @param disp value
     */
    public void instRMO ( int disp )
    {
        int r1 = ( disp & 0x00F0 ) >> 4 ;
        int r2 = disp & 0x000F ;

        rMgr.setRegister ( r2 , rMgr.getRegister ( r1 ) ) ;
    }

    /**
     * RSUB instruction, no memory reference
     *
     */
    public void instRSUB ()
    {
        int originRoutineAddress = rMgr.getRegister ( 2 ) ;
        rMgr.setRegister ( 8 , originRoutineAddress );

        for ( int i = 0 ; i < rMgr.progInfoList.size () ; ++i )
        {
            if ( originRoutineAddress >= rMgr.progInfoList.elementAt ( i ).m_iStartAddress )
            {
                rMgr.iSection = i ;

                break ;
            }
        }
    }

    /**
     * SHIFTL instruction, no memory reference
     *
     * @param disp value
     */
    public void instSHIFTL ( int disp )
    {
        int r1 = ( disp & 0x00F0 ) >> 4 ;
        int n = disp & 0x000F ;
        int iValue = rMgr.getRegister ( r1 ) << n ;

        rMgr.setRegister ( r1 , iValue );
    }

    /**
     * SHIFTR instruction, no memory reference
     *
     * @param disp value
     */
    public void instSHIFTR ( int disp )
    {
        int r1 = ( disp & 0x00F0 ) >> 4 ;
        int n = disp & 0x000F ;
        int iValue = rMgr.getRegister ( r1 ) >> n ;

        rMgr.setRegister ( r1 , iValue );
    }

    /**
     * SIO instruction, no memory reference
     *
     */
    public void instSIO ()
    {
        // Later
    }

    /**
     * SSK instruction, no memory reference
     *
     * @param disp value
     */
    public void instSSK ( int disp )
    {
        // Later
    }

    /**
     * STA instruction, no memory reference
     *
     * @param disp value
     */
    public void instSTA ( int disp )
    {
        int iValue = rMgr.getRegister ( 0 ) ;

        rMgr.setMemory ( disp * 2 , String.valueOf ( rMgr.intToChar ( iValue , 6 ) ) , 6 ) ;
    }

    /**
     * STB instruction, no memory reference
     *
     * @param disp value
     */
    public void instSTB ( int disp )
    {
        int iValue = rMgr.getRegister ( 3 ) ;

        rMgr.setMemory ( disp * 2 , String.valueOf ( rMgr.intToChar ( iValue , 6 ) ) , 6 ) ;
    }

    /**
     * STCH instruction, no memory reference
     *
     * @param disp value
     */
    public void instSTCH ( int disp )
    {
        int iValue = rMgr.getRegister ( 0 ) & 0xFF ;

        rMgr.setMemory ( disp * 2 , String.valueOf ( rMgr.intToChar ( iValue , 2 ) ) , 2 ) ;
    }

    /**
     * STF instruction, no memory reference
     *
     * @param disp value
     */
    public void instSTF ( int disp )
    {
        double dValue = rMgr.register_F ;

        rMgr.setMemory ( disp * 2 , Double.toHexString ( dValue ) , 12 ) ;
    }

    /**
     * STI instruction, no memory reference
     *
     * @param disp value
     */
    public void instSTI ( int disp )
    {
        // Later
    }

    /**
     * STL instruction, no memory reference
     *
     * @param disp value
     */
    public void instSTL ( int disp )
    {
        int iValue = rMgr.getRegister ( 2 ) ;

        rMgr.setMemory ( disp * 2 , String.valueOf ( rMgr.intToChar ( iValue , 6 ) ) , 6 ) ;
    }

    /**
     * STS instruction, no memory reference
     *
     * @param disp value
     */
    public void instSTS ( int disp )
    {
        int iValue = rMgr.getRegister ( 4 ) ;

        rMgr.setMemory ( disp * 2 , String.valueOf ( rMgr.intToChar ( iValue , 6 ) ) , 6 ) ;
    }

    /**
     * STSW instruction, no memory reference
     *
     * @param disp value
     */
    public void instSTSW ( int disp )
    {
        int iValue = rMgr.getRegister ( 9 ) ;

        rMgr.setMemory ( disp * 2 , String.valueOf ( rMgr.intToChar ( iValue , 6 ) ) , 6 ) ;
    }

    /**
     * STT instruction, no memory reference
     *
     * @param disp value
     */
    public void instSTT ( int disp )
    {
        int iValue = rMgr.getRegister ( 5 ) ;

        rMgr.setMemory ( disp * 2 , String.valueOf ( rMgr.intToChar ( iValue , 6 ) ) , 6 ) ;
    }

    /**
     * STX instruction, no memory reference
     *
     * @param disp value
     */
    public void instSTX ( int disp )
    {
        int iValue = rMgr.getRegister ( 1 ) ;

        rMgr.setMemory ( disp * 2 , String.valueOf ( rMgr.intToChar ( iValue , 6 ) ) , 6 ) ;
    }

    /**
     * SUB instruction, no memory reference
     *
     * @param disp value
     */
    public void instSUB ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6 ) ) , 16 ) ;

        rMgr.setRegister ( 0 , rMgr.getRegister ( 0 ) - iValue );
    }

    /**
     * SUBF instruction, no memory reference
     *
     * @param disp value
     */
    public void instSUBF ( int disp )
    {
        double dValue = bIsImmediate ? disp : this.getDouble ( String.valueOf ( rMgr.getMemory ( disp * 2 , 12 ) ) ) ;

        rMgr.register_F -= dValue ;
    }

    /**
     * SUBR instruction, no memory reference
     *
     * @param disp value
     */
    public void instSUBR ( int disp )
    {
        int r1 = ( disp & 0x00F0 ) >> 4 ;
        int r2 = disp & 0x000F ;

        rMgr.setRegister ( r2 , rMgr.getRegister ( r2 ) - rMgr.getRegister ( r1 ) );
    }

    /**
     * SVC instruction, no memory reference
     *
     * @param disp value
     */
    public void instSVC ( int disp )
    {
        // Later
    }

    /**
     * TD instruction, no memory reference
     *
     * @param disp value
     */
    public void instTD ( int disp )
    {
        rMgr.testDevice ( String.valueOf ( rMgr.getMemory ( disp * 2 , 2 ) ) ) ;
    }

    /**
     * TIO instruction, no memory reference
     *
     */
    public void instTIO ()
    {
        // Later
    }

    /**
     * TIX instruction, no memory reference
     *
     * @param disp value
     */
    public void instTIX ( int disp )
    {
        int iValue = bIsImmediate ? disp : Integer.parseInt ( String.valueOf ( rMgr.getMemory ( disp * 2 , 6  ) ) , 16 ) ;

        rMgr.setRegister ( 1 , rMgr.getRegister ( 1 ) + 1 ) ;
        int CC = 0 ;                                                // If same, SW = 0
        if ( rMgr.getRegister ( 1 ) > iValue )              // If origin > operand, SW = 1
            CC = 1 ;
        else if ( rMgr.getRegister ( 1 ) < iValue )         // If origin < operand, SW = -1
            CC = -1 ;

        rMgr.setRegister ( 9 , CC ) ;
    }

    /**
     * TIXR instruction, no memory reference
     *
     * @param disp value
     */
    public void instTIXR ( int disp )
    {
        int r1 = ( disp & 0x00F0 ) >> 4 ;
        int iValue = rMgr.getRegister ( r1 );

        rMgr.setRegister ( 1 , rMgr.getRegister ( 1 ) + 1 ) ;
        int CC = 0 ;                                                // If same, SW = 0
        if ( rMgr.getRegister ( 1 ) > iValue )              // If origin > operand, SW = 1
            CC = 1 ;
        else if ( rMgr.getRegister ( 1 ) < iValue )         // If origin < operand, SW = -1
            CC = -1 ;

        rMgr.setRegister ( 9 , CC ) ;
    }

    /**
     * WD instruction, no memory reference
     *
     * @param disp value
     */
    public void instWD ( int disp )
    {
        String strDevice = String.valueOf ( rMgr.getMemory ( disp * 2 , 2 ) ) ;
        char [] data = new char [ 1 ] ;
        data [ 0 ] = ( char ) ( rMgr.getRegister ( 0 ) & 0xFF ) ;

        rMgr.writeDevice ( strDevice , data , 2 ) ;
    }
}
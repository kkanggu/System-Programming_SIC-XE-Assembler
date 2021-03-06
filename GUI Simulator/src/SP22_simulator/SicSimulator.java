package SP22_simulator;

import java.io.File;
import java.util.Vector;

/**
 * 시뮬레이터로서의 작업을 담당한다. VisualSimulator에서 사용자의 요청을 받으면 이에 따라 ResourceManager에 접근하여
 * 작업을 수행한다.
 * 
 * 작성중의 유의사항 : 1) 새로운 클래스, 새로운 변수, 새로운 함수 선언은 얼마든지 허용됨. 단, 기존의 변수와 함수들을 삭제하거나
 * 완전히 대체하는 것은 지양할 것. 2) 필요에 따라 예외처리, 인터페이스 또는 상속 사용 또한 허용됨. 3) 모든 void 타입의 리턴값은
 * 유저의 필요에 따라 다른 리턴 타입으로 변경 가능. 4) 파일, 또는 콘솔창에 한글을 출력시키지 말 것. (채점상의 이유. 주석에 포함된
 * 한글은 상관 없음)
 * 
 * 
 * 
 * + 제공하는 프로그램 구조의 개선방법을 제안하고 싶은 분들은 보고서의 결론 뒷부분에 첨부 바랍니다. 내용에 따라 가산점이 있을 수
 * 있습니다.
 */
public class SicSimulator {
	ResourceManager rMgr;
	InstLauncher instLauncher;
	Instruction currentInst ;
	Vector < String > instLog ;
	Vector < String > hexLog ;
	int iCurrentPC ;

	public SicSimulator(ResourceManager resourceManager)
	{
		this.rMgr = resourceManager;
		this.instLauncher = new InstLauncher ( resourceManager ) ;
		instLog = new Vector <> () ;
		hexLog = new Vector <> () ;
	}

	/**
	 * 레지스터, 메모리 초기화 등 프로그램 load와 관련된 작업 수행. 단, object code의 메모리 적재 및 해석은
	 * SicLoader에서 수행하도록 한다.
	 */
	public void load(File program)
	{
		iCurrentPC = rMgr.iProgStartAddress ;
	}

	/**
	 * 1개의 instruction이 수행된 모습을 보인다.
	 */
	public void oneStep()
	{
		int iOpcode = rMgr.charToInt ( rMgr.getMemory ( iCurrentPC * 2 , 2 ) ) & 0xFC ;
		currentInst = instLauncher.getOpcode ( iOpcode ) ;
		int iTempFormat = currentInst.m_iFormat ;				// If I used user defined class, CALL BY REFERENCE. So I wanna keep original data, I clone the variable of class
		int iMemory = 0 ;



		rMgr.strCurrentDevice = "" ;

		if ( null != currentInst )					// Instruction
		{
			if ( iTempFormat > 4 )					// Format 3 or 4, check extended
			{
				iMemory = rMgr.charToInt ( rMgr.getMemory ( iCurrentPC * 2 , 8  ) ) ;

				if ( 0 != ( iMemory & 0x00100000 ) )		// If extended
				{
					iTempFormat = 4 ;
				}
				else										// Not extended, Format 3
				{
					iTempFormat = 3 ;
				}

				iMemory >>= ( ( 4 - iTempFormat ) * 8 ) ;
			}
			else
			{
				iMemory = rMgr.charToInt ( rMgr.getMemory ( iCurrentPC * 2 , iTempFormat * 2 ) ) ;
			}

			instLauncher.doInstruction ( iMemory , iTempFormat , iOpcode ) ;

			addLog ( currentInst.m_strInstruction , String.valueOf ( rMgr.intToChar ( iMemory , iTempFormat * 2 ) ) ) ;
		}

	}

	/**
	 * 남은 모든 instruction이 수행된 모습을 보인다.
	 */
	public void allStep ()
	{
		while ( ( 0 == instLog.size () ) || ( ( iCurrentPC != rMgr.iProgStartAddress + rMgr.iProgLength ) && ( rMgr.iProgStartAddress != iCurrentPC ) ) )
		{
			this.oneStep ();
			iCurrentPC = rMgr.getRegister ( 8 ) ;
		}
	}

	/**
	 * 각 단계를 수행할 때 마다 관련된 기록을 남기도록 한다.
	 * CHANGED. Add new parameter String strHex
	 */
	public void addLog ( String log , String strHex )
	{
		instLog.add ( log ) ;
		hexLog.add ( strHex ) ;
	}
}

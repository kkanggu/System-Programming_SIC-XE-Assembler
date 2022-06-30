package SP22_simulator;

import java.io.*;



/**
 * SicLoader는 프로그램을 해석해서 메모리에 올리는 역할을 수행한다. 이 과정에서 linker의 역할 또한 수행한다.
 * 
 * SicLoader가 수행하는 일을 예를 들면 다음과 같다. - program code를 메모리에 적재시키기 - 주어진 공간만큼 메모리에 빈
 * 공간 할당하기 - 과정에서 발생하는 symbol, 프로그램 시작주소, control section 등 실행을 위한 정보 생성 및 관리
 */
public class SicLoader {
	ResourceManager rMgr;
	static int iSection = 0 ;


	public SicLoader(ResourceManager resourceManager) {
		setResourceManager(resourceManager);
		resourceManager.initializeResource () ;
	}

	/**
	 * Loader와 프로그램을 적재할 메모리를 연결시킨다.
	 * 
	 * @param resourceManager
	 */
	public void setResourceManager(ResourceManager resourceManager) {
		this.rMgr = resourceManager;
	}

	/**
	 * object code를 읽어서 load과정을 수행한다. load한 데이터는 resourceManager가 관리하는 메모리에 올라가도록
	 * 한다. load과정에서 만들어진 symbol table 등 자료구조 역시 resourceManager에 전달한다.
	 * 
	 * @param objectCode 읽어들인 파일
	 */
	public void load(File objectCode)
	{
		BufferedReader br = null;
		String strLine ;
		String strProgName = "" ;
		int iProgStartAddress = 0 ;
		int iProgLength = 0 ;

		try
		{
			br = new BufferedReader ( new FileReader ( objectCode ) , 50 );
		}
		catch ( FileNotFoundException e )
		{
			e.printStackTrace ();
		}



		try
		{
			while ( null != ( strLine = br.readLine () ) )				// Read until EOF
			{
				if ( 0 == strLine.length () )							// Skip empty line
					continue ;

				if ( 'H' == strLine.charAt ( 0 ) )						// Header record
				{
					strProgName = strLine.substring ( 1 , strLine.length () - 13 ) ;
					iProgStartAddress = rMgr.iProgStartAddress + rMgr.iProgLength + rMgr.charToInt ( strLine.substring ( strLine.length () - 12 , strLine.length () - 6 ).toCharArray () ) ;
					iProgLength = rMgr.charToInt ( strLine.substring ( strLine.length () - 6 ).toCharArray () ) ;

					for ( int i = 0 ; i < rMgr.refList.size () ; ++i )
					{
						if ( strProgName.equals ( rMgr.refList.elementAt ( i ).m_strVariable ) )		// If routine label is used as reference
						{
							rMgr.defList.add ( new DefSymbol ( strProgName , iProgStartAddress ) ) ;

							break ;
						}
					}

					rMgr.progInfoList.add ( new ProgInfo ( strProgName , iProgStartAddress , iProgLength ) ) ;
					rMgr.iProgLength += iProgLength ;
				}
				else if ( 'D' == strLine.charAt ( 0 ) )					// Define record. Add new DefineSymbol node
				{
					int iStringIndex = 1 ;
					int iAddressIndex ;

					for ( int i = 2 ; i < strLine.length () ; ++i )
					{
						if ( '0' == strLine.charAt ( i ) )				// If start with 0, then address
						{
							iAddressIndex = i ;

							rMgr.defList.add ( new DefSymbol ( strLine.substring ( iStringIndex , iAddressIndex ) ,
									iProgStartAddress + rMgr.charToInt ( strLine.substring ( iAddressIndex , iAddressIndex + 6 ).toCharArray () ) ) ) ;

							i += 6 ;
							iStringIndex = i ;
						}
					}
				}
				else if ( 'R' == strLine.charAt ( 0 ) )						// If section end, ++ section
				{
					// No process
				}
				else if ( 'T' == strLine.charAt ( 0 ) )						// If section end, ++ section
				{
					int iAddress = iProgStartAddress + rMgr.charToInt ( strLine.substring ( 1 , 7 ).toCharArray () ) ;
					int iLength = rMgr.charToInt ( strLine.substring ( 7 , 9 ).toCharArray () ) ;

					rMgr.setMemory ( iAddress * 2 , strLine.substring ( 9 , strLine.length () ) , iLength * 2 );
				}
				else if ( 'M' == strLine.charAt ( 0 ) )						// If section end, ++ section
				{
					int iAddress = iProgStartAddress + rMgr.charToInt ( strLine.substring ( 1 , 7 ).toCharArray () ) ;
					int iLength = rMgr.charToInt ( strLine.substring ( 7 , 9 ).toCharArray () ) ;
					int iSign = '+' == strLine.charAt ( 9 ) ? 1 : -1 ;

					rMgr.refList.add ( new RefSymbol ( strLine.substring ( 10 , strLine.length () ) , iAddress , iLength , iSign ) ) ;
				}
				else if ( 'E' == strLine.charAt ( 0 ) )						// If section end, ++ section
				{
					++ iSection;
					++ rMgr.iSection ;
				}
			}
		}
		catch ( IOException e )
		{
			e.printStackTrace () ;
		}


		rMgr.processEXTVariable () ;		// Process about EXTDEF, EXTREF
	};
}

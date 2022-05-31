package SP22_simulator;

import javax.naming.ldap.StartTlsRequest;
import java.io.*;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Vector;

class DefSymbol            // For define record
{
	String m_strVariable ;
	int m_iAddress ;

	public DefSymbol ( String strVariable , int iAddress )
	{
		this.m_strVariable = strVariable ;
		this.m_iAddress = iAddress ;
	}
}

class RefSymbol			// For reference record
{
	String m_strVariable ;
	int m_iAddress ;
	int m_iLength ;
	int m_iSign ;

	public RefSymbol ( String strVariable , int iAddress , int iLength , int iSign )
	{
		this.m_strVariable = strVariable ;
		this.m_iAddress = iAddress ;
		this.m_iLength = iLength ;
		this.m_iSign = iSign ;
	}
}

class ProgInfo
{
	String m_strName ;
	int m_iStartAddress ;
	int m_iLength ;

	public ProgInfo ( String strName , int iStartAddress , int iLength )
	{
		this.m_strName = strName ;
		this.m_iStartAddress = iStartAddress ;
		this.m_iLength = iLength ;
	}
}

/**
 * ResourceManager는 컴퓨터의 가상 리소스들을 선언하고 관리하는 클래스이다. 크게 네가지의 가상 자원 공간을 선언하고, 이를
 * 관리할 수 있는 함수들을 제공한다.
 *
 *
 * 1) 입출력을 위한 외부 장치 또는 device 2) 프로그램 로드 및 실행을 위한 메모리 공간. 여기서는 64KB를 최대값으로 잡는다.
 * 3) 연산을 수행하는데 사용하는 레지스터 공간. 4) SYMTAB 등 simulator의 실행 과정에서 사용되는 데이터들을 위한 변수들.
 *
 * 2번은 simulator위에서 실행되는 프로그램을 위한 메모리공간인 반면, 4번은 simulator의 실행을 위한 메모리 공간이라는 점에서
 * 차이가 있다.
 */
public class ResourceManager {
	/**
	 * 디바이스는 원래 입출력 장치들을 의미 하지만 여기서는 파일로 디바이스를 대체한다. 즉, 'F1'이라는 디바이스는 'F1'이라는 이름의
	 * 파일을 의미한다. deviceManager는 디바이스의 이름을 입력받았을 때 해당 이름의 파일 입출력 관리 클래스를 리턴하는 역할을 한다.
	 * 예를 들어, 'A1'이라는 디바이스에서 파일을 read모드로 열었을 경우, hashMap에 <"A1", scanner(A1)> 등을
	 * 넣음으로서 이를 관리할 수 있다.
	 *
	 * 변형된 형태로 사용하는 것 역시 허용한다. 예를 들면 key값으로 String대신 Integer를 사용할 수 있다. 파일 입출력을 위해
	 * 사용하는 stream 역시 자유로이 선택, 구현한다.
	 *
	 * 이것도 복잡하면 알아서 구현해서 사용해도 괜찮습니다.
	 */
	HashMap< String , FileWriter > fwDeviceManager = new HashMap < String , FileWriter > () ;				// CHANGED. Separate into 2 containers.
	HashMap< String , FileReader > frDeviceManager = new HashMap < String , FileReader > () ;
	char[] memory = new char[65536]; // String으로 수정해서 사용하여도 무방함.
	int[] register = new int[10];
	double register_F;
	int iSection = 0 ;
	int iProgStartAddress = 0 ;
	int iProgLength = 0 ;
	Vector < DefSymbol > defList = new Vector<> () ;
	Vector < RefSymbol > refList = new Vector<> () ;
	Vector < ProgInfo > progInfoList = new Vector<> () ;
	int iTargetAddress = 0 ;
	String strCurrentDevice ;

	/**
	 * 메모리, 레지스터등 가상 리소스들을 초기화한다.
	 */
	public void initializeResource() {
		Arrays.fill ( memory , ( char ) 0 ) ;
		Arrays.fill ( register , 0 ) ;
		register_F = 0 ;
		iProgStartAddress = 0 ;
		iProgLength = 0 ;
	}

	/**
	 * deviceManager가 관리하고 있는 파일 입출력 stream들을 전부 종료시키는 역할. 프로그램을 종료하거나 연결을 끊을 때
	 * 호출한다.
	 */
	public void closeDevice()
	{
		for ( FileWriter fwTemp : fwDeviceManager.values () )
		{
			try
			{
				fwTemp.close () ;
			}
			catch ( IOException e )
			{
				e.printStackTrace ();
			}
		}
	}

	/**
	 * 디바이스를 사용할 수 있는 상황인지 체크. TD명령어를 사용했을 때 호출되는 함수. 입출력 stream을 열고 deviceManager를
	 * 통해 관리시킨다.
	 *
	 * @param devName 확인하고자 하는 디바이스의 번호,또는 이름
	 */
	public void testDevice(String devName)
	{
		strCurrentDevice = devName ;

		if ( ( this.fwDeviceManager.containsKey ( devName ) ) || ( this.frDeviceManager.containsKey ( devName ) ) )
		{
			this.setRegister ( 9 , 1 ) ;
		}
		else
		{
			try
			{
				this.fwDeviceManager.put ( devName , new FileWriter ( devName , true ) ) ;
				this.frDeviceManager.put ( devName , new FileReader ( devName ) ) ;

				this.setRegister ( 9 , 0 ) ;
			}
			catch ( IOException e )
			{
				e.printStackTrace ();
			}
		}
	}

	/**
	 * 디바이스로부터 원하는 개수만큼의 글자를 읽어들인다. RD명령어를 사용했을 때 호출되는 함수.
	 *
	 * @param devName 디바이스의 이름
	 * @param num     가져오는 글자의 개수
	 * @return 가져온 데이터
	 */
	public char[] readDevice(String devName, int num)
	{
		try
		{
			FileReader fr = this.frDeviceManager.get ( devName ) ;

			if ( this.fwDeviceManager.containsKey ( devName ) )
			{
				this.frDeviceManager.remove ( devName ) ;
				this.fwDeviceManager.remove ( devName ) ;

				fr = new FileReader ( devName ) ;
				this.frDeviceManager.put ( devName , fr ) ;
			}

			char [] target = new char [ num ] ;

			fr.read ( target ) ;

			return target ;
		}
		catch ( FileNotFoundException e )
		{
			e.printStackTrace ();
		}
		catch ( IOException e )
		{
			e.printStackTrace ();
		}


		return null;
	}

	/**
	 * 디바이스로 원하는 개수 만큼의 글자를 출력한다. WD명령어를 사용했을 때 호출되는 함수.
	 *
	 * @param devName 디바이스의 이름
	 * @param data    보내는 데이터
	 * @param num     보내는 글자의 개수
	 */
	public void writeDevice(String devName, char[] data, int num)
	{
		try
		{
			FileWriter fw = this.fwDeviceManager.get ( devName ) ;

			if ( this.frDeviceManager.containsKey ( devName ) )
			{
				this.frDeviceManager.remove ( devName ) ;
				this.fwDeviceManager.remove ( devName ) ;

				fw = new FileWriter ( devName ) ;
				this.fwDeviceManager.put ( devName , fw ) ;
			}

			fw.write ( data ) ;
		}
		catch ( FileNotFoundException e )
		{
			e.printStackTrace ();
		}
		catch ( IOException e )
		{
			e.printStackTrace ();
		}
	}

	/**
	 * 메모리의 특정 위치에서 원하는 개수만큼의 글자를 가져온다.
	 *
	 * @param location 메모리 접근 위치 인덱스
	 * @param num      데이터 개수
	 * @return 가져오는 데이터
	 */
	public char[] getMemory(int location, int num) {
		StringBuffer sb = new StringBuffer () ;

		for ( int i = 0 ; i < num ; ++i )
		{
			sb.append ( memory [ location + i ] ) ;
		}

		return sb.toString ().toCharArray () ;
	}

	/**
	 * 메모리의 특정 위치에 원하는 개수만큼의 데이터를 저장한다.
	 * CHANGED. char [] data -> String data
	 * @param locate 접근 위치 인덱스
	 * @param data   저장하려는 데이터
	 * @param num    저장하는 데이터의 개수
	 */
	public void setMemory(int locate, String data, int num)
	{
		for ( int i = 0 ; i < num ; ++i )
		{
			memory [ locate + i ] = data.charAt ( i ) ;
		}
	}

	/**
	 * 번호에 해당하는 레지스터가 현재 들고 있는 값을 리턴한다. 레지스터가 들고 있는 값은 문자열이 아님에 주의한다.
	 *
	 * @param regNum 레지스터 분류번호
	 * @return 레지스터가 소지한 값
	 */
	public int getRegister(int regNum) {
		return register [ regNum ] ;

	}

	/**
	 * 번호에 해당하는 레지스터에 새로운 값을 입력한다. 레지스터가 들고 있는 값은 문자열이 아님에 주의한다.
	 *
	 * @param regNum 레지스터의 분류번호
	 * @param value  레지스터에 집어넣는 값
	 */
	public void setRegister(int regNum, int value) {
		register [ regNum ] = value ;
	}

	/**
	 * 주로 레지스터와 메모리간의 데이터 교환에서 사용된다. int값을 char[]형태로 변경한다.
	 * CHANGED. Add new parameter int iLength, to use for length 1~7
	 *
	 * @param data
	 * @param iLength 리턴받을 문자열 길이
	 * @return data를 iLength 길이의 char []
	 */
	public char[] intToChar ( int data , int iLength )
	{
		char [] crgData = new char [ iLength ] ;



		for ( int i = iLength - 1 ; i >= 0 ; --i )
		{
			int iTemp = data & 0xF ;
			data >>= 4 ;

			crgData [ i ] = ( char ) ( iTemp + '0' ) ;

			if ( iTemp > 9 )		// For 'A' ~ 'F'
			{
				crgData [ i ] += 7 ;
			}
		}

		return crgData ;
	}

	/**
	 * 주로 레지스터와 메모리간의 데이터 교환에서 사용된다. char[]값을 int형태로 변경한다.
	 * CHANGED. byte [] data -> char [] data, byteToInt -> charToInt
	 *
	 * @param data
	 * @return 16진수로 된 data의 10진수 값
	 */
	public int charToInt ( char [] data )
	{
		int iReturn = 0 ;
		char cTemp = 0 ;

		for ( int i = 0 ; i < data.length ; ++i )
		{
			iReturn <<= 4 ;

			cTemp = data [ i ] ;

			iReturn += cTemp - '0' ;

			if ( ( 'A' <= cTemp ) && ( cTemp <= 'F' ) )
			{
				iReturn -= 7 ;
			}
		}

		return iReturn ;
	}

	/**
	 * 저장한 EXTDEF를 이용하여 EXTREF를 바꾼다.
	 */
	public void processEXTVariable ()
	{
		for ( int i = 0 ; i < defList.size () ; )
		{
			DefSymbol tempDEF = defList.get ( i ) ;


			for ( int j = 0 ; j < refList.size () ; ++j )
			{
				RefSymbol tempREF = refList.get ( j ) ;

				if ( tempDEF.m_strVariable.equals ( tempREF.m_strVariable ) )
				{
					int iMemory = tempREF.m_iSign * tempDEF.m_iAddress;
					iMemory &= ( ( 1 << ( 4 * tempREF.m_iLength ) ) - 1 ) ;		// If length n, 0xFF...F, number of F is n

					int iOrigin = this.charToInt ( this.getMemory ( tempREF.m_iAddress * 2 + ( tempREF.m_iLength & 1 ) , tempREF.m_iLength ) ) ;

					this.setMemory ( tempREF.m_iAddress * 2 + ( tempREF.m_iLength & 1 ) , String.valueOf ( intToChar ( iMemory + iOrigin , tempREF.m_iLength ) ) , tempREF.m_iLength );

					refList.remove ( j-- ) ;
				}
			}

			defList.remove ( 0 ) ;
		}
	}
}
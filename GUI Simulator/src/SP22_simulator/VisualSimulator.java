package SP22_simulator;

import javax.swing.*;
import javax.swing.border.LineBorder;
import javax.swing.border.TitledBorder;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.IOException;

/**
 * VisualSimulator는 사용자와의 상호작용을 담당한다. 즉, 버튼 클릭등의 이벤트를 전달하고 그에 따른 결과값을 화면에 업데이트
 * 하는 역할을 수행한다.
 * 
 * 실제적인 작업은 SicSimulator에서 수행하도록 구현한다.
 */
public class VisualSimulator
{
	ResourceManager rMgr = new ResourceManager();
	SicLoader sicLoader = new SicLoader( rMgr );
	SicSimulator sicSimulator = new SicSimulator( rMgr );

	JFrame frame = new JFrame () ;									// Main Frame
	JPanel pnUp = new JPanel () ;
	JPanel pnHeader = new JPanel () ;
	JPanel pnRegister = new JPanel () ;
	JPanel pnRight = new JPanel () ;
	JPanel pnEnd = new JPanel () ;
	JPanel pnDown = new JPanel () ;
	JLabel labelFileName = new JLabel ( "FileName" ) ;					// Upper Pane, File name and open
	JTextField textFileName = new JTextField ( 10 ) ;
	JButton buttonOpen = new JButton ( "Open" ) ;
	JLabel labelProgName = new JLabel ( "Program Name" ) ;
	JTextField textProgName = new JTextField ( 8 ) ;
	JLabel labelHeaderStartAddr = new JLabel ( "<HTML>Start Address of<br>Object Program</HTML>" ) ;
	JTextField textHeaderStartAddr = new JTextField ( 8 ) ;
	JLabel labelProgLength = new JLabel ( "Program Length" ) ;
	JTextField textProgLength = new JTextField ( 8 ) ;
	JLabel labelDec = new JLabel ( "Dec" ) ;
	JLabel labelHex = new JLabel ( "Hex" ) ;
	String strRegister [] = { "A (#0)" , "X (#1)" , "L (#2)" , "B (#3)" , "S (#4)" , "T (#5)" , "F (#6)" , "EMPTY" , "PC (#8)" , "SW (#9)" } ;
	JLabel labelRegister [] = new JLabel [ 10 ] ;
	JTextField textRegister [] [] = new JTextField [ 10 ] [ 2 ] ;
	JLabel labelInstruction = new JLabel ( "<HTML>Address of<br>First Instruction" ) ;
	JTextField textInstruction = new JTextField ( 8 ) ;
	JLabel labelEndStartAddr = new JLabel ( "<HTML>Start Address<br>in Memory</HTML>" ) ;
	JTextField textEndStartAddr = new JTextField ( 8 ) ;
	JLabel labelTargetAddress = new JLabel ( "Target Address" ) ;
	JTextField textTargetAddress = new JTextField ( 8 ) ;
	JLabel labelListInstruction = new JLabel ( "Instructions" ) ;
	JTextArea areaHexInstruction = new JTextArea () ;
	JScrollPane scrollPane ;
	JLabel labelDevice = new JLabel ( "Using Device" ) ;
	JTextField textDevice = new JTextField ( 8 ) ;
	JButton buttonExcute1Step = new JButton ( "<HTML>Execute<br>(1 Step)</HTML>" ) ;
	JButton buttonExcuteAll = new JButton ( "<HTML>Execute<br>(ALL)</HTML>" ) ;
	JButton buttonExit = new JButton ( "Exit" ) ;
	JLabel labelLog = new JLabel ( "Log (Instruction)" ) ;
	JTextArea areaStringInstruction = new JTextArea () ;
	JScrollPane scrollPaneLog ;
	int iLineCnt ;

	public VisualSimulator () throws IOException
	{
		pnUp.setLayout ( new FlowLayout ( FlowLayout.LEFT ) );
		labelFileName.setSize ( 80 , 100 );
		textFileName.setEditable ( false ) ;
		textFileName.setBackground ( Color.WHITE );
		textFileName.setForeground ( Color.BLACK );
		textFileName.setBorder ( new LineBorder ( Color.BLACK , 1 ) ) ;
		textFileName.setHorizontalAlignment ( JTextField.CENTER ) ;
		buttonOpen.addActionListener ( new ActionListener () {
			@Override
			public void actionPerformed ( ActionEvent e )
			{
				FileDialog fd = new FileDialog ( frame , "File Explorer" ) ;
				fd.setFile ( "*.obj" );
				fd.setDirectory ( VisualSimulator.class.getProtectionDomain ().getCodeSource ().getLocation ().getPath () );
				fd.setSize ( 400 , 200  );
				fd.setVisible ( true ) ;

				String strFileName = fd.getFile () ;
				String strFileDirectory = fd.getDirectory () ;

				try
				{
					rMgr.initializeResource ();

					load ( new File ( strFileDirectory , strFileName ) );

					textFileName.setText ( strFileName );
					rMgr.iSection = 0 ;

					update ();
				}
				catch ( IOException ex )
				{
					ex.printStackTrace ();
				}
			}
		} );

		pnUp.add ( labelFileName ) ;
		pnUp.add ( textFileName ) ;
		pnUp.add ( buttonOpen ) ;
		pnUp.setSize ( 400 , 100 );



		pnHeader.setLayout ( null );
		pnHeader.setBorder ( new TitledBorder ( new LineBorder ( Color.darkGray , 1 ) , "Header Record" ) ) ;
		pnHeader.setPreferredSize ( new Dimension ( 270 , 135 ) ) ;
		labelProgName.setBounds ( 20 , 20 , 110 , 20 ) ;
		textProgName.setBounds ( 140 , 22 , 110 , 20 ) ;
		textProgName.setEditable ( false ) ;
		textProgName.setBackground ( Color.WHITE );
		textProgName.setForeground ( Color.BLACK );
		textProgName.setBorder ( new LineBorder ( Color.BLACK , 1 ) ) ;labelHeaderStartAddr.setBounds ( 20 , 49 , 110 , 40 ) ;
		textProgName.setHorizontalAlignment ( JTextField.CENTER ) ;
		textHeaderStartAddr.setBounds ( 140 , 60 , 110 , 20 ) ;
		textHeaderStartAddr.setEditable ( false ) ;
		textHeaderStartAddr.setBackground ( Color.WHITE );
		textHeaderStartAddr.setForeground ( Color.BLACK );
		textHeaderStartAddr.setBorder ( new LineBorder ( Color.BLACK , 1 ) ) ;
		textHeaderStartAddr.setHorizontalAlignment ( JTextField.CENTER ) ;
		labelProgLength.setBounds ( 20 , 65 , 110 , 20 ) ;
		labelProgLength.setSize ( 100 , 80 );
		textProgLength.setBounds ( 140 , 97 , 110 , 20 ) ;
		textProgLength.setEditable ( false ) ;
		textProgLength.setBackground ( Color.WHITE );
		textProgLength.setForeground ( Color.BLACK );
		textProgLength.setBorder ( new LineBorder ( Color.BLACK , 1 ) ) ;
		textProgLength.setHorizontalAlignment ( JTextField.CENTER ) ;

		pnHeader.add ( labelProgName ) ;
		pnHeader.add ( textProgName ) ;
		pnHeader.add ( labelHeaderStartAddr ) ;
		pnHeader.add ( textHeaderStartAddr ) ;
		pnHeader.add ( labelProgLength ) ;
		pnHeader.add ( textProgLength ) ;



		pnRegister.setLayout ( null );
		pnRegister.setBorder ( new TitledBorder ( new LineBorder ( Color.darkGray , 1 ) , "Register" ) ) ;
		pnRegister.setPreferredSize ( new Dimension ( 270 , 300 ) ) ;
		labelDec.setBounds ( 70 , 20 , 80 , 30 ) ;
		labelHex.setBounds ( 170 , 20 , 80 , 30 ) ;
		pnRegister.add ( labelDec ) ;
		pnRegister.add ( labelHex ) ;



		for ( int i = 0 ; i < 10 ; ++i )
		{
			if ( 7 == i )
				++i ;

			labelRegister [ i ] = new JLabel ( strRegister [ i ] ) ;
			labelRegister [ i ].setBounds ( 10 , 55 + 25 * ( i - i / 7 ) , 80 , 20 ) ;
			textRegister [ i ] [ 0 ] = new JTextField ( 4 ) ;
			textRegister [ i ] [ 1 ] = new JTextField ( 4 ) ;
			textRegister [ i ] [ 0 ].setSize ( 50 , 20 );
			textRegister [ i ] [ 1 ].setSize ( 50 , 20 );
			textRegister [ i ] [ 0 ].setBounds ( 70 , 55 + 25 * ( i - i / 7 ) , 80 , 20 ) ;
			textRegister [ i ] [ 1 ].setBounds ( 170 , 55 + 25 * ( i - i / 7 ) , 80 , 20 ) ;
			textRegister [ i ] [ 0 ].setEditable ( false ) ;
			textRegister [ i ] [ 1 ].setEditable ( false ) ;

			textRegister [ i ] [ 0 ].setBackground ( Color.WHITE );
			textRegister [ i ] [ 0 ].setForeground ( Color.BLACK );
			textRegister [ i ] [ 0 ].setBorder ( new LineBorder ( Color.BLACK , 1 ) ) ;
			textRegister [ i ] [ 0 ].setHorizontalAlignment ( JTextField.RIGHT ) ;
			textRegister [ i ] [ 1 ].setBackground ( Color.WHITE );
			textRegister [ i ] [ 1 ].setForeground ( Color.BLACK );
			textRegister [ i ] [ 1 ].setBorder ( new LineBorder ( Color.BLACK , 1 ) ) ;
			textRegister [ i ] [ 1 ].setHorizontalAlignment ( JTextField.RIGHT ) ;

			pnRegister.add ( labelRegister [ i ] ) ;
			pnRegister.add ( textRegister [ i ] [ 0 ] ) ;
			pnRegister.add ( textRegister [ i ] [ 1 ] ) ;
		}



		pnEnd.setLayout ( null );
		pnEnd.setBorder ( new TitledBorder ( new LineBorder ( Color.darkGray , 1 ) , "End Record" ) ) ;
		pnEnd.setPreferredSize ( new Dimension ( 270 , 60 ) ) ;
		labelInstruction.setBounds ( 20 , 20 , 100 , 30 ) ;
		textInstruction.setBounds ( 135 , 25 , 110 , 20 ) ;
		textInstruction.setEditable ( false ) ;
		textInstruction.setBackground ( Color.WHITE );
		textInstruction.setForeground ( Color.BLACK );
		textInstruction.setBorder ( new LineBorder ( Color.BLACK , 1 ) ) ;
		textInstruction.setHorizontalAlignment ( JTextField.CENTER ) ;

		pnEnd.add ( labelInstruction ) ;
		pnEnd.add ( textInstruction ) ;



		pnRight.setLayout ( null );
		pnRight.setPreferredSize ( new Dimension ( 270 , 600 ) ) ;

		labelEndStartAddr.setBounds ( 20 , 15 , 100 , 30 );
		textEndStartAddr.setBounds ( 135 , 20 , 110 , 20 );
		textEndStartAddr.setEditable ( false ) ;
		textEndStartAddr.setBackground ( Color.WHITE );
		textEndStartAddr.setForeground ( Color.BLACK );
		textEndStartAddr.setBorder ( new LineBorder ( Color.BLACK , 1 ) ) ;
		textEndStartAddr.setHorizontalAlignment ( JTextField.CENTER ) ;
		labelTargetAddress.setBounds ( 20 , 50 , 100 , 30 );
		textTargetAddress.setBounds ( 135 , 57 , 110 , 20  );
		textTargetAddress.setEditable ( false ) ;
		textTargetAddress.setBackground ( Color.WHITE );
		textTargetAddress.setForeground ( Color.BLACK );
		textTargetAddress.setBorder ( new LineBorder ( Color.BLACK , 1 ) ) ;
		textTargetAddress.setHorizontalAlignment ( JTextField.CENTER ) ;

		pnRight.add ( labelEndStartAddr ) ;
		pnRight.add ( textEndStartAddr ) ;
		pnRight.add ( labelTargetAddress ) ;
		pnRight.add ( textTargetAddress ) ;

		labelListInstruction.setBounds ( 20 , 100 , 100 , 15 );

		areaHexInstruction.setEditable ( false ) ;
		scrollPane = new JScrollPane ( areaHexInstruction ) ;
		scrollPane.setBounds ( 20 , 120 , 140 , 259 );

		pnRight.add ( labelListInstruction ) ;
		pnRight.add ( scrollPane , "Center" ) ;

		labelDevice.setBounds ( 180 , 120 , 80 , 30 );
		textDevice.setBounds ( 180 , 150 , 80 , 20  );
		textDevice.setEditable ( false ) ;
		textDevice.setBackground ( Color.WHITE );
		textDevice.setForeground ( Color.BLACK );
		textDevice.setBorder ( new LineBorder ( Color.BLACK , 1 ) ) ;
		textDevice.setHorizontalAlignment ( JTextField.CENTER ) ;

		pnRight.add ( labelDevice ) ;
		pnRight.add ( textDevice ) ;

		buttonExcute1Step.setBounds ( 180 , 240 , 80 , 40 );
		buttonExcute1Step.addActionListener ( new ActionListener () {
			@Override
			public void actionPerformed ( ActionEvent e )
			{
				oneStep () ;
 			}
		} );
		buttonExcuteAll.setBounds ( 180 , 290 , 80 , 40 );
		buttonExcuteAll.addActionListener ( new ActionListener () {
			@Override
			public void actionPerformed ( ActionEvent e )
			{
				allStep () ;
			}
		} );
		buttonExit.setBounds ( 180 , 340 , 80 , 30 );
		buttonExit.addActionListener ( new ActionListener () {
			@Override
			public void actionPerformed ( ActionEvent e )
			{
				System.exit ( 0 ) ;
			}
		} );

		pnRight.add ( buttonExcute1Step ) ;
		pnRight.add ( buttonExcuteAll ) ;
		pnRight.add ( buttonExit ) ;



		pnDown.setLayout ( null ) ;
		pnDown.setPreferredSize ( new Dimension ( 600 , 300 ) );

		labelLog.setBounds ( 0 , 20 , 200 , 30 ) ;

		areaStringInstruction.setEditable ( false ) ;
		scrollPaneLog = new JScrollPane ( areaStringInstruction ) ;
		scrollPaneLog.setBounds ( 0 , 50 , 551 , 200 );

		pnDown.add ( labelLog ) ;
		pnDown.add ( scrollPaneLog ) ;



		frame.setLayout ( null ) ;

		frame.setTitle ( "SIC/XE Simulator" );
		frame.setSize ( 600 , 800 );
		frame.setBackground ( Color.lightGray );
		frame.setVisible ( true );
		frame.setResizable ( false ) ;

		pnUp.setBounds ( 10 , 5 , 400 , 40 );
		frame.add ( pnUp ) ;
		pnHeader.setBounds ( 10 , 50 , 270 , 130 );
		frame.add ( pnHeader ) ;
		pnRegister.setBounds ( 10 , 190 , 270 , 290 );
		frame.add ( pnRegister ) ;
		pnEnd.setBounds ( 300 , 50 , 270 , 60 );
		frame.add ( pnEnd ) ;
		pnRight.setBounds ( 300 , 100 , 270 , 400 );
		frame.add ( pnRight ) ;
		pnDown.setBounds ( 10 , 470 , 600 , 300 );
		frame.add ( pnDown ) ;
	}

	/**
	 * 프로그램 로드 명령을 전달한다.
	 */
	public void load(File program) throws IOException
	{
		sicLoader.load(program);
		sicSimulator.load(program);
		iLineCnt = 0 ;
	}

	/**
	 * 하나의 명령어만 수행할 것을 SicSimulator에 요청한다.
	 */
	public void oneStep()
	{
		if ( ( 0 == sicSimulator.instLog.size () ) ||
				( ( sicSimulator.iCurrentPC != rMgr.iProgStartAddress + rMgr.iProgLength ) && ( rMgr.iProgStartAddress != sicSimulator.iCurrentPC ) ) )
		{
			sicSimulator.oneStep ();

			this.update ();

			sicSimulator.iCurrentPC = rMgr.getRegister ( 8 ) ;
		}
		else
		{
			rMgr.closeDevice () ;
			System.exit ( 0 ) ;
		}
	}

	/**
	 * 남아있는 모든 명령어를 수행할 것을 SicSimulator에 요청한다.
	 */
	public void allStep()
	{
		if ( ( 0 == sicSimulator.instLog.size () ) ||
				( ( sicSimulator.iCurrentPC != rMgr.iProgStartAddress + rMgr.iProgLength ) && ( rMgr.iProgStartAddress != sicSimulator.iCurrentPC ) ) )
		{
			sicSimulator.allStep () ;
		}
		else
		{
			rMgr.closeDevice () ;
			System.exit ( 0 ) ;
		}

		this.update ();
	}

	/**
	 * 화면을 최신값으로 갱신하는 역할을 수행한다.
	 */
	public void update()
	{
		textProgName.setText ( rMgr.progInfoList.elementAt ( rMgr.iSection ).m_strName ) ;
		textHeaderStartAddr.setText ( String.valueOf ( rMgr.intToChar ( rMgr.progInfoList.elementAt ( rMgr.iSection ).m_iStartAddress , 6 ) ) ) ;
		textProgLength.setText ( String.valueOf ( rMgr.intToChar ( rMgr.progInfoList.elementAt ( rMgr.iSection ).m_iLength , 6 ) ) ) ;
		textInstruction.setText ( String.valueOf ( rMgr.intToChar ( rMgr.iProgStartAddress , 6 ) ) ) ;
		textEndStartAddr.setText ( String.valueOf ( rMgr.intToChar ( sicSimulator.iCurrentPC , 6 ) ) ) ;
		textTargetAddress.setText ( String.valueOf ( rMgr.intToChar ( rMgr.iTargetAddress , 6 ) ) ) ;
		textDevice.setText ( rMgr.strCurrentDevice ) ;

		for ( int i = 0 ; i < 10 ; ++i )
		{
			if ( 7 == i )
				++i ;

			int Dec = rMgr.getRegister ( i ) ;
			if ( 0 != ( 0x00800000 & Dec ) )
			{
				Dec |= 0xFF000000 ;
			}
			textRegister [ i ] [ 0 ].setText ( Integer.toString ( Dec ) + " " ) ;
			textRegister [ i ] [ 1 ].setText ( String.valueOf ( rMgr.intToChar ( rMgr.getRegister ( i ) , 6 ) ) + " " ) ;
		}

		for ( ; iLineCnt < sicSimulator.instLog.size () ; ++ iLineCnt )
		{
			areaHexInstruction.append ( " " + sicSimulator.hexLog.elementAt ( iLineCnt ) + "\n" ) ;
			areaHexInstruction.setCaretPosition ( areaHexInstruction.getDocument ().getLength () ) ;
			areaStringInstruction.append ( " " + sicSimulator.instLog.elementAt ( iLineCnt ) + "\n" ) ;
			areaStringInstruction.setCaretPosition ( areaStringInstruction.getDocument ().getLength () ) ;
		}
	}

	public static void main(String[] args) throws IOException
	{
		VisualSimulator vs = new VisualSimulator () ;
	}
}

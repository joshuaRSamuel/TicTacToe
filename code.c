
//*** ELNC6007JMLab4PIC ***
/**********************************************************************************************************************************************************
	File Name:	ELNC600JMLAB4.c
	Author:		JMichael
	Date:		15/10/2020
	Modified:	05/12/2020
	© Fanshawe College, 2016

	Description: Program that works with pic18fk22 platform to create a  3x3 tictactoe gameboard interface using led's, pushButtons and a live terminal display
				 Includes a 30 turntime which decrements every one second , after 30 seconds it generates an automatic input into the tictactoe grid.
				 Receives command string from Mbed and makes placement from user. Includes a decision making system that decides the win or draw condition


*********************************************************************************************************************************************************/


// Preprocessor ===============================================================================================================================================


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usart.h>
#include "pragmas.h"
#include <p18f45k22.h>


#define TRUE		1
#define FALSE		0

#define ON		1 
#define OFF		0 

//PUSHBUTTON OPERATION
#define PBMASK			0xF0
#define PBMODEPRESS		0xE0
#define PBENTERPRESS	0xD0
#define PBINCRPRESS		0x70
#define PBDECPRESS		0xB0
#define NOPRESS			0xF0
#define PBS		PORTA & PBMASK

#define PBMODE PORTAbits. RA4
#define PBENTER PORTAbits. RA5
#define PBINCR PORTAbits. RA7
#define PBDECR PORTAbits. RA6

//LED'S OPERATION

#define LED00IP TRISCbits.TRISC3=	1			//RC3 IP MODE
#define LED00OP TRISCbits.TRISC3=	0			//RC3 OP MODE
#define LED00RON LATCbits.LATC3=	1			//RC3 RED LED ON
#define LED00GON LATCbits.LATC3=	0			//RC3 GREEN LED ON

#define LED01IP TRISDbits.TRISD2=	1			//RD2 IP MODE
#define LED01OP TRISDbits.TRISD2=	0			//RD2 OP MODE
#define LED01RON LATDbits.LATD2=	1			//RD2 RED LED ON
#define LED01GON LATDbits.LATD2=	0			//RD2 GREEN LED ON

#define LED02IP TRISCbits.TRISC1=	1			//RC1 IP MODE
#define LED02OP TRISCbits.TRISC1=	0			//RC1 OP MODE
#define LED02RON LATCbits.LATC1=	1			//RC1 RED LED ON
#define LED02GON LATCbits.LATC1=	0			//RC1 GREEN LED ON

#define LED10IP TRISDbits.TRISD0=	1			//RD0 IP MODE
#define LED10OP TRISDbits.TRISD0=	0			//RD0 OP MODE
#define LED10RON LATDbits.LATD0=	1			//RD0 RED LED ON
#define LED10GON LATDbits.LATD0=	0			//RD0 GREEN LED ON

#define LED11IP TRISDbits.TRISD3=	1			//RD3 IP MODE
#define LED11OP TRISDbits.TRISD3=	0			//RD3 OP MODE
#define LED11RON LATDbits.LATD3=	1			//RD3 RED LED ON
#define LED11GON LATDbits.LATD3=	0			//RD3 GREEN LED ON

#define LED12IP TRISCbits.TRISC2=	1			//RC2 IP MODE
#define LED12OP TRISCbits.TRISC2=	0			//RC2 OP MODE
#define LED12RON LATCbits.LATC2=	1			//RC2 RED LED ON
#define LED12GON LATCbits.LATC2=	0			//RC2 GREEN LED ON

#define LED20IP TRISDbits.TRISD1=	1			//RD1 IP MODE
#define LED20OP TRISDbits.TRISD1=	0			//RD1 OP MODE
#define LED20RON LATDbits.LATD1=	1			//RD1 RED LED ON
#define LED20GON LATDbits.LATD1=	0			//RD1 GREEN LED ON

#define LED21IP	TRISCbits.TRISC4=	1			//RC4 IP MODE
#define LED21OP TRISCbits.TRISC4=	0			//RC4 OP MODE
#define LED21RON LATCbits.LATC4=	1			//RC4 RED LED ON
#define LED21GON LATCbits.LATC4=	0			//RC4 GREEN LED ON

#define LED22IP TRISDbits.TRISD5=	1			//RD5 OP MODE
#define LED22OP TRISDbits.TRISD5=	0			//RD5 IP MODE
#define LED22RON LATDbits.LATD5=	1			//RD5 RED LED ON
#define LED22GON LATDbits.LATD5=	0 			//RD5 GREEN LED ON


//CONSTANTS
#define ONESECH			0x0B			//ONE SEC HIGH REGISTER VALUE
#define ONESECL			0xDC			//ONE SEC LOW REGISTER VALUE
#define CONTRADDR		1
#define TTTADDY			285
#define CMDSTMNT		0
#define ADDYTO			1
#define ADDYFM			2
#define TTTROW			3
#define TTTCOL			4
#define TTTCHAR			5
#define PLYPMT			("PLYPMT")
#define GAMRST			("GAMRST")
#define GAMEND			("GAMEND")
#define ROW				3
#define COLUMN			3
#define PLACEMENTSIZE	30
#define TOKENSIZE		10
#define STATUSSIZE		40
#define BUFSIZE			30

#define TMR0FLAG INTCONbits.TMR0IF			//TMRO flag 
#define RCFLAG	PIR3bits.RC2IF				//Flag to denote byte received
#define RCIE	PIE3bits.RC2IE				//flag to enable serial port 2 receiver
#define RXBYTE	RCREG2

typedef struct gameBoard
{
	char ticTacToe[ROW][COLUMN];
	char playerNo;
	int turnCounter;
	char winFlag;
}
game_t;

typedef struct time				//data structure for time 
{
	char min;
	char sec;
	char turnSec;
}
time_t;
typedef struct ticTacToeSystem
{
	int address;
	game_t gameTtt;
	char selection;
	int rowSelect;
	int columnSelect;
	time_t gameTime;
	char statusFlag[5];				//status flag to indicate READY,PLAYING,WIN FOR PLAYER X, WIN FOR PLAYER O and RESET condition 

}
ttSys_t;
ttSys_t ttt285;


//GLOBAL VARIABLES==================================================================================================================

char count = 0;
char msgCount = 0;
char plcmntRdy = FALSE;
char plcmntUpdt[PLACEMENTSIZE];
char userPlcmnt = FALSE;
char autoPlcmnt = FALSE;
char gameStatus[STATUSSIZE];
char stringRcvd = FALSE;
char* tokens[TOKENSIZE];
char rxBuf[BUFSIZE] = { 0 };
char* rxPtr = rxBuf;
char plypmt[] = ("PLYPMT");
char gameReset[] = ("GAMERST");
char gameEnd[] = ("GAMEND");
char resetFlag = FALSE;

// Functions =========================================================================================================================


/*** set_osc_p18f45k22_4MHz: ******************************************************
Author:		JMichael
Date:		21/09/2020
Modified:	None
Desc:		Sets Fosc to 4MHZ.
Input: 		None
Returns:	None
**********************************************************************************/
void set_OSC_p18f45k22_4MHz(void)
{
	OSCCON = 0x53;		//setting on IDLE and FOsc of 4MHz 
	OSCCON2 = 0x04;		//turning on oscillator drive circuit 
	OSCTUNE = 0x80;		//

	while (OSCCONbits.HFIOFS != 1);
} //eo set_OSC_p18f45k22_4Mhz

/*** portConfig ***********************************************************
Author:		JMichael
Date:		21/09/2020
Modified:
Desc:		Sets the I/O configuration for Ports A, B, C and D
Input: 		None
Returns:	None


**********************************************************************************/
void portConfig()
{
	//PORTA
	ANSELA = 0x0F;	//RA4,5,6,7 are digital 
	LATA = 0x00;
	TRISA = 0xFF;

	//PORTD 
	ANSELD = 0x00;	//PORTD is digital
	LATD = 0x00;
	TRISD = 0x80;	//Except for serial port all other ports are o/p mode
//PORTC
	ANSELC = 0x00;	//PORT C is digital 
	LATC = 0x00;
	TRISC = 0xC0;		//Except for serial port all other ports are o/p mode
} //eo portConfig

/*** Serial  Configuration ***********************************************************
Author:		JMichael
Date:		21/09/2020
Modified:	05/12/2020
Desc:		Sets the Serial link at 9600 Baud rate for serial port1 and serial port2
Input: 		None
Returns:	None

************************************************************************************/
void serialConfig()
{
	SPBRG1 = 25;
	TXSTA1 = 0x26;
	RCSTA1 = 0x90;
	BAUDCON1 = 0x40;

	SPBRG2 = 25;
	TXSTA2 = 0x26;
	RCSTA2 = 0x90;
	BAUDCON2 = 0x40;
}// eo serialConfig

/*-------configTMR0----------------------------------------------------------------
Author: JMichael
Date: 21/09/2020
Description: config of timer0 to 1 second
Modifier: None
Input: int highreg, int lowReg
Output: None
------------------------------------------------------------------------------------*/
void configTMR0(int highReg, int lowReg)
{
	T0CON = 0x93;			//enabline Timer0 as 16bit timer and high to low transistion. 
	TMR0H = ONESECH;
	TMR0L = ONESECL;
} //eo configTMR0

/*-------resetTMR0----------------------------------------------------------------
Author: JMichael
Date: 21/09/2020
Description: reset the TMR0 back to count 1 second
Modifier: None
Input: None
Output: None
---------------------------------------------------------------------------------*/
void resetTMR0()
{
	TMR0FLAG = FALSE;
	TMR0H = ONESECH;
	TMR0L = ONESECL;
} // eo resetTMR0

/*** configIntrpts***********************************************************
Author:		JMichael
Date:		16/11/2020
Modified:	None
Desc:		Sets the interrupts required for the system
Input: 		None
Returns:	None


**********************************************************************************/

void configIntrpts()
{
	RCFLAG = FALSE;
	RCIE = TRUE;
	IPR3bits.RC2IP = FALSE;

	INTCON |= 0xC0;

} //eo configIntrpts

void ISR(void);

#pragma code highVector = 0x008
void highVector()
{
	_asm
	GOTO ISR
		_endasm
}
#pragma code 
/*** collectSentence***********************************************************
Author:		JMichael
Date:		16/11/2020
Modified:	None
Desc:		Sets the interrupts required for the system
Input: 		None
Returns:	None


**********************************************************************************/

void collectSentence()
{
	char hold = 0;

	if (RCFLAG)
	{
		hold = RXBYTE;

		if (hold == '$')
		{
			rxPtr = rxBuf;
		}
		if (hold == '^')
		{
			*(rxPtr + 1) = 0x00;
			stringRcvd = TRUE;
		}
		*rxPtr = hold;
		rxPtr++;
	}

}

#pragma interrupt ISR
void ISR()
{
	if (RCFLAG == TRUE)
	{

		collectSentence();

	}


	INTCON |= 0xC0;

}


/*** initialConfig: ****************************************************************
Author:		JMichael
Date:		21/09/2020
Modified:
Desc:		sets the microcontroller to 4MHz , I/O ports are configured accordingly, serial port 1 configured at 9600 baud rate
			and timer 0 set to create a TMROFLAG every 1 second.
Input: 		None
Returns:	None
**************************************************************************************/
void initialConfig()
{
	set_OSC_p18f45k22_4MHz();
	portConfig();
	serialConfig();
	configTMR0(ONESECH, ONESECL);
	configIntrpts();
} // eo initialConfig

/*** clearBoard: ********************************************************************
Author:		JMichael
Date:		05/10/2020
Modified:	NONE
Desc:		clears the content of 3x3 matrix
Input: 		None
Returns:	None
******************************************************************************************/
void clearBoard()
{
	int rowClear = 0, colClear = 0;
	for (rowClear = 0; rowClear < ROW; rowClear++)
	{
		for (colClear = 0; colClear < COLUMN; colClear++)
		{
			ttt285.gameTtt.ticTacToe[rowClear][colClear] = 0;
		}
	}
} //eo clearBoard

/*** initializettSys ***********************************************************
Author:		JMichael
Date:		21/09/2020
Modified:	15/10/2020
Desc:		Initial COnfiguration for the ticTacToe system
Input: 		None
Returns:	None


**********************************************************************************/

void initializeTttSys()
{

	ttt285.address = TTTADDY;
	ttt285.gameTtt.playerNo = TRUE;
	ttt285.gameTtt.turnCounter = 0;
	ttt285.gameTtt.winFlag = FALSE;
	ttt285.selection = TRUE;
	ttt285.rowSelect = 0;
	ttt285.columnSelect = 0;
	ttt285.gameTime.min = 0;
	ttt285.gameTime.sec = 0;
	ttt285.gameTime.turnSec = 30;
	ttt285.statusFlag[0] = FALSE; 			//setting READY state to false
	ttt285.statusFlag[1] = FALSE;			//Setting PLAYING state to false
	ttt285.statusFlag[2] = FALSE;			//Setting WIN FOR PLAYER X to false
	ttt285.statusFlag[3] = FALSE;			//Setting WIN FOR PLAYER O to false
	ttt285.statusFlag[4] = FALSE;			//Setting RESET condition to false 





	LED00IP;			//SETTING ALL LED PINS TO IP MODE 
	LED01IP;
	LED02IP;
	LED10IP;
	LED11IP;
	LED12IP;
	LED20IP;
	LED21IP;
	LED22IP;

	clearBoard();




} // eo initializeTttSys 

/*** calcCheckSum::************************************************************
Author:		JMichael
Date:		07/11/2020
Modified:	None
Desc:		Program for calculaing checkSum of a string
Input: 		char ptr (pointer variable for calculating checksum for the contents of the string)
Returns:	char checkSum
**********************************************************************************/

char calcCheckSum(char* cPtr)
{
	char checkSum = 0;
	while (*cPtr)
	{
		checkSum += *cPtr;
		cPtr++;
	}
	return checkSum;
}
// eo calcCheckSum()::**********************************************************
/*** validateSentence::************************************************************
Author:		JMichael
Date:		17/11/2020
Modified:	None
Desc:		Function for validating cheksum of received and transmitted message
Input: 		None
Returns:	None
**********************************************************************************/

char validateSentence(char* vPtr)
{
	char rcs = 0, ncs = 0;
	char csFlag = FALSE;


	unsigned char count = strlen(vPtr);
	while (csFlag != TRUE)
	{
		if (*(vPtr + count) == '^')
		{
			*(vPtr + count) = 0x00;
		}
		if (*(vPtr + count) == ',')
		{
			*(vPtr + count) = 0x00;
			rcs = atoi(vPtr + count + 1);
			csFlag = TRUE;
		}
		count--;
	}
	ncs = calcCheckSum(vPtr);
	if (ncs == rcs)
	{
		return TRUE;

	}
	else
	{
		return FALSE;
	}
}

// eo validateSentence()::**********************************************************


/*** parseSentence::************************************************************
Author:		JMichael
Date:		03/04/2020
Modified:	None
Desc:		Function for breaking up received message and storing into tokens
Input: 		None
Returns:	None
**********************************************************************************/
void parseSentence(char* parsePtr)
{
	unsigned char tokenNumber = 0;
	while (*parsePtr)
	{
		if (*parsePtr == '$' || *parsePtr == ',')
		{
			*parsePtr = 0x00;
			tokens[tokenNumber] = parsePtr + 1;
			tokenNumber++;
		}
		parsePtr++;

	} // eo while
}
// eo parseSentence()::**********************************************************

/*** characterSelection::***************************************************************************
Author:		JMichael
Date:		03/12/2020
Modified:	None
Desc:		Function for inputting X or O input based received command sentence from mbed controller
Input: 		int rowNuber, int colNumber
Returns:	None
***********************************************************************************************/
void characterSelection(int rowNumber, int colNumber)
{
	if (*tokens[TTTCHAR] == 'X')
	{
		ttt285.gameTtt.ticTacToe[rowNumber][colNumber] = 'X';

	}
	if (*tokens[TTTCHAR] == 'O')
	{
		ttt285.gameTtt.ticTacToe[rowNumber][colNumber] = 'O';

	}
	userPlcmnt = TRUE;


}


/*** executeSentence::************************************************************
Author:		JMichael
Date:		17/11/2020
Modified:	None
Desc:		Function for executing functions based on Command statement.
Input: 		None
Returns:	None
**********************************************************************************/

void executeSentence()
{
	if (atoi(tokens[ADDYTO]) == TTTADDY)
	{
		if (atoi(tokens[ADDYFM]) == CONTRADDR)
		{
			if (strcmp(tokens[CMDSTMNT], plypmt) == 0)
			{
				characterSelection((atoi(tokens[TTTROW])), (atoi(tokens[TTTCOL])));
			}
			if (strcmp(tokens[CMDSTMNT], gameReset) == 0)
			{
				resetFlag = TRUE;
			}
		}
	} // eo if 
} // eo executeSentence	

/*** checkWin: ***********************************************************
Author:		checkWin
Date:		05/10/2020
Modified:	08/12/2020
Desc:		checks the ticTacToe board for winning condition between PLAER X or PLAYER O.
Input: 		None
Returns:	None
**********************************************************************************/
void checkWin()
{
	char rowCheck = 0, columnCheck = 0;
	for (rowCheck = 0; rowCheck < ROW; rowCheck++)
	{
		for (columnCheck = 0; columnCheck < COLUMN; columnCheck++)
		{
			if (ttt285.gameTtt.ticTacToe[rowCheck][columnCheck] == 'X' && ttt285.gameTtt.ticTacToe[rowCheck][columnCheck + 1] == 'X' && ttt285.gameTtt.ticTacToe[rowCheck][columnCheck + 2] == 'X')		//GENERAL FORMULA TO CHECK FOR WINNING ROW
			{
				ttt285.statusFlag[2] = TRUE; 		// Setting status WIN FOR PLAYER X to TRUE 

			}
			if (ttt285.gameTtt.ticTacToe[rowCheck][columnCheck] == 'X' && ttt285.gameTtt.ticTacToe[rowCheck + 1][columnCheck] == 'X' && ttt285.gameTtt.ticTacToe[rowCheck + 2][columnCheck] == 'X')		//GENERAL FORMULA TO CHECK WINNING CONDITIN FOR COLUMN
			{
				ttt285.statusFlag[2] = TRUE; 		// Setting status WIN FOR PLAYER X to TRUE


			}

			if (ttt285.gameTtt.ticTacToe[rowCheck][columnCheck] == 'O' && ttt285.gameTtt.ticTacToe[rowCheck][columnCheck + 1] == 'O' && ttt285.gameTtt.ticTacToe[rowCheck][columnCheck + 2] == 'O')
			{
				ttt285.statusFlag[3] = TRUE; 		// Setting status WIN FOR PLAYER O to TRUE

			}
			if (ttt285.gameTtt.ticTacToe[rowCheck][columnCheck] == 'O' && ttt285.gameTtt.ticTacToe[rowCheck + 1][columnCheck] == 'O' && ttt285.gameTtt.ticTacToe[rowCheck + 2][columnCheck] == 'O')
			{
				ttt285.statusFlag[3] = TRUE; 		// Setting status WIN FOR PLAYER O to TRUE


			}
		} // eo for columnCheck
	} // eo for rowcheck
	if (ttt285.gameTtt.ticTacToe[0][0] == 'X' && ttt285.gameTtt.ticTacToe[1][1] == 'X' && ttt285.gameTtt.ticTacToe[2][2] == 'X') 		//condition to check for diagnal win condition 
	{
		ttt285.statusFlag[2] = TRUE;
	}
	if (ttt285.gameTtt.ticTacToe[0][2] == 'X' && ttt285.gameTtt.ticTacToe[1][1] == 'X' && ttt285.gameTtt.ticTacToe[2][0] == 'X')
	{
		ttt285.statusFlag[2] = TRUE;
	}
	if (ttt285.gameTtt.ticTacToe[0][0] == 'O' && ttt285.gameTtt.ticTacToe[1][1] == 'O' && ttt285.gameTtt.ticTacToe[2][2] == 'O')
	{
		ttt285.statusFlag[3] = TRUE;
	}
	if (ttt285.gameTtt.ticTacToe[0][2] == 'O' && ttt285.gameTtt.ticTacToe[1][1] == 'O' && ttt285.gameTtt.ticTacToe[2][0] == 'O')
	{
		ttt285.statusFlag[3] = TRUE;
	}
} // eo checkWin

/*** clearLED: *********************************************************************
Author:		JMichael
Date:		03/10/2020
Modified:	NONE
Desc:		function that sets all the LED's to Input state thereby switching off all LED's
Input: 		None
Returns:	None
**********************************************************************************/
void clearLED()
{
	LED00IP;
	LED01IP;
	LED02IP;
	LED10IP;
	LED11IP;
	LED12IP;
	LED20IP;
	LED21IP;
	LED22IP;
} // eo clearLED


/*** ledOperation: ************************************************************************************************************************************
Author:		ledOpeation
Date:		05/10/2020
Modified:	03/04/2020
Desc:		Function that checks the content of ticTacToe array
Input: 		None
Returns:	None
******************************************************************************************************************************************************/
void ledOperation()
{
	if (ttt285.gameTtt.ticTacToe[0][0] == 'X')
	{
		LED00OP;		//CONTROL FOR LED PIN RC3 TO O/P
		LED00RON;		//TURNING ON RED LED FOR X
	}
	else if (ttt285.gameTtt.ticTacToe[0][0] == 'O')
	{
		LED00OP; 		//CONTROL FOR (0,0) LED TO O/P
		LED00GON;		//TURNING ON GREEN LED FOR O
	}
	else
	{
		LED00IP;		//OTHERWISE BOTH LED'S REMAIN OFF
	}

	if (ttt285.gameTtt.ticTacToe[0][1] == 'X')
	{
		LED01OP;
		LED01RON;
	}
	else if (ttt285.gameTtt.ticTacToe[0][1] == 'O')
	{
		LED01OP;
		LED01GON;
	}
	else
	{
		LED01IP;
	}


	if (ttt285.gameTtt.ticTacToe[0][2] == 'X')
	{
		LED02OP;
		LED02RON;
	}
	else if (ttt285.gameTtt.ticTacToe[0][2] == 'O')
	{
		LED02OP;
		LED02GON;
	}
	else
	{
		LED02IP;
	}


	if (ttt285.gameTtt.ticTacToe[1][0] == 'X')
	{
		LED10OP;
		LED10RON;
	}
	else if (ttt285.gameTtt.ticTacToe[1][0] == 'O')
	{
		LED10OP;
		LED10GON;
	}
	else
	{
		LED10IP;
	}

	if (ttt285.gameTtt.ticTacToe[1][1] == 'X')
	{
		LED11OP;
		LED11RON;
	}
	else if (ttt285.gameTtt.ticTacToe[1][1] == 'O')
	{
		LED11OP;
		LED11GON;
	}
	else
	{
		LED11IP;
	}

	if (ttt285.gameTtt.ticTacToe[1][2] == 'X')
	{
		LED12OP;
		LED12RON;
	}
	else if (ttt285.gameTtt.ticTacToe[1][2] == 'O')
	{
		LED12OP;
		LED12GON;
	}
	else
	{
		LED12IP;
	}

	if (ttt285.gameTtt.ticTacToe[2][0] == 'X')
	{
		LED20OP;
		LED20RON;
	}
	else if (ttt285.gameTtt.ticTacToe[2][0] == 'O')
	{
		LED20OP;
		LED20GON;
	}
	else
	{
		LED20IP;
	}

	if (ttt285.gameTtt.ticTacToe[2][1] == 'X')
	{
		LED21OP;
		LED21RON;
	}
	else if (ttt285.gameTtt.ticTacToe[2][1] == 'O')
	{
		LED21OP;
		LED21GON;
	}
	else
	{
		LED21IP;
	}

	if (ttt285.gameTtt.ticTacToe[2][2] == 'X')
	{
		LED22OP;
		LED22RON;
	}
	else if (ttt285.gameTtt.ticTacToe[2][2] == 'O')
	{
		LED22OP;
		LED22GON;
	}
	else
	{
		LED22IP;
	}
} // eo ledOperation


/*** time: *********************************************************************************************************************************
Author:		JMichael
Date:		15/10/2020
Modified:	08/12/2020
Desc:		Function that performs the working of a gameTime Clock and 30 sec turn timer upon which default input is assumed  using TIMER0
Input: 		None
Returns:	None
********************************************************************************************************************************************/

void time()
{

	char row = 0, col = 0, ready = FALSE;
	ttt285.gameTime.sec++;			//incrementing game clock second
	ttt285.gameTime.turnSec--;				//condition to decrement turn time 
	msgCount++;						//incrementing msgcount for $PLYPMT string dispay

	if (ttt285.gameTime.sec >= 60)	//condition for 1 min
	{
		ttt285.gameTime.sec = 0;
		ttt285.gameTime.min++;
		if (ttt285.gameTime.min >= 15)		//if max game time is reached 
		{
			ttt285.gameTime.min = 0;
			ttt285.gameTime.sec = 0;
		}
	}


	if (ttt285.gameTime.turnSec <= 0)			// if turntime reaches to zero 
	{
		while (ready != TRUE)
		{
			if (ttt285.gameTtt.ticTacToe[row][col] != 'X' && ttt285.gameTtt.ticTacToe[row][col] != 'O')		//condition to check for empty cell 
			{
				if (ttt285.gameTtt.turnCounter % 2 == 0)			//condition to check for X player 
				{
					ttt285.gameTtt.playerNo = TRUE;
					ttt285.gameTtt.ticTacToe[row][col] = 'X';
					sprintf(plcmntUpdt, "$PLYPMT,%i,%i,%i,%i,%c\0", CONTRADDR, TTTADDY, row, col, 'X');		//loading string values 
					sprintf(plcmntUpdt, "%s,%i^\0", plcmntUpdt, calcCheckSum(plcmntUpdt));
					puts2USART(plcmntUpdt);
					plcmntRdy = TRUE;																						//toggling flag to indicate player placemet 
					ready = TRUE;
					msgCount = 0;										//initializing msgCount for 5 sec display to 0 
				}
				else if (ttt285.gameTtt.turnCounter % 2 == 1)			//condition for O player 
				{
					ttt285.gameTtt.playerNo = FALSE;
					ttt285.gameTtt.ticTacToe[row][col] = 'O';
					sprintf(plcmntUpdt, "$PLYPMT,%i,%i,%i,%i,%c\0", CONTRADDR, TTTADDY, row, col, 'O');
					sprintf(plcmntUpdt, "%s,%i^\0", plcmntUpdt, calcCheckSum(plcmntUpdt));
					puts2USART(plcmntUpdt);
					plcmntRdy = TRUE;
					ready = TRUE;
					msgCount = 0;

				}
			}
			col++;				//if empty cell is not found move on to next cell column vise 
			if (col > COLUMN)
			{
				col = 0;
				row++;			//move to next row after checking all columns of particular row
				if (row > ROW)
				{
					row = 0;
					col = 0;
				}
			}
		} //eo while 	

		ttt285.gameTime.turnSec = 30;		//restarting turn time 
		ttt285.gameTtt.turnCounter++;		//incrementing turnCounter  
	} //eo if
} //eo time	


/*** updateDisplay: ***********************************************************
Author:		JMichael
Date:		05/10/2020
Modified:	05/12/2020
Desc:		Terminal Display that shows interface for the 2 players
Input: 		None
Returns:	None
**********************************************************************************/
void updateDisplay()
{

	if (ttt285.gameTtt.turnCounter == 0)				//CONDITION FOR READY STATUS
	{
		ttt285.statusFlag[0] = TRUE;
		printf("\e[1;55H Status: READY\t  ");
		ttt285.statusFlag[1] = FALSE;
		ttt285.statusFlag[4] = FALSE;
	}
	if (ttt285.gameTtt.turnCounter > 0 && ttt285.gameTtt.turnCounter <= 9)		//CONDITION FOR PLAYING STATUS
	{
		ttt285.statusFlag[1] = TRUE;
		printf("\e[1;55H Status: PLAYING\t  ");
		ttt285.statusFlag[0] = FALSE;
		ttt285.statusFlag[4] = FALSE;
		//time();
	}

	if (ttt285.statusFlag[2] == TRUE)
	{
		ttt285.statusFlag[1] = FALSE;
		printf("\e[1;55H Status: PLAYER X WINS!!\t");
		sprintf(gameStatus, "$GAMEND,%i,%i,%c,%c,%2d%2d\0", CONTRADDR, TTTADDY, 'W', 'X', ttt285.gameTime.min, ttt285.gameTime.sec);
		sprintf(gameStatus, "%s,%i^\0", gameStatus, calcCheckSum(gameStatus));
		puts2USART(gameStatus);
		printf("\e[19;25H%s", gameStatus);
		ttt285.statusFlag[2] = FALSE;

	}
	if (ttt285.statusFlag[3] == TRUE)
	{
		ttt285.statusFlag[1] = FALSE;
		printf("\e[1;55H Status: PLAYER O WINS!!\t");
		sprintf(gameStatus, "$GAMEND,%i,%i,%c,%c,%2d%2d\0", CONTRADDR, TTTADDY, 'W', 'O', ttt285.gameTime.min, ttt285.gameTime.sec);
		sprintf(gameStatus, "%s,%i^\0", gameStatus, calcCheckSum(gameStatus));
		puts2USART(gameStatus);
		printf("\e[19;25H%s", gameStatus);
		ttt285.statusFlag[3] = FALSE;

	}

	printf("\e[1;5H TicTacToe285 \t\t GameTime: %02d : %02d ", ttt285.gameTime.min, ttt285.gameTime.sec);		//gametime display
	printf("\e[4;55H TurnTime: %2d", ttt285.gameTime.turnSec);

	printf("\e[2;5H Player: %d \t\t TurnCount: %d", ttt285.gameTtt.playerNo, ttt285.gameTtt.turnCounter);
	if (ttt285.selection == TRUE)		//Condition for row 
	{
		printf("\e[4;5H  ROW\t");
		printf("\e[5;5H Number: %d", ttt285.rowSelect);
	}
	if (ttt285.selection == FALSE)		//Condition for Column
	{
		printf("\e[K\e[4;5H  COLUMN\t");
		printf("\e[5;5H Number: %d", ttt285.columnSelect);
	}
	printf("\e[8;50HCOLUMN");
	printf("\e[9;30H0\t\t1\t\t2");
	printf("\e[10;25H0\t%c |\t %c |\t %c ", ttt285.gameTtt.ticTacToe[0][0], ttt285.gameTtt.ticTacToe[0][1], ttt285.gameTtt.ticTacToe[0][2]);
	printf("\e[11;20HR ------------------------------------------------------------");
	printf("\e[12;25H1\t%c |\t %c |\t %c ", ttt285.gameTtt.ticTacToe[1][0], ttt285.gameTtt.ticTacToe[1][1], ttt285.gameTtt.ticTacToe[1][2]);
	printf("\e[14;20HW ------------------------------------------------------------");
	printf("\e[15;25H2\t%c |\t %c |\t %c ", ttt285.gameTtt.ticTacToe[2][0], ttt285.gameTtt.ticTacToe[2][1], ttt285.gameTtt.ticTacToe[2][2]);

	if (plcmntRdy == TRUE)		//condition to check if player placement has been made 
	{
		if (msgCount < 5)		//condition to print display for only 5 seconds 
		{
			printf("\e[18;25H%s", plcmntUpdt);
		}
		if (msgCount >= 5)
		{
			msgCount = 0;
			printf("\e[18;25H                                                                       ");			// else print nothing 
			plcmntRdy = FALSE;

		}

	} // eo if 	
	if (userPlcmnt == TRUE)
	{
		userPlcmnt = FALSE;
		while (userPlcmnt != 0);
		ttt285.gameTime.turnSec = 30;
		ttt285.gameTtt.turnCounter++;


	}

} //eo updateDisplay


/*--- MAIN FUNCTION -------------------------------------------------------------------------
-------------------------------------------------------------------------------------------*/
void main()
{
	initialConfig();
	initializeTttSys();

	while (ttt285.gameTtt.turnCounter < 10)
	{

		if (TMR0FLAG)			//flag that denotes completion of 1 second
		{
			if (stringRcvd == TRUE)
			{
				stringRcvd = FALSE;
				printf("\e[22;25H%s", rxBuf);
				if (validateSentence(rxBuf))
				{
					parseSentence(rxBuf);
					executeSentence();
				}
			}
			if (ttt285.statusFlag[1] == TRUE)
			{

				time();
			}
			if (resetFlag == TRUE)
			{
				ttt285.statusFlag[1] = FALSE;
				printf("\e[1;55H Status: RESET\t");
				initializeTttSys();
				resetFlag = FALSE;
			}
			if (ttt285.gameTtt.turnCounter >= 9)		//condition for draw
			{
				ttt285.statusFlag[1] = FALSE;
				ttt285.statusFlag[0] = FALSE;
				ttt285.statusFlag[2] = FALSE;
				ttt285.statusFlag[3] = FALSE;

				sprintf(gameStatus, "$GAMEND,%i,%i,%c,%c,%02d%02d\0", CONTRADDR, TTTADDY, 'D', 'N', ttt285.gameTime.min, ttt285.gameTime.sec);
				sprintf(gameStatus, "%s,%i^\0", gameStatus, calcCheckSum(gameStatus));
				puts2USART(gameStatus);
				printf("\e[19;25H%s", gameStatus);
				clearLED();
			}
			updateDisplay();
			ledOperation();
			checkWin();
			resetTMR0();
		}
	} //eo while 

} //eo main

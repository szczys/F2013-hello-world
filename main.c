#define F_CPU 1000000UL

#include <io.h>

#define LCD_PORT	P1OUT	//PORTB
#define LCD_DDR		P1DIR	//DDRB
#define LCD_CLK 	BIT1	//(1<<PB0)
#define LCD_SIO		BIT2	//(1<<PB1)
#define LCD_CS		BIT3	//(1<<PB2)
#define LCD_RST		BIT4	//(1<<PB3)

#define red	0b00000111
#define yellow	0b00111111
#define green	0b00111100
#define cyan	0b11111000
#define blue	0b11000000
#define magenta	0b11000111
#define white	0b11111111
#define black	0b00000000

#define page_size	97
#define row_size	67

#define color_bg	white
#define color_fg	black

#define ball_diameter	4

#define paddle_thickness	4
#define paddle_length		15

/**Device Information**
* MPS430 F2013        *
* Power=3.3v          *
**********************/

/**LCD Pinout*****************
* 1=Reset------>3.3v+        *
* 2=CS--------->Logic        *
* 3=GND-------->GND          *
* 4=SIO-------->Logic        *
* 5=SCL-------->Logic        *
* 6=VDigital--->3.3v+        *
* 7=VBoost----->3.3v+        *
* 8=VLCD--->0.22uf Cap-->GND *
*  (This cap may not be      *
*   optimal, other schematics*
*   have suggested 1uf)      *
*****************************/

//LCD_Out function comes from source code found here:
//http://hobbyelektronik.org/Elo/AVR/3510i/index.htm
//Unfortunately this is the only way I know to attribute
//this code to the writer.

static void __inline__ delay(register unsigned int n)
{
    __asm__ __volatile__ (
		"1: \n"
		" dec	%[n] \n"
		" jne	1b \n"
        : [n] "+r"(n));
}

void delay_ms(unsigned int n)
{
  while (n--)
  {
    delay(F_CPU/4000);
  }
}

void LCD_Out(unsigned char Data, unsigned char isCmd) {
    if(isCmd) LCD_PORT |= LCD_CS; 
    LCD_PORT &= ~(LCD_CLK|LCD_CS);  //Clock and CS low

    LCD_PORT |= LCD_SIO;        //SData High
    if(isCmd) LCD_PORT &= ~LCD_SIO; //If it is a command, SData Low

    LCD_PORT |= LCD_CLK;        //Clock High

    for(char x=0; x<8; x++)    {
        LCD_PORT &= ~(LCD_SIO|LCD_CLK);        //Clock and SData low
        if(Data & 128) LCD_PORT |= LCD_SIO;      // Mask MSB - SData high if it is a 1
        LCD_PORT |= LCD_CLK;            //Clock High
        Data=Data<<1;                //Shift bits 1 left (new MSB to be read)
    }
}

void LCD_init(void)
{
  LCD_DDR |= (LCD_CLK | LCD_SIO | LCD_CS | LCD_RST);

  //Hardware Reset
  LCD_PORT &= ~LCD_RST;
  LCD_PORT |= LCD_RST;
  delay_ms(5);

  LCD_PORT |= (LCD_CLK | LCD_SIO | LCD_CS);

  //Software Reset
  LCD_Out(0x01, 1);
  delay_ms(10);

/*
  //Refresh set
  LCD_Out(0xB9, 1);
  LCD_Out(0x00, 0);
*/

  //Display Control
  LCD_Out(0xB6, 0);
  LCD_Out(128, 0);
  LCD_Out(128, 0);
  LCD_Out(129, 0);
  LCD_Out(84, 0);
  LCD_Out(69, 0);
  LCD_Out(82, 0);
  LCD_Out(67, 0);

/*
  //Temperature gradient set
  LCD_Out(0xB7, 1);
  for(char i=0; i<14; i++)  LCD_Out(0, 0);
*/

  //Booster Voltage On
  LCD_Out(0x03, 1);
  delay_ms(50);  //NOTE: At least 40ms must pass between voltage on and display on.
          //Other operations may be carried out as long as the display is off
          //for this length of time.

/*
  //Test Mode
  LCD_Out(0x04, 1);
*/

/*
  // Power Control
  LCD_Out(0xBE, 1);
  LCD_Out(4, 0);
*/

  //Sleep Out
  LCD_Out(0x11, 1);

  //Display mode Normal
  LCD_Out(0x13, 1);

  //Display On
  LCD_Out(0x29, 1);

  //Set Color Lookup Table
  LCD_Out(0x2D, 1);        //Red and Green (3 bits each)
  char x, y;
  for(y = 0; y < 2; y++) {
      for(x = 0; x <= 14; x+=2) {
          LCD_Out(x, 0);
      }
  }
  //Set Color Lookup Table    //Blue (2 bits)
  LCD_Out(0, 0);
  LCD_Out(4, 0);
  LCD_Out(9, 0);
  LCD_Out(14, 0);

  //Set Pixel format to 8-bit color codes
  LCD_Out(0x3A, 1);
  LCD_Out(0b00000010, 0);

//***************************************
//Initialization sequence from datasheet:

//Power to chip
//RES pin=low
//RES pin=high -- 5ms pause
//Software Reset
//5ms Pause
//INIESC
  //<Display Setup 1>
    //REFSET
    //Display Control
    //Gray Scale position set
    //Gamma Curve Set
    //Common Driver Output Select
  //<Power Supply Setup>
    //Power Control
    //Sleep Out
    //Voltage Control
    //Write Contrast
    //Temperature Gradient
    //Boost Voltage On
  //<Display Setup 2>
    //Inversion On
    //Partial Area
    //Vertical Scroll Definition
    //Vertical Scroll Start Address
  //<Display Setup 3>
    //Interface Pixel Format
    //Colour Set
    //Memory access control
    //Page Address Set
    //Column Address Set
    //Memory Write
  //Display On

//****************************************

}

void fillScreen(unsigned char color)
{
  LCD_Out(0x2A, 1); //Set Column location
  LCD_Out(0, 0);
  LCD_Out(97, 0);
  LCD_Out(0x2B, 1); //Set Row location
  LCD_Out(0, 0);
  LCD_Out(66, 0);
  LCD_Out(0x2C, 1); //Write Data
  for (int i=0; i<6566; i++) LCD_Out(color, 0);
}

void Hello_World(void)
{
  //Binary representation of "Hello World"
  unsigned char Hello_World[5][5] = {
    0b10101110, 0b10001000, 0b01001010, 0b10010011, 0b00100110,
    0b10101000, 0b10001000, 0b10101010, 0b10101010, 0b10100101,
    0b11101100, 0b10001000, 0b10101010, 0b10101011, 0b00100101,
    0b10101000, 0b10001000, 0b10101010, 0b10101010, 0b10100101,
    0b10101110, 0b11101110, 0b01000101, 0b00010010, 0b10110110
  };

    LCD_Out(0x2A, 1);
    LCD_Out(8, 0);
    LCD_Out(87, 0);
    LCD_Out(0x2B, 1);
    LCD_Out(23, 0);
    LCD_Out(32, 0);
    LCD_Out(0x2C, 1);
    for (unsigned char i=0; i<5; i++) //Scan Rows
    {
      char h=2;
      while(h)
      {
	for (unsigned char k=0; k<5; k++) //Scan Columns
	{
	  for (char j=0; j<8; j++)
	  {
	    if (Hello_World[i][k] & 1<<(7-j))	//Should there be a letter pixel here?
	    {
	      LCD_Out(0x00, 0);			//yes - draw it in black
	      LCD_Out(0x00, 0);			
	    }
	    else 
	    {
	      LCD_Out(0xFF, 0);			//no - draw background in white
	      LCD_Out(0xFF, 0);
	    }
	  }
	}
	--h;
      }
    }
}

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

  LCD_init();
  
  fillScreen(color_bg);
   
  Hello_World();
  while(1)
  {
    
  }
}

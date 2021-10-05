#include <8052.h>
#include <stdlib.h>
#include <stdio.h>


/*--------------------------------------------------------------------------------*/


	#define OSC_FREQ (11059200UL)
	#define OSC_PER_INST (1)

	#define PRELOAD01  (65536 - (unsigned int)(OSC_FREQ / (OSC_PER_INST * 1063)))    
	#define PRELOAD01H (PRELOAD01 / 256)
	#define PRELOAD01L (PRELOAD01 % 256)

/*--------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/

	/*硬件延时N ms*/
	void Hardware_Delay_T0(unsigned int N)   
	{
		unsigned int ms;
		         
		// Configure Timer 0 as a 16-bit timer 
		TMOD &= 0xF0; // Clear all T0 bits (T1 left unchanged)
		TMOD |= 0x01; // Set required T0 bits (T1 left unchanged) 

		ET0 = 0;  // No interupts

		// Delay value is *approximately* 1 ms per loop   
		for (ms = 0; ms < N; ms++)
		{                 
			TH0 = PRELOAD01H;  
			TL0 = PRELOAD01L;   
			TF0 = 0;          // clear overflow flag
			TR0 = 1;          // start timer 0

			while (TF0 == 0); // Loop until Timer 0 overflows (TF0 == 1)

			TR0 = 0;          // Stop Timer 0
		}
	}        
/*--------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------*/
	
	/*端口定义
		PIN DESCRIPTION   
		P17 P16 P15 P14
 		|   |   |   |
 		\---\---\---\---P10        1  2  3  F1
 		\---\---\---\---P11        4  5  6  F2
 		\---\---\---\---P12        7  8  9  F3
 		\---\---\---\---P13        *  0  #  F4
	*/
	#define KeyPort P1

	#define key_state_0 0      //定义按键状态（基于状态机）
	#define key_state_1 1
	#define key_state_2 2
	#define key_state_3 3

	#define NO_key        0

	/*按键扫描函数*/
	unsigned char KeyBoardScan(void)
	{   
		unsigned char key_temp;
		unsigned char key_num = NO_key;
		static unsigned char key_status = key_state_0;         //按键状态

		//线反转法扫描键盘
		KeyPort = 0x0f;
		key_temp = KeyPort;
		KeyPort = 0xf0;
		key_temp |= KeyPort;
		KeyPort = 0xff;

	 switch(key_status){
		/*状态零为初始状态*/
		case key_state_0:                                    
			if(key_temp != 0xff)
				key_status = key_state_1;
		break;
					
		/*状态一为按键按下状态*/
		case key_state_1:                                   
			if(key_temp == 0xff)
				key_status = key_state_0;             //若只是抖动，调回状态零
			else{	key_status = key_state_2;            //若确实按下，调到状态二
				switch(key_temp){
					case 0x7e:        //0111 1110
						key_num = '1';		
						break;	
					case 0xbe:        //1011 1110	
						key_num = '2';	
						break;	
					case 0xde:        //1101 1110
							key_num = '3';
						break;
					case 0x7d:        //0111 1101
							key_num = '4';
						break;
					case 0xbd:        //1011 1101
							key_num = '5';
						break;
					case 0xdd:        //1101 1101
							key_num = '6';	
						break;
					case 0x7b:        //0111 1011
							key_num = '7';
						break;
					case 0xbb:        //1011 1011
							key_num = '8';
						break;
					case 0xdb:        //1101 1011
							key_num = '9';
						break;
					case 0xb7:        //1011 0111
							key_num = '0';
						break;
					case 0x77:        //0111 0111
							key_num = '*';                  //*
						break;
					case 0xd7:        //1101 0111
							key_num = '#';                  //#
						break;
					case 0xee:        //1110 1110
							key_num = 'A';                  //F1
						break;
					case 0xed:        //1110 1101
							key_num = 'B';                  //F2
						break;
					case 0xeb:        //1110 1011
							key_num = 'C';                  //F3
						break;
					case 0xe7:        //1110 0111
							key_num = 'D';                  //F4
						break;
						}
				}
		break;

		/*状态二表示确实有按键按下*/
		case key_state_2:
			if(key_temp == 0xff)
				key_status = key_state_3;	//若检测到松手，调到状态三
		break;

		case key_state_3:
			if(key_temp == 0xff)
				key_status = key_state_0;	//若判断确实松手后，调回初始状态零
			else
				key_status = key_state_2;	//若只是抖动并不是真的松手，调回状态二
		break;
	 }		
	 return key_num;
	}


/*--------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------
LCD输入的正确格式
1.英文字符每行16个
2.汉字字符每行8个。(GBK编码)
3.注意空格也要写上。
------------------------------------------------------------------------------------*/

#define	CS   P2_7//第4根线  RS
#define SID  P2_6//第5根线  RW
#define SCK  P2_5//第6根线  E
unsigned char const AC_TABLE[]={
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,    //第一行汉字位置  
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,    //第二行汉字位置  
0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,    //第三行汉字位置 
0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,    //第四行汉字位置 
};
/******************************** 函数名称    ：SendByte  串口发送一个字节*****/
void SendByte(unsigned char Dbyte)
{
     unsigned char i;
     for(i=0;i<8;i++)
     {
           SCK = 0;
           Dbyte=Dbyte<<1;     
           SID = CY;           
           SCK = 1;
           SCK = 0;
     }
}
 
/***********接收一个字节***/
unsigned char ReceiveByte(void)
{
     unsigned char i,temp1,temp2;
     temp1 = 0;
     temp2 = 0;
     for(i=0;i<8;i++)
     {
           temp1=temp1<<1;
           SCK = 0;
           SCK = 1;            
           SCK = 0;
           if(SID) temp1++;
     }
     for(i=0;i<8;i++)
     {
           temp2=temp2<<1;
           SCK = 0;
           SCK = 1;
           SCK = 0;
           if(SID) temp2++;
     }
     return ((0xf0&temp1)+(0x0f&temp2));
}
 
void CheckBusy( void )
{
     do   SendByte(0xfc);      
     while(0x80&ReceiveByte());    
}
 
void WriteCommand(unsigned char Cbyte )
{
     CS = 1;
     CheckBusy();
     SendByte(0xf8);            
     SendByte(0xf0&Cbyte);     
     SendByte(0xf0&Cbyte<<4);
     CS = 0;
}
 
void WriteData(unsigned char Dbyte )
{
     CS = 1;
     CheckBusy();
     SendByte(0xfa);            
     SendByte(0xf0&Dbyte);      
     SendByte(0xf0&Dbyte<<4);
     CS = 0;
}
 
unsigned char ReadData( void )
{
     CheckBusy();
     SendByte(0xfe);            
     return ReceiveByte();
}
 
 
void LcmInit( void )
{
     WriteCommand(0x30);      
     WriteCommand(0x03);      
     WriteCommand(0x0C);      
     WriteCommand(0x01);      
     WriteCommand(0x06);      
}
 
void LcmClearTXT( void )
{
     unsigned char i;
     WriteCommand(0x30);     
     WriteCommand(0x80);      
     for(i=0;i<64;i++)
     WriteData(0x20);
}
 
 
void PutStr(unsigned char row,unsigned char col,unsigned char *puts)
{
     WriteCommand(0x30);     
     WriteCommand(AC_TABLE[8*row+col]);   
     while(*puts != '\0')      
     {
           if(col==8)           
           {            
                 col='0';
                 row++;
           }
           if(row==4) row='0';     
           WriteCommand(AC_TABLE[8*row+col]);
           WriteData(*puts);      
           puts++;
           WriteData(*puts);
           puts++;
           col++;
     }
}
 
 
void DisplayDots(unsigned char DotByte)
{
     unsigned char i,j;
     WriteCommand(0x34);     
     WriteCommand(0x36);     
     for(i=0;i<32;i++)           
     {
           WriteCommand(0x80|i);     
           WriteCommand(0x80);     
           for(j=0;j<32;j++)     
           {            
              WriteData(DotByte);                  
           }
           DotByte=~DotByte;
     }
}
 
void main( void )
{     
     //unsigned char  j[]="wewe";
	//unsigned int Number=0xa7;
    //unsigned char buf[30];
	unsigned char Menu_Status = 0;
	
	 LcmInit();
     LcmClearTXT();

    /*
	 while(1)
     {
		Number+=0.01;
        PutStr(0,0,"  Jankin Bai  ");
        PutStr(1,0,"****************");
		sprintf(buf, "T%5.3f",Number);
		PutStr(2,0,buf);
        PutStr(3,0,"****************");
		Hardware_Delay_T0(10);
		
		
		PutStr(0,0,"智能大棚控制系统");
		PutStr(1,0,"温度：24.5 'C ");
		PutStr(2,0,"光照：2346 Lux");
		PutStr(3,0,"湿度：64.8 %");

     }
	*/
	 
    //unsigned int Menu_Status = 0;
while(1){
    switch(Menu_Status){
        case 0: {
			PutStr(0,0,"     -MENU-     ");
			PutStr(1,0,"输入：          ");
			PutStr(2,0," <-:返回 ->:确定");
			PutStr(3,0," Waiting...     ");

			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 1; break;
        				case '8' :   Menu_Status = 1; break;
        				case '6' :	 Menu_Status = 1; break;
						case '4':    Menu_Status = 0; break;
						default:     Menu_Status = 0; break;
						}
		};break;
        case 1: {
			PutStr(0,0,"   -SUB_MENU-   ");
			PutStr(1,0," ->Status_1     ");
			PutStr(2,0,"  Status_2      ");
			PutStr(3,0,"  Status_3      ");
			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 3; break;
        				case '8' : Menu_Status = 2; break;
        				case '6' :Menu_Status = 4; break;
						case'4'  :Menu_Status = 0; break;
						default:     Menu_Status = 1; break;
						}
		};break;
        case 2 : {
			PutStr(0,0,"   -SUB_MENU-   ");
			PutStr(1,0,"  Status_1      ");
			PutStr(2,0," ->Status_2     ");
			PutStr(3,0,"  Status_3      ");
			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 1; break;
        				case '8' : Menu_Status = 3; break;
        				case '6' :Menu_Status = 7; break;
						case'4'  :Menu_Status = 0; break;
						default:     Menu_Status = 2; break;
						}
		};break;
        case 3: {
			PutStr(0,0,"   -SUB_MENU-   ");
			PutStr(1,0,"  Status_1      ");
			PutStr(2,0,"  Status_2      ");
			PutStr(3,0," ->Status_3     ");
			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 2; break;
        				case '8' : Menu_Status = 1; break;
        				case '6' :Menu_Status = 10; break;
						case'4'  :Menu_Status = 0; break;
						default:     Menu_Status = 3; break;
						}
		};break;
        case 4: {
			PutStr(0,0,"   *Status_1*   ");
			PutStr(1,0," ->Status_1_1   ");
			PutStr(2,0,"  Status_1_2    ");
			PutStr(3,0,"  Status_1_3    ");
			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 6; break;
        				case '8' : Menu_Status = 5; break;
        				case '6' :Menu_Status = 0; break;
						case'4'  :Menu_Status = 1; break;
						default:     Menu_Status = 4; break;
						}
		};break;
        case 5: {
			PutStr(0,0,"   *Status_1*   ");
			PutStr(1,0,"  Status_1_1    ");
			PutStr(2,0," ->Status_1_2   ");
			PutStr(3,0,"  Status_1_3    ");
			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 4; break;
        				case '8' : Menu_Status = 5; break;
        				case '6' :Menu_Status = 0; break;
						case'4'  :Menu_Status = 1; break;
						default:     Menu_Status = 5; break;
						}
		};break;
        case 6 : {
			PutStr(0,0,"   *Status_1*   ");
			PutStr(1,0,"  Status_1_1    ");
			PutStr(2,0,"  Status_1_2    ");
			PutStr(3,0," ->Status_1_3   ");
			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 5; break;
        				case '8' : Menu_Status = 4; break;
        				case '6' :Menu_Status = 0; break;
						case'4'  :Menu_Status = 1; break;
						default:     Menu_Status = 6; break;
						}
		};break;
        case 7: {
			PutStr(0,0,"   *Status_2*   ");
			PutStr(1,0," ->Status_2_1   ");
			PutStr(2,0,"  Status_2_2    ");
			PutStr(3,0,"  Status_2_3    ");
			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 9; break;
        				case '8' : Menu_Status = 8; break;
        				case '6' :Menu_Status = 0; break;
						case'4'  :Menu_Status = 2; break;
						default:     Menu_Status = 7; break;
						}
		};break;
        case 8 : {
			PutStr(0,0,"   *Status_2*   ");
			PutStr(1,0,"  Status_2_1    ");
			PutStr(2,0," ->Status_2_2   ");
			PutStr(3,0,"  Status_2_3    ");
			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 7; break;
        				case '8' : Menu_Status = 9; break;
        				case '6' :Menu_Status = 0; break;
						case'4'  :Menu_Status = 2; break;
						default:     Menu_Status = 8; break;
						}
		};break;
        case 9: {
			PutStr(0,0,"   *Status_2*   ");
			PutStr(1,0,"  Status_2_1    ");
			PutStr(2,0,"  Status_2_2    ");
			PutStr(3,0," ->Status_2_3   ");
			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 8; break;
        				case '8' : Menu_Status = 7; break;
        				case '6' :Menu_Status = 0; break;
						case'4'  :Menu_Status = 2; break;
						default:     Menu_Status = 9; break;
						}
		};break;
        case 10: {
			PutStr(0,0,"   *Status_3*   ");
			PutStr(1,0," ->Status_3_1   ");
			PutStr(2,0,"  Status_3_2    ");
			PutStr(3,0,"  Status_3_3    ");
			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 12; break;
        				case '8' : Menu_Status = 11; break;
        				case '6' :Menu_Status = 0; break;
						case '4'  :Menu_Status = 3; break;
						default:     Menu_Status = 10; break;
						}
		};break;
        case 11:{
			PutStr(0,0,"   *Status_3*   ");
			PutStr(1,0,"  Status_3_1    ");
			PutStr(2,0," ->Status_3_2   ");
			PutStr(3,0,"  Status_3_3    ");
			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 10; break;
        				case '8' : Menu_Status = 12; break;
        				case '6':Menu_Status = 0; break;
						case'4'  :Menu_Status = 3; break;
						default:     Menu_Status = 11; break;
						}
		};break;
        case 12 :{
			PutStr(0,0,"   *Status_3*   ");
			PutStr(1,0,"  Status_3_1    ");
			PutStr(2,0,"  Status_3_2    ");
			PutStr(3,0," ->Status_3_3   ");
			switch(KeyBoardScan()){
   					    case '2' :   Menu_Status = 11; break;
        				case '8' : Menu_Status = 10; break;
        				case '6' :Menu_Status = 0; break;
						case'4'  :Menu_Status = 3; break;
						default:     Menu_Status = 12; break;
						}
		};break;
	default:     Menu_Status = 0; break;
    }
}
}

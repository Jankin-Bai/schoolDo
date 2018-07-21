#include <8052.h>
#include <stdlib.h>
#include <stdio.h>
void _nop_(void);
__sfr __at (0xBC) ADC_CONTR; //A/D 转换控制寄存器 ADC_POWER SPEED1 SPEED0 ADC_FLAG ADC_START CHS2 CHS1 CHS0 0000,0000
__sfr __at (0xBD) ADC_RES ;  //A/D 转换结果高8位 ADCV.9 ADCV.8 ADCV.7 ADCV.6 ADCV.5 ADCV.4 ADCV.3 ADCV.2  0000,0000
__sfr __at (0xBE) ADC_RESL;  //A/D 转换结果低2位                                           ADCV.1 ADCV.0  0000,0000


//                                      7     6     5     4     3     2     1     0         Reset Value
__sfr __at (0x80) P0; //8 bitPort0          P0.7  P0.6  P0.5  P0.4  P0.3  P0.2  P0.1  P0.0       1111,1111
__sfr __at (0x94) P0M0;//                                                                         0000,0000
__sfr __at (0x93) P0M1;//                                                                         0000,0000
__sfr __at (0x90) P1; //8 bitPort1          P1.7  P1.6  P1.5  P1.4  P1.3  P1.2  P1.1  P1.0       1111,1111
__sfr __at (0x92) P1M0;//                                                                         0000,0000
__sfr __at (0x91) P1M1;//                                                                         0000,0000
__sfr __at (0x9D) P1ASF;//P1 analog special function
__sfr __at (0xA0) P2;//8 bitPort2          P2.7  P2.6  P2.5  P2.4  P2.3  P2.2  P2.1  P2.0       1111,1111
__sfr __at (0x96) P2M0;//                                                                         0000,0000
__sfr __at (0x95) P2M1;//                                                                         0000,0000
__sfr __at (0xB0) P3;//8 bitPort3          P3.7  P3.6  P3.5  P3.4  P3.3  P3.2  P3.1  P3.0       1111,1111
__sfr __at (0xB2) P3M0;//                                                                         0000,0000
__sfr __at (0xB1) P3M1;//                                                                         0000,0000
__sfr __at (0xC0) P4;//8 bitPort4          P4.7  P4.6  P4.5  P4.4  P4.3  P4.2  P4.1  P4.0       1111,1111
__sfr __at (0xB4) P4M0;//                                                                         0000,0000
__sfr __at (0xB3) P4M1;//                                                                         0000,0000
//                                      7      6         5         4      3     2     1     0     Reset Value
__sfr __at (0xBB) P4SW;//Port-4 switch     -   LVD_P4.6  ALE_P4.5  NA_P4.4   -     -     -     -     x000,xxxx

__sfr __at (0xC8) P5;//8 bitPort5           -     -       -      -    P5.3  P5.2  P5.1  P5.0    xxxx,1111
__sfr __at (0xCA) P5M0;//                                                                         0000,0000
__sfr __at (0xC9) P5M1;//                                                                         0000,0000
/*--------------------------------------------------------------------------------*/


	#define OSC_FREQ (11059200UL)
	#define OSC_PER_INST (1)

	#define PRELOAD01  (65536 - (unsigned int)(OSC_FREQ / (OSC_PER_INST * 1063)))    
	#define PRELOAD01H (PRELOAD01 / 256)
	#define PRELOAD01L (PRELOAD01 % 256)

#define u8  unsigned char
#define u16 unsigned short
#define u32 unsigned int

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
/*	#define KeyPort P1

	#define key_state_0 0      //定义按键状态（基于状态机）
	#define key_state_1 1
	#define key_state_2 2
	#define key_state_3 3

	#define NO_key        0

	//按键扫描函数
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
		//状态零为初始状态
		case key_state_0:                                    
			if(key_temp != 0xff)
				key_status = key_state_1;
		break;
					
		//状态一为按键按下状态
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
							key_num = '*';                  //
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

		//状态二表示确实有按键按下
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

*/
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
 

/*由于12C单片机不支持AD相关寄存器的位寻址,需使用“|”对寄存器执行位,故定义一下内容*/

/*“|”说明,之后不再提到X指未知量0x80=1000 0000B 0xXX | 0x80 = 1XXX XXXXB*/ /*“&”说明,0xEF=0111 1111B 0xXX & 0xEF=0XXX XXXXB*/

#define ADC_POWER 0x80 //ADC power control bit

#define ADC_FLAG 0x10 //ADC complete flag

#define ADC_START 0x08 //ADC start control bit

#define ADC_SPEEDLL 0x00 //540 clocks

#define ADC_SPEEDL 0x20 //360 clocks

#define ADC_SPEEDH 0x40 //180 clocks

#define ADC_SPEEDHH 0x60 //90 clocks

//AD的初始化

void InitADC(){
	P1ASF = 0xff; //Set all P1 as analog input port 0xff=1111 1111B 即P1全部用作AD,使用时根据实际情况赋值
	ADC_RES = 0; //清空转换结果存储寄存器
	ADC_RESL=0;
	ADC_CONTR = 0x00;
	_nop_;_nop_;_nop_;_nop_;//等待ADC_CONTR值写入
}

//AD转换函数编写时,我们设计函数返回值为转换结果,函数参数包括要转换的引脚、转换速率,如果全部程序AD转换速率不变,可直接将speed的值在初始化时写入,

unsigned int GetADC(unsigned char ch,unsigned char speed){
		unsigned int res;
		ADC_CONTR =ADC_CONTR | ADC_POWER | speed | ADC_START | ch;
		_nop_;_nop_;_nop_;_nop_;//确保ADC_CONTR的值写入

		while(!(ADC_CONTR & 0x10)); //如果AD转换未结束FLAG位为0,程序在此等待,如果为1,跳出循环
		res=ADC_RES*4+ADC_RESL; //读AD转换结果,公式自己理解
		ADC_RES=0;
		ADC_RESL=0;
		ADC_CONTR=0; //寄存器复位
	return res;

}
#define N 5
unsigned char value_buff[N];
unsigned char i=0;
unsigned char filter()
{
     unsigned char count;
     unsigned int sum=0;
     value_buff[i++]=get_data();
     if(i==N)
         i=0;
for(count=0;count<N;count++)
       sum=value_buff[count];
return (unsigned char)(sum/N);
}

/*
unsigned short	Get_ADC10bitResult(unsigned char channel)	//channel = 0~7
{
	ADC_RES = 0;
	ADC_RESL = 0;

	ADC_CONTR = (ADC_CONTR & 0xe0) | 0x08 | channel; 	//start the ADC
	_nop_;_nop_;_nop_;_nop_;

	while((ADC_CONTR & 0x10) == 0)	;	//wait for ADC finish
	ADC_CONTR &= ~0x10;		//清除ADC结束标志
	return	(((unsigned short)ADC_RES << 2) | (ADC_RESL & 3));			
}
*/

void main( void )
{     
    unsigned char buf[30];
	//LCD初始化
	LcmInit();
    LcmClearTXT();
	//IO口初始化
	P0M1 = 0;	P0M0 = 0;	//设置为准双向口
	P1M1 = 0;	P1M0 = 0;	//设置为准双向口
	P2M1 = 0;	P2M0 = 0;	//设置为准双向口
	P3M1 = 0;	P3M0 = 0;	//设置为准双向口
	P4M1 = 0;	P4M0 = 0;	//设置为准双向口
	P5M1 = 0;	P5M0 = 0;	//设置为准双向口
	
	//ADC初始化
	P1M1 |= (1<<3);		// 把ADC口设置为高阻输入
	P1M0 &= ~(1<<3);
	//P1ASF = (1<<3);		//P1.3做ADC
	//ADC_CONTR = 0xE0;	//90T, ADC power on
    InitADC();

	 while(1)
     {  
		
		unsigned char Fr;
	 	unsigned char V;
		
		/*
		unsigned char j;
		u16	msecond;

		u16	Bandgap;	//


		Fr=Get_ADC10bitResult(1);
	 	V=Get_ADC10bitResult(2);
		P1ASF = 0;
				Get_ADC10bitResult(0);	//改变P1ASF后先读一次并丢弃结果, 让内部的采样电容的电压等于输入值.
				Bandgap = Get_ADC10bitResult(0);	//读内部基准ADC, P1ASF=0, 读0通道
				P1ASF = 1<<3;
				j = Get_ADC10bitResult(3);	//读外部电压ADC
				j = (u16)((u32)j * 123 / Bandgap);	//计算外部电压, Bandgap为1.23V, 测电压分辨率0.01V
		*/

		Fr=GetADC(0,00);
		V=filter(GetADC(3,00));
		PutStr(0,0,"      f-V       ");
		sprintf(buf, " f = %d  ",Fr);
		PutStr(1,0,buf);
		sprintf(buf, " V = %d  ",V);
		PutStr(2,0,buf);
		PutStr(3,0,"****************");
		PutStr(0,0,"                ");		//消隐
		PutStr(1,0,"                ");		//消隐
		PutStr(2,0,"                ");		//消隐
		PutStr(3,0,"                ");		//消隐
	
     }
}

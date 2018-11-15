
//传感器
#include "DHT11.h"
#include <U8glib.h>
#include <Wire.h>
#include <BH1750FVI.h>
#include "SoftwareSerial.h"
//SLC->A5,SDA->A4,AD0->GND
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);


#define CO2_IN 10

const byte rxPin = 12;
const byte txPin = 13;
SoftwareSerial mySerial(rxPin, txPin); // RX, TX
int preheatSec = 120;      //�A��r�g
byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
unsigned char response[9]; 
uint16_t ppm_uart, ppm_pwm=0, th, tl;
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);  // HW SPI Com: CS = 10, A0 = 9 (Hardware Pins are  SCK = 13 and MOSI = 11)

/*
static unsigned char gImage_fire[] U8G_PROGMEM ={ 
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X01,0XC0,0X00,
0X00,0X02,0X60,0X00,0X00,0X04,0X20,0X00,0X00,0X08,0X10,0X00,0X00,0X08,0X10,0X00,
0X03,0XF8,0X1F,0XC0,0X04,0X1E,0X78,0X20,0X04,0X11,0X88,0X20,0X04,0X16,0X68,0X20,
0X04,0X18,0X18,0X20,0X06,0X30,0X0C,0X60,0X02,0X71,0XCE,0X40,0X01,0X93,0XC9,0X80,
0X01,0X93,0XC9,0X80,0X02,0X51,0XCA,0X40,0X06,0X30,0X0C,0X60,0X04,0X18,0X18,0X20,
0X04,0X16,0X68,0X20,0X04,0X11,0X88,0X20,0X04,0X1E,0X78,0X20,0X03,0XF8,0X1F,0XC0,
0X00,0X08,0X10,0X00,0X00,0X08,0X10,0X00,0X00,0X04,0X20,0X00,0X00,0X02,0X60,0X00,
0X00,0X01,0XC0,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
};
*/


uint16_t lux;

/*
A�����ƣ���λֵ�˲���
B��������
    ��������N�Σ�Nȡ����������N�β���ֵ����С���У�
    ȡ�м�ֵΪ������Чֵ��
C���ŵ㣺
    ����Ч�˷���żȻ��������Ĳ������ţ�
    ���¶ȡ�Һλ�ı仯�����ı�����������õ��˲�Ч����
D��ȱ�㣺
    ���������ٶȵȿ��ٱ仯�Ĳ������ˡ�
E������shenhaiyu 2013-11-01
*/

int HUMI_Buffer_Int_Value, TEM_Buffer_Int_Value, lux_Value, ppm_uart_Value,ppm_pwm_Value;

// ��λֵ�˲���
#define FILTER_N 101
int Filter(unsigned int Get_input) {
  int filter_buf[FILTER_N];
  int i, j;
  int filter_temp;
  for(i = 0; i < FILTER_N; i++) {
    filter_buf[i] = Get_input;
    delay(1);
  }
  // ����ֵ��С�������У�ð�ݷ���
  for(j = 0; j < FILTER_N - 1; j++) {
    for(i = 0; i < FILTER_N - 1 - j; i++) {
      if(filter_buf[i] > filter_buf[i + 1]) {
        filter_temp = filter_buf[i];
        filter_buf[i] = filter_buf[i + 1];
        filter_buf[i + 1] = filter_temp;
      }
    }
  }
  return filter_buf[(FILTER_N - 1) / 2];
}

//OLED��ʾ
void draw(void) {
  //u8g.drawRFrame(0,0,127,63,5);
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_osb21);
  u8g.setPrintPos(0,14);
  u8g.print("pH: ");u8g.print("NO DEVICE");
  u8g.setPrintPos(0,26);
  u8g.print("Hu: ");if(HUMI_Buffer_Int == 255)u8g.print("NO DEVICE");else{u8g.print(HUMI_Buffer_Int);u8g.print("%");if((HUMI_Buffer_Int_Value>=60)&&(HUMI_Buffer_Int_Value<=65)){u8g.print(" ChuSi");} else if((HUMI_Buffer_Int_Value<=60)||((HUMI_Buffer_Int_Value>=65)&&(HUMI_Buffer_Int_Value<=85))){u8g.print(" Water...");} else if((HUMI_Buffer_Int_Value>=85)&&(HUMI_Buffer_Int_Value<=95)){u8g.print(" ChuJun");}else{u8g.print(" enough");}}//   HUMI_Buffer_Int_Value,TEM_Buffer_Int_Value,lux_Value;
  u8g.setPrintPos(0,38);
  u8g.print("Te: ");if(TEM_Buffer_Int == 255)u8g.print("NO DEVICE");else{u8g.print(TEM_Buffer_Int);u8g.print("C"); if(TEM_Buffer_Int_Value<=25){u8g.print(" Up");}else if(TEM_Buffer_Int_Value>=28){u8g.print(" Down");}else{u8g.print(" OK");}}
  //u8g.drawBitmapP(100,12,4,32,gImage_fire);
  u8g.setPrintPos(0,50);
  u8g.print("Li: ");if(lux == 65534)u8g.print("NO DEVICE");else{u8g.print(lux);u8g.print("lux");if(lux_Value<1000){u8g.print(" light");}else{u8g.print(" OK");}}  
  u8g.setPrintPos(0,62);
  u8g.print("CO2:");
  if(ppm_uart == 0)u8g.print("NO DEVICE");
  else{
    if (preheatSec > 0) {
     u8g.print(" ... ");
     u8g.print(preheatSec);
      preheatSec--;
    }
    else {      
    u8g.print(" ");u8g.print(ppm_uart);u8g.print("ppm");
    }
  }
   
    
}

void setup(void) {
  mySerial.begin(9600);
  pinMode(CO2_IN, INPUT);
    
    //����ǿ�ȴ�����
  LightSensor.begin();
    if ( u8g.getMode() == U8G_MODE_R3G3B2 ) 
    u8g.setColorIndex(255);     // white
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT )
    u8g.setColorIndex(3);         // max intensity
  else if ( u8g.getMode() == U8G_MODE_BW )
    u8g.setColorIndex(1);         // pixel on
}

void loop(void) {
  mySerial.write(cmd,9);
  memset(response,0,9);
  mySerial.readBytes(response, 9);
  if((response[0]!=0xff)||(response[1]!=0x86)){
   ppm_uart=-1; 
  }
  unsigned int responseHigh = (unsigned int) response[2];
  unsigned int responseLow = (unsigned int) response[3];
   ppm_uart = (256*responseHigh)+responseLow;
 
  
  // picture loop
 
 lux = LightSensor.GetLightIntensity();

  DHT11_Read();
  delay(500);
  
  HUMI_Buffer_Int_Value = Filter(HUMI_Buffer_Int);       
  TEM_Buffer_Int_Value = Filter(TEM_Buffer_Int);
  lux_Value = Filter(lux);
  ppm_uart_Value = Filter(ppm_uart);
  
  
  
  u8g.firstPage();  
  do {
     
    draw();
 } while( u8g.nextPage() );

}
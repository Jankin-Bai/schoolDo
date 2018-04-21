/*------------------------------------------------*-

			ReadKeyBoard.C

			键盘扫描函数----->FSM
-*------------------------------------------------*/


#include	"Main.h"
#include	"Port.h"

char read_keyboard()
{
    static char key_state = 0, key_value, key_line;
    char key_return = No_key, i;
    switch (key_state) {
    case 0:
        key_line = B00001000;
        for (i = 1; i <= 4; i++) { 			//扫描键盘
            PORTD = ~key_line; 					//输出行线电平
            PORTD = ~key_line; 					//必须送 2 次！！！（注 1）
            key_value = Key_mask & PIND; // 读列电平
            if (key_value == Key_mask)
                key_line <<= 1; // 没有按键，继续扫描
            else {
                key_state++; // 有按键，停止扫描
                break; // 转消抖确认状态
            }
        }
        break;
    case 1:
        if (key_value == (Key_mask & PIND)) { // 再次读列电平，
            switch (key_line | key_value) { // 与状态 0 的相同，确认按键
            // 键盘编码，返回编码值
            case B00001110:
                key_return = K1_1;
                break;
            case B00001101:
                key_return = K1_2;
                break;
            case B00001011:
                key_return = K1_3;
                break;
            case B00010110:
                key_return = K2_1;
                break;
            case B00010101:
                key_return = K2_2;
                break;
            case B00010011:
                key_return = K2_3;
                break;
            case B00100110:
                key_return = K3_1;
                break;
            case B00100101:
                key_return = K3_2;
                break;
            case B00100011:
                key_return = K3_3;
                break;
            case B01000110:
                key_return = K4_1;
                break;
            case B01000101:
                key_return = K4_2;
                break;
            case B01000011:
                key_return = K4_3;
                break;
            }
            key_state++; // 转入等待按键释放状态
        } else
            key_state--; // 两次列电平不同返回状态 0，（消抖处理）
        break;
    case 2: // 等待按键释放状态
        PORTD = B00000111; // 行线全部输出低电平
        PORTD = B00000111; // 重复送一次
        if ( (Key_mask & PIND) == Key_mask)
            key_state = 0; // 列线全部为高电平返回状态 0
        break;
    }
    return key_return;
}
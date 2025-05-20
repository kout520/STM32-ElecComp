#include "headfile.h"

//定义矩形阵列引脚
#define ROW1_PIN GPIO_PIN_0
#define ROW2_PIN GPIO_PIN_1
#define ROW3_PIN GPIO_PIN_2
#define ROW4_PIN GPIO_PIN_3
#define COL1_PIN GPIO_PIN_4
#define COL2_PIN GPIO_PIN_5
#define COL3_PIN GPIO_PIN_6
#define COL4_PIN GPIO_PIN_7
#define ROW_PORT GPIOC
#define COL_PORT GPIOC

// 键盘映射4x4
char key_map[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
char key;
  char msg[20];

// 键盘扫描函数
char scan_keypad(void) {
    uint16_t row_pins[4] = {ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN};
    uint16_t col_pins[4] = {COL1_PIN, COL2_PIN, COL3_PIN, COL4_PIN};

    // 遍历每一行
    for (int row = 0; row < 4; row++) {
        // 将当前行拉低，其余行拉高
        for (int i = 0; i < 4; i++) {
            HAL_GPIO_WritePin(ROW_PORT, row_pins[i], (i == row) ? GPIO_PIN_RESET : GPIO_PIN_SET);
        }

        // 检测列的状态
        for (int col = 0; col < 4; col++) {
            if (HAL_GPIO_ReadPin(COL_PORT, col_pins[col]) == GPIO_PIN_RESET) {
                return key_map[row][col]; // 返回按键值
            }
        }
    }

    return 0; // 未检测到按键
}

void key_handle(void)
{
    key = scan_keypad();
    if (key != 0) { // 如果有按键被按下
        switch (key) {
            case '1':
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
                break;
            case '2':
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
                break;
            case '3':
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
                break;
            case '4':
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
                break;
            case '5':
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
                break;
            case '6':
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
                break;
            case '7':
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
                break;
            case '8':
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
                break;
            case '9':
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
                break;
            case 'A':
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
                break;
            case 'B':
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
                break;
//后续自己加
            default:
                break;
        }
    }
}





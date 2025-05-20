//使用说明
//根据不同摇杆可以调节g_iAdcx的取值范围相对应
//这里我将读取函数放入定时器中断里了（HAL_TIM_Base_Start_IT(&htim6);）自行更改
//首先开始ADC  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)g_iAdcx, sizeof(g_iAdcx) / sizeof(g_iAdcx[0]));
//        // 获取摇杆状态
//        E_RKEYState currentState = Get_KeyState();

        // 调用 Test 函数，根据状态执行操作
//        Test(currentState);
//
//
//
//

#include "headfile.h"

xyzVal_struct xyzVal;
uint16_t g_iAdcx[2];
uint8_t sendbuf_y = 1;
uint8_t sendbuf_x = 1;
/*函数名称:获取xyz轴的值*/
/*形参:无              */
/*返回值:无            */
void Get_xyzVal(void)
{ 
  
  if(g_iAdcx[0]<700 )          //y0
  xyzVal.Y = Yup;
  else if(g_iAdcx[0]<1530 && g_iAdcx[0]>700)  //y1
  xyzVal.Y = Ysup; 
  else if(g_iAdcx[0]<1730 && g_iAdcx[0]>1530)  //y2
  xyzVal.Y = Ysp; 
  else if(g_iAdcx[0]<2000 && g_iAdcx[0]>1730)  //y3
  xyzVal.Y = Ysdn; 
  else if(g_iAdcx[0]<4096 && g_iAdcx[0]>2000)  //y4
  xyzVal.Y = Ydn;
  
  
  if(g_iAdcx[1]<800 )          //x0
  xyzVal.X = Xlf;
  else if(g_iAdcx[1]<3000 && g_iAdcx[1]>800)  //x1
  xyzVal.X = Xslf;  
  else if(g_iAdcx[1]<3300 && g_iAdcx[1]>3000)  //x2
  xyzVal.X = Xsp;   
  else if(g_iAdcx[1]<3500 && g_iAdcx[1]>3300)  //x3
  xyzVal.X = Xsri;
  else if(g_iAdcx[1]<4096 && g_iAdcx[1]>3500)  //x4
  xyzVal.X = Xri; 
  
  //下压按键(本次不加入考虑）
  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_RESET)
    xyzVal.Z = Zdn;    
  else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET)
    xyzVal.Z = Zup;
}

/*函数名称:获取摇杆的状态 */
/*形参:无                */
/*返回值:keystate        */
/*说明:keystate为E_RKEYState的变量，返回按键状态值*/

E_RKEYState Get_KeyState(void)
{
  E_RKEYState keystate;
  Get_xyzVal(); 
  //摇杆状态编码
  if((xyzVal.Y == Ysp) && (xyzVal.X == Xsp))//&& (xyzVal.Z == Zup)) //x2 y2 z0 0
  keystate = KEYSTOP; 
  else if((xyzVal.Y == Yup) && (xyzVal.X == Xsp))                 //x2 y0 z0 1
  keystate = KEYUP;            
  else if((xyzVal.Y == Ysup) && (xyzVal.X == Xsp))                //x2 y1 z0 2
    keystate = KEYSUP;                                        
  else if((xyzVal.Y == Ydn) && (xyzVal.X == Xsp))                 //x2 y4 z0 3
    keystate = KEYDOWN;                                        
  else if((xyzVal.Y == Ysdn) && (xyzVal.X == Xsp))                //x2 y3 z0 4
    keystate = KEYSDOWN;                                       
  else if((xyzVal.Y == Ysp) && (xyzVal.X == Xlf))                 //x0 y2 z0 5
    keystate = KEYLEFT;                                        
  else if((xyzVal.Y == Ysp) && (xyzVal.X == Xslf))                //x1 y2 z0 6
    keystate = KEYSLEFT;                                       
  else if((xyzVal.Y == Ysp) && (xyzVal.X == Xri))                 //x4 y2 z0 7
    keystate = KEYRIGHT;                                      
  else if((xyzVal.Y == Ysp) && (xyzVal.X == Xsri))                //x3 y2 z0 8
    keystate = KEYSRIGHT;                                    
  else if(xyzVal.Z == Zdn)                                        //x2 y2 z1 9
    keystate = KEYPRESS;
  else if(((xyzVal.Y==Yup)&&(xyzVal.X==Xlf))||((xyzVal.Y==Yup)&&(xyzVal.X==Xslf))||((xyzVal.Y==Ysup)&&(xyzVal.X==Xlf)))                 //x01 y01 z0 10
    keystate = KEYUPLF;
  else if((xyzVal.Y == Ysup) && (xyzVal.X == Xslf))               //x1 y1 z0 11
    keystate = KEYSUPLF;
  else if(((xyzVal.Y==Yup)&&(xyzVal.X==Xri))||((xyzVal.Y==Yup)&&(xyzVal.X==Xsri))||((xyzVal.Y==Ysup)&&(xyzVal.X==Xri)))                 //x34 y01 z0 12
    keystate = KEYUPRI;
  else if((xyzVal.Y == Ysup) && (xyzVal.X == Xsri))               //x3 y1 z0 13
    keystate = KEYSUPRI;
  else if(((xyzVal.Y==Ydn)&&(xyzVal.X==Xlf))||((xyzVal.Y==Ydn)&&(xyzVal.X==Xslf))||((xyzVal.Y==Ysdn)&&(xyzVal.X==Xlf)))                 //x01 y34 z0 14
    keystate = KEYDOWNLF;
  else if((xyzVal.Y == Ysdn) && (xyzVal.X == Xslf))               //x1 y3 z0 15
    keystate = KEYSDOWNLF;
  else if(((xyzVal.Y==Ydn)&&(xyzVal.X==Xri))||((xyzVal.Y==Ydn)&&(xyzVal.X==Xsri))||((xyzVal.Y==Ysdn)&&(xyzVal.X==Xri)))                 //x34 y34 z0 16
    keystate = KEYDOWNRI;
  else if((xyzVal.Y == Ysdn) && (xyzVal.X == Xsri))               //x3 y3 z0 17
    keystate = KEYSDOWNRI;
  else
    keystate = KEYState_MAX;
  return keystate;
}

/*函数名称:测试摇杆功能   */
/*形参:E_RKEYState state */
/*返回值:无              */

void Test(E_RKEYState state)
{ 
  switch(state)
  {
    case KEYSTOP:  
       HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
        
      break;
    case KEYUP:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
        
        
    
      break;
    case KEYSUP:
        
      break;
    case KEYDOWN:
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
      break;
    case KEYSDOWN:
        
      break;
    case KEYLEFT:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
      break;
    case KEYSLEFT:
        
      break;
    case KEYRIGHT:
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
      break;
    case KEYSRIGHT:
        
      break;
    case KEYPRESS:
        
      break;
    
    case KEYUPLF:
        
      break;
    case KEYSUPLF:
        
      break;
    case KEYUPRI:
        
      break;
    case KEYSUPRI:
        
      break;
    case KEYDOWNLF:
        
      break;
    case KEYSDOWNLF:
        
      break;
    case KEYDOWNRI:
        
      break;
    case KEYSDOWNRI:
        
      break;
    case KEYState_MAX:
        
      break;
    default:
      break;
  }
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM6)
    {
        //Get_xyzVal();
        // 获取摇杆状态
        E_RKEYState currentState = Get_KeyState();

        // 调用 Test 函数，根据状态执行操作
        Test(currentState);
    }
}














#ifndef __JLNC_DAC_H__
#define __JLNC_DAC_H__

double linearmapping(double as,double ae,double bs,double be,double val);
void TIM_Config(void);

typedef struct SinWavePar {
  int length;
  uint16_t *DACbuf;
}SinWavePar;
 
SinWavePar GenerateSinWave(int f);  // 函数前向声明


#endif


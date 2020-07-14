// ***** 0. Documentation Section *****
// TableTrafficLight.c 
// Runs on TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian sensor connected to PE2 (1=pedestrian present)
// north/south car sensor connected to PE1 (1=car present)
// east/west car sensor connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"
	
struct State {
  unsigned long PBOut;  // 6-bit pattern to output
	unsigned long PFOut; // output to PF1 and PF3
  unsigned long Time; // delay in 10ms units 
  unsigned long Next[8];}; // next state for inputs 0,1,2,3,4,5,6,7,8
  typedef const struct State STyp;
  enum {GoW,WaitW,GoS,WaitS,GoPed,PedF1Off,PedF1On,PedF2Off,PedF2On};

STyp FSM[9]={
 {0x0C,0x02,5,{GoW,GoW,WaitW,WaitW,WaitW,WaitW,WaitW,WaitW}}, 
 {0x14,0x02,5,{GoS,GoS,GoS,GoS,GoPed,GoPed,GoS,GoS}},
 {0x21,0x02,5,{GoS,WaitS,GoS,WaitS,WaitS,WaitS,WaitS,WaitS}},
 {0x22,0x02,5,{GoPed,GoW,GoPed,GoW,GoPed,GoPed,GoPed,GoPed}},
 {0x24,0x08,5,{GoPed,PedF1Off,PedF1Off,PedF1Off,GoPed,PedF1Off,PedF1Off,PedF1Off}},
 {0x24,0x00,5,{PedF1On,PedF1On,PedF1On,PedF1On,PedF1On,PedF1On,PedF1On,PedF1On}},
 {0x24,0x02,5,{PedF2Off,PedF2Off,PedF2Off,PedF2Off,PedF2Off,PedF2Off,PedF2Off,PedF2Off}},
 {0x24,0x00,5,{PedF2On,PedF2On,PedF2On,PedF2On,PedF2On,PedF2On,PedF2On,PedF2On}},
 {0x24,0x02,5,{GoW,GoW,GoW,GoW,GoW,GoW,GoW,GoW}}
 };
// ***** 2. Global Declarations Section *****
unsigned long S;  // index to the current state 
unsigned long Input;

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Port_E_Init(void);
void Port_B_Init(void);
void Port_F_Init(void);
void SysTick_Init(void);
void SysTick_Wait(unsigned long delay);
void SysTick_Wait500ms(unsigned long delay);

// ***** 3. Subroutines Section *****

int main(void){ 
 TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
 EnableInterrupts();
  SysTick_Init();
  Port_E_Init();
  Port_B_Init();
  Port_F_Init();
	S=GoW;
  while(1){
		GPIO_PORTB_DATA_R = FSM[S].PBOut;  
    GPIO_PORTF_DATA_R = FSM[S].PFOut; 
    SysTick_Wait500ms(FSM[S].Time);		
		Input=GPIO_PORTE_DATA_R & 0x07; // reads input from PE0-2
		S=FSM[S].Next[Input];
    }
}

void Port_E_Init(void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x10;      // 1)  E
  delay = SYSCTL_RCGC2_R;      // 2) no need to unlock
  GPIO_PORTE_AMSEL_R &= ~0x07; // 3) disable analog function on PE1-0
  GPIO_PORTE_PCTL_R &= ~0x00000FFF; // 4) enable regular GPIO
  GPIO_PORTE_DIR_R &= ~0x07;   // 5) inputs on PE1-0
  GPIO_PORTE_AFSEL_R &= ~0x07; // 6) regular function on PE1-0
  GPIO_PORTE_DEN_R |= 0x07;    // 7) enable digital on PE1-0
}
void Port_B_Init(void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x02;      // 1) B 
  delay = SYSCTL_RCGC2_R;      
	GPIO_PORTB_AMSEL_R &= ~0x3F; // 3) disable analog function on PB5-0
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // 4) enable regular GPIO
  GPIO_PORTB_DIR_R |= 0x3F;    // 5) outputs on PB5-0
  GPIO_PORTB_AFSEL_R &= ~0x3F; // 6) regular function on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;    // 7) enable digital on PB5-0
}
void Port_F_Init(void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x20;
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTF_AMSEL_R &= ~0x0A; // disable analog function on PF1 and PF3
  GPIO_PORTF_PCTL_R &=~0x0000F0F0;
	GPIO_PORTF_DIR_R |=0x0A; // outputs on PF1 and PF3
  GPIO_PORTF_AFSEL_R &= ~0x0A; // regular function on PF1 and PF3
  GPIO_PORTF_DEN_R |= 0x0A; // enable digital on PF1 and PF3
}
void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}
void SysTick_Wait(unsigned long delay){
NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}
void SysTick_Wait500ms(unsigned long delay){
	unsigned long i;
  for(i=0; i<delay; i++){
    SysTick_Wait(40000000);  // wait 5ms
	}
}

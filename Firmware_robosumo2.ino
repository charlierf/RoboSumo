#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>


//Configuração da millis
volatile unsigned long timer1_millis;
ISR(TIMER1_COMPA_vect)
{
  timer1_millis++;  
}

void inicia_millis(unsigned long f_cpu)
{
  unsigned long ctc_match_overflow;  
  ctc_match_overflow = ((f_cpu / 1000) / 8); // overflow do timer em 1ms
    
  // (limpar timer qdo cooresponder a ctc_match_overflow) | (clock divisor por 8)
  TCCR1B |= (1 << WGM12) | (1 << CS11);
  
  // high byte primeiro, depois low byte
  OCR1AH = (ctc_match_overflow >> 8);
  OCR1AL = ctc_match_overflow;
 
  // habilita compare match interrupt
  TIMSK1 |= (1 << OCIE1A); 
  
  //habilita GLOBAL INTERRUPTS 
  sei();
}

unsigned long nossamillis ()
{
  unsigned long millis_return; 
  // bloca execucao
  ATOMIC_BLOCK(ATOMIC_FORCEON) {
  millis_return = timer1_millis;
  }
  return millis_return;
} 


//Funções de controle dos motores
int forward()
{
  return B01011000;
}

int reverse()
{
  return B00100100;
}

int reverseLeft()
{
  return B00000100;
}

int reverseRight()
{
  return B00100000;
}

int left()
{
  return B00001000;
}

int right()
{
  return B00010000;
}

int neutral ()
{
  return B00000000;
}

int spinClock(){
  return B00101000; 
}

int spinAClock(){
  return B00010100;
}

int main (void) {

  inicia_millis(16000000UL);    
  unsigned long previousMillis = 0;
  Serial.begin(9600);
  DDRD = B00111100; //Configuração dos registradores 
  DDRB = B00100000; //Configuração dos registradores
  long int duracao;
  double distancia;
  



  
  while (1)
  {   
    unsigned char IR_frente = (PIND & 128) >> 7;
    unsigned char IR_fundo = (PIND & 64) >> 6;
    unsigned long currentMillis = nossamillis();

//Prioridade verificar os sensores IR. Somente após isso, verificar outros sensores.
if ((IR_frente != 0) || (IR_fundo != 0)){ 
  //Caso o IR da frente detecte a borda preta, motores no reverso
    if (IR_frente == 1){
      PORTD = reverse();
    } 

 //Caso o IR do fundo detecte a borda preta, motores para frente
    if (IR_fundo == 1) {
      PORTD = forward(); 
    }
 //Caso ambos os sensores IR detectem algo, desligar motores
    if ((IR_frente == 1) && (IR_fundo == 1)){
      PORTD = neutral();
    }
} else { //Somente se não houver detecção nos sensores IR

   PORTB = 0B00000000;  
   delayMicroseconds(2);   
   PORTB = 0B00101000;  //Disparo trigger nos dois sensores Ultrassom 
   delayMicroseconds(10);   
   PORTB = 0B00000000; 
   
   // detectar eco sem uso de biblioteca
   duracao = 0;

   
   // Ultrassom dianteiro.
   while (!(PINB & 16) >> 4);
   while ((PINB & 16) >> 4)
   {
     duracao++;
   }   
   if (duracao > 0)
   {
    distancia = duracao * 0.009805795; //calibracao com sensor
   }


 //Reação ao Ultrassom

 if (distancia < 4.00)
   {
    if (currentMillis - previousMillis >= 500){
      previousMillis = currentMillis;
      PORTD = reverse(); //Ir para trás por 500ms se detectar algo na sua frente.
      distancia = 0.00;
    }
   }
   

//Ação padrão enquanto nenhum sensor detecta nada

if (currentMillis - previousMillis >= 2000){ //2000 milissegundos para cada estado     
      previousMillis = currentMillis;
      if (PORTD == B00000100) {
        PORTD = neutral();
      } else {
       
      switch (PORTD) 
      {
        case B00000000:
        PORTD = forward();
        break;
        
        case B01011000:
        PORTD = spinClock();
        break;
        
        case B00101000:
        PORTD = left();
        break;
        
        case B00001000:
        PORTD = spinAClock();
        break;
        
        case B00010100:
        PORTD = right();
        break;
        
        case B00010000:
        PORTD = forward();
        break;

        case B00100100:
        PORTD = spinAClock();
        break;

        default:
        PORTD = forward();
        break;
      
   
}}}}}}

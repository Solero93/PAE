/******************************
 *
 * PR�CTICA 4_PIH Programaci� de Ports i pr�ctica de: "do-while" "case" "if" i "for"
 *
 *****************************/

#include <msp430x54xA.h>
#include <stdio.h>
#include "hal_lcd.h"
#include "envio_recibir_instruc.h"
#include "control_pantalla.h"

#include "motores.h"
#include "sensor.h"

long int i;

byte referencia;

int isNotReferencia, bparar;
long int calcul;


/**************************************************************************
 * INICIALIZACI�N DE LOS BOTONES.
 * Configuramos botones y leds:
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/

void init_botons(void)
{
  //Configuramos botones y leds:
  P1DIR |= 0x03;	//Puertos P1.0 y P1.1 como salidas (Leds)
  P1OUT |= 0x01;	//Inicializamos puerto P1.0 a 1,
  P1OUT &= 0xFD;	// y P1.1 a 0, para leds en alternancia


  P2DIR &= ~0xC0;	//Puertos P2.6 y P2.7 como entradas (botones S1 y S2)
  P2SEL &= ~0xC0;	//Puertos P2.6 y P2.7 como I/O digitales,
  P2REN |= 0xC0;	//con resistencia activada
  P2OUT |= 0xC0;	// de pull-up
  P2IE |= 0xC0; 	//Interrupciones activadas en P2.6 y P2.7,
  P2IES &= ~0xC0;	// con transicion L->H


  //Configuramos el joystick:
  P2DIR &= ~0x3E;	//Puertos P2.1 a P2.5 como entradas (joystick)
  P2SEL &= ~0x3E;	//Puertos P2.1 y P2.5 como I/O digitales,
  P2REN |= 0x3E;	//con resistencia activada
  P2OUT |= 0x3E;	// de pull-up
  P2IE |= 0x3E; 	//Interrupciones activadas en P2.1 a P2.5,
  P2IES &= ~0x3E;	// con transicion L->H

}


void escribirRx(struct RxReturn respuesta ){
	byte bCount;
    for(bCount = 0; bCount < respuesta.StatusPacket[3]; bCount++){

    sprintf(cadena," %x ", respuesta.StatusPacket[bCount+4]);
    escribir(cadena, bCount+1);
    }
}

void escribirRxSensor(int linea, struct RxReturn respuesta ,char on){

    sprintf(cadena," %x ", respuesta.StatusPacket[5]);
    escribir(cadena, linea);
}

void tratarRespuesta(byte respuesta) {

	 if(((respuesta & 2) == 2) && ((respuesta & referencia) == referencia)){//
		 	 girar_90_derecha();
			recolocar_izquierda();
		}
	else if ((respuesta & referencia) != referencia){
		if(referencia==4) {//referencia derecha
			girar_derecha();
		}else if(referencia==1){//referencia izquierda

			girar_izquierda();
		}
	}

	else if((respuesta & 2) == 2){//
		recolocar_izquierda();
	}

	else if((respuesta & 4) == 4){//
		girar_izquierda();
	}

	else{
		mover_delante();
	}
}

void recolocar_izquierda(){
	mover_delante();
	while(obstacle_detection() & 1 == 1);
	girar_derecha();
	while(obstacle_detection() & 1 != 1);
//	parar();
}

void recolocar_dercha(){
	girar_izquierda();
	while(obstacle_detection() != 4);
	girar_derecha();
	while(obstacle_detection() == 4);
	parar();
}

void initRef(){
	int respuesta;
	while(isNotReferencia){
		respuesta= obstacle_detection();
		if(respuesta & 1 == 1){
			recolocar_izquierda();
		}else if(respuesta & 4 == 4){
			recolocar_derecha();
		}else if(respuesta & 2 == 2){
			//TODO
		}else {
			 mover_delante();
		}

	}
}
/************
 * MAIN
 ************
 */
void main(void)
{
	init_UCS();
	init_UART();
   	WDTCTL = WDTPW+WDTHOLD;       	// Paramos el watchdog timer
   	isNotReferencia=1;
   	bparar=1;

  	init_botons();					// Iniciamos los botones y Leds.

    init_LCD();						// Inicializamos la pantalla
    _enable_interrupt();
  	linea++; 					//Aumentamos el valor de linea y con ello pasamos a la linea siguiente

  	referencia=1;

  	P4OUT = 0x01;
  	calcul=0;


  	do//do While de todo el programa
   	{
  		//do{
  		//el programa espera a una interrupcion

  			read_left();
  			read_center();
  			 read_right();
  			sprintf(cadena," %d ", obstacle_detection());
  			escribir(cadena, 4);
  		//tratarRespuesta(obstacle_detection());
  		//}while(bparar);*/
  		//sprintf(cadena," %d ", calcul);
  		//escribir(cadena, 1);
  		//calcul++;
  		tratarRespuesta(obstacle_detection());
   	}while(1);

}


/**************************************************************************
 * MINIPROGRAMA DE LOS BOTONES:
 * Mediante este programa, se detectar� que bot�n se ha pulsado
 *
 * Sin Datos de entrada
 *
 * Sin datos de salida
 *
 * Actualiza el valor estado
 *
 **************************************************************************/

#pragma vector=PORT2_VECTOR  //interrupci�n de los botones. Actualiza el valor de la variable global estado.
__interrupt void Port2_ISR(void)
{
	P2IE &= 0xC0; 	//interrupciones botones S1 y S2 (P2.6 y P2.7) desactivadas
	P2IE &= 0x3E;   //interrupciones joystick (2.1-2.5) desactivadas

	/**********************************************************+
		A RELLENAR POR EL ALUMNO BLOQUE CASE

	Boton S1, estado =1 y editar horas(del reloj o la alarma)
	Boton S2, estado =2 y editar minutos(del reloj o la alrma)
	Joystick left, estado =3 y los LEDs rotan a la izquierda
	Joystick right, estado =4 y los LEDs rotan a la derecha
	Joystick center, estado = 5 y Canvia Modo a Editar Reloj/Alarma
	Joystick up, estado =6 y disminuye el timer y van mas rapidos los LEDs
	Joystick down, estado =7 y aumenta el timer y van mas lentos los LEDs
	 * *********************************************************/

	switch(P2IFG){

	case 2://Joystick a la izquierda
		girar_izquierda();
		break;
	case 4:// Joystick a l derecha
		girar_derecha();
		break;
	case 8://Joystick centro

		break;
	case 16://Joystick Arriba
		calcul=0;
		girar_izquierda();
		break;
	case 32://Joystick abajo
		parar();

		break;
	case 64://Boton S1
		parar();
		break;
	case 128://boton S2
		if(bparar==0){
			bparar=1;
		}else{
			bparar=0;
		}
		break;
	}

	/***********************************************
   	 * HASTA AQUI BLOQUE CASE
   	 ***********************************************/
	
	P2IFG = 0;		//limpiamos todas las interrupciones
	P2IE |= 0xC0; 	//interrupciones botones S1 y S2 (P2.6 y P2.7) reactivadas
	P2IE |= 0x3E;  //interrupciones joystick (2.1-2.5) reactivadas
 return;
}





#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
//interrupcion de recepcion en la uart A0
UCA0IE &= ~UCRXIE; //Interrupciones desactivadas en RX
DatoLeido_UART = UCA0RXBUF;
Byte_Recibido=1;

UCA0IE |= UCRXIE; //Interrupciones reactivadas en RX
}

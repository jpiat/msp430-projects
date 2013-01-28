#include <msp430g2553.h>
#ifdef __GNUC__
#include <legacymsp430.h>
#endif



char * Nmea_Gprmc = "$GPRMC,154936.000,A,4338.6124,N,00126.7337,E,0.26,326.08,200113,,,A*60\r\n";
unsigned int wdt_count ;


int strlen(char * data);
void uart_rx(unsigned char val);
void setup_uart_9600();
void uart_send_data(unsigned char * data, unsigned char length);
void uart_send_char(unsigned char val);
void configureWDT(void);


int strlen(char * data){
	unsigned int i = 0 ;
	while(data[i] != '\0' && i < 1000) i ++ ;
	if(i >= 1000){
		return -1 ;	
	}
	return i+1 ;
}


void uart_rx(unsigned char val){		
		
}


void setup_uart_9600(){
	P1SEL |= BIT1 | BIT2 ;
	P1SEL2 |= BIT1 | BIT2 ;
	UCA0CTL1 |= UCSSEL_2 ;
	UCA0BR0 = 130 ;
	UCA0BR1 = 6 ;
	UCA0MCTL = UCBRS1 + UCBRS2;
	UCA0CTL1 &= ~UCSWRST ;
	IE2 |= UCA0RXIE ;
}

void uart_send_data(unsigned char * data, unsigned char length){
	unsigned int i ;
	for(i = 0 ; i < length ; i ++){
		uart_send_char(data[i]);	
	}	
}

void uart_send_char(unsigned char val){
	while(UCA0STAT & UCBUSY);
	UCA0TXBUF = val ;
}


void configureWDT(void)
{
	wdt_count = 0 ;
	WDTCTL = WDT_MDLY_32; 
	IE1 |= WDTIE;
}



int main(){
	int length ;
	WDTCTL = WDTPW + WDTHOLD ;
	BCSCTL1 = CALBC1_16MHZ ;
	DCOCTL = CALDCO_16MHZ ;
	P1DIR |= BIT0 ;
	P1OUT &= ~BIT0 ;
	length = strlen(Nmea_Gprmc);
	setup_uart_9600();
	configureWDT();
	while(1){
		if(length > 0){
			uart_send_data(Nmea_Gprmc, length);		
		}
		__bis_SR_register(CPUOFF + GIE);
	}
	return 0 ;
}

#ifdef __GNUC__
interrupt(WDT_VECTOR) WDT_ISR(void)
#else
#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void)
#endif
{
	wdt_count ++ ;
	if(wdt_count > (30*16)){
		 P1OUT ^= 0x01;
		 wdt_count = 0 ;
		 __bic_SR_register_on_exit(CPUOFF);	
	}
}

#ifdef __GNUC__
interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void)
#else
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#endif
{
	if (IFG2 & UCA0RXIFG) {
		while(!(IFG2&UCA0TXIFG));
		uart_rx(UCA0RXBUF);
		IFG2 &= ~UCA0RXIFG;
	}
}



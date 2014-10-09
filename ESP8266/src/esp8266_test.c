#include <msp430g2553.h>
#include <legacymsp430.h>
#include "my_types.h"
#include "uart.h"
#include "fifo.h"
#include "access_point.h"


#define LED_ERROR BIT0
#define LED_BUSY  BIT6

#define DHCP_RETRY 10
#define CONNECT_RETRY 10
#define POST_INTERVAL 30.0
#define ERROR_INTERVAL  5.0 

//#define DEBUG 1


#define LEN_WPA_KEY strlen(WPA_KEY)
#define LEN_SSID strlen(AP_SSID)


char * request_string_base = "GET /input/";
char * request_public_key = "G2EMYvV6loI5N4wZ5WRn";
char * request_arg_prefix = "?";
char * request_private_key_base = "private_key=";
char * request_private_key = "NWnE2XMbkaUR17dZRDmv";
char * request_string_temp = "&temp=";
char * request_string_light = "&light=";
char * request_string_switch0 = "&switch0=";
char * request_string_switch1 = "&switch1=";
char * request_string_switch2 = "&switch2=";

char * request_string_end = " HTTP/1.0\n\r\n\r";

#define REQUEST_BASE_SIZE strlen(request_string_base)+strlen(request_public_key)+strlen(request_arg_prefix)
#define REQUEST_PRIVATE_KEY_SIZE strlen(request_private_key_base)+strlen(request_private_key)
#define REQUEST_STRING_ARG_SIZE strlen(request_string_temp)+strlen(request_string_light)+strlen(request_string_switch0)+strlen(request_string_switch1)+strlen(request_string_switch2)
#define REQUEST_STRING_END  strlen(request_string_end)

#define REQUEST_SIZE REQUEST_BASE_SIZE+REQUEST_PRIVATE_KEY_SIZE+REQUEST_STRING_ARG_SIZE+REQUEST_STRING_END



int strlen(char * string){
	unsigned int i = 0;
	while( i < 128 && string[i] != '\0' ) i ++ ;
	return (i < 128)? i : -1 ;
}

char buffer [10];
unsigned long int wdt_tick = 0 ;

struct my_fifo uart_fifo ;
unsigned char rcv_enabled = 0 ;


void setTimerInterval(float ms);
unsigned char intervalDone();

void wait(float sec){
	setTimerInterval((sec*1000.0)); // timer count 0.5ms
	while(intervalDone() == 0) __bis_SR_register(CPUOFF + GIE);
	/*while(sec > 0){
		__delay_cycles(16000);
		sec = sec - 0.001 ;
	}*/
}


void disable_echo(){
	rcv_enabled = 0 ;
}

void enable_echo(){
	rcv_enabled = 1 ;
}

void uart_rx(unsigned char byte){
	if(rcv_enabled == 1){
		fifo_write(&uart_fifo, byte);
	}
	return ;
}

unsigned char my_getc(){
	while(fifo_available(&uart_fifo) == 0);
	return fifo_read(&uart_fifo);	
}


void my_putc(char c){
	uart_send_char(c);
}


void flush(){
	while(fifo_available(&uart_fifo) > 0) fifo_read(&uart_fifo);
}

void issue_command(char * cmd){
    char c = '\0';
    unsigned int length = 0;
    while(cmd[length] != '\0') length ++ ;
#ifdef DEBUG
    uart_send_data("sending command\n\r", 17);
#endif
    flush();
    disable_echo();
    uart_send_data((unsigned char *) cmd, length);
    enable_echo();
    flush();
#ifdef DEBUG
    uart_send_data("sending carriage return\n\r", 25);
#endif
    my_putc('\r');
#ifdef DEBUG
    my_putc('\n');
#endif
    while(c != '\r'){
         c = my_getc();
    }
#ifdef DEBUG
    uart_send_data("command valid\n\r", 15);
#endif
}


#define TIMEOUT -1
#define NOT_CHECK 0
#define CHECK 1
char check_return(char * expected, float timeout){
    int i = 0 ;
    setTimerInterval(timeout);
    while(fifo_available(&uart_fifo) == 0){
	if(timeout > 0 && intervalDone() == 1){
		return TIMEOUT ; 
	}
    }
    if(fifo_available(&uart_fifo) > 0){
        while(1){
	    if(timeout > 0 && intervalDone() == 1){
		return TIMEOUT ; 
	    }
	    if(fifo_available(&uart_fifo) == 0) continue ;
            char c = my_getc();
            if(c == '\n' || c == '\r') continue ;
            if(c == expected[i]){
             	i ++ ;
             	if(expected[i] == '\0'){
                 	 return CHECK ;
            	}
            }else{
                return NOT_CHECK ;
            }
        }
    }
    return NOT_CHECK ;
}

unsigned int ftoa(float n, char * buffer) {
	unsigned char i = 0;
	float div = 1000;
	unsigned int f;
	unsigned char enable_zero = 0;
	char c;
	if (n < 0) {
		buffer[i] = '-';
		i++;
		n = -n;
	}
	if(n == 0){
		buffer[i] = 48 ;
		i ++ ;
		return i ;
	}
	for (div = 1000; div >= 0.001 && i < 4; div = div / 10) {
		if (div == 0.1) {
			if(!enable_zero){
				buffer[i] = '0';
				i += 1;
			}
			buffer[i] = '.';
			i += 1;
			enable_zero = 1;
		}
		f = n / div;
		if(f != 0 || enable_zero == 1){
			c = f + 48;
			buffer[i] = c;
			i++;
			n = n - (f * div);
			enable_zero = 1 ;
		}
	}
	buffer[i] = '\0';
	return (i);
}

unsigned int itoa(int n, char * buffer) {
	unsigned char i = 0;
	unsigned int div = 10000;
	unsigned int f;
	char c;
	unsigned char enable_zero = 0 ;
	if (n < 0) {
		buffer[i] = '-';
		i++;
		n = -n;
	}
	for (div = 1000; div >= 1; div = div / 10) {
		f = n / div;
		if(f == 0){
			if(enable_zero == 1){			
				c = f + 48;		
				buffer[i] = c;
				i++;
			}
		}else{
			c = f + 48;		
			buffer[i] = c;
			enable_zero = 1 ;
			i++;
		}
		n = n - (f * div);
	}
	if(i == 0){
		buffer[i] = 48 ;
		i ++ ;	
	}
	buffer[i] = '\0';
	return (i);
}

void chiptemp_setup()
{
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;
    ADC10CTL1 = INCH_10 + ADC10DIV_7;   // Channel 10 = Temp Sensor
}
 
int chiptemp_read()
{
    unsigned int adc;
    chiptemp_setup();
    // ENC = enable conversion, ADC10SC = start conversion
    ADC10CTL0 |= ENC + ADC10SC;
    while (!(ADC10CTL0 & ADC10IFG))
        /* wait until conversion is completed */ ;
 
    adc = ADC10MEM;
 
    // shut off conversion and lower flag to save power.
    // ADC10SC is reset automatically.
    while (ADC10CTL0 & ADC10BUSY)
        /* wait for non-busy per section 22.2.6.6 in User's Guide */ ;
    ADC10CTL0 &= ~ENC;
    ADC10CTL0 &= ~ADC10IFG;
 
    // return degrees C
    return (int)((adc * 27069L - 18169625L) >> 16);
}

void adc_setup()
{
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + REF2_5V;
    ADC10CTL1 = INCH_5 + ADC10DIV_7;   // Channel 5
}

int adc_read()
{
    unsigned int adc;
    adc_setup() ;
    // ENC = enable conversion, ADC10SC = start conversion
    ADC10CTL0 |= ENC + ADC10SC;
    while (!(ADC10CTL0 & ADC10IFG))
        /* wait until conversion is completed */ ;
 
    adc = ADC10MEM;
 
    // shut off conversion and lower flag to save power.
    // ADC10SC is reset automatically.
    while (ADC10CTL0 & ADC10BUSY)
        /* wait for non-busy per section 22.2.6.6 in User's Guide */ ;
    ADC10CTL0 &= ~ENC;
    ADC10CTL0 &= ~ADC10IFG;
 
    // return degrees C
    return adc ;
}

void init_switches(){
	P1DIR &= ~BIT3 ;
	P1REN |= BIT3 ;
	P1OUT |= BIT3 ;
}

void read_switches(unsigned int * tab){
	unsigned char i = 0 ;		
	for(i = 0; i < 3; i ++){
		tab[i] = 0 ;
	}
	tab[0] = ((P1IN & BIT3) != 0)? 1 : 0 ;
}

int main(){
	unsigned char count;
	unsigned char fail_counter = 0 ;
	float temp = 39.7, light=0.0 ;
	unsigned int switch_tab[3] ;
	int dummy ;
	//WDTCTL = WDTPW + WDTHOLD ;
	WDTCTL = WDT_MDLY_8 ; //using watchdog to generate interval. At 16Mhz 8ms(@1Mhz)=> 0.5ms
	IE1 |= WDTIE; //enable watchdog interrupt 
	BCSCTL1 = CALBC1_16MHZ ;
	DCOCTL = CALDCO_16MHZ ;
	setup_uart_115200(); // baudrate cannot be exactly 115200, hope for the best ...
	init_switches();

	// configure adc for light sensing with 
	P1DIR |= BIT5 ;
	P1REN |= BIT5 ; //trying to enable pull-up to act as divider bridge ...
	ADC10AE0 |= BIT5 ; // ... not sure of that

	P1DIR |= BIT3 | LED_ERROR | LED_BUSY;
	P1OUT &= ~BIT3;	 // reset module
	fifo_init(&uart_fifo);
	flush();
	__bis_SR_register(GIE);
	wait(1.0);
	do{
		P1OUT |= BIT3;	 // enable module
		P1OUT |= LED_BUSY ;
		P1OUT &= ~LED_ERROR ;
		wait(1.0);
		flush();
	    	issue_command("AT+RST");
		while(check_return("OK", 500.0) == NOT_CHECK);
		do{
			dummy = check_return("ready", 1000.0) ;
	    	}while(dummy == TIMEOUT || dummy == NOT_CHECK); // wait for error, if error keep going
	    	/*if(fail_counter > DHCP_RETRY){
			P1OUT &= ~BIT3;	
			P1OUT &= ~LED_BUSY;
			P1OUT |= LED_ERROR ;
			wait((float) ERROR_INTERVAL);
			continue ; 	
		}*/
	#ifdef DEBUG
		uart_send_data((unsigned char *)"modem ready", 11);
	#endif
		wait(0.5);
	    	uart_send_data((unsigned char *)"AT+CWJAP=", 9);
		uart_send_data((unsigned char *)AP_SSID, LEN_SSID);
		uart_send_data((unsigned char *)",", 1);
		uart_send_data((unsigned char *)WPA_KEY, LEN_WPA_KEY);
		issue_command("");
	    	check_return("OK", 500.0);
	    	flush();
	    	issue_command("AT+CIPMUX=1");
	    	check_return("OK", 500.0);
	    	flush();
		fail_counter = 0 ;
	    	do{
			wait(0.5);
	    		flush();
	    		issue_command("AT+CIFSR");
			dummy = check_return("ERROR", 500.0) ;
			if(dummy == CHECK){
				fail_counter = fail_counter + 1 ;
			}
			if(fail_counter > DHCP_RETRY){
					break ;				
			}
			#ifdef DEBUG
				if(dummy == TIMEOUT){
					uart_send_data((unsigned char *)"timeout\n", 8);
				}
			#endif
	    	}while(dummy == TIMEOUT || dummy == CHECK); // wait for error, if error keep going
		if(fail_counter > DHCP_RETRY){
			// failure on DHCP, reset connection and restart from beginning
			P1OUT &= ~LED_BUSY;
			P1OUT |= LED_ERROR ;
			wait((float) ERROR_INTERVAL);
			continue ;
		}
	    	flush();
		temp = (float) chiptemp_read();
		light = (float) adc_read();
		read_switches(switch_tab);
		fail_counter = 0 ;
	    	do{
			fail_counter = fail_counter + 1 ; // init to one to count on each loop	
			#ifdef DEBUG		
			uart_send_data((unsigned char *)"\n\r", 2);
			#endif
			wait(0.5);
	       		flush();
			issue_command("AT+CIPSTART=0,\"TCP\",\"data.sparkfun.com\",80");
			do{
				dummy = check_return("DNS Fail", 1000.0);
				fail_counter = fail_counter + 1 ;
				if(fail_counter > CONNECT_RETRY){
					break ;				
				}
			}while(dummy == TIMEOUT);
			if(fail_counter > CONNECT_RETRY){
				break ;				
			}
	    	}while(dummy == CHECK);
		if(fail_counter > CONNECT_RETRY){
			// failure on connect, reset connection and restart from beginning
			P1OUT &= ~LED_BUSY;
			P1OUT |= LED_ERROR ;
			wait((float) ERROR_INTERVAL);
			continue ;
		}
		flush();
	    	count = REQUEST_SIZE + ftoa(temp, buffer) + ftoa(light, buffer) + itoa(switch_tab[0], buffer) +itoa(switch_tab[1], buffer) + itoa(switch_tab[2], buffer);
	    	uart_send_data((unsigned char *)"AT+CIPSEND=0,", 13);
		uart_send_data((unsigned char *)buffer, itoa(count, buffer));
	    	issue_command("");
	    	if(check_return(">", 1000.0) == TIMEOUT){
			P1OUT &= ~LED_BUSY;
			P1OUT |= LED_ERROR ;
			wait((float) ERROR_INTERVAL);
			continue ; 			
		}
		disable_echo();
		// sending http request
		uart_send_data((unsigned char *) request_string_base, strlen(request_string_base));
		uart_send_data((unsigned char *) request_public_key, strlen(request_public_key));
		uart_send_data((unsigned char *) request_arg_prefix, strlen(request_arg_prefix));
		uart_send_data((unsigned char *) request_private_key_base, strlen(request_private_key_base));
		uart_send_data((unsigned char *) request_private_key, strlen(request_private_key));
		uart_send_data((unsigned char *)request_string_temp, strlen(request_string_temp));
		uart_send_data((unsigned char *)buffer, ftoa(temp, buffer));
		uart_send_data((unsigned char *)request_string_light, strlen(request_string_light));
		uart_send_data((unsigned char *)buffer, ftoa(light, buffer));
		uart_send_data((unsigned char *)request_string_switch0, strlen(request_string_switch0));
		uart_send_data((unsigned char *)buffer, itoa(switch_tab[0], buffer));
		uart_send_data((unsigned char *)request_string_switch1, strlen(request_string_switch1));
		uart_send_data((unsigned char *)buffer, itoa(switch_tab[1], buffer));
		uart_send_data((unsigned char *)request_string_switch2, strlen(request_string_switch2));
		uart_send_data((unsigned char *)buffer, itoa(switch_tab[2], buffer));
		uart_send_data((unsigned char *)request_string_end, strlen(request_string_end));    	
		enable_echo();
		// wait for unlink
		fail_counter = 0 ;
		do{	
				
	    		dummy = check_return("Unlink", 1000.0);
			if(dummy == TIMEOUT) fail_counter ++ ;
			if(fail_counter > CONNECT_RETRY) break ;
		}while(dummy == NOT_CHECK || dummy == TIMEOUT);
		if(fail_counter > CONNECT_RETRY){
			P1OUT |= LED_ERROR ;
		}else{
			P1OUT &= ~LED_ERROR ;
		}
	    	P1OUT &= ~BIT3;
		P1OUT &= ~LED_BUSY ;
		wait((float) POST_INTERVAL);
	}while(1);
}


interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void) {
	if (IFG2 & UCA0RXIFG) {
		UartRxInterruptService();
		IFG2 &= ~UCA0RXIFG;
	}
}

 
void setTimerInterval(float ms){
	unsigned long int nb_interval = (unsigned int) (ms/0.5);
	wdt_tick = nb_interval ;	
}

unsigned char intervalDone(){
	return (wdt_tick == 0) ? 1 : 0 ;
}

interrupt(WDT_VECTOR) watchdog_timer(void) {   
 	if(wdt_tick > 0) wdt_tick = wdt_tick - 1 ;
	__bic_SR_register_on_exit(CPUOFF);
}

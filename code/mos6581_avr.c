/* MOS6581 SID driver for AVR
 * (c) 2011, 2017 Nils Stec <stecdose@gmail.com>
 *
 *  pin connections on atmega 16/32/64/164p/324p/644p/1284p, ... :
 *    SID            AVR
 *    ---------------------------
 *    D0-D7         PA0 - PA7
 *    A0-A5          PB0 - PB5
 *    CLOCK (o2)     OC1A (PD5)
 *    RESET       PD4
 *    R/W            PC2
 *    CS             PC3
 */

#define SET_CS		PORTC &= ~(1<<PC3)
#define CLR_CS		PORTC |= (1<<PC3)
#define SID_READ	PORTC |= (1<<PC2)
#define SID_WRITE	PORTC &= ~(1<<PC2)


void sid_reset(void) {
	PORTD &= (1<<PD4);  // do a reset
	_delay_ms(10);
	PORTD |= (1<<PD5);
	_delay_ms(50);

	return;
}

void sid_setaddr(unsigned char addr) {
	PORTB &= ~((1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3)|(1<<PB4));  // first disable all address lines
	PORTB |= addr&0x1f;            // now set the ones which have to be set

	return;
}


unsigned char sid_read(unsigned char sid_register) {
	unsigned char sid_data;
	DDRA = 0;
	PORTA = 0;
	sid_setaddr(sid_register);
	SID_READ;
	CLR_CS;
	while(!(PIND & (1<<PD5)));  // we can only read while clock is H
	sid_data = PINA;
	SET_CS;

	return sid_data;
}

void sid_write(unsigned char sid_register, unsigned char sid_data) {
	DDRA = 0xff;
	sid_setaddr(sid_register);
	SID_WRITE;
	CLR_CS;
	while(!(PIND & (1<<PD5)));  // we can only write while clock is H
	PORTA = sid_data;
	SET_CS;

	return;
}

void sid_clear(void) {
	unsigned char i;
	for(i = 0; i < 27; i++) sid_write(i, 0);
	return;
}

void sid_init(void) {
	// PORTA is used for datalines in bidirectional mode, since the avrs has no REAL bidir ports we have to
	// change DDRA every time we want to read/write, so no need for doing this here
	DDRB |= (1<<PB0)|(1<<PB1)|(1<<PB2)|(1<<PB3)|(1<<PB4);  // address line A0..A5
	DDRC |= (1<<PC2)|(1<<PC3);        // RW, CS
	DDRD |= (1<<PD4);          // RESET
	DDRD |=(1<<PD5);          // clock line
	TCCR1A |=(1<<COM1A0); // OC1A toggle on compare match
	TCCR1B |=(1<<WGM12);  // CTC mode 4
	TCCR1B |=(1<<CS10);   // no prescaling
	OCR1A=9;        // 1mhz clock @ 20mhz F_CPU

	SET_CS;

	sid_reset();
	sid_clear();

	return;
}

void sid(void) {
	sid_init();

	sid_reset();

	sid_write(24, 0x0f);
	sid_write(1, 0x06);
	sid_write(5, 12);
	sid_write(6,4);
	sid_write(4,33);

	for(;;) {
		//locate(0,0);
		//printf("$%02x $%02x $%02x", sid_read(25), sid_read(26), sid_read(27));
		sid_write(1, sid_read(25));
	}

	return;  // never reached!
}


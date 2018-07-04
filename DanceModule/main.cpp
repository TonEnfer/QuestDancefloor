#include "main.h"
#define DEBUG_PIN PD5
#define DEBUG_PORT PORTD
#define DEBUG_PP PIND

#define ROW_PIN PD3
#define ROW_PORT PIND

#define COL_PIN PD2
#define COL_PORT PIND

#define DAT_PIN PA0
#define DAT_PORT PORTA
#define DAT_DDR DDRA
#define DAT_PP PINA

#define RW_PIN PA1


#define COMPARE_PIN PB0

enum{
	INPUT=0,
	OUTPUT=1,
	INPUT_PULLUP=2
};
enum {
	LOW=0,
	HIGH=1
	
};
class PIN {
volatile uint8_t *ddr;
volatile uint8_t *outport;
volatile uint8_t *inport;
uint8_t pinshift;
public:
	PIN(volatile uint8_t &ddr, volatile uint8_t &outport, volatile uint8_t &inport, uint8_t pinshift){
		this->ddr = &ddr;
		this->outport = &outport;
		this->inport = &inport;
		this->pinshift = pinshift;
	}
	void write(bool val){
		if(val){
			(*outport) |= _BV(pinshift);
		}else{
			(*outport) &= ~(_BV(pinshift));
		}
	}
	void toggle(){
		(*outport) ^= _BV(pinshift);
	}
	bool read(){
		return (*inport) & _BV(pinshift);
	}
	// 0 - input, 1 - output, 2 - input pullup
	void mode(uint8_t mode){
		if(mode & 0x01){
			(*ddr) |= _BV(pinshift);
		}else{
			(*ddr) &= ~(_BV(pinshift));
			if(mode & 0x02){
				write(true);
			}else{
				write(false);
			}
		}
	}
};

class LED {
PIN* pin;
public:
	LED(PIN pin){
		this->pin = &pin;
		pin.mode(OUTPUT);
		off();
	}
	void on(){
		pin->write(LOW);
	}
	void off(){
		pin->write(HIGH);
	}
	void toggle(){
		pin->toggle();
	}
	
} led1(PIN(DDRB, PORTB, PINB, PB7)), led2(PIN(DDRB, PORTB, PINB, PB6));

class Databus{
PIN* col;
PIN* row;
PIN* rw;
PIN* dat;
public:
	Databus(PIN col, PIN row, PIN rw, PIN dat){
		this->col = &col;
		this->row = &row;
		this->rw = &rw;
		this->dat = &dat;
		// Все пины подтянуты.
		// выбор колонки и ряда active LOW
		col.mode(INPUT_PULLUP);
		row.mode(INPUT_PULLUP);
		// По умолчанию мы считаем, что записывают в нас (HIGH)
		// В случае если отпадёт конакт, мы не забьём линию данных
		rw.mode(INPUT_PULLUP);
		// Данные в правильном отображении - true=1=HIGH, false=0=LOW
		dat.mode(INPUT_PULLUP);
	}
	bool isSelected(){
		return !(col->read() || row->read());
	}
	bool isReading(){
		return !rw->read();
	}
	bool readData(){
		return dat->read();
	}
	void writeData(bool value){
		if(value){
		//Если хотим записать едииницу, то просто делаем подтяжку
			dat->mode(INPUT_PULLUP);
		}else{
		// Иначе записываем ноль в PORT и переводим в OUTPUT
			dat->write(LOW);
			dat->mode(OUTPUT);
		}
	}
} bus(PIN(DDRD, PORTD, PIND, PD2), PIN(DDRD, PORTD, PIND, PD3), PIN(DDRA, PORTA, PINA, PA1), PIN(DDRA, PORTA, PINA, PA0));

PIN debugPin(DDRD, PORTD, PIND, PD5);
PIN colorSelect(DDRD, PORTD, PIND, PD4);
PIN transistor(DDRD, PORTD, PIND, PD6);

bool isDebug(){
	return debugPin.read();
}

bool compare(){
	return ACSR & _BV(ACO);
}


void output(uint8_t value){
	cli();
	if(value){
		uint8_t colordata[3] = {0xFF, 0x00, 0x00};;
		if(colorSelect.read()){
			colordata[1] = 0xFF;
			colordata[2] = 0xFF;
		}
		for(uint8_t i=0; i<16; i++){
			output_rbg(colordata, 3);
		}
	}else{
		uint8_t colordata[3] = {0x00, 0x00, 0x00};
		for(uint8_t i=0; i<16; i++){
			output_rbg(colordata, 3);
		}
	}
	sei();
}

void setup(){
	transistor.mode(OUTPUT);
	debugPin.mode(INPUT_PULLUP);
	colorSelect.mode(INPUT_PULLUP);

	output(false);
}

void debugging(){
	if(bus.isSelected())
		led1.on();
	else
		led1.off();
	
	if(compare())
		led2.on();
	else
		led2.off();
}

uint8_t dancedFilter = 0;
bool danced = false;
void normal(){ 
	if(compare()){
		if(dancedFilter<200){
			dancedFilter++;
		}else{
			danced = true;
			led2.on();
		}
	}else{
		if(dancedFilter>3){
			dancedFilter-=2;
		}
	}
	if(bus.isSelected()){
		// У нас читают данные. Должны выдать на линию статус пляски
		if(bus.isReading()){
			// отдаём данные о том, был ли прыжок
			// и сбрасываем данные о прыжке
			led1.on();
			
			bus.writeData(danced);
			
			while(bus.isSelected());
			bus.writeData(1);
			
			danced = false;
			dancedFilter = 0;
			
			led1.off();
			led2.off();
		}else{
		// Нам передают данные. Включаем-выключаем транзистор в зависимости от данных
			output(bus.readData());
			while(bus.isSelected());
		}
	}
}


void loop(){
	if(isDebug())
		debugging();
	else
		normal();
}

int main(void){
	setup();
	while(1) loop();
}

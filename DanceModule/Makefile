MCU           = attiny2313a
F_CPU         = 8000000
TARGET        = main
objects       = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.cpp,%.o,$(wildcard *.cpp)) $(patsubst %.S,%.o,$(wildcard *.S))

C_WARNINGS    = -Wbad-function-cast -Wstrict-prototypes
WARNINGS      = -Wall -Wextra -Werror -Wpedantic -pedantic -Wchar-subscripts -Wcomment -Wdouble-promotion -Wformat -Wformat-y2k -Wswitch-default -Wunused -Wfloat-equal -Wlogical-op -Waggregate-return -Wmissing-field-initializers -Wpacked -Wpadded -Wredundant-decls -Winline
#WARNINGS      = -Wall -Werror -Wpedantic
C_OPTIONS     = -fdiagnostics-color=always -fdata-sections -ffunction-sections
OPTIMIZATION  = -Os
DEBUGGING     = -g -fverbose-asm
MACHINE_OPTS  = -mmcu=$(MCU)

ASFLAGS     = $(MACHINE_OPTS) $(WARNINGS) $(C_WARNINGS) $(C_OPTIONS) $(OPTIMIZATION) $(DEBUGGING)
ASFLAGS    += -x assembler-with-cpp -Wa,-gdwarf2
CFLAGS      = $(MACHINE_OPTS) $(WARNINGS) $(C_WARNINGS) $(C_OPTIONS) $(OPTIMIZATION) $(DEBUGGING) -std=c99
CXXFLAGS    = $(MACHINE_OPTS) $(WARNINGS) $(C_OPTIONS) $(OPTIMIZATION) $(DEBUGGING) -std=c++14
CPPFLAGS    = -DF_CPU=$(F_CPU)

#LDPRINTF    = -Wl,-u,vfprintf 
LDFLAGS     = -Wl,--error-unresolved-symbols,--fatal-warnings,--gc-sections $(MACHINE_OPTS)
LDLIBS      = -lm
#LDLIBS     += -lprintf_flt
AVRDUDE_PROGRAMMER = dragon_isp
AVRDUDE_PORT       = usb
AVRDUDE_TARGET     = attiny2313

CC = avr-gcc
CXX = avr-g++
AS = avr-gcc
OBJCOPY = avr-objcopy


#Стандартная цель - это собрать бинарник и показать его размер
all: $(TARGET) size

#Включаем зависимости файлов. Иначе не все изменения приведут к перекомпиляции
-include $(TARGET).d
-include $(objects:.o=.d)
#Основной бинарник собирается по неявному правилу из своего объектного файла
#Дополняем это правило своими объектными файлами, которые точно должны быть связаны с нашим бинарником и собираться вместе с ним
$(TARGET): $(objects)
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
#	@printf "\n\n"

#А вот и правило сборки зависимостей для C/C++ файлов
%.d: %.c
	@printf "Собираем зависимости для $<\n"
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@
%.d: %.cpp
	@printf "Собираем зависимости для $<\n"
	@set -e; rm -f $@; \
	$(CXX) -MM $(CPPFLAGS) $< > $@
%.d: %.S
	@printf "Собираем зависимости для $<\n"
	@set -e; rm -f $@; \
	$(AS) -MM $(ASFLAGS) $< > $@

#Правила, которые не собирают цель, а просто что-то показывают на экран или же вообще удаляют
.PHONY : clean size avrdude fuse

#Размер бинарника можно показать вот так
size: $(TARGET)
	@printf "\n\n\n"
	@avr-size -B --mcu=$(MCU) $(TARGET)
	@avr-size -C --mcu=$(MCU) $(TARGET)

#Очищаем рабочую директорию вот таким образом
clean:
	@printf "Производим зачистку...\n"
	@$(RM) $(objects) $(TARGET) $(TARGET).o $(TARGET).hex $(TARGET)_eeprom.hex $(TARGET)_eeprom.bin $(TARGET).d $(objects:.o=.d) $(TARGET).map $(TARGET).bin $(TARGET).elf $(TARGET).lst

#Так создаются hex файлы для загрузки в flash
%.hex: %
	@$(OBJCOPY) -j .text -j .data -O ihex $< $@
	@printf "Сливаем программу в HEX из бинарника\n"

#Так создаются hex файлы для загрузки в eeprom
%_eeprom.hex: %.elf
	@$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@
	@printf "Сливаем EEPROM в HEX из бинарника\n"
#Правило для загрузки на чип
bitclock = 4
verbosity = -v

fuse:
	@avrdude -i $(bitclock) -B $(bitclock) -p $(AVRDUDE_TARGET) -c $(AVRDUDE_PROGRAMMER) -P $(AVRDUDE_PORT) $(verbosity) -U lfuse:w:0xe4:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

avrdude: $(TARGET).hex
	@printf "Заливаем прошивку flash на кристалл\n"
	avrdude -i $(bitclock) -B $(bitclock) -p $(AVRDUDE_TARGET) -c $(AVRDUDE_PROGRAMMER) -P $(AVRDUDE_PORT) $(verbosity) -U flash:w:$(TARGET).hex:i

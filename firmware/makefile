CFLAGS  = -V -mmcs51 --model-large --xram-size 0x1800 --xram-loc 0x0000 --code-size 0xec00 --stack-auto --Werror -Ichlib/src --opt-code-speed
CC      = sdcc
TARGET  = ku88
OBJS	= main.rel key.rel ch559.rel hid.rel hid_guncon3.rel hid_keyboard.rel hid_switch.rel hid_xbox.rel led.rel serial.rel timer3.rel uart1.rel usb_host.rel

.PHONY: clean program run clean build

all: build $(TARGET).bin

program: $(TARGET).bin
	ch559flasher -w -c $(TARGET).bin

clean:
	rm -rf build $(TARGET).bin

.SILENT:
build:
	mkdir -p build

build/%.rel: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

build/%.rel: ./chlib/src/%.c ./chlib/src/*.h
	$(CC) -c $(CFLAGS) -o $@ $<

build/$(TARGET).ihx: $(addprefix build/,$(OBJS))
	$(CC) $(CFLAGS) $(addprefix build/,$(OBJS)) -o $@

%.bin: build/%.ihx
	sdobjcopy -I ihex -O binary $< $@

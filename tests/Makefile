

CURDIR:=$(shell pwd)
BUILD_PATHS:=$(CURDIR)/build:$(CURDIR)/build/Debug:$(CURDIR)/build/Release:$(CURDIR)/scripts
export PATH:=$(BUILD_PATHS):$(PATH)

all: 
	make -C ucom43 clean all
	make -C s2000 clean all
	make -C pic16c5x clean all
	make -C 6502 clean all
	make -C 6502/nanochess clean all
	make -C marc4 clean all
	make -C sm83 clean all
	make -C tlcs47 clean all
	make -C 6805 clean all
	make -C fcard clean all
	make -C chip8 clean all
	make -C z80 clean all
#	make -C z8 clean all
	make -C 8080 clean all
	make -C 8051 clean all
	make -C arm7tdmi clean all
	make -C weird clean all
	make selftests fuzz

clean:
	make -C ucom43 clean
	make -C s2000 clean
	make -C pic16c5x clean
	make -C 6502 clean
	make -C 6502/nanochess clean
	make -C marc4 clean
	make -C sm83 clean
	make -C tlcs47 clean
	make -C 6805 clean
	make -C fcard clean
	make -C chip8 clean
	make -C z80 clean
#	make -C z8 clean
	make -C 8080 clean
	make -C 8051 clean
	make -C arm7tdmi clean
	make -C weird clean

selftests:
# Passing self-tests.
	goodasm --test --ucom43
	goodasm --test --6502
	goodasm --test --marc4
	goodasm --test --sm83
	goodasm --test --tlcs47
	goodasm --test --s2000
	goodasm --test --8051
	goodasm --test --6805
	goodasm --test --chip8
	goodasm --test --z80
	goodasm --test --z8
	goodasm --test --8080
	goodasm --test --h83
#	goodasm --test --arm7tdmi
#	goodasm --pic16c5x --test
#	goodasm --test --ti80

fuzz:
# Passing self-tests.
	goodasm --fuzz --ucom43
	goodasm --fuzz --6502
	goodasm --fuzz --marc4
	goodasm --fuzz --sm83
	goodasm --fuzz --tlcs47
	goodasm --fuzz --s2000
	goodasm --fuzz --8051
	goodasm --fuzz --6805
	goodasm --fuzz --st7
	goodasm --fuzz --chip8
	goodasm --fuzz --z80
	goodasm --fuzz --z8
	goodasm --fuzz --8080
	goodasm --fuzz --pic16c5x
	goodasm --fuzz --h83
	goodasm --fuzz --arm7tdmi
#	goodasm --fuzz --ti80
# Identify fuzzing.
	goodasm --fuzz -i


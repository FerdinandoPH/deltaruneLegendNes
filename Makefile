# makefile for nesdoug example code, for Linux
# this version won't move the final files to BUILD folder
# also won't rebuild on changes to neslib/nesdoug/famitone files

CC65 = cc65
CA65 = ca65
LD65 = ld65
NAME = theLegend
CFG = ./libs/VRC6.cfg
DBG = -g

.PHONY: default clean

default: $(NAME).nes


#target: dependencies

$(NAME).nes: $(NAME).o crt0.o $(CFG)
	$(LD65) -C $(CFG) -o $(NAME).nes libs/crt0.o $(NAME).o nes.lib -Ln labels.txt --dbgfile dbg.txt
	del /Q *.o
	@echo $(NAME).nes created

crt0.o: ./libs/crt0.s ./graphics/AllOk.chr ./graphics/Delta.chr ./graphics/Disaster.chr ./graphics/Desert.chr ./graphics/LightInDesert.chr ./graphics/The3Heroes.chr ./graphics/SealAngelHeaven.chr ./graphics/CastleA.chr ./graphics/CastleB.chr ./graphics/EndCastle.chr ./graphics/AllWrong.chr
	$(CA65) ./libs/crt0.s $(DBG)

$(NAME).o: $(NAME).s
	$(CA65) $(NAME).s $(DBG)

$(NAME).s: $(NAME).c ./graphics/Clear.h ./graphics/AllOk.h ./graphics/Disaster.h ./graphics/Desert.h ./graphics/LightInDesert.chr ./graphics/The3Heroes.h ./graphics/SealAngelHeaven.h ./graphics/Castle.h ./graphics/EndCastleA.h ./graphics/EndCastleB.h ./graphics/AllWrong.h
	$(CC65) -Oirs $(NAME).c --add-source $(DBG)

clean:
	del /Q $(NAME).nes

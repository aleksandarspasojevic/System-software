ASSEMBLER=assembler.exe
LINKER=linker.exe
EMULATOR=emulator.exe

${ASSEMBLER} -o
${ASSEMBLER} -o math.o math.s
${ASSEMBLER} -o handler.o handler.s
${ASSEMBLER} -o isr_timer.o isr_timer.s
${ASSEMBLER} -o isr_terminal.o isr_terminal.s
${ASSEMBLER} -o isr_software.o isr_software.s
# ${LINKER} -hex \
#   -place=my_code@0x40000000 -place=math@0xF0000000 \
#   -o program.hex \
#   handler.o math.o main.o isr_terminal.o isr_timer.o isr_software.o
# ${EMULATOR} program.hex



# ../../assembler.exe -o main.o main.s
# ../../assembler.exe -o math.o math.s
# ../../assembler.exe -o handler.o handler.s
# ../../assembler.exe -o isr_timer.o isr_timer.s
# ../../assembler.exe -o isr_terminal.o isr_terminal.s
# ../../assembler.exe -o isr_software.o isr_software.s
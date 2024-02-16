BUILD_DIR = build
OUTPUT_DIR = out
LAB_NAME = lab3

OBJS = ast.o error.o main.o lex.yy.o parser.tab.o painter.o preprocess_ast.o semantic_analyser.o symbolic_table.o asm_generator.o builtin_functions.o
OBJS_WITH_PATH = $(patsubst %,$(BUILD_DIR)/%,$(OBJS))

ifeq ($(OS),Windows_NT)
    UNAME_S := Windows
else
    UNAME_S := $(shell uname -s)
endif

ifeq ($(UNAME_S),Windows)
    VM_COMMAND=./vm/Portable.RemoteTasks.Manager.exe -ul $$(cat .env | grep RemoteTasksLogin | cut -f2 -d"=" | tr -d '\r') -up $$(cat .env | grep RemoteTasksPassword | cut -f2 -d"=")
else ifeq ($(UNAME_S),Linux)
    VM_COMMAND=mono vm/Portable.RemoteTasks.Manager.exe -ul $$(cat .env | grep RemoteTasksLogin | cut -f2 -d"=" | tr -d '\r') -up $$(cat .env | grep RemoteTasksPassword | cut -f2 -d"=")
endif

#./vm/Portable.RemoteTasks.Manager.exe -ul $(cat .env | grep RemoteTasksLogin | cut -f2 -d"=" | tr -d '\r') -up $(cat .env | grep RemoteTasksPassword | cut -f2 -d"=")

remote-task:
	@echo Running VM Command
	$(VM_COMMAND) -h

vm_listing:
	@echo Running VM Command
	$(VM_COMMAND) -w -s Assemble definitionFile arch/spo.target.pdsl archName spo asmListing $(OUTPUT_DIR)/out.asm > $(OUTPUT_DIR)/assemble_res.txt

assemble_res.txt:
	$(VM_COMMAND) -s Assemble -w definitionFile arch/spo.target.pdsl archName spo asmListing $(OUTPUT_DIR)/out.asm > $(OUTPUT_DIR)/assemble_res.txt; cat $(OUTPUT_DIR)/assemble_res.txt


out.ptptb: assemble_res.txt
	$(VM_COMMAND) -g $$(cat $(OUTPUT_DIR)/assemble_res.txt | head -1 | awk  '{print $$6}') -r out.ptptb -o $(OUTPUT_DIR)/out.ptptb

vm_worker: out.ptptb
	$(VM_COMMAND) -s ExecuteBinaryWithInput -w \
		definitionFile arch/spo.target.pdsl \
		archName spo \
		codeRamBankName code_ram \
		ipRegStorageName ip \
		stdinRegStName inp \
        stdoutRegStName outp \
		finishMnemonicName hlt \
		inputFile tests/stdin.txt \
		binaryFileToRun $(OUTPUT_DIR)/out.ptptb

vm_worker_without_build:
	$(VM_COMMAND) -s ExecuteBinaryWithInput -w \
		definitionFile arch/spo.target.pdsl \
		archName spo \
		codeRamBankName code_ram \
		ipRegStorageName ip \
		stdinRegStName inp \
        stdoutRegStName outp \
		finishMnemonicName hlt \
		inputFile tests/stdin.txt \
		binaryFileToRun $(OUTPUT_DIR)/out.ptptb

all_lab3: clean out build_lab3 run_calc
all_lab3t: clean out build_lab3 run_test

clean:
	rm -rf ${BUILD_DIR}
	rm -rf ${OUTPUT_DIR}
	rm -f *.tab.*
	rm -f lex.yy.c
	rm -f *.exe
	rm -f *.exe.*
	rm -f *.asm.*
	rm -f *.ptptb

out:
	mkdir ${BUILD_DIR}
	mkdir ${OUTPUT_DIR}

build_lab3: ${BUILD_DIR}
	gcc -c -o ${BUILD_DIR}/error.o error/error.c
	gcc -c -o ${BUILD_DIR}/painter.o painter/painter.c
	gcc -c -o ${BUILD_DIR}/preprocess_ast.o ast/preprocess_ast.c
	gcc -c -o ${BUILD_DIR}/semantic_analyser.o analyser/semantic_analyser.c
	gcc -c -o ${BUILD_DIR}/symbolic_table.o analyser/symbolic_table.c
	gcc -c -o ${BUILD_DIR}/asm_generator.o asm/asm_generator.c
	flex parser/lexer.l
	bison -d -t parser/parser.y && echo '#include "ast/ast.h"' | cat - parser.tab.h > temp && mv temp parser.tab.h
	gcc -c -o ${BUILD_DIR}/main.o main.c
	gcc -c -o ${BUILD_DIR}/ast.o ast/ast.c
	gcc -c -o ${BUILD_DIR}/builtin_functions.o analyser/builtin_functions.c
	gcc -c -o ${BUILD_DIR}/lex.yy.o lex.yy.c
	gcc -c -o ${BUILD_DIR}/parser.tab.o parser.tab.c
	gcc $(OBJS_WITH_PATH) -o $(LAB_NAME) && chmod +x lab3

run_out:
	./lab3 tests/program/calc.spo
	cat $(OUTPUT_DIR)/out.asm.data $(OUTPUT_DIR)/out.asm.code > $(OUTPUT_DIR)/out.asm

run_calc:
	./lab3 tests/program/calc.spo
	cat $(OUTPUT_DIR)/out.asm.data $(OUTPUT_DIR)/out.asm.code > $(OUTPUT_DIR)/calc.asm

run_test:
	./lab3 tests/program/test.spo
	cat $(OUTPUT_DIR)/out.asm.data $(OUTPUT_DIR)/out.asm.code > $(OUTPUT_DIR)/test.asm

run_art:
	./lab3 tests/program/art.spo
	cat $(OUTPUT_DIR)/out.asm.data $(OUTPUT_DIR)/out.asm.code > $(OUTPUT_DIR)/art.asm


error.o: error/error.c
	gcc -c -o ${BUILD_DIR}/error.o error/error.c

lex.yy.c:
	flex parser/lexer.l

parser.tab.c: parser/parser.y
	bison -d -t parser/parser.y && echo '#include "ast/ast.h"' | cat - parser.tab.h > temp && mv temp parser.tab.h

ast.o: ast/ast.c
	gcc -c -o ${BUILD_DIR}/ast.o ast/ast.c

main.o: main.c out
	gcc -c -o ${BUILD_DIR}/main.o main.c

lex.yy.o: lex.yy.c
	gcc -c -o ${BUILD_DIR}/lex.yy.o lex.yy.c

parser.tab.o: parser.tab.c
	gcc -c -o ${BUILD_DIR}/parser.tab.o parser.tab.c

result: $(OBJS_WITH_PATH)
	gcc $(OBJS_WITH_PATH) -o result && chmod +x result



CC       = clang
SANITIZE = -fsanitize=address -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error
COVERAGE = -fprofile-instr-generate -fcoverage-mapping
OPTS     = $(SANITIZE) $(COVERAGE) -Weverything -Wno-padded -Wno-poison-system-directories

.PHONY : all
all : ini.coverage

%.coverage : %.profdata
	xcrun llvm-cov show $*.unittest -instr-profile=$< $*.c > $@
	! grep " 0|" $@ 
	echo PASS $@

%.profdata : %.profraw
	xcrun llvm-profdata merge -sparse $< -o $@

%.profraw : %.unittest
	LLVM_PROFILE_FILE=$@ ./$< > actual
	diff expected actual
	echo PASS $<

%.unittest : test_ini.c ini.c
	$(CC) $(OPTS) $^ -o $@

.PHONY : clean
clean :
	rm -rf *.coverage *.profdata *.profraw *.unittest* actual

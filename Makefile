TESTS=$(sort $(wildcard *.fun))
PROGS=$(subst .fun,,$(TESTS))
OUTS=$(patsubst %.fun,%.out,$(TESTS))
DIFFS=$(patsubst %.fun,%.diff,$(TESTS))
RESULTS=$(patsubst %.fun,%.result,$(TESTS))

CFILES=$(sort $(wildcard *.c))

PPC=/u/gheith/public/ppc64-linux/bin
AS=${PPC}/as
LD=${PPC}/ld

.SECONDARY:

.PROCIOUS : %.o %.S %.out

CFLAGS=-g -std=gnu99 -O0 -Werror -Wall
OFILES=$(subst .c,.o,$(CFILES))

p5 : $(OFILES) Makefile
	gcc $(CFLAGS) -o p5 $(OFILES)

%.o : %.c Makefile
	gcc $(CFLAGS) -MD -c $*.c

%.o : %.S Makefile
	($(AS) -o $*.o $*.S) || touch $@

%.S : %.fun p5
	@echo "========== $* =========="
	(./p5 < $*.fun > $*.S) || touch $@

progs : $(PROGS)

ppc.o : ppc.asm
	($(AS) -o $*.o $*.asm) || touch $@

$(PROGS) : % : %.o ppc.o
	($(LD) -e entry -o $@ $*.o ppc.o) || touch $@

outs : $(OUTS)

$(OUTS) : %.out : %
	(qemu-ppc64 $* > $*.out) || touch $@

diffs : $(DIFFS)

%.ok:
	touch $@

$(DIFFS) : %.diff : Makefile %.out %.ok
	@(((diff -b $*.ok $*.out > /dev/null 2>&1) && (echo "===> $* ... pass")) || (echo "===> $* ... fail" ; echo "----- test -----"; cat $*.fun; echo "----- expected ------"; cat $*.ok ; echo "----- found -----"; cat $*.out)) > $*.diff 2>&1

$(RESULTS) : %.result : Makefile %.diff
	@cat $*.diff

test : Makefile $(DIFFS)
	@cat $(DIFFS)

clean :
	rm -f $(PROGS)
	rm -f *.S
	rm -f *.out
	rm -f *.d
	rm -f *.o
	rm -f p5
	rm -f *.diff

-include *.d

	.text

	.global exit
exit:
	li 0,1                 # system call number
	li 3,0                 # status code
	sc                     # issue system call


        /* This is a macro not a function */
	/* assumes buffer pointer in r4 */
.macro write
	li 0,4                  # write system call number
	li 3,1                  # write to stdout
	li 5,1                  # write 1 byte
	sc			# issue the system call
.endm

	/* doesn't honor standard PPC linkage convention, nor yours */
	/* argument in r15 */
 	/* preserves r1,r2 */
	.global print
print:
	addi 1,1,-16
	or 10,1,1               # r10 -> buffer
	
doit:
	cmpdi 15,0
	beq isZero

        li 20,10                # r20 = 10
	li 21,0                 # r21 = 0 (counter)

loop:
	divdu 22,15,20          # r22 = v / 10
	mulld 23,22,20          # r23 = (v / 10) * 10
	subf 23,23,15		# r23 = v % 10
	or 15,22,22		# r15 = v / 10
	addi 23,23,'0'		# r23 = ascii(digit)
	stbx 23,21,10		# buffer[count] = ascii(digit)
	addi 21,21,1		# count += 1
	cmpdi 15,0
	bne loop

writeLoop:
	cmpdi 21,0
	beq leave
	addi 21,21,-1
	add 4,10,21
	write
	b writeLoop
	



isZero:
        li 4,'0'
	stb 4,0(10)
	or 4,10,10
	write


leave:
	li 4,10                   # nl
        stb 4,0(10)
	or 4,10,10
	write

	addi 1,1,16
	blr	

.globl main

.data
count_prompt: .asciiz "How many ints? "
enter_prompt: .asciiz "Enter that many ints, one per line.\n"
blank_space: .asciiz " " # and I'll write your name
your_array: .asciiz "Your array:\n"
sorted_array: .asciiz "\nSorted array:\n"

.text
main:
	# x is $t0
	# loop counter is $t1
	# size is $t2
	# $t4 is 4
	
	# main is not really a fucntion here
	# it starts the program and ends it and never gets called again 
	# therefore, there is no need to maintain a dedicated stack
	
	li $v0, 4
	la $a0, count_prompt
	syscall
		
	# read n
	li $v0, 5
	syscall
	move $t2, $v0
	beq $t2, $t1, main_exit
	
	li $v0, 4
	la $a0, enter_prompt
	syscall

	# allocate n words on heap
	li $t4, 4
	mul $t3, $t2, $t4
	
	li $v0, 9
	move $a0, $t3
	syscall 
	move $t6, $v0
	
	# loop structure from class
	li $t1, 0
	input_loop:
		# read item
		li $v0, 5
		syscall
		move $t0, $v0
		
		mul $t3, $t1, $t4
		add $t3, $t6, $t3
	
		sw $t0, 0($t3)
	
		addi $t1, $t1, 1
		blt $t1, $t2, input_loop

		li $t1, 0
		li $t5, 0
		
	move $v0, $t6 # move array pointer into $v0
	move $v1, $t2 # move size into $v1
	
	jal isort # sort the array
	
	main_exit:
		li $v0, 10
		syscall
	
isort:
	#t0 is our array
	#t1 is a copy of our array
	#t2 is n
	#t3 is a loop counter
	#t4 is the number 4
	subi $sp, $sp, 100
	sw $ra, 0($sp)
	sw $t0, 4($sp)
	sw $t1, 8($sp)
	sw $t2, 4($sp)
	sw $t3, 12($sp)
	sw $t4, 16($sp)
	sw $t5, 20($sp)
	sw $t6, 24($sp)
	sw $t7, 28($sp)
	sw $t8, 32($sp)
	sw $t9, 36($sp)
	
	move $t0, $v0 # move the array pointer in $t0
	move $t2, $v1 # move the size of the array into $t2
	li $t4, 4 # put constant 4 in $t4
	
	# allocate space for the array copy on the heap
	li $v0, 9 
	move $a0, $t2
	syscall 
	move $t1, $v0
	
	# copy our array
	li $t3, 0
	copy_loop:
		mul $t5, $t3, $t4 # multiply the counter by 4
		add $t6, $t5, $t0 # calculate the offset for the original array and store it in $t6
		add $t7, $t5, $t1 # calulaute offset for copy array and store it in $t7
		lw $t6, 0($t6) # load the $t3-th value out of the original array...
		sw $t6, 0($t7) # ...and store it in the copy array
		
		addi $t3, $t3, 1 # increment the loop counter by 1
		blt $t3, $t2, copy_loop # if we haven't yet gone over the whole array, repeat the loop
	
	li $t3, 1 # outer loop counter (i)
	beq $t3, $t2 isort_done
	sort_outer:
		move $t5, $t3 # $t5 is the inner loop counter (j)
		sort_inner:
			beq $t5, $zero, sort_outer_exit
			mul $t6, $t5, $t4 # calculate the offset (4xj)
			li $t9, 4
			add $t6, $t6, $t1 # get the memory address (offset+t1)
			lw $t8, 0($t6)																	
			subi $t6, $t6, 4
			lw $t7, 0($t6)
			ble $t7, $t8, sort_outer_exit	
			
			# do the swap
			sw $t8, 0($t6)
			addi $t6, $t6, 4
			sw $t7, 0($t6)
			
			subi $t5, $t5, 1
			j sort_inner 
		sort_outer_exit:
			addi $t3, $t3, 1
			blt $t3, $t2, sort_outer
	
	isort_done:	
		move $v1, $t2 # pass size of arrays as an argument in $t1
	
		# print "your array"
		li $v0, 4
		la $a0, your_array
		syscall
		move $v0, $t0
		jal print_list # and actually print it
		
		# print "sorted array"
		li $v0, 4
		la $a0, sorted_array
		syscall
		move $v0, $t1
		jal print_list # and actually print it
	
		lw $t9, 36($sp)
		lw $t8, 32($sp)
		lw $t7, 28($sp)
		lw $t6, 24($sp)
		lw $t5, 20($sp)
		lw $t4, 16($sp)
		lw $t3, 12($sp)
		lw $t2, 4($sp)
		lw $t1, 8($sp)
		lw $t0, 4($sp)
		lw $ra, 0($sp)
		addi $sp, $sp, 100
	
		jr $ra
	
print_list:
	# $t0 is the array
	# $t1 is the size
	subi $sp, $sp, 24
	sw $ra, 0($sp)
	sw $t0, 4($sp)
	sw $t1, 8($sp)
	sw $t2, 12($sp)
	sw $t3, 16($sp)
	sw $t4, 20($sp)
	
	move $t0, $v0
	move $t1, $v1
	li $t4, 4
	
	li $t3, 0 # loop counter
	print_loop:
		# this is direct, no need to deal with temporary registers for a very, very simple function like this one
		li $v0, 1
		lw $a0, 0($t0)
		syscall
		
		li $v0, 4
		la $a0, blank_space
		syscall
		
		addi $t3, $t3, 1
		addi $t0, $t0, 4 # add to the array pointer directly, this is fine because we only need to access the elements ONCE in this function
		blt $t3, $t1, print_loop
	
	lw $t4, 20($sp)
	lw $t3, 16($sp)
	lw $t2, 12($sp)
	lw $t1, 8($sp)
	lw $t0, 4($sp)
	lw $ra, 0($sp)
	addi $sp, $sp, 24
	
	jr $ra

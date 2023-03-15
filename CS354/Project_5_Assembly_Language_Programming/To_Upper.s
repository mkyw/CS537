	.file	"To_Upper.s"
	.text
	.globl	To_Upper
	.type	To_Upper, @function

/* **********************
    Name: Michael Wong
    Wisc ID Number: 9083708769
    Wisc Email: mwong49@wisc.edu
************************ */


To_Upper:

# C version
/* **********************
    Write the equivalent C code to this function here (in the comment block)

void To_Upper(char *str) {
	while (*str != '\0') {
		if (*str >= 97 && *str <= 122) {
			*str -= 32;
		}
		str++;
	}
}

************************ */


# Memory Layout
/* ************************ 
    Make a table showing the stack frame for your function (in the comment block)
    The first column should have a memory address (e.g. 8(%rsp))
    The second column should be the name of the C variable used above
    Update the example below with your variables and memory addresses
        -4(%rbp)   |  int x
        -8(%rbp)   |  int y
        -16(%rbp)  |  int *px
************************ */


# Prologue
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp

# This code prints the letter 'a' (ascii value 97)
# Use this for debugging
# Comment out when finished - your function should not print anything
# Note putchar overwrites all caller saved registers including argument registers
	movl	$97, %eax
	movl	%eax, %edi
	call	putchar@PLT


# Body of function
// Initialization block
    movq %rdi, %rax
    movzbl (%rdi), %ecx
    
// While Block
.CONDITION:
    cmpb $0, %cl
    jz .END

// Loop 
.BODY:
    leal -97(%rcx), %ebx
    cmpb $25, %bl
    ja .IFCOND
    jmp .JUMP_TO_CONDITION

.IFCOND:
    addq $1, %rax
    movb (%rax), %cl
    jmp .CONDITION

// Increment Pointer
.JUMP_TO_CONDITION:
    subl $32, %ecx
    movb %cl, (%rax)
    addq $1, %rax
    movzbl (%rax), %ecx
    cmpb $0, %cl
    jmp .CONDITION

# Epilogue
.END:
    addq $32, %rsp
    popq %rbp
    ret


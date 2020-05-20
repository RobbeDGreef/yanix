[global FAULT_STACK]
section 	.data

_fault_stack_size: equ 1024
_fault_stack: times _fault_stack_size dd 0x0
FAULT_STACK: dd (_fault_stack + _fault_stack_size)
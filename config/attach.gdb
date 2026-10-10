set pagination off
file build/firmware-gcc/ethercat_joint.elf
target extended-remote localhost:3333
monitor halt
echo Target halted without reset; inspect values, then resume and detach.\n

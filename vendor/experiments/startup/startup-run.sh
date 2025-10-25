clang -nostdlib  startup.c -o startup
./startup 1 2
echo $?
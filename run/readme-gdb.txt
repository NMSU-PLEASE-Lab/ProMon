gdb process:
================
mpic++ -g -o mpi_hello main.cpp
gdb mpi_hello

(gdb) b 2
Breakpoint 1 at 0x40850e: file main.cpp, line 2.

(gdb) run
Starting program: /home/ujjwal/Source/applications/mpi_hello/mpi_hello
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".

Breakpoint 1, main (argc=1, argv=0x7fffffffd9c8) at main.cpp:4
4	int main(int argc, char** argv) {

(gdb) n
6	    MPI_Init(NULL, NULL);

(gdb) n
[New Thread 0x7ffff3864700 (LWP 778)]
10	    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

(gdb) n
14	    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

(gdb) n
19	    MPI_Get_processor_name(processor_name, &name_len);

(gdb) p processor_name

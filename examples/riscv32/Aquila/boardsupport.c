#include "boardsupport.h"
#include "elibc/stdio.h"

unsigned long start_cycles = 0;
unsigned long end_cycles = 0;

void initialise_board (void) {
    printf("=======================================================\n");
    printf("============= Embench Program Initilizing =============\n");
    printf("=======================================================\n");
}

void start_trigger(void) {
    printf("================ Embench Program Start ================\n"); // output result on UART
    start_cycles = clock();
}

void stop_trigger(void) {
    end_cycles = clock();
    printf("Embench Score(Total Ticks): %lu\n", end_cycles - start_cycles); // output result on UART
    printf("================ Embench Program End   ================\n");
}
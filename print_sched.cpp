// Convenience script to print the min and max priority for each valid
// scheduling priority on a Raspberry Pi

#include <iostream>
#include <sched.h>

int main() {
    int min;
    int max;

    min = sched_get_priority_min (SCHED_OTHER);
    max = sched_get_priority_max (SCHED_OTHER);
    std::cout << "SCHED_OTHER min: " << min << "\tmax: " << max << std::endl;

    min = sched_get_priority_min (SCHED_BATCH);
    max = sched_get_priority_max (SCHED_BATCH);
    std::cout << "SCHED_BATCH min: " << min << "\tmax: " << max << std::endl;

    min = sched_get_priority_min (SCHED_IDLE);
    max = sched_get_priority_max (SCHED_IDLE);
    std::cout << "SCHED_IDLE min: " << min << "\tmax: " << max << std::endl;

    min = sched_get_priority_min (SCHED_RR);
    max = sched_get_priority_max (SCHED_RR);
    std::cout << "SCHED_RR min: " << min << "\tmax: " << max << std::endl;

    min = sched_get_priority_min (SCHED_FIFO);
    max = sched_get_priority_max (SCHED_FIFO);
    std::cout << "SCHED_FIFO min: " << min << "\tmax: " << max << std::endl;

    return 0;
}

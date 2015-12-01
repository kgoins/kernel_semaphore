#include "../proc_sim.h"

int main (int argc, char const* argv[])
{
    proc_sim_t p1,p2,p3,p4;

    p1.pid = 1;
    p2.pid = 2;
    p3.pid = 3;
    p4.pid = 4;

    p1.parent_pid = 0;
    
    return 0;
}

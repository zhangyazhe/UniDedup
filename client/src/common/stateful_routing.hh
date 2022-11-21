#ifndef _STATEFUL_ROUTING_HH_
#define _STATEFUL_ROUTING_HH_

#include "../inc/include.hh"
#include "Group.hh"

/**
 * @brief Get the destination node for stateful routing.
 * @param sample_unit_list sample unit info of current group
 * @param node_num node number
 * @return destination node index
*/
int getNodeForStatefulRouting(vector<SampleUnit> & sample_unit_list, int node_num, unsigned int local_ip);

#endif
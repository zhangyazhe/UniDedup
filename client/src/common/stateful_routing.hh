#ifndef _STATEFUL_ROUTING_HH_
#define _STATEFUL_ROUTING_HH_

#include "../inc/include.hh"
#include "sw/redis++/redis++.h"
#include <unordered_set>
#include "sw/redis++/errors.h"

typedef struct SampleUnit {
    fingerprint feature;
    uint64_t size;
} SampleUnit;

/**
 * @brief Get the destination node for stateful routing.
 * @param sample_unit_list sample unit info of current group
 * @param node_num node number
 * @return destination node index, -1 if error occurs
*/
int getNodeForStatefulRouting(vector<SampleUnit> & sample_unit_list, int node_num, unsigned int local_ip, int cluster_enabled);

#endif
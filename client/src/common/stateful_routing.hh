#ifndef _STATEFUL_ROUTING_HH_
#define _STATEFUL_ROUTING_HH_

#include "../inc/include.hh"
#include "sw/redis++/redis++.h"
#include <unordered_set>
#include "sw/redis++/errors.h"
#include <ctime>
#include <cstdlib>
#include "Config.hh"

class Config;

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
int getNodeForStatefulRouting(vector<SampleUnit> & sample_unit_list, Config* conf);

/**
 * @brief invoke by getNodeForStatefulRouting, select the destination node
 * @param node_score score for each node
 * @return destination node index
*/
int getSelectedNode(vector<double> & node_score);

#endif
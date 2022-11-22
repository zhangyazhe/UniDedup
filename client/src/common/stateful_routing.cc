#include "stateful_routing.hh"
#include "../util/RedisUtil.hh"



int getNodeForStatefulRouting(vector<SampleUnit> & sample_unit_list, int node_num, unsigned int local_ip) {
    vector<int> node_match_num(node_num, 0);
    vector<uint64_t> node_match_size(node_num, 0);
    redisContext* redis_ctx;
    try {
        redis_ctx = RedisUtil::createContext(local_ip);
    } catch(exception & e) {
        return 0;
    }
    // compute match count and match size for each node
    for (int i = 0; i < node_num; i++) {
        // send command 'smembers (node index)'
        string cmd = "smembers " + to_string(i);
        redisReply* reply = (redisReply*)redisCommand(redis_ctx, cmd.c_str());
        if (reply->type == REDIS_REPLY_ERROR || reply->type != REDIS_REPLY_ARRAY) {
            cerr << "Command '" << cmd <<"' failed, reply->type = " << reply->type << endl;
            freeReplyObject(reply);
            return -1;
        }
        for (int j = 0; j < reply->elements; j++) {
            for (int k = 0; k < sample_unit_list.size(); k++) {
                string fp_str;
                for (int m = 0; m < 20; m++) {
                    fp_str = fp_str + to_string(sample_unit_list[k].feature[m]) + "_";
                }
                if (strcmp(fp_str.c_str(), reply->element[j]->str) == 0) {
                    node_match_num[i]++;
                    node_match_size[i] += sample_unit_list[k].size;
                }
            }
        }
    }
    // compute score for each node
    vector<double> node_score(node_num, 0);
    uint64_t group_size = 0;
    for (int i = 0; i < sample_unit_list.size(); i++) {
        group_size += sample_unit_list[i].size;
    }
    for (int i = 0; i < node_num; i++) {
        node_score[i] = (double)node_match_num[i] * node_match_size[i] / group_size;
    }
    cout << "[Stateful Routing] routing score: ";
    for (auto score : node_score) {
        cout << score << " ";
    }
    cout << endl;
    // select node
    int selected_node = 0;
    double max_score = 0.0;
    for (int i = 0; i < node_score.size(); i++) {
        if (node_score[i] > max_score) {
            max_score = node_score[i];
            selected_node = i;
        }
    }
    cout << "[Stateful Routing] Routing destination is node" << selected_node << endl;
    // update redis
    string cmd = "sadd " + to_string(selected_node) + " ";
    for (int i = 0; i < sample_unit_list.size(); i++) {
        // send command 'sadd selected_node feature_x_str feature_y_str ...'
        for (int j = 0; j < 20; j++) {
            cmd += to_string(sample_unit_list[i].feature[j]);
            cmd += "_";
        }
        if (i != sample_unit_list.size() - 1) {
            cmd += " ";
        }
    }
    redisReply* reply = (redisReply*)redisCommand(redis_ctx, cmd.c_str());
    if (reply->type == REDIS_REPLY_ERROR || strcmp(reply->str, "OK") != 0) {
        cerr << "Command sadd failed, reply->type = " << reply->type << ", reply->str = " << string(reply->str) << endl;
        freeReplyObject(reply);
        return -1;
    }
    freeReplyObject(reply);
    return selected_node;
}
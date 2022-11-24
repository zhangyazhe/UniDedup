#include "redis_cluster.hh"
#include "../util/RedisUtil.hh"
#include <exception>

int setFileRecipeToRedis(struct fileRecipe* fr, unsigned int local_ip) {
    // redisContext* redis_ctx;
    // try {
    //     redis_ctx = RedisUtil::createContext(local_ip);
    // } catch(exception & e) {
    //     return 0;
    // }

    // // send command 'set filename group_num'
    // string cmd = "set " + string(fr->filename) + " " + to_string(fr->num);
    // redisReply* reply = (redisReply*)redisCommand(redis_ctx, cmd.c_str());
    // if (reply->type == REDIS_REPLY_ERROR) {
    // cerr << "Command '" << cmd <<"' failed, reply->type = " << reply->type << endl;
    // freeReplyObject(reply);
    // return -1;
    // }
    // freeReplyObject(reply);

    // // send command to create hash for each group
    // for (int i = 0; i < fr->num; i++) {
    //     string hash_key = string(fr->filename) + "_" + to_string(i);
    //     string group_name = fr->gm[i].groupName;
    //     string node_id_str = to_string(fr->gm[i].nodeId);
    //     string cmd = "hmset " + hash_key + " group_name " + group_name + " node_id " + node_id_str;
    //     redisReply* reply = (redisReply*)redisCommand(redis_ctx, cmd.c_str());
    //     if (reply->type == REDIS_REPLY_ERROR) {
    //         cerr << "Command '" << cmd << "' failed, reply->type = " << reply->type << endl;
    //         freeReplyObject(reply);
    //         return -1;
    //     }
    //     freeReplyObject(reply);
    // }
    // return 0;
    string ip = RedisUtil::ip2Str(local_ip);
    auto cluster = sw::redis::RedisCluster("tcp://" + ip + ":6380");
    bool ok = cluster.set(string(fr->filename), to_string(fr->num));
    if (!ok) {
        cout << "[Redis Cluster] set filename group_num failed" << endl;
        return -1;
    }
    unordered_map<string, string> mp;
    for (int i = 0; i < fr->num; i++) {
        string hash_key = string(fr->filename) + "_" + to_string(i);
        string group_name = fr->gm[i].groupName;
        string node_id_str = to_string(fr->gm[i].nodeId);
        mp["group_name"] = group_name;
        mp["node_id"] = node_id_str;
        try {
            cluster.hmset(hash_key, mp.begin(), mp.end());
        } catch (const sw::redis::Error &err) {
            cerr << "[Redis cluster] hmset failed: " << err.what() << endl;
            return -1;
        }
        mp.clear();
    }
    return 0;
}

struct fileRecipe* getFileRecipeFromRedis(string filename, unsigned int local_ip) {
    // redisContext* redis_ctx;
    // try {
    //     redis_ctx = RedisUtil::createContext(local_ip);
    // } catch(exception & e) {
    //     return nullptr;
    // }

    // // send command 'get filename'
    // string cmd = "get " + filename;
    // redisReply* reply = (redisReply*)redisCommand(redis_ctx, cmd.c_str());
    // if (reply->type == REDIS_REPLY_ERROR || reply->type != REDIS_REPLY_INTEGER) {
    //     cerr << "Command '" << cmd <<"' failed, reply->type = " << reply->type << endl;
    //     freeReplyObject(reply);
    //     return nullptr;
    // }
    // int group_num = (int)reply->integer;
    // freeReplyObject(reply);
    string ip = RedisUtil::ip2Str(local_ip);
    sw::redis::RedisCluster cluster("tcp://" + ip + ":6380");
    sw::redis::OptionalString val;
    try {
        val = cluster.get(filename);
    } catch (const sw::redis::Error & err) {
        cerr << "[Redis Cluster] get filname failed: " << err.what() << endl;
        return nullptr;
    }
    int group_num;
    if (val) {
        group_num = stoi(*val);
    } else {
        cerr << "[Redis Cluster] Key does not exist. Key is " << filename << endl;
        return nullptr;
    }

    struct fileRecipe fr;
    fr.filename = (char*)malloc(filename.size() + 1);
    strcpy(fr.filename, filename.c_str());
    fr.num = group_num;
    fr.gm = (groupMeta*)malloc(group_num * sizeof(groupMeta));

    for (int i = 0; i < group_num; i++) {
        string hash_key = filename + "_" + to_string(i);
        vector<sw::redis::OptionalString> vals;
        try {
            cluster.hmget(hash_key, {"group_name", "node_id"}, back_inserter(vals));
        } catch (const sw::redis::Error & err) {
            cerr << "[Redis Cluster] hmget failed: " << err.what() << endl;
            free(fr.filename);
            for (int j = 0; j < i; j++) {
                free(fr.gm[j].groupName);
            }
            free(fr.gm);
            return nullptr;
        }
        string group_name = *vals[0];
        int node_id = stoi(*vals[1]);
        fr.gm[i].groupName = (char*)malloc(group_name.size() + 1);
        strcpy(fr.gm[i].groupName, group_name.c_str());
        fr.gm[i].
        nodeId = node_id;
    }

    return &fr;
    // send hmget command
    // for (int i = 0; i < group_num; i++) {
    //     string hash_key = filename + "_" + to_string(i);
    //     string cmd = "hmget " + hash_key + " group_name node_id";
    //     redisReply* reply = (redisReply*)redisCommand(redis_ctx, cmd.c_str());
    //     if (reply->type == REDIS_REPLY_ERROR || reply->type != REDIS_REPLY_ARRAY) {
    //         cerr << "Command '" << cmd <<"' failed, reply->type = " << reply->type << endl;
    //         freeReplyObject(reply);
    //         free(fr.filename);
    //         for (int j = 0; j < i; j++) {
    //             free(fr.gm[j].groupName);
    //         }
    //         free(fr.gm);
    //         return nullptr;
    //     }
    //     assert(reply->elements == 2);
    //     if (reply->element[0]->type != REDIS_REPLY_STRING || reply->element[1]->type != REDIS_REPLY_INTEGER) {
    //         cerr << "Command hmget gets wrong type: " << reply->element[0]->type << " " << reply->element[1]->type << endl;
    //     }
    //     string group_name = string(reply->element[0]->str);
    //     int node_id = (int)reply->element[1]->integer;
    //     freeReplyObject(reply);
    //     fr.gm[i].groupName = (char*)malloc(group_name.size() + 1);
    //     strcpy(fr.gm[i].groupName, group_name.c_str());
    //     fr.gm[i].nodeId = node_id;
    // }
    // return &fr;
}
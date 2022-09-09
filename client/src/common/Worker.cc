#include "Worker.hh"

struct chunk_meta_data chunkMetaData;

Worker::Worker(Config* conf) : _conf(conf)
{
    // create local context
    try
    {
        _processCtx = RedisUtil::createContext(_conf->_localIP);
        _localCtx = RedisUtil::createContext(_conf->_localIP);
        // _destorCtx = RedisUtil::createContext(_conf->_localIP);
    }
    catch (int e)
    {
        // TODO: error handling
        cerr << "initializing redis context error" << endl;
    }
    chunkMetaData.chunk_algorithm = _conf->chunking_algorithm;
    chunkMetaData.chunk_avg_size = _conf->destor_chunk_avg_size;
    chunkMetaData.chunk_max_size = _conf->destor_chunk_max_size;
    chunkMetaData.chunk_min_size = _conf->destor_chunk_min_size;
}

Worker::~Worker()
{
}

void Worker::doProcess() {
  redisReply* rReply;
  while (true) {
    cout << "Worker::doProcess" << endl;
    // will never stop looping
    rReply = (redisReply*)redisCommand(_processCtx, "blpop dup_agent_request 0");
    if (rReply -> type == REDIS_REPLY_NIL) {
      cerr << "Worker::doProcess() get feed back empty queue " << endl;
      //freeReplyObject(rReply);
    } else if (rReply -> type == REDIS_REPLY_ERROR) {
      cerr << "Worker::doProcess() get feed back ERROR happens " << endl;
    } else {
      struct timeval time1, time2;
      gettimeofday(&time1, NULL);
      char* reqStr = rReply -> element[1] -> str;
      AgentCommand* agCmd = new AgentCommand(reqStr);
      int type = agCmd->getType();
      cout << "Worker::doProcess() receive a request of type " << type << endl;
      //agCmd->dump();
      switch (type) {
        case 0: clientWrite(agCmd); break;
        
        default:break;
      }
      delete agCmd;
    }
    // free reply object
    freeReplyObject(rReply); 
  }
}

void Worker::clientWrite(AgentCommand *agCmd) {
    // get info
    string filepath = agCmd->getFilepath();
    string filename = agCmd->getFilename();
    int filesize = agCmd->getFilesize();

    /* This part is for Zewen */

    // file to groups
    vector<struct group*> gps = split2Groups(filepath.c_str(), filename.c_str(), _conf->node_num);

    cout << "gps.size(): " << gps.size() << endl;
    // cout << string((char *)gps[0]->data).size() << endl;
    cout << "gps[0].size: " << gps[0]->size << endl;

    /* This part is for Lin */
    // generate file recipe
    struct fileRecipe* fr = getFileRecipe(filename.c_str(), gps);
    assert(fr != NULL);

    // set file recipe by echash
    int ret = setFileRecipe(fr);
    assert(ret == 0);

    // distribute groups to different nodes
    // thread sendThrd[gps.size()];
    for(int i = 0; i < gps.size(); i++) {
      destorCommand *dstCmd = new destorCommand();
      // tell destor name and size
      dstCmd->buildType0(0, (const char*)gps[i]->groupName, gps[i]->size);
      unsigned int nodeIp = _conf->id2Ip[gps[i]->nodeId];
      dstCmd->sendTo(nodeIp);
      // send data
      // extand pipelining (if need)
      _destorCtx = RedisUtil::createContext(nodeIp);
      string append_data_key = string(gps[i]->groupName)+"_data";
      redisAppendCommand(_destorCtx, "RPUSH %s %b", append_data_key.c_str(), gps[i]->data, gps[i]->size);
      // cout << append_data_key.c_str() << endl;
      redisReply* destorrReply;
      redisGetReply(_destorCtx, (void**)&destorrReply);
      freeReplyObject(destorrReply);
      // wait for finished
      string wait_finished_key = append_data_key + "_finished";
      destorrReply = (redisReply*)redisCommand(_localCtx, "blpop %s 0", wait_finished_key.c_str());
      freeReplyObject(destorrReply);
      
      redisFree(_destorCtx);

      // for debug
      // std::cout << "Worker::debug::Command type 0, groupName is " << gps[i]->groupName
      //           << " data size is " << gps[i]->size
      //           // << " data is " << gps[i]->data
      //           << std::endl;
      // std::cout << "Worker::debug::Command send to " << RedisUtil::ip2Str(_conf->id2Ip[gps[i]->nodeId]) << std::endl;

      // sendThrd[i] = thread([&](){
      //   destorCommand *dstCmd = new destorCommand();
      //   dstCmd->buildType0(0, (const char*)gps[i]->groupName, (const char*)gps[i]->data);
      //   unsigned int nodeIp = _conf->id2Ip(gps[i]->nodeId)
      //   dstCmd->sendTo(nodeIp);
      // });
      // close(fd);
    }

    // for(int i = 0; i < gps.size(); i++) {
    //   sendThrd[i].join();
    // }

}
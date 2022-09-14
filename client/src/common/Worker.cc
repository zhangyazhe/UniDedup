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
        case 1: clientRead(agCmd); break;
        
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

    /* This part is for Lin */
    // generate file recipe
    struct fileRecipe* fr = genFileRecipe(filename.c_str(), gps);
    assert(fr != NULL);

    // set file recipe by echash
    int ret = setFileRecipe(fr);
    assert(ret == 0);

    // distribute groups to different nodes
    // thread sendThrd[gps.size()];
    // printf("gps.size: %d\n", gps.size());
    for(int i = 0; i < gps.size(); i++) {
      cout << "Worker::group:" << string(gps[i]->groupName) << " start!" << endl;
      destorCommand *dstCmd = new destorCommand();
      // tell destor name and size
      dstCmd->buildType0(0, (const char*)gps[i]->groupName, gps[i]->size);
      unsigned int nodeIp = _conf->id2Ip[gps[i]->nodeId];
      dstCmd->sendTo(nodeIp);
      // send data
      // extand pipelining
      redisContext* _destorCtx = RedisUtil::createContext(nodeIp);
      int pktid = 0;
      int pktnum = 0;
      uint32_t written_size = 0;
      while(written_size < gps[i]->size) {
        uint32_t this_time_size = 0;
        string append_data_key = string(gps[i]->groupName)+"_data_"+to_string(pktid++);
        if (gps[i]->size - written_size >= DEFAULT_BLOCK_SIZE) {
          this_time_size = DEFAULT_BLOCK_SIZE;
        }
        else this_time_size = gps[i]->size - written_size;
        char* buf = (char*)calloc(this_time_size + 4, sizeof(char));
        uint32_t tmp_size = htonl(this_time_size);
        memcpy(buf, (char*)&tmp_size, 4);
        memcpy(buf + 4, gps[i]->data + written_size, this_time_size);
        
        redisAppendCommand(_destorCtx, "RPUSH %s %b", 
                            append_data_key.c_str(), 
                            buf,
                            this_time_size+4);
        written_size += this_time_size;
        free(buf);
      }
      pktnum = pktid;
      // cout << append_data_key.c_str() << endl;

      
      // std::cout << "Worker::debug::Command type 0, groupName is " << gps[i]->groupName
      //           << " data size is " << gps[i]->size
      //           // << " data is " << gps[i]->data
      //           << std::endl;
      
      for (int i = 0; i < pktnum; i++) {
        redisReply* destorrReply;
        redisGetReply(_destorCtx, (void**)&destorrReply);
        freeReplyObject(destorrReply);
      }

      
      // wait for finished
      string wait_finished_key = string(gps[i]->groupName)+"_data_finished";
      redisReply* destorrReply = (redisReply*)redisCommand(_destorCtx, "blpop %s 0", wait_finished_key.c_str());
      freeReplyObject(destorrReply);
      
      cout << "Worker::group:" << string(gps[i]->groupName) << " finished!" << endl;

      redisFree(_destorCtx);

      // for debug
      // std::cout << "Worker::debug::Command send to " << RedisUtil::ip2Str(_conf->id2Ip[gps[i]->nodeId]) << std::endl;

      // sendThrd[i] = thread([&](){
      //   destorCommand *dstCmd = new destorCommand();
      //   dstCmd->buildType0(0, (const char*)gps[i]->groupName, (const char*)gps[i]->data);
      //   unsigned int nodeIp = _conf->id2Ip(gps[i]->nodeId)
      //   dstCmd->sendTo(nodeIp);
      // });
      // close(fd);
      delete dstCmd;
    }

    // for(int i = 0; i < gps.size(); i++) {
    //   sendThrd[i].join();
    // }

}

void Worker::clientRead(AgentCommand *agCmd) {
  // 1. get filename
  string filename = agCmd->getFilename();

  /* Lin */
  // 2. get fileRecipe (file to groups) from EChash
  struct fileRecipe* fr = getFileRecipe(filename.c_str());
  if(fr == NULL) {

  }

  /* Qi */
  // 3. send reuqests to each node to get group (call queue_term when the last chunk is poped)
  for(int i = 0; i < fr->num; i++) {
    char* groupName = fr->gm[i].groupName;
    uint64_t nodeId = fr->gm[i].nodeId;
    unsigned int nodeIp = _conf->id2Ip[nodeId];

    // TO DO:

  }

  // 4. receive group (chunking, ending flag is queue_term)
  // TO DO:
  start_receive_phase();

  /* Zewen */
  // 5. assemble file
  start_assemble_phase();

  stop_receive_pash();

  stop_assemble_phase();

}
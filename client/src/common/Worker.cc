#include "Worker.hh"

Worker::Worker(Config* conf) : _conf(conf)
{
    // create local context
    try
    {
        _processCtx = RedisUtil::createContext(_conf->_localIp);
        _localCtx = RedisUtil::createContext(_conf->_localIp);
        _destorCtx = RedisUtil::createContext(_conf->_coorIp);
    }
    catch (int e)
    {
        // TODO: error handling
        cerr << "initializing redis context error" << endl;
    }
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
    string filepath = agCmd->getFilename();
    int filesize = agCmd->getFilesize();

    /* This part is for Zewen */

    // open file (read to the memory pool)
    int fd = openFile(filepath.c_str());
    assert(fd != -1);

    // file to groups
    vector<struct group*> gps = split2Groups(fd);
    
    for(auto gp : gps) {
      // delegate fingerprint of groups
      fingerprint fp = delegate(gp);
      gp->delegate = fp;

      // get node id by consistent hash
      int nodeId = getNodeId(gp);
      gp->nodeId = nodeId;
    }

    /* This part is for Lin */

    // generate file recipe
    struct fileRecipe* fr = getFileRecipe(filepath.c_str(), gps);
    assert(fr != NULL);

    // set file recipe by echash
    int ret = setFileRecipe(fr);
    assert(ret == 0);

    // distribute groups to different nodes
    for(int i = 0; i < gps.size(); i++) {
      destorCommand *dstCmd = new destorCommand();
      dstCmd->buildType0(0, (const char*)gps[i]->groupName, (const char*)gps[i]->data);
      unsigned int nodeIp = _conf->id2Ip(gps[i]->nodeId)
      dstCmd->sendTo(nodeIp);
    }
}
#include "StripeStore.hh"

StripeStore::StripeStore(Config* conf) {
  _conf = conf;
  // by default, encode scheduling is delayed
  _enableScan = false;
  _enableRepair = false;
  _startTime = false;
  _enableSetStart = false;

//   if (_conf->_repair_scheduling == "delay") _enableRepair = false;
//   else if (_conf->_repair_scheduling == "threshold") _enableRepair = false;
//   else _enableRepair = true;

  // TODO: if metaStore exists, we need to load metadata into memory
  //       if not, we need to create a metaStore 
  // check whether entryStore exists, and read data from entryStore
  ifstream entryStore(_entryStorePath);
  if (entryStore.is_open()) {
    cout << "StripeStore::read entryStore" << endl;
    string line;
    while (getline(entryStore, line)) {
      SSEntry* ssentry = new SSEntry(line);
      ssentry->dump();
      // insert ssentry into stripestore
      insertEntry(ssentry);
    }
    entryStore.close();
  }
  // check whether poolStore exists, and read data from poolStore
  ifstream poolStore(_poolStorePath);
  if (poolStore.is_open()) {
    cout << "StripeStore::read poolStore" << endl;
    string line;
    while (getline(poolStore, line)) {
      vector<string> entryitems = RedisUtil::str2container(line);
      string ecpoolid = entryitems[0];
      string ecid = _conf->_offlineECMap[ecpoolid];
      int basesizeMB = _conf->_offlineECBase[ecpoolid];
      ECPolicy* ecpolicy = _conf->_ecPolicyMap[ecid];
      OfflineECPool* ecpool = getECPool(ecpoolid, ecpolicy, basesizeMB);
      ecpool->constructPool(entryitems);
    }
    poolStore.close();
  }
}

bool StripeStore::existEntry(string filename) {
  unordered_map<string, SSEntry*>::iterator it = _ssEntryMap.find(filename);
  return it == _ssEntryMap.end() ? false:true;
}

void StripeStore::insertEntry(SSEntry* entry) {
  unordered_map<string, SSEntry*>::iterator it = _ssEntryMap.find(entry->getFilename());
  if (it == _ssEntryMap.end()) {
    // the entry does not exist, insert this entry
    _lockSSEntryMap.lock();
    _ssEntryMap.insert(make_pair(entry->getFilename(), entry));
    _lockSSEntryMap.unlock();

    for (auto obj: entry->getObjlist()) {
      _lockObjEntryMap.lock();
      _objEntryMap.insert(make_pair(obj, entry));
      _lockObjEntryMap.unlock();
    }
  } else {
//     // the entry exist, only need to update the entry
//     _lockSSEntryMap.lock();
//     SSEntry* oldEntry = _ssEntryMap[entry->_filename];
//     _ssEntryMap[entry->_filename] = entry;
//     _lockSSEntryMap.unlock();
//     // need to delete the old entry?
//     delete oldEntry;
  }

  // TODO: add it to metaStore
//  string entrystr = entry->toString();
//  backupEntry(entrystr);
//  cout << "StripeStore::insertEntry.entrystr: " << entrystr << endl;
}

SSEntry* StripeStore::getEntry(string filename) {
  if (existEntry(filename)) return _ssEntryMap[filename];
  else return NULL;
}

SSEntry* StripeStore::getEntryFromObj(string objname) {
  if (_objEntryMap.find(objname) != _objEntryMap.end()) return _objEntryMap[objname];
  else return NULL;
}

void StripeStore::insertECPool(string ecpoolid, OfflineECPool* pool) {
  assert (_offlineECPoolMap.find(ecpoolid) == _offlineECPoolMap.end());
  _lockECPoolMap.lock();
  _offlineECPoolMap.insert(make_pair(ecpoolid, pool));
  _lockECPoolMap.unlock();
}

OfflineECPool* StripeStore::getECPool(string ecpoolid, ECPolicy* ecpolicy, int basesize) {
  OfflineECPool* toret;
  _lockECPoolMap.lock();
  if (_offlineECPoolMap.find(ecpoolid) != _offlineECPoolMap.end()) toret = _offlineECPoolMap[ecpoolid];
  else {
    toret = new OfflineECPool(ecpoolid, ecpolicy, basesize);
    _offlineECPoolMap.insert(make_pair(ecpoolid, toret));
  }
  _lockECPoolMap.unlock();
  return toret;
}

OfflineECPool* StripeStore::getECPool(string poolname) {
  // TODO: the poolname must exist!
  OfflineECPool* toret;
  _lockECPoolMap.lock();
  unordered_map<string, OfflineECPool*>::iterator it = _offlineECPoolMap.find(poolname);
  assert (it != _offlineECPoolMap.end());
  _lockECPoolMap.unlock();
  return _offlineECPoolMap[poolname];
}

int StripeStore::getControlLoad(unsigned int ip) {
  int toret;
  _lockCLMap.lock();
  unordered_map<unsigned int, int>::iterator it = _controlLoadMap.find(ip);
  if (it == _controlLoadMap.end()) toret = 0;
  else toret = _controlLoadMap[ip];
  _lockCLMap.unlock();
  return toret;
}

void StripeStore::increaseControlLoadMap(unsigned int ip, int load) {
  _lockCLMap.lock();
  unordered_map<unsigned int, int>::iterator it = _controlLoadMap.find(ip);
  if (it == _controlLoadMap.end()) _controlLoadMap.insert(make_pair(ip, load));
  else _controlLoadMap[ip] += load;
  _lockCLMap.unlock();
}

int StripeStore::getDataLoad(unsigned int ip) {
  int toret;
  _lockDLMap.lock();
  unordered_map<unsigned int, int>::iterator it = _dataLoadMap.find(ip);
  if (it == _dataLoadMap.end()) toret = 0;
  else toret = _dataLoadMap[ip];
  _lockDLMap.unlock();
  return toret;
}

void StripeStore::increaseDataLoadMap(unsigned int ip, int load) {
  _lockDLMap.lock();
  unordered_map<unsigned int, int>::iterator it = _dataLoadMap.find(ip);
  if (it == _dataLoadMap.end()) _dataLoadMap.insert(make_pair(ip, load));
  else _dataLoadMap[ip] += load;
  _lockDLMap.unlock();
}

int StripeStore::getRepairLoad(unsigned int ip) {
  int toret;
  _lockRLMap.lock();
  unordered_map<unsigned int, int>::iterator it = _repairLoadMap.find(ip);
  if (it == _repairLoadMap.end()) toret = 0;
  else toret = _repairLoadMap[ip];
  _lockRLMap.unlock();
  return toret;
}

void StripeStore::increaseRepairLoadMap(unsigned int ip, int load) {
  _lockRLMap.lock();
  unordered_map<unsigned int, int>::iterator it = _repairLoadMap.find(ip);
  if (it == _repairLoadMap.end()) _repairLoadMap.insert(make_pair(ip, load));
  else _repairLoadMap[ip] += load;
  _lockRLMap.unlock();
}


int StripeStore::getEncodeLoad(unsigned int ip) {
  int toret;
  _lockELMap.lock();
  unordered_map<unsigned int, int>::iterator it = _encodeLoadMap.find(ip);
  if (it == _encodeLoadMap.end()) toret = 0;
  else toret = _encodeLoadMap[ip];
  _lockELMap.unlock();
  return toret;
}

void StripeStore::increaseEncodeLoadMap(unsigned int ip, int load) {
  _lockELMap.lock();
  unordered_map<unsigned int, int>::iterator it = _encodeLoadMap.find(ip);
  if (it == _encodeLoadMap.end()) _encodeLoadMap.insert(make_pair(ip, load));
  else _encodeLoadMap[ip] += load;
  _lockELMap.unlock();
}

void StripeStore::setECStatus(int op, string ectype) {
  if (ectype == "encode") {
    if (op == 1) _enableScan = true;
    else _enableScan = false;
  } else if (ectype == "repair") {
    if (op == 1) _enableRepair = true;
    else _enableRepair = false;
  }
}

// offline encoding
void StripeStore::scanning() {
  int concurrentNum = _conf->_ec_concurrent;
  while(true) {
    std::this_thread::sleep_for (std::chrono::seconds(1));
    if (!_enableScan) continue;
    // offline encoding is enabled
    cout << "StripeStore::scanning the pendingECQueue" << endl;
    _lockPECQueue.lock();
    int ecInProgressNum = getECInProgressNum();
    cout << "StripeStore::pendingECQueue.size = " << _pendingECQueue.getSize() << ", ecInProgress = "  << ecInProgressNum << ", concurrentNum = " << concurrentNum << endl;
    while (_pendingECQueue.getSize() && ecInProgressNum < concurrentNum) {
      pair<string, string> curpair = _pendingECQueue.pop();
      string ecpoolid = curpair.first;
      string stripename = curpair.second;
      startECStripe(stripename);

      // send offline encode request to coordinator
      CoorCommand* coorCmd = new CoorCommand();
      coorCmd->buildType4(4, _conf->_localIp, ecpoolid, stripename);
      coorCmd->sendTo(_conf->_coorIp);
      delete coorCmd;

      // obtain latest ecInProgress
      ecInProgressNum = getECInProgressNum();
    } 
    _lockPECQueue.unlock();
  }
}

void StripeStore::addEncodeCandidate(string ecpoolid, string stripename) {
  _lockPECQueue.lock();
  _pendingECQueue.push(make_pair(ecpoolid, stripename));
  _lockPECQueue.unlock();
}

int StripeStore::getECInProgressNum() {
  int toret;
  _lockECInProgress.lock();
  toret = _ECInProgress.size();
  _lockECInProgress.unlock();
  return toret;
}

void StripeStore::startECStripe(string stripename) {
  _lockECInProgress.lock();
  if (_ECInProgress.size() == 0) gettimeofday(&_startEnc, NULL);
  _ECInProgress.push_back(stripename);
  _lockECInProgress.unlock();
}

void StripeStore::finishECStripe(OfflineECPool* pool, string stripename) {
  _lockECInProgress.lock();
  vector<string>::iterator pos = find(_ECInProgress.begin(), _ECInProgress.end(), stripename);
  if (pos != _ECInProgress.end()) _ECInProgress.erase(pos);
  if (_ECInProgress.size() == 0) {
    // _enableScan = false;
    gettimeofday(&_endEnc, NULL);
    cout << "StripeStore::finishECStripe.encodeTime = " << RedisUtil::duration(_startEnc, _endEnc) << endl;
  }
  _lockECInProgress.unlock();

  // we need to backup offlineecpool
  backupPoolStripe(pool->stripe2String(stripename));
}

int StripeStore::getRPInProgressNum() {
  _lockRPInProgress.lock();
  int toret = _RPInProgress.size();
  _lockRPInProgress.unlock();
  return toret;
}


void StripeStore::addLostObj(string objname) {
  // check whether objname is in _RPInProgress
  bool inrepair = false;
  _lockRPInProgress.lock();
  if (find(_RPInProgress.begin(), _RPInProgress.end(), objname) != _RPInProgress.end()) {
    inrepair = true;
  }
  _lockRPInProgress.unlock();
  if (inrepair) return;
  
  _lockLostMap.lock();
  _lockStripeErrNum.lock();
  _lockOfflineLostStripe.lock();
  if (_lostMap.find(objname) == _lostMap.end()) {
    _lostMap.insert(make_pair(objname, 1));
    if (_conf->_repair_scheduling == "delay" && objIsOffline(objname)) {
      string stripename;
      OfflineECPool *ecpool = offlineObjToEcpool(objname);
      stripename = ecpool->getStripeForObj(objname);
      if (_stripeErrNum.find(stripename) == _stripeErrNum.end()) {
        _stripeErrNum.insert(make_pair(stripename, 1));
        _offlineLostStripe.insert(make_pair(stripename, vector<string>{objname}));
      }
      else {
        _stripeErrNum[stripename]++;
        _offlineLostStripe[stripename].push_back(objname);
      }
    }
  } else {
    _lostMap[objname]++;
  }
  // cout << "[debug] objname is " << objname << ", LostMap size is " << _lostMap.size() << endl;
  _lockLostMap.unlock();
  _lockStripeErrNum.unlock();
  _lockOfflineLostStripe.unlock();
}

void StripeStore::scanRepair() {
  int concurrentNum = _conf->_ec_concurrent;
  while (true) {
     std::this_thread::sleep_for(std::chrono::seconds(1));
     if (!_enableRepair) continue;
     cout << "StripeStore::scanning repair queue" << endl;
     int rpInProgressNum = getRPInProgressNum();
     cout << "StripeStore::scanRepair.rpInProgressNum = " << rpInProgressNum << endl;
     cout << "_lostMap.size: " << _lostMap.size() << endl;
     while (_lostMap.size() && (rpInProgressNum < concurrentNum)) {
       string objname;
       // search the lost map and find the one with the most request num
       int maxreq=0;
       _lockLostMap.lock();
       for (auto item: _lostMap) {
         if (item.second > maxreq) {
           maxreq = item.second;
           objname = item.first;
         }
       }
       _lockLostMap.unlock();
 
       // now we have the obj with the most request num

      //  if (_conf->_repair_scheduling == "threshold") {
      //    if (!_enableRepair) continue;
      //    if (maxreq < _conf->_repair_threshold) continue;
      //  }

      // type is online, go to normal.
      if (_conf->_repair_scheduling == "delay" 
            && objIsOffline(objname)) {
        // delay
        OfflineECPool *ecpool = offlineObjToEcpool(objname);
        ECPolicy *ecpolicy = ecpool->getEcpolicy();
        string stripename = ecpool->getStripeForObj(objname);
        _lockStripeErrNum.lock();
        assert(_stripeErrNum.find(stripename) != _stripeErrNum.end());
        // number of errs < some value, continue.
        // debug
        int errnum = _stripeErrNum[stripename];
        // threshold is temporarily set m.
        int threshold = (int)((ecpolicy->getN() - ecpolicy->getK())*DELAY_THRESHOLD);
        cout << "in SS:" << endl << "\t\tobjname is " << objname <<
            ", stripe name is " << stripename << ", failed num is " << 
            errnum;

        _lockLostMap.lock();
        _lockOfflineLostStripe.lock();

        if (errnum < threshold) {
          cout << ". delay..." << endl;
          _lostMap[objname] /= 2; 

          _lockStripeErrNum.unlock();
          _lockLostMap.unlock();
          _lockOfflineLostStripe.unlock();
          break;
        }

        //assert(_stripeErrNum[stripename] == threshold);
        cout << ". repair..." << endl;
        unordered_map<string, vector<string>>::iterator olsit = _offlineLostStripe.find(stripename);
        assert(olsit != _offlineLostStripe.end());


        CoorCommand *coorCmd = new CoorCommand();
        coorCmd->buildType8(8, _conf->_localIp, objname);
        coorCmd->sendTo(_conf->_coorIp);
        delete coorCmd;

        for (int i = 0; i < olsit->second.size(); i++)
        {
          string tmpobjname = olsit->second.at(i);

          // now we move the obj to RPInProgress
          startRepair(tmpobjname);

          _lostMap.erase(_lostMap.find(tmpobjname));
        }

        _lockStripeErrNum.unlock();
        _lockLostMap.unlock();
        _lockOfflineLostStripe.unlock();

      }
      else {
        // normal
        // send repair request to coordinator
        CoorCommand *coorCmd = new CoorCommand();
        coorCmd->buildType8(8, _conf->_localIp, objname);
        coorCmd->sendTo(_conf->_coorIp);

        delete coorCmd;

        // now we move the obj to RPInProgress
        startRepair(objname);
        _lockLostMap.lock();
        _lostMap.erase(_lostMap.find(objname));
        _lockLostMap.unlock();
      }
      rpInProgressNum = getRPInProgressNum();
    }
  }
}

void StripeStore::startRepair(string objname) {
  _lockRPInProgress.lock();
  _lockEnableSetStart.lock();
  if (!_startTime && !_RPInProgress.size()) {
    _enableSetStart = true;
  }
  _RPInProgress.push_back(objname);
  _lockRPInProgress.unlock();
  _lockEnableSetStart.unlock();
}

void StripeStore::finishRepair(string objname) {
  _lockRPInProgress.lock();
  _lockStripeErrNum.lock();
  vector<string>::iterator pos = find(_RPInProgress.begin(), _RPInProgress.end(), objname);
  if (pos != _RPInProgress.end()) _RPInProgress.erase(pos);
  if (_conf->_repair_scheduling == "delay" && objIsOffline(objname)) {
    OfflineECPool *ecpool = offlineObjToEcpool(objname);
    string stripename = ecpool->getStripeForObj(objname);
    assert(_stripeErrNum.find(stripename) != _stripeErrNum.end());
    if (--_stripeErrNum[stripename] == 0) {
      _stripeErrNum.erase(stripename);
      _offlineLostStripe.erase(stripename);
    }
  }
  gettimeofday(&_endRP, NULL);
  // cout << "[debug] Repair::finishObj " << objname << ", time is " << RedisUtil::duration(_startRP, _endRP) << endl;
  if (_RPInProgress.size() == 0 && _lostMap.size() == 0 && _startTime) {
    // _enableRepair = false;
    _startTime = false;
    gettimeofday(&_endRP, NULL);
    double repairtime = RedisUtil::duration(_startRP, _endRP);
    cout << "Repair::finishECStripe.repairTime = " << repairtime << endl;
    // get ecclass
    if (objIsOffline(objname)) {
      OfflineECPool* ecpool = offlineObjToEcpool(objname);
      ECPolicy* ecpolicy = ecpool->getEcpolicy();
      string ecid = ecpolicy->getPolicyId();
      string ecclass = ecpolicy->getClassName();
      _log.logRepairTime(ecid, ecclass, repairtime);
    }
   
  }
  _lockRPInProgress.unlock();
  _lockStripeErrNum.unlock();
}

void StripeStore::backupEntry(string entrystr) {
  struct timeval time1, time2;
  gettimeofday(&time1, NULL);
  _lockEntryStore.lock();
  _entryStore.open(_entryStorePath, ios::out | ios::app);
  _entryStore << entrystr;
  _entryStore.close();
  _lockEntryStore.unlock();
  gettimeofday(&time2, NULL);
//  cout << "StripeStore::backupEntry.duration = " << RedisUtil::duration(time1, time2) << endl;
}

void StripeStore::backupPoolStripe(string poolstr) {
  struct timeval time1, time2;
  gettimeofday(&time1, NULL);
  _lockPoolStore.lock();
  _poolStore.open(_poolStorePath, ios::out | ios::app);
  _poolStore << poolstr;
  _poolStore.close();
  _lockPoolStore.unlock();
  gettimeofday(&time2, NULL);
//  cout << "StripeStore::backupPool.duration = " << RedisUtil::duration(time1, time2) << endl;
}

bool StripeStore::objIsOffline(string objname)
{
  unordered_map<string, SSEntry*>::iterator objit = _objEntryMap.find(objname);
  assert(objit != _objEntryMap.end());
  return (*objit).second->getType();
}

OfflineECPool* StripeStore::offlineObjToEcpool(string objname) {
  OfflineECPool *ecpool = nullptr;
  if (objIsOffline(objname)) {
    // get ecpool
    unordered_map<string, SSEntry*>::iterator objit = _objEntryMap.find(objname);
    string ecpoolid = (*objit).second->getEcidpool();
    ecpool = getECPool(ecpoolid);
  }
  return ecpool;
}

vector<string> StripeStore::getLostObjs(string stripename) {
  assert (_offlineLostStripe.find(stripename) != _offlineLostStripe.end());
  vector<string> toret;
  _lockOfflineLostStripe.lock();
  for (int i = 0; i < _offlineLostStripe[stripename].size(); i++) {
    toret.push_back(_offlineLostStripe[stripename].at(i));
  }
  _lockOfflineLostStripe.unlock();
  return toret;
}

bool StripeStore::findIfInLostMapOrRPProc(string objname) {
  _lockLostMap.lock();
  _lockRPInProgress.lock();

  bool res = _lostMap.find(objname) != _lostMap.end();
  res =  res ||
    (find(_RPInProgress.begin(), _RPInProgress.end(), objname) != _RPInProgress.end());
    
  _lockLostMap.unlock();
  _lockRPInProgress.unlock();
  return res;
}

void StripeStore::setStartTime(bool status) {
  _startTime = status;
  if (status){
    
    gettimeofday(&_startRP, NULL);
  }
}

void StripeStore::setEnableStart() {
  _lockEnableSetStart.lock();
  if (_enableSetStart) {
    setStartTime(true);
    _enableSetStart = false;
  }
  _lockEnableSetStart.unlock();
}
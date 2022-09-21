#ifndef _ECPOLICY_HH_
#define _ECPOLICY_HH_

#include "BUTTERFLY64.hh"
#include "DRC643.hh"
#include "DRC963.hh"
#include "ECBase.hh"
#include "IA.hh"
#include "RSBINDX.hh"
#include "RSCONV.hh"
#include "RSPIPE.hh"
#include "RSPPR.hh"
#include "WASLRC.hh"
#include "RSAS.hh"
#include "RSASP.hh"
#include "RSRPCONV.hh"
#include "RSPPCT.hh"
#include "RSNCONV.hh"
#include "RSSMARTLZ.hh"
#include "LRC32202.hh"
#include "CL262059.hh"
// #include "Hitchhiker.hh"
#include "RSNSYS.hh"
#include "Clay.hh"
#include "../inc/include.hh"

using namespace std;

class ECPolicy {
  private:
    string _id;
    string _classname;
    int _n;
    int _k;
    int _w;
    bool _locality = false;
    int _opt;
    int _createdRPConvClass;
    int _nconvBindy;

    vector<string> _param;
    string _smlzSch = "single"; // multi
    bool _smlzConvEnhanced = false;
    bool _smlzRPEnhanced = false;
  public:
//    ECPolicy(string id, string classname, int n, int k, int w, bool locality, int opt, vector<string> param);
    ECPolicy(string id, string classname, int n, int k, int w, int opt, vector<string> param);
    ECBase* createECClass();
    string getPolicyId();
    string getClassName();
    int getN();
    int getK();
    int getW();
    bool getLocality();
    int getOpt();
    void setSMLZSch(string str);
    void setSMLZConvEnhanced(bool x);
    void setSMLZRPEnhanced(bool x);
};

#endif

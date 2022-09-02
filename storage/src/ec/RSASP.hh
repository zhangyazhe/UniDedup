#ifndef _RSASP_HH_
#define _RSASP_HH_

#include "../inc/include.hh"
#include "Computation.hh"

#include "ECBase.hh"

#define RSASP_N_MAX 32

using namespace std;

class RSASP : public ECBase {
  private:
    int _m;
    int _encode_matrix[RSASP_N_MAX * RSASP_N_MAX];

    void generate_matrix(int* matrix, int rows, int cols, int w);
  public:
    RSASP(int n, int k, int w, int opt, vector<string> param);
    ECDAG* Encode();
    ECDAG* Decode(vector<int> from, vector<int> to);
    ECDAG* DelayRepair();
    void Place(vector<vector<int> >& group);
};

#endif

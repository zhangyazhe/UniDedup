#ifndef _RSSMARTLZ_HH_
#define _RSSMARTLZ_HH_

#include "../inc/include.hh"
#include "Computation.hh"

#include "ECBase.hh"

#define RSSMARTLZ_N_MAX 128

#define RPNUM 1
#define CONVNUM 2

using namespace std;

class RSSMARTLZ : public ECBase {
  private:
    int _m;
    int _encode_matrix[RSSMARTLZ_N_MAX * RSSMARTLZ_N_MAX];
    int _convbindY;

    void generate_matrix(int* matrix, int rows, int cols, int w);
  public:
    RSSMARTLZ(int n, int k, int w, int opt, vector<string> param);
    ECDAG* Encode();
    ECDAG* Decode(vector<int> from, vector<int> to);
    void Place(vector<vector<int> >& group);
    void setConvBindY(int y);
};

#endif

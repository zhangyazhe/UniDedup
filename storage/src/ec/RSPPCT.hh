#ifndef _RSPPCT_HH_
#define _RSPPCT_HH_

#include "../inc/include.hh"
#include "Computation.hh"

#include "ECBase.hh"

#define RSPPCT_N_MAX 128

using namespace std;

class RSPPCT : public ECBase {
  private:
    int _m;
    int _encode_matrix[RSPPCT_N_MAX * RSPPCT_N_MAX];

    void generate_matrix(int* matrix, int rows, int cols, int w);
  public:
    RSPPCT(int n, int k, int w, int opt, vector<string> param);
    ECDAG* Encode();
    ECDAG* Decode(vector<int> from, vector<int> to);
    void Place(vector<vector<int> >& group);
};

#endif

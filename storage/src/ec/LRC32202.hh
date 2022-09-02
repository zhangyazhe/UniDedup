#ifndef _LRC32202_HH_
#define _LRC32202_HH_

#include "Computation.hh"
#include "ECBase.hh"

using namespace std;

#define LRC32202_N_MAX (128)

class LRC32202 : public ECBase {
  private:
    int _l;
    int _r;
    int _encode_matrix[LRC32202_N_MAX * LRC32202_N_MAX];     

    void generate_matrix(int* matrix, int k, int l, int r, int w);
  public:
    LRC32202(int n, int k, int cps, int opt, vector<string> param);

    ECDAG* Encode();
    ECDAG* Decode(vector<int> from, vector<int> to);
    void Place(vector<vector<int>>& group);
};

#endif

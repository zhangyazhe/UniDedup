#ifndef _CL262059_HH_
#define _CL262059_HH_

#include "Computation.hh"
#include "ECBase.hh"

using namespace std;

#define CL_N_MAX (128)

class CL262059 : public ECBase {
  private:
    int _l;
    int _r;
    int _encode_matrix[CL_N_MAX * CL_N_MAX];     

    void generate_matrix(int* matrix, int k, int l, int r, int w);
  public:
    CL262059(int n, int k, int cps, int opt, vector<string> param);

    ECDAG* Encode();
    ECDAG* Decode(vector<int> from, vector<int> to);
    void Place(vector<vector<int>>& group);
};

#endif

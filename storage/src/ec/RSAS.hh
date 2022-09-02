#ifndef _RSAS_HH_
#define _RSAS_HH_

#include "../inc/include.hh"
#include "Computation.hh"

#include "ECBase.hh"

#define RSAS_N_MAX 32

#define RSCONV_DEBUG_ENABLE false

using namespace std;

class RSAS : public ECBase {
  private:
    int _m;
    int _encode_matrix[RSAS_N_MAX * RSAS_N_MAX];

    void generate_matrix(int* matrix, int rows, int cols, int w);
  public:
    RSAS(int n, int k, int w, int opt, vector<string> param);
    ECDAG* Encode();
    ECDAG* Decode(vector<int> from, vector<int> to);
    void Place(vector<vector<int> >& group);
};

#endif

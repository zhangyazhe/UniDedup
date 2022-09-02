#ifndef _RSNCONV_HH_
#define _RSNCONV_HH_

#include "../inc/include.hh"
#include "Computation.hh"

#include "ECBase.hh"

using namespace std;

#define RSNCONV_N_MAX 128

class RSNCONV : public ECBase {
  private:
    int _encode_matrix[RSNCONV_N_MAX * RSNCONV_N_MAX];
    int _m;
    int _convbindY;

    void generate_matrix(int* matrix, int rows, int cols, int w);  // This w is for galois field, which is different from our sub-packetization level _w

  public:
    RSNCONV(int n, int k, int w, int opt, vector<string> param);
 
    ECDAG* Encode();
    ECDAG* Decode(vector<int> from, vector<int> to);
    void Place(vector<vector<int>>& group);
    void setConvBindY(int y);
};

#endif

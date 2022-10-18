#ifndef _RSNSYS_HH_
#define _RSNSYS_HH_

#include "../inc/include.hh"
#include "Computation.hh"

#include "ECBase.hh"

using namespace std;

#define RSNSYS_N_MAX (256)

class RSNSYS : public ECBase {
  private:
    // (k + m)*m, k*m
    int _encode_matrix[RSNSYS_N_MAX * RSNSYS_N_MAX];
    // m * (k + m -1)
    int _repair_matrix[RSNSYS_N_MAX * RSNSYS_N_MAX];
    int _m;
    int _convbindY;

    void generate_matrix(int* matrix, int rows, int cols, int w);  // This w is for galois field, which is different from our sub-packetization level _w

  public:
    static int _row_idx;
    static int _central_idx;
    mutex _lockCentIdx;

    RSNSYS(int n, int k, int w, int opt, vector<string> param);
 
    ECDAG* Encode();
    ECDAG* Decode(vector<int> from, vector<int> to);
    ECDAG* NormalRead();
    void Place(vector<vector<int>>& group);
};

#endif

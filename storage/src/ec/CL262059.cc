#include "CL262059.hh"

CL262059::CL262059(int n, int k, int w, int opt, vector<string> param) {
  _n = n;
  _k = k;
  _w = w;
  _opt = opt;
  // there should be two other parameters in param
  // 0. l (local parity num)
  // 1. r (global parity num)
  _l = 4;
  _r = 2;

  memset(_encode_matrix, 0, _n * _k * sizeof(int));
}

ECDAG* CL262059::Encode() {
  ECDAG* ecdag = new ECDAG();
  vector<int> data;
  vector<int> code;
  for (int i=0; i<_k; i++) data.push_back(i);
  for (int i=_k; i<_n; i++) code.push_back(i);
  
  generate_matrix(_encode_matrix, _k, _l, _r, 8); 
  for (int i=0; i<code.size(); i++) {
    vector<int> coef;
    for (int j=0; j<_k; j++) {
      coef.push_back(_encode_matrix[(i+_k)*_k+j]);
    }
    ecdag->Join(code[i], data, coef);
  } 
  if (code.size() > 1) ecdag->BindX(code);
  return ecdag;
}

ECDAG* CL262059::Decode(vector<int> from, vector<int> to) {
  ECDAG* ecdag = new ECDAG();
  if (to.size() == 1) {
    // can recover by local parity
    int ridx = to[0];
    vector<int> data;
    vector<int> coef;
    // nr = 5
    int nr = _k/_l;
    int tmpname = BINDSTART + 320;
    if (ridx < _k) {
      // source data
      int gidx = ridx/nr;
      int inneridx = ridx % nr;
      vector<int> tmpd;
      vector<int> tmpc;
      int tobindy;
      if (inneridx < 3) {
        for (int i = 0; i < 3; i++) {
            int idxinstripe = gidx * nr + i;
            if (ridx != idxinstripe) {
                data.push_back(idxinstripe);
                coef.push_back(1);
            }
        }
        
        for (int i = 3; i < nr; i++) {
            int idxinstripe = gidx * nr + i;
            tmpd.push_back(idxinstripe);
            tmpc.push_back(1);
        }
        tmpd.push_back(_k+gidx);
        tmpc.push_back(1);
        tobindy = gidx * nr + 3;
      } else {
        for (int i = 0; i < 3; i++) {
            int idxinstripe = gidx * nr + i;
            tmpd.push_back(idxinstripe);
            tmpc.push_back(1);
        }

        for (int i = 3; i < nr; i++) {
            int idxinstripe = gidx * nr + i;
            if (ridx != idxinstripe) {
                data.push_back(idxinstripe);
                coef.push_back(1);
            }
        }
        data.push_back(_k+gidx);
        coef.push_back(1);
        tobindy = gidx * nr + 0;

      }
      ecdag->Join(tmpname, tmpd, tmpc);
      ecdag->BindY(tmpname, tobindy);
      data.push_back(tmpname);
      coef.push_back(1);
    } else if (ridx < (_k+_l)) {
      // local parity
      int gidx = ridx - _k;
      vector<int> tmpd;
      vector<int> tmpc;
      int tobindy;
      for (int i = 0; i < 3; i++) {
        int idxinstripe = gidx * nr + i;
        tmpd.push_back(idxinstripe);
        tmpc.push_back(1);
      }
      for (int i=3; i<nr; i++) {
        int idxinstripe = gidx*nr+i;
        data.push_back(idxinstripe);
        coef.push_back(1);
      }
      tobindy = gidx * nr + 0;
      ecdag->Join(tmpname, tmpd, tmpc);
      ecdag->BindY(tmpname, tobindy);
      data.push_back(tmpname);
      coef.push_back(1);
    } else {
      // global parity
      generate_matrix(_encode_matrix, _k, _l, _r, 8);
      for (int i=0; i<_k; i++) {
        data.push_back(i);
        coef.push_back(_encode_matrix[ridx*_k+i]);
      }
    }
    ecdag->Join(ridx, data, coef);
  }
  // debug
  ecdag->dump();
  return ecdag;
}

void CL262059::generate_matrix(int* matrix, int k, int l, int r, int w) {
  int n = k + l + r;
  memset(matrix, 0, n*k*sizeof(int));
  // set first k lines
  for (int i=0; i<k; i++) {
    matrix[i*k+i] = 1;
  }

  // set the following l lines as local parity
  int nr = k/l;
  for (int i=0; i<l; i++) {
    for (int j=0; j<nr; j++) {
      matrix[(k+i)*k + i*nr + j] = 1;
    }
  }

  // set the last r lines
  for (int i=0; i<r; i++) {
    for (int j=0; j<l; j++) {
      int tmp = 1;
      for (int ii=0; ii<nr; ii++) {
        matrix[(k+l+i)*k+j*nr+ii]=tmp;
        tmp = Computation::singleMulti(tmp, i+2, w);
      }
    }
  }
}

void CL262059::Place(vector<vector<int>>& group) {
    for (int i = 0; i < _n; i++) {
        vector<int> tmpv;
        tmpv.push_back(i);
        group.push_back(tmpv);
    }
}

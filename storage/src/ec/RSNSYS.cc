#include "RSNSYS.hh"

RSNSYS::RSNSYS(int n, int k, int w, int opt, vector<string> param) {
  _n = n;
  _k = k;
  _w = w;
  _opt = opt;
  _convbindY = 0;
  _row_idx = 0;

  _m = _n - _k;

  // one chunk should have _m slices.
  assert(_m == _w);

  // init _encode_matrix
  generate(_encode_matrix, (_k+_m)*_m, _k*_m, 8);

  // init _repair_matrix
  generate(_repair_matrix, _m, _k + _m -1, 8);
 
}

ECDAG* RSNSYS::Encode() {
    ECDAG* ecdag = new ECDAG();
    vector<int> data;
    vector<int> code;
    for (int i=0; i<_k*_w; i++) data.push_back(i);
    
    // generate_matrix(_encode_matrix, _n, _k, 8);

    // the coefs in (chunk_i, slice_j) is at the (i*_m+j)*_m row in _encode_matrix
    vector<int> tobindx;
    for (int i = 0; i < _n; i++) {
      for (int j = 0; j < _w; j++) {
        vector<int> coef;
        for (int coef_i = 0; coef_i < _k*_m; coef_i++) coef.push_back(_encode_matrix[(i*_m+j)*_m + coef_i]);
        ecdag->Join(i*_w + j, data, coef);
        tobindx.push_back(i*_w+j);
      }
    }
    int vn = ecdag->BindX(tobindx);
    ecdag->BindY(vn, 0);
  
    return ecdag;
}

ECDAG* RSNSYS::Decode(vector<int> from, vector<int> to) {
  // w
  ECDAG *ecdag = new ECDAG();

  int lostidx = to[0] / _w;

  vector<int> fromsids;
  for (int i = 0; i < _n; i ++) {
    if (i != lostidx) fromsids.push_back(i);
  }

  // generate a tmp_matrix to store (_k+_m-1) slices' coefs
  int* tmp_matrix = (int*)malloc((_k+_m-1)*(_k*_m)*sizeof(int));
  int cnt = 0;
  for (int i = 0; i < _k+_m; i++) {
    if (i == lostidx) continue;
    int row = i * _m * (_k *_m) + _row_idx * _k * _m;
    memcpy(tmp_matrix + (cnt*_k*_m),
          _repair_matrix+(row),
          sizeof(int)*_k*_m);

    cnt++;
  }
  int *new_encode_matrix = jerasure_matrix_multiply(_repair_matrix, tmp_matrix,
                          _m, _k+_m-1,
                          _k+_m-1, _k*_m,
                          8);
  // update _encode_matrix
  for (int i = 0; i < _m; i++) {
    int row = lostidx*_m*_k*_m + i*_m*_k;
    memcpy(_encode_matrix+row, 
          new_encode_matrix + (i * _k * _m),
          sizeof(int) * _k * _m);
  }

  // repair lost chunk
  vector<int> data;
  vector<int> coef;
  for (int i = 0; i < _k + _m; i++) {
    if (i == lostidx) continue;
    data.push_back(i*_m + _row_idx);
  }
  vector<int> tobindx;
  for (int i = 0; i < _w; i++) {
    int cidx = lostidx*_w+i;
    vector<int> coef;
    for (int j = 0; j < _k+_m; j++) {
      if (i != lostidx) coef.push_back(_repair_matrix[i*(_k+_m-1)+j]);
    }
    ecdag->Join(cidx, data, coef);
    // tobindx.push_back(cidx);
  }
  // int vn = ecdag->BindX(tobindx);
  // ecdag->BindY(vn, data[0]);

  free(new_encode_matrix);
  free(tmp_matrix);

  _row_idx = (_row_idx + 1) % _m;
  return ecdag;
}

ECDAG* RSNSYS::NormalRead() {
  ECDAG* ecdag = new ECDAG();

  int *select_matrix = (int*)malloc((_m*_k)*(_m*k)*sizeof(int));
  int *invert_matrix = (int*)malloc((_m*_k)*(_m*k)*sizeof(int));

  // select _m*_k row from _encode_matrix
  for (int i = 0; i < _k*_m; i++)
  {
      memcpy(select_matrix + i * _k*_m,
              _encode_matrix + i * _k*_m,
              sizeof(int) * _k*_m);
  }
  jerasure_invert_matrix(select_matrix, invert_matrix, _k*_m, 8);

  vector<int> data;
  for (int i = 0; i < _k*_m; i++) data.push_back(i);
  
  vector<int> tobindx;
  for (int i = 0; i < _k; i++) {
    for (int wi = 0; wi <= _w; wi++) {
      int cidx = i*_w + wi;
      vector<int> coefs;
      for (int j = 0; j < _k*_m; j++) coefs.push_back(invert_matrix[cidx*(_k*_m)+j]);
      ecdag->Join(cidx, data, coefs);
      // tobindx.push_back(cidx);
    }
  }

  free(invert_matrix);
  free(select_matrix);

  return ecdag;
}

void RSNSYS::Place(vector<vector<int>>& group) {
  // for (int i = 0; i < _n; i++) {
  //   vector<int> tmpv;
  //   tmpv.push_back(i);
  //   group.push_back(tmpv);
  // }
}

void RSNSYS::generate_matrix(int* matrix, int rows, int cols, int w) {
  int k = cols;
  int n = rows;

  memset(matrix, 0, rows * cols * sizeof(int));

  for (int i=0; i<rows; i++) {
    int tmp = 1;
    for (int j=0; j<cols; j++) {
      matrix[i*cols+j] = tmp;
      tmp = Computation::singleMulti(tmp, i+1, w);
    }
  }
}

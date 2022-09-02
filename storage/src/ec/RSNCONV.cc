#include "RSNCONV.hh"

RSNCONV::RSNCONV(int n, int k, int w, int opt, vector<string> param) {
  _n = n;
  _k = k;
  _w = w;
  _opt = opt;
  _convbindY = 0;

  _m = _n - _k;
  memset(_encode_matrix, 0, (_k+_m) * _k * sizeof(int));
}

ECDAG* RSNCONV::Encode() {
    ECDAG* ecdag = new ECDAG();
    vector<int> data;
    vector<int> code;
    for (int i=0; i<_k; i++) data.push_back(i);
    for (int i=_k; i<_n; i++) code.push_back(i);
    // if (RSCONV_DEBUG_ENABLE) {
    //   cout << "RSCONV::Encode.data:";
    //   for (int i=0; i<data.size(); i++) cout << " " << data[i];
    //   cout << endl;
    //   cout << "RSCONV::Encode.code:";
    //   for (int i=0; i<code.size(); i++) cout << " " << code[i];
    //   cout << endl;
    // }
    
    generate_matrix(_encode_matrix, _n, _k, 8);
    for (int wi = 0; wi < _w; wi++)
    {
        for (int i = 0; i < _m; i++)
        {
            vector<int> wdata;
            vector<int> coef;
            for (int j = 0; j < _k; j++)
            {
                coef.push_back(_encode_matrix[(i + _k) * _k + j]);
            }
            for (int j = 0; j < _k; j++)
                wdata.push_back(data[j] * _w + wi);
            ecdag->Join(code[i] * _w + wi, wdata, coef);
        }
    }
    return ecdag;
}

ECDAG* RSNCONV::Decode(vector<int> from, vector<int> to) {
  // ECDAG* ecdag = new ECDAG();
  // generate_matrix(_encode_matrix, _n, _k, 8);
  // vector<int> data;
  // int _select_matrix[_k*_k];
  // for (int i=0; i<_k; i++) {
  //   data.push_back(from[i]);
  //   int sidx = from[i];
  //   memcpy(_select_matrix + i * _k,
  //          _encode_matrix + sidx * _k,
 	//    sizeof(int) * _k);
  // }
  // int _invert_matrix[_k*_k];

  // jerasure_invert_matrix(_select_matrix, _invert_matrix, _k, _k); 
  // for (int i=0; i<to.size(); i++) {
  //   int ridx = to[i];
  //   int _select_vector[_k];
  //   memcpy(_select_vector,
  //          _encode_matrix + ridx * _k,
	//    _k * sizeof(int));
  //   int* _coef_vector = jerasure_matrix_multiply(
  //       _select_vector, _invert_matrix, 1, _k, _k, _k, 8);
  //   vector<int> coef;
  //   for (int i=0; i<_k; i++) coef.push_back(_coef_vector[i]);
  //   ecdag->Join(ridx, data, coef);
  // }
  // return ecdag;

  // w
  ECDAG *ecdag = new ECDAG();

  generate_matrix(_encode_matrix, _n, _k, 8);

  vector<int> fromsids;
  vector<int> tosids;
  for (int i = 0; i < from.size(); i += _w) {
      fromsids.push_back(from[i]/_w);
  }
  for (int i = 0; i < to.size(); i += _w) {
      tosids.push_back(to[i]/_w);
  }
  // prepare coefs.
  vector<vector<int>> coefs;
  int _select_matrix[_k * _k];
  for (int i = 0; i < _k; i++)
  {
      int sidx = fromsids[i];
      memcpy(_select_matrix + i * _k,
              _encode_matrix + sidx * _k,
              sizeof(int) * _k);
  }
  int _invert_matrix[_k * _k];
  jerasure_invert_matrix(_select_matrix, _invert_matrix, _k, 8);
  for (int i = 0; i < tosids.size(); i++) {
      int ridx = tosids[i];
      int _select_vector[_k];
      memcpy(_select_vector,
              _encode_matrix + ridx * _k,
              _k * sizeof(int));
      int *_coef_vector = jerasure_matrix_multiply(
          _select_vector, _invert_matrix, 1, _k, _k, _k, 8);
      vector<int> coef;
      for (int i=0; i<_k; i++) coef.push_back(_coef_vector[i]);
      free(_coef_vector);
      coefs.emplace_back(std::move(coef));
  }
  // set ecdag
  int tmpname = BINDSTART + (_k + _m)*_w;
  for (int wi = 0; wi < _w; wi++) {
    vector<int> data;
    vector<int> tobindx;
    for (int ki = 0; ki < _k; ki++) data.push_back(fromsids[ki]*_w + wi);
    for (int i = 0; i < tosids.size(); i++) {
        int cidx = tosids[i]*_w + wi;
        ecdag->Join(cidx, data, coefs[i]);
        tobindx.push_back(cidx);
    }
    int bindxidx = ecdag->BindX(tobindx);
    ecdag->BindY(bindxidx, tosids[_convbindY % tosids.size()]*_w + wi);
  }
  return ecdag;
}

void RSNCONV::Place(vector<vector<int>>& group) {
}

void RSNCONV::generate_matrix(int* matrix, int rows, int cols, int w) {
  int k = cols;
  int n = rows;
  int m = n - k;

  memset(matrix, 0, rows * cols * sizeof(int));
  for(int i=0; i<k; i++) {
    matrix[i*k+i] = 1;
  }

  for (int i=0; i<m; i++) {
    int tmp = 1;
    for (int j=0; j<k; j++) {
      matrix[(i+k)*cols+j] = tmp;
      tmp = Computation::singleMulti(tmp, i+1, w);
    }
  }
}

void RSNCONV::setConvBindY(int y)
{
    _convbindY = y;
}


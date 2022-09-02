#include "RSPPCT.hh"

RSPPCT::RSPPCT(int n, int k, int w, int opt, vector<string> param) {
  _n = n;
  _k = k;
  _w = w;
  _opt = opt;

  _m = _n - _k;
  memset(_encode_matrix, 0, (_k+_m) * _k * sizeof(int));

}

ECDAG* RSPPCT::Encode() {
    ECDAG* ecdag = new ECDAG();
    vector<int> data;
    vector<int> code;
    for (int i=0; i<_k; i++) data.push_back(i);
    for (int i=_k; i<_n; i++) code.push_back(i);

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

ECDAG* RSPPCT::Decode(vector<int> from, vector<int> to) {
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
    // set ecdag.
    int f = tosids.size();
    int depth = _k / f + 1;
    int tmpname = BINDSTART + (_k + _m)*_w;
    assert(_k % f == 0);
    for (int wi = 0; wi < _w; wi++) {
        for (int i = 0; i < f; i++) {
            // recover the i-th lost chunk.
            vector<vector<int> > wdatas;
            vector<vector<int> > dcoefs;
            wdatas.push_back(vector<int>{});
            dcoefs.push_back(vector<int>{});
            // get the leaf nodes.
            for (int j = 0; j < f; j++) wdatas[0].push_back(fromsids[j]*_w + wi);
            for (int j = 0; j < f; j++) dcoefs[0].push_back(coefs[i][j]);
            for (int di = 1; di < depth; di++) {
                int toadd;
                if (di < depth - 1) {
                    wdatas[di-1].push_back(fromsids[di*f + i]*_w + wi);
                    dcoefs[di-1].push_back(coefs[i][di*f + i]);
                    toadd = tmpname++;
                }
                else toadd = tosids[i]*_w + wi;
                
                ecdag->Join(toadd, wdatas[di-1], dcoefs[di-1]);
                if (di < depth - 1) {
                    ecdag->BindY(toadd, fromsids[di*f + i]*_w + wi);
                    wdatas.push_back(vector<int>{});
                    dcoefs.push_back(vector<int>{});
                    for (int j = 0; j < f; j++) {
                        if (j != i) {
                            wdatas[di].push_back(fromsids[di*f + j]*_w + wi);
                            dcoefs[di].push_back(coefs[i][di*f + j]);
                        }
                        else {
                            wdatas[di].push_back(toadd);
                            dcoefs[di].push_back(1);
                        }
                    }
                    
                }

            }
        }
    }
    return ecdag;
}

void RSPPCT::Place(vector<vector<int>>& group) {
}

void RSPPCT::generate_matrix(int* matrix, int rows, int cols, int w) {
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

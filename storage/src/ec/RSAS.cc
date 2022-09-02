#include "RSAS.hh"

RSAS::RSAS(int n, int k, int w, int opt, vector<string> param)
{
    _n = n;
    _k = k;
    _w = w;
    _opt = opt;

    _m = _n - _k;
    memset(_encode_matrix, 0, (_k + _m) * _m * sizeof(int));
    if(RSCONV_DEBUG_ENABLE) cout << "RSCONV::constructor ends" << endl;
}

ECDAG* RSAS::Encode()
{
    ECDAG *ecdag = new ECDAG();

    generate_matrix(_encode_matrix, _n, _k, 8);

    assert(_k == _w);
    int bindXIdx;
    for (int wi = 0; wi < _w; wi++) {
        vector<int> data;
        vector<int> code;
        vector<int> tobindX;

        for (int ki = 0; ki < _k; ki++) data.push_back(ki*_w + wi);
        for (int mi = _k; mi < _n; mi++) code.push_back(mi*_w + wi);
        
        for (int mi = 0; mi < _m; mi++) {
            vector<int> coef;
            for (int ci = 0; ci < _k; ci++) {
                coef.push_back(_encode_matrix[(mi+_k)*_k + ci]);
            }
            ecdag->Join((mi+_k)*_w + wi, data, coef);
            tobindX.push_back((mi+_k)*_w + wi);
        }
        bindXIdx = ecdag->BindX(tobindX);
        ecdag->BindY(bindXIdx, data[wi]);
    }
    ecdag->dump();
    return ecdag;
}

ECDAG* RSAS::Decode(vector<int> from, vector<int> to)
{
    ECDAG* ecdag = new ECDAG();
    generate_matrix(_encode_matrix, _n, _k, 8);

    vector<int> notmfrom;
    vector<int> notmto;
    for (int i = 0; i < from.size(); i += _w) {
        notmfrom.push_back(from[i]/_w);
    }
    for (int i = 0; i < to.size(); i += _w) {
        notmto.push_back(to[i]/_w);
    }

    vector<int> data;
    int _select_matrix[_k * _k];
    for (int i = 0; i < _k; i++)
    {
        data.push_back(notmfrom[i]);
        int sidx = notmfrom[i];
        memcpy(_select_matrix + i * _k,
               _encode_matrix + sidx * _k,
               sizeof(int) * _k);
    }
    int _invert_matrix[_k * _k];
    jerasure_invert_matrix(_select_matrix, _invert_matrix, _k, _k);
    vector<vector<int>> coefs;
    for (int i = 0; i < notmto.size(); i++) {
        int ridx = notmto[i];
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

    for (int wi = 0; wi < _w; wi++) {
        vector<int> wdata;
        vector<int> tobindx;
        for (int ki = 0; ki < _k; ki++) wdata.push_back(data[ki]*_w + wi);
        for (int ri = 0; ri < notmto.size(); ri++) {
            int ridx = notmto[ri];
            ecdag->Join(ridx*_w + wi, wdata, coefs[ri]);
            tobindx.push_back(ridx*_w + wi);
        }
        int bindXIdx;
        if (notmto.size() > 1) {
            bindXIdx = ecdag->BindX(tobindx);
        }
        else {
            bindXIdx = ecdag->BindXSp(tobindx[0]);
        }
        ecdag->BindY(bindXIdx, wdata[wi]);
    }
    return ecdag;

}

void RSAS::Place(vector<vector<int> >& group)
{
    
}

void RSAS::generate_matrix(int* matrix, int rows, int cols, int w)
{
    int k = cols;
    int n = rows;
    int m = n - k;

    memset(matrix, 0, rows * cols * sizeof(int));
    for (int i = 0; i < k; i++)
    {
        matrix[i * k + i] = 1;
    }

    for (int i = 0; i < m; i++)
    {
        int tmp = 1;
        for (int j = 0; j < k; j++)
        {
            matrix[(i + k) * cols + j] = tmp;
            tmp = Computation::singleMulti(tmp, i + 1, w);
        }
    }
}



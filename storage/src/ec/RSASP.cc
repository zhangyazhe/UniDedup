#include "RSASP.hh"

RSASP::RSASP(int n, int k, int w, int opt, vector<string> param)
{
    _n = n;
    _k = k;
    _w = w;
    _opt = opt;

    _m = _n - _k;
    memset(_encode_matrix, 0, (_k + _m) * _m * sizeof(int));
}

ECDAG* RSASP::Encode()
{
    // vector<int> from;
    // vector<int> to;
    // for (int i = 0; i < _n; i++)
    // {
    //     if (i < k)
    //     {
    //         for (int j = 0; j < _w; j++)
    //             from.push_back(i * _w + j);
    //     }
    //     else
    //     {
    //         for (int j = 0; j < _w; j++)
    //             to.push_back(i * _w + j);
    //     }
    // }
    // ECDAG *ecdag = Decode(from, to);
    // return ecdag;

    ECDAG *ecdag = new ECDAG();
    vector<int> data;
    vector<int> code;
    for (int i = 0; i < _k; i++)
        data.push_back(i);
    for (int i = _k; i < _n; i++)
        code.push_back(i);

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

ECDAG* RSASP::Decode(vector<int> from, vector<int> to)
{
    ECDAG *ecdag = new ECDAG();
    // the function Place shows topo
    assert (_w == _m);

    generate_matrix(_encode_matrix, _n, _k, 8);

    vector<int> fromsids;
    vector<int> tosids;
    for (int i = 0; i < from.size(); i += _w) {
        fromsids.push_back(from[i]/_w);
    }
    for (int i = 0; i < to.size(); i += _w) {
        tosids.push_back(to[i]/_w);
    }

    vector<int> data;
    // 1. get the _k helpers from fromsids.
    assert(fromsids.size() >= _k);

    int remain = _k;
    unordered_map<int, bool> selectedsid;
    vector<vector<int> > rack;
    int racknum = _n / _m + ((_n % _m) ? 1 : 0);
    for (int i = 0; i < racknum; i++) {
        rack.emplace_back(vector<int>{});
    }
    for (int i = 0; i < fromsids.size(); i++) {
        rack[fromsids[i]/_m].push_back(fromsids[i]);
    }
    for (int i = 0; i < racknum; i++) {
        if (rack[i].size() == _m && remain >= _m) {
            remain -= _m;
            for (int j = 0; j < rack[i].size(); j++) {
                data.push_back(rack[i][j]);
                selectedsid[rack[i][j]] = true;
            }
        }
        if (remain < _m) break;
    }
    if (remain) {
        for (int i = 0; i < fromsids.size(); i++) {
            if (selectedsid.find(fromsids[i]) == selectedsid.end()) {
                remain--;
                data.push_back(fromsids[i]);
            }
            if (!remain) break;
        }
    }
    assert(!remain);

    // now we get the _k helpers included in the data.
    // 2. get the coefs.
    vector<vector<int>> coefs;
    int _select_matrix[_k * _k];
    for (int i = 0; i < _k; i++)
    {
        int sidx = data[i];
        memcpy(_select_matrix + i * _k,
               _encode_matrix + sidx * _k,
               sizeof(int) * _k);
    }
    int _invert_matrix[_k * _k];
    jerasure_invert_matrix(_select_matrix, _invert_matrix, _k, _k);
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
    // 3. set the ecdag.
    int tmpname = BINDSTART + (_k + _m)*_w;
    int vracknum = _k / _m + ((_k % _m) ? 1 : 0);
    remain = _k % _m;
    // assert(!remain);
    for (int wi = 0; wi < _w; wi++) {
        vector<vector<int>> tmpridxs;
        vector<int> ridxs;
        for (int i = 0; i < tosids.size(); i++) {
            ridxs.push_back(tosids[i]*_w + wi);
            tmpridxs.emplace_back(vector<int>{});
        }
        // inner-rack
        for (int ri = 0; ri < vracknum; ri++) {
            int startindx = ri * _m;
            // there are _m nodes in the same rack.
            vector<int> rdata;
            vector<vector<int>> rcoefs;
            vector<int> tobindx;
            // inner-rack ecdag data.
            for (int mi = 0; mi < _m; mi++) {
                rdata.push_back(data[startindx+mi]*_w + wi);
            }
            // get ineer-rack ecdag coef then join to the ecdag.
            for (int i = 0; i < tosids.size(); i++) {
                rcoefs.emplace_back(vector<int>{});
                for (int j = 0; j < _m; j++) {
                    rcoefs[i].push_back(coefs[i][startindx+j]);
                }
                ecdag->Join(tmpname, rdata, rcoefs[i]);
                
                tobindx.push_back(tmpname);
                tmpridxs[i].push_back(tmpname++);
            }
            int bindxret = ecdag->BindX(tobindx);
            ecdag->BindY(bindxret, rdata[wi]);
            for (int i = 0; i < tosids.size(); i++) {
                ecdag->BindY(tobindx[i], bindxret);
            }
        }
        // cross-rack pipelining
        for (int i = 0; i < tosids.size(); i++) {
            deque<int> dataqueue;
            deque<int> coefqueue;
            for (int j = 0; j < vracknum; j++) {
                dataqueue.push_back(tmpridxs[i][j]);
                coefqueue.push_back(1);
            }
            while (dataqueue.size() >= 2)
            {
                vector<int> datav;
                vector<int> coefv;

                for (int j = 0; j < 2; j++) {
                    int tmpd(dataqueue.front());
                    dataqueue.pop_front();
                    int tmpc(coefqueue.front());
                    coefqueue.pop_front();
                    datav.push_back(tmpd);
                    coefv.push_back(tmpc);
                }
                int toadd;
                toadd = tmpname++;
                ecdag->Join(toadd, datav, coefv);
                ecdag->BindY(toadd, datav[1]);  
                if (dataqueue.size() != 0) {
                    dataqueue.push_front(toadd);
                    coefqueue.push_front(1);
                }
                else {    
                    ecdag->Join(tosids[i]*_w + wi, vector<int>{toadd}, vector<int>{1});
                }
            }

        }
    }
    return ecdag;
}

void RSASP::Place(vector<vector<int> >& group)
{
    assert (group.empty());
    int racknum = _n / _m + ((_n % _m) ? 1 : 0);
    for (int i = 0; i < racknum; i++) {
        group.emplace_back(vector<int>{});
    }
    for (int i = 0; i < _n; i++) {
        group[i/_m].push_back(i);
    }
}

void RSASP::generate_matrix(int* matrix, int rows, int cols, int w)
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
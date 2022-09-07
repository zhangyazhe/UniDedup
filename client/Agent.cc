#include "src/inc/include.hh"
#include "src/common/Worker.hh"
#include "src/common/Config.hh"

int main(int argc, char** argv)
{
    Config* conf = new Config(config_path);

    int worker_num = conf->worker_num;

    Worker** workers = (Worker**)calloc(worker_num, sizeof(Worker*));

    thread thrds[worker_num];
    for (int i = 0; i < worker_num; i++) {
        workers[i] = new Worker(conf);
        thrds[i] = thread([=]{workers[i]->doProcess();});
    }
    cout << "Agent started ..." << endl;

    // never reach here
    for (int i = 0; i < worker_num; i++) thrds[i].join();
    for (int i = 0; i < worker_num; i++) delete workers[i];

    free(workers);
    delete conf;

    return 0;
}
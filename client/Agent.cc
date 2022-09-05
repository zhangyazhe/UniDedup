#include "inc/include.hh"
#include "common/Worker.hh"
#include "common/Config.hh"

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
    std::cout << "Agent started ..." << std::endl;

    // never reach here
    for (int i = 0; i < worker_num; i++) thrds[i].join();
    for (int i = 0; i < worker_num; i++) delete workers[i];

    free(workers);
    delete conf;

    return 0;
}
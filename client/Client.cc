#include "inc/include.hh"
#include "common/Config.hh"
#include "protocol/AgentCommand.hh"

using namespace std;

void usage() {
    cout << "usage:\t./Client write filepath sizeinBytes" << endl;
}

void write(string filepath, int sizeinBytes) {
    string conf_path(config_path);
    Config * conf = new Config(conf_path);
    // tell local Agent filepath and filesize
    AgentCommand *agCmd = new AgentCommand();
    agCmd->buildType0(0, filepath, sizeinBytes);
    agCmd->sendTo(conf->_localIp);

    delete agCmd;
    delete conf;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        usage();
        return -1;
    }

    string reqType(argv[1]);

    if (reqType == "write") {
        if (argc != 4) {
            usage();
            return -1;
        }
        string input_filepath(argv[2]);
        string input_fileszie(argv[3]);
        int input_size = atoi(input_fileszie.c_str());
        write(input_filepath, input_size);
    }

}
#include "src/inc/include.hh"
#include "src/common/Config.hh"
#include "src/protocol/AgentCommand.hh"

void usage() {
    cout << "usage:\t./Client write filepath saveas sizeinBytes" << endl;
}

void write(string filepath, string saveas, int sizeinBytes) {
    string conf_path(config_path);
    Config * conf = new Config(conf_path);
    // tell local Agent filepath and filesize
    AgentCommand *agCmd = new AgentCommand();
    agCmd->buildType0(0, filepath, saveas, sizeinBytes);
    agCmd->sendTo(conf->_localIP);

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
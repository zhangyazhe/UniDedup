/**
 * Client reads user's command and executes
 */
#include "src/inc/include.hh"
#include "src/common/Config.hh"
#include "src/protocol/AgentCommand.hh"

void usage() {
    cout << "usage:\t./Client write filepath saveas sizeinBytes" << endl;
    cout << "usage:\t./Client read filename saveas" << endl;
}

void write(string filepath, string saveas, int sizeinBytes) {
    string conf_path(config_path);
    Config * conf = new Config(conf_path); // read config
    // tell local Agent filepath and filesize
    // create a command and send it to LOCAL agent
    AgentCommand *agCmd = new AgentCommand();
    agCmd->buildType0(0, filepath, saveas, sizeinBytes);
    agCmd->sendTo(conf->_localIP);

    delete agCmd;
    delete conf;
}

void read(string filename, string saveas) {
    string conf_path(config_path);
    Config* conf = new Config(conf_path);
    // tell local Agent filename
    AgentCommand *agCmd = new AgentCommand();
    agCmd->buildType1(1, filename, saveas);
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
        if (argc != 5) {
            usage();
            return -1;
        }
        string input_filepath(argv[2]);
        string input_saveas(argv[3]);
        string input_fileszie(argv[4]);
        int input_size = atoi(input_fileszie.c_str());
        write(input_filepath, input_saveas, input_size);
    }
    else if (reqType == "read") {
        if (argc != 4) {
            usage();
            return -1;
        }
        string input_filename(argv[2]);
        string intput_saveas(argv[3]);
        read(input_filename, intput_saveas);
    }

}
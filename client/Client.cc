#include "inc/include.hh"
#include "common/Config.hh"
#include "protocol/AgentCommand.hh"

void usage() {
    cout << "usage:\t./Client write filepath sizeinBytes" << endl;
}

void write(string filepath, int sizeinBytes) {
    Config * conf = new Config(config_path);
    // tell local Agent filepath and filesize
    AgentCommand *agCmd = new AgentCommand();
    agCmd->buildType0(0, filepath, sizeinBytes);
    agCmd->sendTo(conf->_localIp);

    delete agCmd;
    delete conf;
}
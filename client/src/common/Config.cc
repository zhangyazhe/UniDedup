#include "Config.hh"

Config::Config(const std::string& configPath)
{
    getIpFromIpList(ip_list_path);
    // read config
    std::ifstream reader(configPath);
    if (reader.is_open()) {
        std::cout << "Config::read Config" << std::endl;
        std::string line;
        while (std::getline(reader, line)) {
            if (line.size() <= 0) continue;
            if (line.at(0) == '#') continue;
            getConfigFromLine(line);
        }
        if (stateful_routing_enabled == 1 && redis_cluster_enabled == 0) {
            cerr << "Err: stateful routing is enabled but redis cluster is not." << endl;
        }
    }
}

Config::~Config()
{
}

void Config::getConfigFromLine(std::string& line) {
    std::stringstream ss(line);
    std::vector<std::string> confs;
    std::string tmp;
    while (ss >> tmp)
    {
        confs.push_back(tmp);
    }
    if (confs[0] == "chunking_algorithm") {
        if (confs[1] == "tttd") chunking_algorithm = CHUNK_TTTD;
        else if (confs[1] == "ae") chunking_algorithm = CHUNK_AE;
        else if (confs[1] == "file") chunking_algorithm = CHUNK_FILE;
        else if (confs[1] == "normalized_rabin") chunking_algorithm = CHUNK_NORMALIZED_RABIN;
        else if (confs[1] == "rabin") chunking_algorithm = CHUNK_RABIN;
        else if (confs[1] == "fixed") chunking_algorithm = CHUNK_FIXED;
        else chunking_algorithm = CHUNK_FIXED;
    }
    else if (confs[0] == "grouping_algorithm") {
        if (confs[1] == "fixed") grouping_algorithm = GROUPING_FIXED;
        else grouping_algorithm = GROUPING_FIXED;
    }
    else if (confs[0] == "grouping_number") {
        std::stringstream gnss(confs[1]);
        gnss >> grouping_number;
    }
    else if (confs[0] == "worker_number") {
        std::stringstream wnss(confs[1]);
        wnss >> worker_num;
    }
    else if(confs[0] == "node_number") {
        std::stringstream nnss(confs[1]);
        nnss >> node_num;
    }
    else if(confs[0] == "destor_chunk_avg_size") {
        std::stringstream nnss(confs[1]);
        nnss >> destor_chunk_avg_size;
    }
    else if(confs[0] == "destor_chunk_min_size") {
        std::stringstream nnss(confs[1]);
        nnss >> destor_chunk_min_size;
    }
    else if(confs[0] == "destor_chunk_max_size") {
        std::stringstream nnss(confs[1]);
        nnss >> destor_chunk_max_size;
    } 
    else if(confs[0] == "local_ip") {
        std::stringstream ipss(confs[1]);
        string ip_str;
        ipss >> ip_str;
        _localIP = inet_addr(ip_str.c_str());
    }
    else if(confs[0] == "redis_cluster_enabled") {
        std::stringstream nnss(confs[1]);
        nnss >> redis_cluster_enabled;
        if (redis_cluster_enabled != 0 && redis_cluster_enabled != 1) {
            cerr << "Err: redis_cluster_enabled get invalid config value, shoule be 0 or 1." << endl;
        }
    }
    else if(confs[0] == "stateful_routing_enabled") {
        std::stringstream nnss(confs[1]);
        nnss >> stateful_routing_enabled;
        if (stateful_routing_enabled != 0 && stateful_routing_enabled != 1) {
            cerr << "Err: stateful_routing_enabled get invalid config value, shoule be 0 or 1." << endl;
        }
    }
}

void Config::getIpFromIpList(const std::string& ipListPath) {
    // read IPs from ip_list_path
    std::ifstream reader(ipListPath);
    uint64_t idth = 0;
    if (reader.is_open()) {
        std::cout << "Config::read IPs" << std::endl;
        std::string line;
        while (std::getline(reader, line))
        {
            unsigned int iip = inet_addr(line.c_str());
            id2Ip.insert(std::make_pair(idth, iip));
            idth++;
        }
        // assert(node_num == idth);
        reader.close();
    }
    
}
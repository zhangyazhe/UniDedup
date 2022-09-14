#include "Receiving.hh"

SyncQueue *receive_queue;

static pthread_t receive_t;

static chunk* new_chunk(int size) {
    if (!size) return nullptr;
    chunk* ret = (chunk*)malloc(sizeof(chunk));
    ret->size = size;
    ret->data = (char*)malloc(size*sizeof(char));
    return ret;
}

static receive_data(const fileRecipe* fr) {
    // serial
    for (int i = 0; i < fr->num; i++) {
        // get chunks from destor[i]
        redisContext* readCtx = RedisUtil::createContext(string("127.0.0.1"));
        redisReply* readReply;
        int pkt_id = 0;
        cout << "Receiving::Receive from node" << fr->gm[i].nodeId 
            << " , fetching " << fr->gm[i].groupName 
            << endl; 
        while(1){
            string pkt_key = string(fr->gm[i].groupName)+":"+to_string(pkt_id++);
            readReply = (redisReply*)redisCommand(readCtx, "blpop %s 0", pkt_key.c_str());
            char* content = readReply->element[1]->str;
            // 1. get data len
            int data_len;
            memcpy((char*)&data_len, content, 4);
            data_len = ntohl(data_len);
            if (data_len == 0) {
                freeReplyObject(readReply);
                break;
            }
            // 2. get data
            chunk* ck = new_chunk(data_len);
            memcpy(ck->data, content+4, ck->size);
            sync_queue_push(receive_queue, ck);
            freeReplyObject(readReply);
        }
        
    }
    // end flag
    sync_queue_push(receive_queue, new_chunk(0));
}

static void *receive_thread(void *argv) {
    const fileRecipe* fr = (const fileRecipe*)argv;
    receive_data(fr);
    sync_queue_term(receive_queue);
    return NULL;
}

void start_receive_phase(const char* buf) {
    receive_queue = sync_queue_new(100);
    pthread_create(&receive_t, NULL, receive_thread, (void*)buf);
}

void stop_receive_phase() {
    pthread_join(receive_t, NULL);
}
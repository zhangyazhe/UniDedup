
#include "destor_server.h"

extern void destor_shutdown();

static pthread_t read1_t;

char* intToChar(unsigned int a) {
	if(a == 0) return "0";
	uint32_t tmpa = a;
	int len = 0;
	while(tmpa > 0) {
		tmpa /= 10;
		len++;
	}
	char* p = (char*)calloc(len, sizeof(char));
	int i = len-1;
	while(a > 0) {
		p[i] = a%10 + '0';
		a/=10;
		i--;
	}
	p[len] = '\0';
	return p;
}

static void read_data(void) {
	sds filename = sdsdup(jcr.path);
    // struct readParam* rp = (struct readParam*)argv;
	uint32_t size = jcr.size;

    struct chunk *c = new_chunk(sdslen(filename) + 1);
	strcpy((char*)c->data, filename);

	VERBOSE("Read phase: %s", filename);

	SET_CHUNK(c, CHUNK_FILE_START);

	sync_queue_push(read_queue, c);

	TIMER_DECLARE(1);
	TIMER_BEGIN(1);

	// get data
	uint32_t readsize = 0;
	int pktid = 0;
	int pktnum = 0;
	redisContext* readCtx = createContextByUint(destor.local_ip);
	printf("jcr.size: %d\n", size);
	pktnum = size / DEFAULT_BLOCK_SIZE;
	if (size % DEFAULT_BLOCK_SIZE != 0) {
		pktnum += 1;
	}
	printf("pkt size: %d\n", pktnum);
	for (int i = 0; i < pktnum; i++) {
		// 1. get key
		int gpname_len = strlen(filename);
		char fix[20] = "_data_";
		strcat(fix, intToChar(pktid++));
		char* data_key = (char *)calloc(gpname_len+1+20, 1);
		memcpy(data_key, filename, gpname_len+1);
		strcat(data_key, fix);
		printf("%s\n", data_key);
		// 2. create redis context
		redisAppendCommand(readCtx, "blpop %s 0", data_key);
		free(data_key);
	}

	for (int i = 0; i < pktnum; i++) {
		// 1. get |len|data|
		redisReply* readrReply;
		redisGetReply(readCtx, (void**)&readrReply);
		char* pkt = readrReply->element[1]->str;
		// 2. get pkt size
		uint32_t pkt_size;
		memcpy((char*)&pkt_size, pkt, 4);
		pkt_size = ntohl(pkt_size);
		// 3. get pkt
		c = new_chunk(pkt_size);
		memcpy(c->data, pkt+4, pkt_size);
		TIMER_END(1, jcr.read_time);
		VERBOSE("Read phase: read %d bytes", pkt_size);
		readsize += pkt_size;
		printf("pktsize: %d\n", pkt_size);
		// 4. push
		sync_queue_push(read_queue, c);
		TIMER_BEGIN(1);
		freeReplyObject(readrReply);
	}
	printf("read size : %d\n", readsize);

	redisFree(readCtx);
			
	// int writtenSize = 0;
	// int size = 0;

    // while(writtenSize < rp->size) {
    //     if(rp->size - writtenSize <= DEFAULT_BLOCK_SIZE) {
	// 		printf("last\n");
	// 		size = rp->size - writtenSize;

    //         TIMER_END(1, jcr.read_time);

    //         VERBOSE("Read phase: read %d bytes", size);

    //         c = new_chunk(size);
    //         memcpy(c->data, rp->data+writtenSize, size);

    //         sync_queue_push(read_queue, c);

    //         TIMER_BEGIN(1);

    //         break;
    //     } else {
	// 		printf("not last\n");
	// 		size = DEFAULT_BLOCK_SIZE;

    //         TIMER_END(1, jcr.read_time);

    //         VERBOSE("Read phase: read %d bytes", size);

    //         c = new_chunk(size);
    //         memcpy(c->data, rp->data+writtenSize, DEFAULT_BLOCK_SIZE);

    //         sync_queue_push(read_queue, c);

    //         TIMER_BEGIN(1);

    //         writtenSize += DEFAULT_BLOCK_SIZE;
    //     }
    // }

    c = new_chunk(0);
	SET_CHUNK(c, CHUNK_FILE_END);
	sync_queue_push(read_queue, c);

	sdsfree(filename);
}

static void* read_thread(void *argv) {
	/* Each file will be processed separately */
	printf("read_thread start\n");
	read_data();
	sync_queue_term(read_queue);
	printf("read thread end\n");
	return NULL;
}

void start_read_phase_from_data() {
    /* running job */
	printf("start_read_phase_from_data\n");
    jcr.status = JCR_STATUS_RUNNING;
	read_queue = sync_queue_new(10);
	pthread_create(&read1_t, NULL, read_thread, NULL);
}

void stop_read_phase_from_data() {
	pthread_join(read1_t, NULL);
	NOTICE("read phase stops successfully!");
}

void destor_write(char *path, uint32_t size) {
	printf("size: %d\n", size);
    /* 初始化recipe目录 */
	init_recipe_store();
	/* 创建一个container_buffer队列 */
	init_container_store();
	init_index();

	// 初始化backup job control record
	init_backup_jcr(path);

	jcr.size = size;

    puts("==== backup begin ====");

	TIMER_DECLARE(1);
	TIMER_BEGIN(1);

    time_t start = time(NULL);
	
	if (destor.simulation_level == SIMULATION_ALL) {
		start_read_trace_phase();
	} else {
		// 将jcr.path目录下的所有文件以块为单位读取出来压入到read_queue中
        // struct readParam* rp = newReadParam(size);
		// uint32_t* psize = (uint32_t*) calloc(sizeof(uint32_t), 1);
		// *psize = size;
		start_read_phase_from_data();
		// free(psize);
        // deleteReadParam(rp);
		// 根据jcr.chunk_algorithm设置分块算法
		start_chunk_phase();
		// 从chunk_queue队列里pop出块，并计算该块的指纹，放到chunk.fp中
		start_hash_phase();
	}
	// 以segment为单位将chunk组织在一起，然后标记出duplicate的chunk
	// 最后将标记好的chunk推入dedup_queue
	start_dedup_phase();
	// 将dedup_queue弹出，并判断是否需要重写，将弹出的块以及需要重写的块push到rewrite_queue
	start_rewrite_phase();
	// 
	start_filter_phase();
	printf("wait for end 1\n");
    do{
        sleep(5);
        /*time_t now = time(NULL);*/
        fprintf(stderr,"job %" PRId32 ", %" PRId64 " bytes, %" PRId32 " chunks, %d files processed\r", 
                jcr.id, jcr.data_size, jcr.chunk_num, jcr.file_num);
    }while(jcr.status == JCR_STATUS_RUNNING || jcr.status != JCR_STATUS_DONE);
    fprintf(stderr,"job %" PRId32 ", %" PRId64 " bytes, %" PRId32 " chunks, %d files processed\n", 
        jcr.id, jcr.data_size, jcr.chunk_num, jcr.file_num);
	printf("wait for end 2\n");
	if (destor.simulation_level == SIMULATION_ALL) {
		stop_read_trace_phase();
	} else {
		stop_read_phase_from_data();
		stop_chunk_phase();
		stop_hash_phase();
	}
	stop_dedup_phase();
	stop_rewrite_phase();
	stop_filter_phase();	
	printf("has ended\n");
	// free(psize);

	TIMER_END(1, jcr.total_time);

	close_index();
	close_container_store();
	close_recipe_store();

	update_backup_version(jcr.bv);

	free_backup_version(jcr.bv);

	puts("==== backup end ====");

	printf("job id: %" PRId32 "\n", jcr.id);
	printf("backup path: %s\n", jcr.path);
	printf("number of files: %d\n", jcr.file_num);
	printf("number of chunks: %" PRId32 " (%" PRId64 " bytes on average)\n", jcr.chunk_num,
			jcr.data_size / jcr.chunk_num);
	printf("number of unique chunks: %" PRId32 "\n", jcr.unique_chunk_num);
	printf("total size(B): %" PRId64 "\n", jcr.data_size);
	printf("stored data size(B): %" PRId64 "\n",
			jcr.unique_data_size + jcr.rewritten_chunk_size);
	printf("deduplication ratio: %.4f, %.4f\n",
			jcr.data_size != 0 ?
					(jcr.data_size - jcr.unique_data_size
							- jcr.rewritten_chunk_size)
							/ (double) (jcr.data_size) :
					0,
			jcr.data_size
					/ (double) (jcr.unique_data_size + jcr.rewritten_chunk_size));
	printf("total time(s): %.3f\n", jcr.total_time / 1000000);
	printf("throughput(MB/s): %.2f\n",
			(double) jcr.data_size * 1000000 / (1024 * 1024 * jcr.total_time));
	printf("number of zero chunks: %" PRId32 "\n", jcr.zero_chunk_num);
	printf("size of zero chunks: %" PRId64 "\n", jcr.zero_chunk_size);
	printf("number of rewritten chunks: %" PRId32 "\n", jcr.rewritten_chunk_num);
	printf("size of rewritten chunks: %" PRId64 "\n", jcr.rewritten_chunk_size);
	printf("rewritten rate in size: %.3f\n",
			jcr.rewritten_chunk_size / (double) jcr.data_size);

	destor.data_size += jcr.data_size;
	destor.stored_data_size += jcr.unique_data_size + jcr.rewritten_chunk_size;

	destor.chunk_num += jcr.chunk_num;
	destor.stored_chunk_num += jcr.unique_chunk_num + jcr.rewritten_chunk_num;
	destor.zero_chunk_num += jcr.zero_chunk_num;
	destor.zero_chunk_size += jcr.zero_chunk_size;
	destor.rewritten_chunk_num += jcr.rewritten_chunk_num;
	destor.rewritten_chunk_size += jcr.rewritten_chunk_size;

	printf("read_time : %.3fs, %.2fMB/s\n", jcr.read_time / 1000000,
			jcr.data_size * 1000000 / jcr.read_time / 1024 / 1024);
	printf("chunk_time : %.3fs, %.2fMB/s\n", jcr.chunk_time / 1000000,
			jcr.data_size * 1000000 / jcr.chunk_time / 1024 / 1024);
	printf("hash_time : %.3fs, %.2fMB/s\n", jcr.hash_time / 1000000,
			jcr.data_size * 1000000 / jcr.hash_time / 1024 / 1024);

	printf("dedup_time : %.3fs, %.2fMB/s\n",
			jcr.dedup_time / 1000000,
			jcr.data_size * 1000000 / jcr.dedup_time / 1024 / 1024);

	printf("rewrite_time : %.3fs, %.2fMB/s\n", jcr.rewrite_time / 1000000,
			jcr.data_size * 1000000 / jcr.rewrite_time / 1024 / 1024);

	printf("filter_time : %.3fs, %.2fMB/s\n",
			jcr.filter_time / 1000000,
			jcr.data_size * 1000000 / jcr.filter_time / 1024 / 1024);

	printf("write_time : %.3fs, %.2fMB/s\n", jcr.write_time / 1000000,
			jcr.data_size * 1000000 / jcr.write_time / 1024 / 1024);

	//double seek_time = 0.005; //5ms
	//double bandwidth = 120 * 1024 * 1024; //120MB/s

	/*	double index_lookup_throughput = jcr.data_size
	 / (index_read_times * seek_time
	 + index_read_entry_counter * 24 / bandwidth) / 1024 / 1024;

	 double write_data_throughput = 1.0 * jcr.data_size * bandwidth
	 / (jcr->unique_chunk_num) / 1024 / 1024;
	 double index_read_throughput = 1.0 * jcr.data_size / 1024 / 1024
	 / (index_read_times * seek_time
	 + index_read_entry_counter * 24 / bandwidth);
	 double index_write_throughput = 1.0 * jcr.data_size / 1024 / 1024
	 / (index_write_times * seek_time
	 + index_write_entry_counter * 24 / bandwidth);*/

	/*	double estimated_throughput = write_data_throughput;
	 if (estimated_throughput > index_read_throughput)
	 estimated_throughput = index_read_throughput;*/
	/*if (estimated_throughput > index_write_throughput)
	 estimated_throughput = index_write_throughput;*/

	char logfile[] = "backup.log";
	FILE *fp = fopen(logfile, "a");
	/*
	 * job id,
	 * the size of backup
	 * accumulative consumed capacity,
	 * deduplication rate,
	 * rewritten rate,
	 * total container number,
	 * sparse container number,
	 * inherited container number,
	 * 4 * index overhead (4 * int)
	 * throughput,
	 */
	fprintf(fp, "%" PRId32 " %" PRId64 " %" PRId64 " %.4f %.4f %" PRId32 " %" PRId32 " %" PRId32 " %" PRId32" %" PRId32 " %" PRId32" %" PRId32" %.2f\n",
			jcr.id,
			jcr.data_size,
			destor.stored_data_size,
			jcr.data_size != 0 ?
					(jcr.data_size - jcr.rewritten_chunk_size - jcr.unique_data_size)/(double) (jcr.data_size)
					: 0,
			jcr.data_size != 0 ? (double) (jcr.rewritten_chunk_size) / (double) (jcr.data_size) : 0,
			jcr.total_container_num,
			jcr.sparse_container_num,
			jcr.inherited_sparse_num,
			// index_overhead.lookup_requests,
			// index_overhead.lookup_requests_for_unique,
			// index_overhead.update_requests,
			// index_overhead.read_prefetching_units,
			(double) jcr.data_size * 1000000 / (1024 * 1024 * jcr.total_time));

	fclose(fp);
}

void destor_server_process()
{
    // todo: where to get local ip
    redisContext* _localCtx = createContextByUint(destor.local_ip);
	// printf("%s\n", ip2Str(destor.local_ip));
    redisReply *rReply;
    while (1) {
        printf("destor_server_process\n");
        // will never stop looping
        rReply = (redisReply *)redisCommand(_localCtx, "blpop destor_request 0");
        if (rReply->type == REDIS_REPLY_NIL)
        {
            printf("destor_server_process: get feed back empty queue\n");
        }
        else if (rReply->type == REDIS_REPLY_ERROR)
        {
            printf("destor_server_process: get feed back ERROR happens\n");
        }	
        else
        {
            printf("destor_server_process: receive a request!\n");
            char *reqStr = rReply->element[1]->str;
            destor_cmd *cmd = (destor_cmd *)calloc(sizeof(destor_cmd), 1);
            destor_cmd_init_with_reqstr(cmd, reqStr);
            int type = cmd->_type;
            printf("type: %d \n", type);
            switch (type)
            {
            case 0:
				printf("[debug] destor has received command type0.\n");
				printf("[debug] groupname: %s\n", cmd->_group_name);
				// printf("[debug] data: %s\n", cmd->_data);
				printf("[debug] size: %d\n", cmd->_size);
				// get data
				
				// 3. destor_write
				printf("cmd->size: %d\n", cmd->_size);
                destor_write(cmd->_group_name, cmd->_size);
                // 4. tell client finished
				redisReply* fReply;
				redisContext* finishedCtx = createContextByUint(destor.local_ip);
				int gpname_len = strlen(cmd->_group_name);
				char* finished_key = (char *)calloc(gpname_len+1+20, 1);
				memcpy(finished_key, cmd->_group_name, gpname_len+1);
				char fikey[] = "_data_finished";
				strcat(finished_key, fikey);
				int tmpval = htonl(1);
				fReply = (redisReply*)redisCommand(finishedCtx, "rpush %s %b", finished_key, (char*)&tmpval, sizeof(tmpval));
				// done
				freeReplyObject(fReply);
				redisFree(finishedCtx);
				free(finished_key);
				
				break;
            default:
                break;
            }
            free(cmd);
        }
        // free reply object
        freeReplyObject(rReply);
		/* persist destor stat into local file */
		// destor_shutdown();
    }
	redisFree(_localCtx);
}
#include "containerstore.h"
#include "../utils/serial.h"
#include "../utils/sync_queue.h"
#include "../jcr.h"

static int64_t container_count = 0;
static FILE* fp;
/* Control the concurrent accesses to fp. */
static pthread_mutex_t mutex;

static pthread_t append_t;

static SyncQueue* container_buffer;

struct metaEntry {
	int32_t off;
	int32_t len;
	fingerprint fp;
};

// zz7 for openec
extern redisContext* createContextByUint(unsigned int ip);
extern void openec_agent_cmd_init(agent_cmd *cmd);
extern void build_openec_agent_command_type0(agent_cmd* cmd, int type, char* filename, char* ecidpool, char* mode, int filesize);
extern void build_openec_agent_command_type1(agent_cmd* cmd, int type, char* filename);

/*
 * We must ensure a container is either in the buffer or written to disks.
 */
static void* append_thread(void *arg) {

	while (1) {
		printf("append thread start\n");
		struct container *c = sync_queue_get_top(container_buffer);
		if (c == NULL){
			printf("[debug] destor in append thread, break.\n");
			break;
		}
		else {
			printf("[debug] destor in append thread, get one chunk from buffer.\n");
		}
			
		TIMER_DECLARE(1);
		TIMER_BEGIN(1);

		write_container(c);

		TIMER_END(1, jcr.write_time);

		sync_queue_pop(container_buffer);

		free_container(c);
		printf("append thread end\n");
	}

	return NULL;
}

void init_container_store() {

	sds containerfile = sdsdup(destor.working_directory);
	containerfile = sdscat(containerfile, "/container.pool");

	if ((fp = fopen(containerfile, "r+"))) {
		printf("[debug] destor init_container_store, in read mode.\n");
		fread(&container_count, 8, 1, fp);
	} else if (!(fp = fopen(containerfile, "w+"))) {
		printf("[debug] destor init_container_store, in write mode.\n");
		perror(
				"Can not create container.pool for read and write because");
		exit(1);
	}

	sdsfree(containerfile);

	container_buffer = sync_queue_new(25);

	pthread_mutex_init(&mutex, NULL);

	pthread_create(&append_t, NULL, append_thread, NULL);

    NOTICE("Init container store successfully");
}

void close_container_store() {
	sync_queue_term(container_buffer);

	pthread_join(append_t, NULL);
	NOTICE("append phase stops successfully!");

	fseek(fp, 0, SEEK_SET);
	fwrite(&container_count, sizeof(container_count), 1, fp);

	fclose(fp);
	fp = NULL;

	pthread_mutex_destroy(&mutex);
}

static void init_container_meta(struct containerMeta *meta) {
	meta->chunk_num = 0;
	meta->data_size = 0;
	meta->id = TEMPORARY_ID;
	meta->map = g_hash_table_new_full(g_int_hash, g_fingerprint_equal, NULL,
			free);
}

/*
 * For backup.
 */
struct container* create_container() {
	struct container *c = (struct container*) malloc(sizeof(struct container));
	if (destor.simulation_level < SIMULATION_APPEND)
		c->data = calloc(1, CONTAINER_SIZE);
	else
		c->data = 0;

	init_container_meta(&c->meta);
	c->meta.id = container_count++;
	return c;
}

inline containerid get_container_id(struct container* c) {
	return c->meta.id;
}

void write_container_async(struct container* c) {
	assert(c->meta.chunk_num == g_hash_table_size(c->meta.map));

	if (container_empty(c)) {
		/* An empty container
		 * It possibly occurs in the end of backup */
		container_count--;
		VERBOSE("Append phase: Deny writing an empty container %lld",
				c->meta.id);
		return;
	}
	printf("push container buffer\n");
	sync_queue_push(container_buffer, c);
}

/*
 * Called by Append phase
 */
// void write_container(struct container* c) {
// 	printf("write_container start\n");
// 	assert(c->meta.chunk_num == g_hash_table_size(c->meta.map));

// 	if (container_empty(c)) {
// 		/* An empty container
// 		 * It possibly occurs in the end of backup */
// 		container_count--;
// 		VERBOSE("Append phase: Deny writing an empty container %lld",
// 				c->meta.id);
// 		return;
// 	}

// 	VERBOSE("Append phase: Writing container %lld of %d chunks", c->meta.id,
// 			c->meta.chunk_num);

// 	if (destor.simulation_level < SIMULATION_APPEND) {

// 		unsigned char * cur = &c->data[CONTAINER_SIZE - CONTAINER_META_SIZE];
// 		ser_declare; // uint8_t *ser_ptr;
// 		ser_begin(cur, CONTAINER_META_SIZE); // ser_ptr = ((uint8_t *)(cur));
// 		ser_int64(c->meta.id); // serial_int64(&ser_ptr, c->meta.id);
// 		ser_int32(c->meta.chunk_num); // serial_int32(&ser_ptr, c->meta.chunk_num);
// 		ser_int32(c->meta.data_size); // serial_int32(&ser_ptr, c->meta.data_size);

// 		GHashTableIter iter;
// 		gpointer key, value;
// 		g_hash_table_iter_init(&iter, c->meta.map);
// 		while (g_hash_table_iter_next(&iter, &key, &value)) {
// 			struct metaEntry *me = (struct metaEntry *) value;
// 			ser_bytes(&me->fp, sizeof(fingerprint));
// 			ser_bytes(&me->len, sizeof(int32_t));
// 			ser_bytes(&me->off, sizeof(int32_t));
// 		}

// 		ser_end(cur, CONTAINER_META_SIZE);

// 		pthread_mutex_lock(&mutex);

// 		if (fseek(fp, c->meta.id * CONTAINER_SIZE + 8, SEEK_SET) != 0) {
// 			perror("Fail seek in container store.");
// 			exit(1);
// 		}
// 		if(fwrite(c->data, CONTAINER_SIZE, 1, fp) != 1){
// 			perror("Fail to write a container in container store.");
// 			exit(1);
// 		}

// 		pthread_mutex_unlock(&mutex);
// 	} else {
// 		char buf[CONTAINER_META_SIZE];
// 		memset(buf, 0, CONTAINER_META_SIZE);

// 		ser_declare;
// 		ser_begin(buf, CONTAINER_META_SIZE);
// 		ser_int64(c->meta.id);
// 		ser_int32(c->meta.chunk_num);
// 		ser_int32(c->meta.data_size);

// 		GHashTableIter iter;
// 		gpointer key, value;
// 		g_hash_table_iter_init(&iter, c->meta.map);
// 		while (g_hash_table_iter_next(&iter, &key, &value)) {
// 			struct metaEntry *me = (struct metaEntry *) value;
// 			ser_bytes(&me->fp, sizeof(fingerprint));
// 			ser_bytes(&me->len, sizeof(int32_t));
// 			ser_bytes(&me->off, sizeof(int32_t));
// 		}

// 		ser_end(buf, CONTAINER_META_SIZE);

// 		pthread_mutex_lock(&mutex);

		// if(fseek(fp, c->meta.id * CONTAINER_META_SIZE + 8, SEEK_SET) != 0){
// 			perror("Fail seek in container store.");
// 			exit(1);
// 		}
// 		if(fwrite(buf, CONTAINER_META_SIZE, 1, fp) != 1){
// 			perror("Fail to write a container in container store.");
// 			exit(1);
// 		}

// 		pthread_mutex_unlock(&mutex);
// 	}
// 	printf("write_container end\n");
// }

// zz7 write container into OpenEC
void write_container(struct container* c) {
	printf("write_container start\n");
	assert(c != NULL);
	assert(c->meta.chunk_num == g_hash_table_size(c->meta.map));

	if (container_empty(c)) {
		/* An empty container
		 * It possibly occurs in the end of backup */
		container_count--;
		VERBOSE("Append phase: Deny writing an empty container %lld",
				c->meta.id);
		return;
	}

	VERBOSE("Append phase: Writing container %lld of %d chunks", c->meta.id,
			c->meta.chunk_num);

	if (destor.simulation_level < SIMULATION_APPEND) {

		unsigned char * cur = &c->data[CONTAINER_SIZE - CONTAINER_META_SIZE];
		// serial
		// ser_declare; // uint8_t *ser_ptr;
		// ser_begin(cur, CONTAINER_META_SIZE); // ser_ptr = ((uint8_t *)(cur));
		// ser_int64(c->meta.id); // serial_int64(&ser_ptr, c->meta.id);
		// ser_int32(c->meta.chunk_num); // serial_int32(&ser_ptr, c->meta.chunk_num);
		// ser_int32(c->meta.data_size); // serial_int32(&ser_ptr, c->meta.data_size);
		unsigned char * tmp_ptr = cur;
		uint64_t tmp_meta_id = redis_util_htonll(c->meta.id);
		memcpy(tmp_ptr, &(tmp_meta_id), sizeof(int64_t));
		tmp_ptr += sizeof(int64_t);
		uint32_t tmp_chunk_num = htonl(c->meta.chunk_num);
		memcpy(tmp_ptr, &(tmp_chunk_num), sizeof(int32_t));
		tmp_ptr += sizeof(int32_t);
		uint32_t tmp_data_size = htonl(c->meta.data_size);
		memcpy(tmp_ptr, &(tmp_data_size), sizeof(int32_t));
		tmp_ptr += sizeof(int32_t);

		GHashTableIter iter;
		gpointer key, value;
		g_hash_table_iter_init(&iter, c->meta.map);
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			struct metaEntry *me = (struct metaEntry *) value;
			// ser_bytes(&me->fp, sizeof(fingerprint));
			// ser_bytes(&me->len, sizeof(int32_t));
			// ser_bytes(&me->off, sizeof(int32_t));
			memcpy(tmp_ptr, &me->fp, sizeof(fingerprint));
			tmp_ptr += sizeof(fingerprint);
			uint32_t tmp_me_len = htonl(me->len);
			memcpy(tmp_ptr, &(tmp_me_len), sizeof(int32_t));
			tmp_ptr += sizeof(int32_t);
			uint32_t tmp_me_off = htonl(me->off);
			memcpy(tmp_ptr, &(tmp_me_off), sizeof(int32_t));
			tmp_ptr += sizeof(int32_t);
		}
		//  write meta data into container.pool file
		fseek(fp, c->meta.id * CONTAINER_META_SIZE + 8, SEEK_SET);
		fwrite(cur, CONTAINER_META_SIZE, 1, fp);
		// ser_end(cur, CONTAINER_META_SIZE);

		pthread_mutex_lock(&mutex);

		// if (fseek(fp, c->meta.id * CONTAINER_SIZE + 8, SEEK_SET) != 0) {
		// 	perror("Fail seek in container store.");
		// 	exit(1);
		// }
		// if(fwrite(c->data, CONTAINER_SIZE, 1, fp) != 1){
		// 	perror("Fail to write a container in container store.");
		// 	exit(1);
		// }

		// we write to OpenEC
		int num = CONTAINER_SIZE / destor.oec_pktsize;
		// init
		
		agent_cmd* agCmd = (agent_cmd*)calloc(sizeof(agent_cmd), 1);
		openec_agent_cmd_init(agCmd);
		
		char *container_file_name = (char*)malloc(MAX_OEC_FILENAME_LEN);
		sprintf(container_file_name, "%s_%s_%d", BASE_OEC_FILENAME, destor.local_ip_str, c->meta.id);
		// printf("[container2openec_name] %s\n", container_file_name);

		build_openec_agent_command_type0(agCmd, 0, container_file_name, destor.ecid_pool, destor.oec_mode, CONTAINER_SIZE);
		openec_agent_cmd_send_to(agCmd, destor.oec_agent_ip);
		// printf("[build_openec_agent_command_type0]\ndestor.ecid_pool is %s\noec_mode is %s\n", destor.ecid_pool, destor.oec_mode);
		
		int pktid = 0;
		redisContext* destor2oecCtx = createContextByUint(destor.oec_agent_ip);
		// printf("num: %d\n", num);
		for (int i = 0; i < num; i++) {
			// printf("[debug] i:%d, 0\n", i);
			// move one oec_packet data and its length to buf.
			unsigned char* buf = (char*)calloc(destor.oec_pktsize+4, sizeof(char));
			
			int tmplen = htonl(destor.oec_pktsize);
			memcpy(buf, (unsigned char*)&tmplen, 4);
			memcpy(buf+4, c->data+i*destor.oec_pktsize, destor.oec_pktsize);
			//outstream write
			char* pkt_name = (char*)malloc(MAX_OEC_FILENAME_LEN);
    		sprintf(pkt_name, "%s:%d", container_file_name, pktid);
			redisAppendCommand(destor2oecCtx, "RPUSH %s %b", pkt_name, buf, destor.oec_pktsize+4);
			pktid++;
			free(pkt_name);
			free(buf);
		}
		redisReply* rReply;
		for (int i = 0; i < num; i++) {
			redisReply* destorrReply;
			redisGetReply(destor2oecCtx, (void**)&destorrReply);
			freeReplyObject(destorrReply);
      	}
		printf("write_container wait for finish\n");
		// wait for finish
		char* wkey = (char*)malloc(MAX_OEC_FILENAME_LEN);
		sprintf(wkey, "writefinish:%s", container_file_name);
		rReply = (redisReply*)redisCommand(destor2oecCtx, "blpop %s 0", wkey);
		freeReplyObject(rReply);
		printf("write_container finish\n");
		free_openec_agent_cmd(agCmd);
		redisFree(destor2oecCtx);
		free(container_file_name);

		pthread_mutex_unlock(&mutex);
	} else {
		/* in our system, should never reach */

		// char buf[CONTAINER_META_SIZE];
		// memset(buf, 0, CONTAINER_META_SIZE);

		// ser_declare;
		// ser_begin(buf, CONTAINER_META_SIZE);
		// ser_int64(c->meta.id);
		// ser_int32(c->meta.chunk_num);
		// ser_int32(c->meta.data_size);

		// GHashTableIter iter;
		// gpointer key, value;
		// g_hash_table_iter_init(&iter, c->meta.map);
		// while (g_hash_table_iter_next(&iter, &key, &value)) {
		// 	struct metaEntry *me = (struct metaEntry *) value;
		// 	ser_bytes(&me->fp, sizeof(fingerprint));
		// 	ser_bytes(&me->len, sizeof(int32_t));
		// 	ser_bytes(&me->off, sizeof(int32_t));
		// }

		// ser_end(buf, CONTAINER_META_SIZE);

		// pthread_mutex_lock(&mutex);

		// if(fseek(fp, c->meta.id * CONTAINER_META_SIZE + 8, SEEK_SET) != 0){
		// 	perror("Fail seek in container store.");
		// 	exit(1);
		// }
		// if(fwrite(buf, CONTAINER_META_SIZE, 1, fp) != 1){
		// 	perror("Fail to write a container in container store.");
		// 	exit(1);
		// }

		// pthread_mutex_unlock(&mutex);
	}
	printf("write_container end\n");
}


// struct container* retrieve_container_by_id(containerid id) {
// 	struct container *c = (struct container*) malloc(sizeof(struct container));

// 	init_container_meta(&c->meta);

// 	unsigned char *cur = 0;
// 	if (destor.simulation_level >= SIMULATION_RESTORE) {
// 		c->data = malloc(CONTAINER_META_SIZE);

// 		pthread_mutex_lock(&mutex);

// 		if (destor.simulation_level >= SIMULATION_APPEND)
// 			fseek(fp, id * CONTAINER_META_SIZE + 8, SEEK_SET);
// 		else
// 			fseek(fp, (id + 1) * CONTAINER_SIZE - CONTAINER_META_SIZE + 8,
// 			SEEK_SET);

// 		fread(c->data, CONTAINER_META_SIZE, 1, fp);

// 		pthread_mutex_unlock(&mutex);

// 		cur = c->data;
// 	} else {
// 		c->data = malloc(CONTAINER_SIZE);

// 		pthread_mutex_lock(&mutex);

// 		fseek(fp, id * CONTAINER_SIZE + 8, SEEK_SET);
// 		fread(c->data, CONTAINER_SIZE, 1, fp);

// 		pthread_mutex_unlock(&mutex);

// 		cur = &c->data[CONTAINER_SIZE - CONTAINER_META_SIZE];
// 	}

// 	unser_declare;
// 	unser_begin(cur, CONTAINER_META_SIZE);

// 	unser_int64(c->meta.id);
// 	unser_int32(c->meta.chunk_num);
// 	unser_int32(c->meta.data_size);

// 	if(c->meta.id != id){
// 		WARNING("expect %lld, but read %lld", id, c->meta.id);
// 		assert(c->meta.id == id);
// 	}

// 	int i;
// 	for (i = 0; i < c->meta.chunk_num; i++) {
// 		struct metaEntry* me = (struct metaEntry*) malloc(
// 				sizeof(struct metaEntry));
// 		unser_bytes(&me->fp, sizeof(fingerprint));
// 		unser_bytes(&me->len, sizeof(int32_t));
// 		unser_bytes(&me->off, sizeof(int32_t));
// 		g_hash_table_insert(c->meta.map, &me->fp, me);
// 	}

// 	unser_end(cur, CONTAINER_META_SIZE);

// 	if (destor.simulation_level >= SIMULATION_RESTORE) {
// 		free(c->data);
// 		c->data = 0;
// 	}

// 	return c;
// }

// zz7 read container from openec
struct container* retrieve_container_by_id(containerid id) {
	struct container *c = (struct container*) malloc(sizeof(struct container));

	init_container_meta(&c->meta);

	unsigned char *cur = 0;
	if (destor.simulation_level >= SIMULATION_RESTORE) {
		// not simulation

		// c->data = malloc(CONTAINER_META_SIZE);

		// pthread_mutex_lock(&mutex);

		// if (destor.simulation_level >= SIMULATION_APPEND)
		// 	fseek(fp, id * CONTAINER_META_SIZE + 8, SEEK_SET);
		// else
		// 	fseek(fp, (id + 1) * CONTAINER_SIZE - CONTAINER_META_SIZE + 8,
		// 	SEEK_SET);

		// fread(c->data, CONTAINER_META_SIZE, 1, fp);

		// pthread_mutex_unlock(&mutex);

		// cur = c->data;
	} else {
		c->data = (unsigned char *)malloc(CONTAINER_SIZE);

		pthread_mutex_lock(&mutex);
		// 1. get container file name by id.
		char *container_file_name = (char*)malloc(MAX_OEC_FILENAME_LEN);
		sprintf(container_file_name, "%s_%s_%d", BASE_OEC_FILENAME, destor.local_ip_str, id);
		// 2. init redisCtx and tell local oec agent what to do
		printf("[debug] destor retrieve_container_by_id, sending request to oec.\n");
		redisContext* localCtx = createContextByUint(destor.local_ip);
		agent_cmd* agCmd = (agent_cmd*)calloc(sizeof(agent_cmd), 1);
		openec_agent_cmd_init(agCmd);

		build_openec_agent_command_type1(agCmd, 1, container_file_name);
		openec_agent_cmd_send_to(agCmd, destor.local_ip);

		free_openec_agent_cmd(agCmd);
		// 3. wait for filesize
		char* wait_key = (char*)malloc(MAX_OEC_FILENAME_LEN+12);
		sprintf(wait_key, "filesize:%s", container_file_name);
		redisReply* rReply = (redisReply*)redisCommand(localCtx, "blpop %s 0", wait_key);
		
		char* response = rReply -> element[1] -> str;
  		int tmpfilesize;
  		memcpy((char*)&tmpfilesize, response, 4); response += 4;
  		
		int container_filesize = ntohl(tmpfilesize);
  		freeReplyObject(rReply);
		// 4. read data
		int pktnum = container_filesize / destor.oec_pktsize;
		redisReply* readReply;
  		redisContext* readCtx = localCtx;

		for (int i = 0; i < pktnum; i++) {
			char* pkt_key = (char*)malloc(MAX_OEC_FILENAME_LEN);
			sprintf(pkt_key, "%s:%d", container_file_name, i);
			redisAppendCommand(readCtx, "blpop %s 0", pkt_key);
			free(pkt_key);
		}

		for (int i = 0; i < pktnum; i++) {
			redisGetReply(readCtx, (void**)&readReply);
			char* content = readReply->element[1]->str;
			int tmplen;
  			memcpy((char*)&tmplen, content, 4);
  			int len = ntohl(tmplen);
			memcpy(c->data+destor.oec_pktsize*i, content+4, len);
			freeReplyObject(readReply);
		}
		redisFree(localCtx);
		// fseek(fp, id * CONTAINER_SIZE + 8, SEEK_SET);
		// fread(c->data, CONTAINER_SIZE, 1, fp);
		pthread_mutex_unlock(&mutex);
		printf("[debug] destor retrieve_container_by_id, retrieved from oec.\n");
		cur = &c->data[CONTAINER_SIZE - CONTAINER_META_SIZE];
	}

	// unser_declare;
	// unser_begin(cur, CONTAINER_META_SIZE);
	printf("[debug] destor retrieve_container_by_id, to serial.\n");
	unsigned char* tmp_ptr = cur;
	uint64_t tmp_meta_id;
	memcpy((unsigned char*)&tmp_meta_id, tmp_ptr, sizeof(int64_t)); tmp_ptr += sizeof(int64_t);
	c->meta.id = redis_util_ntohll(tmp_meta_id);
	uint32_t tmp_chunk_num;
	memcpy((unsigned char*)&tmp_chunk_num, tmp_ptr, sizeof(int32_t)); tmp_ptr += sizeof(int32_t);
	c->meta.chunk_num = ntohl(tmp_chunk_num);
	uint32_t tmp_data_size;
	memcpy((unsigned char*)&tmp_data_size, tmp_ptr, sizeof(int32_t)); tmp_ptr += sizeof(int32_t);
	c->meta.data_size = ntohl(tmp_data_size);
	// unser_int64(c->meta.id);
	// unser_int32(c->meta.chunk_num);
	// unser_int32(c->meta.data_size);

	if(c->meta.id != id){
		printf("[debug] destor retrieve container expect %lld, but id is %lld\n", id, c->meta.id);
		WARNING("expect %lld, but read %lld", id, c->meta.id);
		assert(c->meta.id == id);
	}
	else {
		printf("[debug] destor retrieve container, id is %lld\n", c->meta.id);
	}

	int i;
	for (i = 0; i < c->meta.chunk_num; i++) {
		struct metaEntry* me = (struct metaEntry*) malloc(
				sizeof(struct metaEntry));
		
		memcpy(&me->fp, tmp_ptr, sizeof(fingerprint));
		tmp_ptr += sizeof(fingerprint);
		uint32_t tmp_me_len;
		memcpy((unsigned char*)&tmp_me_len, tmp_ptr, sizeof(int32_t));
		tmp_ptr += sizeof(int32_t);
		me->len = ntohl(tmp_me_len);
		uint32_t tmp_me_off;
		memcpy((unsigned char*)&tmp_me_off, tmp_ptr, sizeof(int32_t));
		tmp_ptr += sizeof(int32_t);
		me->off = ntohl(tmp_me_off);
		// unser_bytes(&me->fp, sizeof(fingerprint));
		// unser_bytes(&me->len, sizeof(int32_t));
		// unser_bytes(&me->off, sizeof(int32_t));
		g_hash_table_insert(c->meta.map, &me->fp, me);
	}

	// unser_end(cur, CONTAINER_META_SIZE);

	if (destor.simulation_level >= SIMULATION_RESTORE) {
		free(c->data);
		c->data = 0;
	}

	return c;
}

static struct containerMeta* container_meta_duplicate(struct container *c) {
	struct containerMeta* base = &c->meta;
	struct containerMeta* dup = (struct containerMeta*) malloc(
			sizeof(struct containerMeta));
	init_container_meta(dup);
	dup->id = base->id;
	dup->chunk_num = base->chunk_num;
	dup->data_size = base->data_size;

	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, base->map);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		struct metaEntry* me = (struct metaEntry*) malloc(
				sizeof(struct metaEntry));
		memcpy(me, value, sizeof(struct metaEntry));
		g_hash_table_insert(dup->map, &me->fp, me);
	}

	return dup;
}

struct containerMeta* retrieve_container_meta_by_id(containerid id) {
	struct containerMeta* cm = NULL;

	/* First, we find it in the buffer */
	cm = sync_queue_find(container_buffer, container_check_id, &id,
			container_meta_duplicate);

	if (cm)
		return cm;

	printf("[debug] container meta not found\n");
	cm = (struct containerMeta*) malloc(sizeof(struct containerMeta));
	init_container_meta(cm);

	unsigned char buf[CONTAINER_META_SIZE];

	pthread_mutex_lock(&mutex);

	// if (destor.simulation_level >= SIMULATION_APPEND)
	// 	fseek(fp, id * CONTAINER_META_SIZE + 8, SEEK_SET);
	// else
	// 	fseek(fp, (id + 1) * CONTAINER_SIZE - CONTAINER_META_SIZE + 8,
	// 	SEEK_SET);

	// only meta data in container.pool
	fseek(fp, id * CONTAINER_META_SIZE + 8, SEEK_SET);

	fread(buf, CONTAINER_META_SIZE, 1, fp);

	pthread_mutex_unlock(&mutex);

	// unser_declare;
	// unser_begin(buf, CONTAINER_META_SIZE);

	// unser_int64(cm->id);
	// unser_int32(cm->chunk_num);
	// unser_int32(cm->data_size);
	unsigned char* tmp_ptr = buf;
	uint64_t tmp_meta_id;
	memcpy((unsigned char*)&tmp_meta_id, tmp_ptr, sizeof(int64_t)); tmp_ptr += sizeof(int64_t);
	cm->id = redis_util_ntohll(tmp_meta_id);
	uint32_t tmp_chunk_num;
	memcpy((unsigned char*)&tmp_chunk_num, tmp_ptr, sizeof(int32_t)); tmp_ptr += sizeof(int32_t);
	cm->chunk_num = ntohl(tmp_chunk_num);
	uint32_t tmp_data_size;
	memcpy((unsigned char*)&tmp_data_size, tmp_ptr, sizeof(int32_t)); tmp_ptr += sizeof(int32_t);
	cm->data_size = ntohl(tmp_data_size);

	if(cm->id != id){
		WARNING("expect %lld, but read %lld", id, cm->id);
		assert(cm->id == id);
	}

	int i;
	for (i = 0; i < cm->chunk_num; i++) {
		struct metaEntry* me = (struct metaEntry*) malloc(
				sizeof(struct metaEntry));
		// unser_bytes(&me->fp, sizeof(fingerprint));
		// unser_bytes(&me->len, sizeof(int32_t));
		// unser_bytes(&me->off, sizeof(int32_t));
		memcpy(&me->fp, tmp_ptr, sizeof(fingerprint));
		tmp_ptr += sizeof(fingerprint);
		uint32_t tmp_me_len;
		memcpy((unsigned char*)&tmp_me_len, tmp_ptr, sizeof(int32_t));
		tmp_ptr += sizeof(int32_t);
		me->len = ntohl(tmp_me_len);
		uint32_t tmp_me_off;
		memcpy((unsigned char*)&tmp_me_off, tmp_ptr, sizeof(int32_t));
		tmp_ptr += sizeof(int32_t);
		me->off = ntohl(tmp_me_off);
		g_hash_table_insert(cm->map, &me->fp, me);
	}

	return cm;
}

static struct metaEntry* get_metaentry_in_container_meta(
		struct containerMeta* cm, fingerprint *fp) {
	return g_hash_table_lookup(cm->map, fp);
}

struct chunk* get_chunk_in_container(struct container* c, fingerprint *fp) {
	struct metaEntry* me = get_metaentry_in_container_meta(&c->meta, fp);

	assert(me);

	struct chunk* ck = new_chunk(me->len);

	if (destor.simulation_level < SIMULATION_RESTORE)
		memcpy(ck->data, c->data + me->off, me->len);

	ck->size = me->len;
	ck->id = c->meta.id;
	memcpy(&ck->fp, &fp, sizeof(fingerprint));

	return ck;
}

int container_overflow(struct container* c, int32_t size) {
	if (c->meta.data_size + size > CONTAINER_SIZE - CONTAINER_META_SIZE)
		return 1;
	/*
	 * 28 is the size of metaEntry.
	 */
	if ((c->meta.chunk_num + 1) * 28 + 16 > CONTAINER_META_SIZE)
		return 1;
	return 0;
}

/*
 * For backup.
 * return 1 indicates success.
 * return 0 indicates fail.
 */
int add_chunk_to_container(struct container* c, struct chunk* ck) {
	assert(!container_overflow(c, ck->size));
	if (g_hash_table_contains(c->meta.map, &ck->fp)) {
		NOTICE("Writing a chunk already in the container buffer!");
		ck->id = c->meta.id;
		return 0;
	}

	struct metaEntry* me = (struct metaEntry*) malloc(sizeof(struct metaEntry));
	memcpy(&me->fp, &ck->fp, sizeof(fingerprint));
	me->len = ck->size;
	me->off = c->meta.data_size;

	g_hash_table_insert(c->meta.map, &me->fp, me);
	c->meta.chunk_num++;

	if (destor.simulation_level < SIMULATION_APPEND)
		memcpy(c->data + c->meta.data_size, ck->data, ck->size);

	c->meta.data_size += ck->size;

	ck->id = c->meta.id;

	return 1;
}

void free_container_meta(struct containerMeta* cm) {
	g_hash_table_destroy(cm->map);
	free(cm);
}

void free_container(struct container* c) {
	g_hash_table_destroy(c->meta.map);
	if (c->data)
		free(c->data);
	free(c);
}

int container_empty(struct container* c) {
	return c->meta.chunk_num == 0 ? 1 : 0;
}

/*
 * Return 0 if doesn't exist.
 */
int lookup_fingerprint_in_container_meta(struct containerMeta* cm,
		fingerprint *fp) {
	return g_hash_table_lookup(cm->map, fp) == NULL ? 0 : 1;
}

int lookup_fingerprint_in_container(struct container* c, fingerprint *fp) {
	return lookup_fingerprint_in_container_meta(&c->meta, fp);
}

gint g_container_cmp_desc(struct container* c1, struct container* c2,
		gpointer user_data) {
	return g_container_meta_cmp_desc(&c1->meta, &c2->meta, user_data);
}

gint g_container_meta_cmp_desc(struct containerMeta* cm1,
		struct containerMeta* cm2, gpointer user_data) {
	return cm2->id - cm1->id;
}

int container_check_id(struct container* c, containerid* id) {
	return container_meta_check_id(&c->meta, id);
}

int container_meta_check_id(struct containerMeta* cm, containerid* id) {
	return cm->id == *id ? 1 : 0;
}

/*
 * foreach the fingerprints in the container.
 * Apply the 'func' for each fingerprint.
 */
void container_meta_foreach(struct containerMeta* cm, void (*func)(fingerprint*, void*), void* data){
	GHashTableIter iter;
	gpointer key, value;
	g_hash_table_iter_init(&iter, cm->map);
	while(g_hash_table_iter_next(&iter, &key, &value)){
		func(key, data);
	}
}

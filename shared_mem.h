#define MAX_MSG_LEN 1024
#define NUMBER_OF_MSG 5

struct message_buffer {
	pthread_mutex_t mutex;
	pthread_cond_t full, empty;
	int front, tail;
	char messages[NUMBER_OF_MSG][MAX_MSG_LEN];
};

struct shm_data {
	struct message_buffer s2c;
	struct message_buffer c2s;
};


#ifndef __COCOFLOW_REDIS_H__
#define __COCOFLOW_REDIS_H__

#include "cocoflow.h"

/* Simplified hiredis.h */
extern "C" {
#ifndef __disable_simplify_hiredis_h__
#define REDIS_UNFINISHED -2
#define REDIS_ERR -1
#define REDIS_OK 0

#define REDIS_ERR_IO 1       /* Error in read or write */
#define REDIS_ERR_EOF 3      /* End of file */
#define REDIS_ERR_PROTOCOL 4 /* Protocol error */
#define REDIS_ERR_OOM 5      /* Out of memory */
#define REDIS_ERR_OTHER 2    /* Everything else... */

#define REDIS_REPLY_STRING 1
#define REDIS_REPLY_ARRAY 2
#define REDIS_REPLY_INTEGER 3
#define REDIS_REPLY_NIL 4
#define REDIS_REPLY_STATUS 5
#define REDIS_REPLY_ERROR 6

	typedef struct redisReply {
		int type; /* REDIS_REPLY_* */
		long long integer; /* The integer when type is REDIS_REPLY_INTEGER */
		int len; /* Length of string */
		char *str; /* Used for both REDIS_REPLY_ERROR and REDIS_REPLY_STRING */
		size_t elements; /* number of elements, for REDIS_REPLY_ARRAY */
		struct redisReply **element; /* elements vector for REDIS_REPLY_ARRAY */
	} redisReply;
#endif
}

namespace ccf {

class redis
{
public:
	class connect : public event_task
	{
	public:
		connect(int* ret, redis& handle, const char* ip, int port);
		virtual ~connect();
	private:
		connect(const connect&);
		connect& operator=(const connect&);
		virtual void run();
		virtual void cancel();
		int* ret;
		redis& handle;
		char* ip;
		int port;
		friend class redis;
	};
	class command : public event_task
	{
	public:
		command(int* ret, const redisReply** reply, redis& handle, const char* format, ...);
		command(int* ret, const redisReply** reply, redis& handle, int argc, const char** argv, const size_t* argvlen = NULL);
		virtual ~command();
	private:
		command(const command&);
		command& operator=(const command&);
		virtual void run();
		virtual void cancel();
		int* ret;
		const redisReply** reply;
		redis& handle;
		command** req;
		int redis_err;
		friend class redis;
	};
	redis();
	~redis();
	const char* errstr();
	int auto_connect(const char* ip, int port, int timeout = 2000); //ms
	/* set_auto_connect_callback is only for log/monitor */
	enum failed_type {
		failed_exception = 1,  //an error occurred on connecting
		failed_disconnect,     //connection is closed by peer
		failed_timeout         //connecting timeout
	};
	typedef void auto_connect_succeed (redis& handle, void* data);
	typedef void auto_connect_failed  (redis& handle, void* data, failed_type type, const char *message);
	void set_auto_connect_callback(auto_connect_succeed* succeed, auto_connect_failed* failed, void* data = NULL);
private:
	void* context;
	void* timer; //for connect
	int cur_reconnect_interval;
	int timeout;
	auto_connect_succeed* succeed;
	auto_connect_failed* failed;
	void* data;
	std::string ip;
	int port;
	bool old_opened; //connect new need close old
	int connect_now(bool);
	int connect_coming(bool);
	static void connect_cb(const struct redisAsyncContext*, int);
	static void command_cb(struct redisAsyncContext*, void*, void*);
	static void auto_connect_cb(const struct redisAsyncContext*, int);
	static void auto_connect_closed_cb(const struct redisAsyncContext*, int);
	static void auto_connect_timeout_cb(uv_timer_t*, int);
	static void auto_reconnect_next_cb(uv_timer_t*, int);
};

} /* end of namespace ccf */

#endif

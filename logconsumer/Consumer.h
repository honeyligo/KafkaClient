#ifndef _CONSUMER_
#define _CONSUMER_

#include <vector>
#include <string>
//#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <thread>
//#include "ustd/thread.h"
#include "singleton.h"
#include "KafkaConsumer.h"

namespace ustd
{

class Consumer
{
USTD_DECLARE_SINGLETON(Consumer)
public:
	bool start();
	void wait();
	void save(bool immediate);

private:
	void read_config(const char* path = "/etc/maxmob/logconsumer/consumer.conf");
	void process_msg(const std::string& topic, const std::vector<char>& data);
	void worker(ustd::KafkaConsumer c, const std::string& topic);
	void write_offset(const std::string& topic);
	int64_t read_offset(const std::string& topic);
	std::string get_full_name(const std::string& path, const std::string& file_name);
private:
	std::string host_;
	int port_;
	int partition_;
	int interval_;
	int count_;
	int64_t offset_;
	int timeout_;
	std::string offset_path_;
	std::string log_path_;
	ustd::KafkaConsumer consumer_;
	std::unordered_map<std::string, int64_t> topics_;
	std::vector<std::thread> threads_;
	time_t last_time_;
};

}

#endif //_CONSUMER_
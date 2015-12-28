#ifndef _KAFKA_PRODUCER_
#define _KAFKA_PRODUCER_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "rdkafkacpp.h"

namespace ustd
{

class KafkaProducer
{
public:
	KafkaProducer();
	~KafkaProducer();
	
public:
	bool init(const std::string &host, const int &port, const bool &async = true, const int &size = 0x7fffffff);
	int send(const char *data,
		const int &size,
		const std::string &topic,
		const int &partition = 0,
		const int &timeout = 0);
	int send_batch(const std::vector<std::pair<const char *, const int &>> &data,
		const std::string &topic,
		const int &partition = 0,
		const int &timeout = 0);
	bool create_topic(const std::string &topic);
	void close();

private:
	void read_config(const char *path);
	RdKafka::Topic* get_topic(const std::string &topic);

private:
	std::shared_ptr<RdKafka::Producer> producer_;
	std::shared_ptr<RdKafka::Conf> conf_;
	std::shared_ptr<RdKafka::Conf> tconf_;
	std::unordered_map<std::string, std::shared_ptr<RdKafka::Topic>> topics_;
};

}

#endif //_KAFKA_PRODUCER_
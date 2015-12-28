#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>

#include "KafkaProducer.h"

#define CONFIG_PATH		""

namespace ustd
{

class HashPartitionerCb : public RdKafka::PartitionerCb
{
public:
	int32_t partitioner_cb(const RdKafka::Topic *topic, const std::string *key,
		int32_t partition_cnt, void *msg_opaque) 
	{
		return djb_hash(key->c_str(), key->size()) % partition_cnt;
	}

private:
	static inline unsigned int djb_hash (const char *str, size_t len)
	{
		unsigned int hash = 5381;
		for (size_t i = 0 ; i < len ; i++)
			hash = ((hash << 5) + hash) + str[i];
		return hash;
	}
};

KafkaProducer::KafkaProducer()
{

}

KafkaProducer::~KafkaProducer()
{

}

bool KafkaProducer::init(const std::string &host, const int &port, const bool &async, const int &size)
{
	std::string errstr;
	this->topics_.clear();

	conf_ = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
	tconf_ = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

	if (conf_.get() == nullptr || tconf_.get() == nullptr)
	{
		return false;
	}

	std::string broker(host);
	broker.append(":").append(std::to_string((long long)port));
	conf_->set("metadata.broker.list", broker, errstr);

	if (async)
	{
		conf_->set("producer.type", "async", errstr);
		conf_->set("queue.buffering.max.messages", std::to_string((long long)size).c_str(), errstr);
	}
	else
	{
		conf_->set("producer.type", "sync", errstr);
	}

	producer_ = std::shared_ptr<RdKafka::Producer>(RdKafka::Producer::create(conf_.get(), errstr));
	if (!producer_) 
	{
		std::cerr << "Failed to create producer: " << errstr << std::endl;
		return false;
	}

	return true;
}

int KafkaProducer::send(const char *data, const int &size, const std::string &topic, const int &partition, const int &timeout)
{
	RdKafka::Topic *tpk = get_topic(topic);
	if (tpk == nullptr)
	{
		return -1;
	}

	RdKafka::ErrorCode resp =
		producer_->produce(tpk, partition,
		RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
		const_cast<char*>(data), size,
		NULL, NULL);

	if (resp != RdKafka::ERR_NO_ERROR)
	{
		std::cerr << "% Produce failed: " <<
		RdKafka::err2str(resp) << std::endl;
		return -1;
	}

	producer_->poll(timeout);

	return size;
}

int KafkaProducer::send_batch(const std::vector<std::pair<const char *, const int &>> &data, 
	const std::string &topic, const int &partition, const int &timeout)
{
	return 0;
}

bool KafkaProducer::create_topic(const std::string &topic)
{
	std::string errstr;
	if (topic.empty() || producer_.get() == nullptr || tconf_.get() == nullptr)
	{
		return false;
	}

	if (this->get_topic(topic) != nullptr)
	{
		return false;
	}

	std::shared_ptr<RdKafka::Topic> tpk = std::shared_ptr<RdKafka::Topic>(RdKafka::Topic::create(producer_.get(), topic, tconf_.get(), errstr));
	if (!tpk) 
	{
		std::cerr << "Failed to create topic: " << errstr << std::endl;
		return false;
	}

	topics_.insert(make_pair(topic, tpk));

	return true;
}

RdKafka::Topic *KafkaProducer::get_topic(const std::string &topic)
{
	auto it = topics_.find(topic);
	if(it != topics_.end())
	{
		return it->second.get();
	}

	return nullptr;
}

void KafkaProducer::read_config(const char *path)
{
	
}

void KafkaProducer::close()
{
	while (producer_->outq_len() > 0)
	{
		std::cerr << "Waiting for " << producer_->outq_len() << std::endl;
		producer_->poll(1000);
	}

	RdKafka::wait_destroyed(1000);
}

}
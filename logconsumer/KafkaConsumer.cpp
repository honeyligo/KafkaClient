#include <vector>
#include <string.h>
#include <string>
#include <iostream>
#include <unordered_map>
#include "KafkaConsumer.h"
#include "rdkafkacpp.h"

namespace ustd
{
int msg_consume(std::vector<char> &data, RdKafka::Message *message, void *opaque)
{
	int len = -1;
	switch (message->err())
	{
	case RdKafka::ERR_NO_ERROR:
		if (message->payload())
		{
			len = static_cast<int>(message->len());				
			data.resize(len);
			memcpy(&data[0], message->payload(), len);
		}
		break;

	case RdKafka::ERR__TIMED_OUT:
	/* Last message */
	case RdKafka::ERR__PARTITION_EOF:
		len = 0;
		break;

	case RdKafka::ERR__UNKNOWN_TOPIC:
	case RdKafka::ERR__UNKNOWN_PARTITION:
	default:
		len = -1;
		/* Errors */
		std::cerr << "Consume failed: " << message->errstr() << std::endl;
	}

	return len;
}

KafkaConsumer::KafkaConsumer()
{

}

KafkaConsumer::~KafkaConsumer()
{

}

bool KafkaConsumer::init(const std::string &host, const int &port)
{
	std::string errstr;
	this->topics_.clear();

	conf_ = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
	tconf_ = std::shared_ptr<RdKafka::Conf>(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

	if(conf_.get() == nullptr || tconf_.get() == nullptr)
	{
		return false;
	}

	std::string broker(host);
	broker.append(":").append(std::to_string((long long)port));
	conf_->set("metadata.broker.list", broker, errstr);

	consumer_ = std::shared_ptr<RdKafka::Consumer>(RdKafka::Consumer::create(conf_.get(), errstr));
	if (consumer_.get() == nullptr)
	{
		std::cerr << "Failed to create consumer: " << errstr << std::endl;
		return false;
	}

	return true;
}

bool KafkaConsumer::create_topic(const std::string &topic)
{
	std::string errstr;
	if (topic.empty() || consumer_.get() == nullptr || tconf_.get() == nullptr)
	{
		return false;
	}

	if (this->get_topic(topic) != nullptr)
	{
		return false;
	}

	std::shared_ptr<RdKafka::Topic> tpk = std::shared_ptr<RdKafka::Topic> (RdKafka::Topic::create(consumer_.get(), topic, tconf_.get(), errstr));
	if (!tpk)
	{
		std::cerr << "Failed to create topic: " << errstr << std::endl;
		return false;
	}

	std::shared_ptr<topic_info_t> topic_info = std::shared_ptr<topic_info_t> (new topic_info_t());
	topic_info->topic_ = tpk;

	topics_.insert(std::make_pair(topic, topic_info));

	return true;
}

void KafkaConsumer::read_config(const char *path)
{

}

bool KafkaConsumer::start_topic(const std::string &topic, const int &partition, const int64_t &offset)
{
	topic_info_t *topic_info = get_topic(topic);
	if (topic_info == nullptr)
	{
		return false;
	}

	RdKafka::ErrorCode resp = consumer_->start(topic_info->topic_.get(), partition, offset);
	if (resp != RdKafka::ERR_NO_ERROR) 
	{
		std::cerr << "Failed to start consumer: " << RdKafka::err2str(resp) << std::endl;
		return false;
	}

	topic_info->offset_ = offset;
	
	return true;
}

int KafkaConsumer::recv(std::vector<char> &data, const std::string &topic, const int &flag, const int &partition, const int &timeout)
{
	data.clear();
	topic_info_t *topic_info = get_topic(topic);
	
	std::shared_ptr<RdKafka::Message> msg = std::shared_ptr<RdKafka::Message>(consumer_->consume(topic_info->topic_.get(), partition, timeout));
	int len = msg_consume(data, msg.get(), NULL);

	if (len > 0)
	{
		topic_info->offset_ = msg->offset();
	}

	consumer_->poll(0);
	
	return len;
}

bool KafkaConsumer::get_last_offset(const std::string &topic, int64_t &offset)
{
	topic_info_t *topic_info = this->get_topic(topic);
	if (topic_info == nullptr)
	{
		return false;
	}

	offset = topic_info->offset_;
	return true;
}

topic_info_t* KafkaConsumer::get_topic(const std::string &topic)
{
	auto it = topics_.find(topic);
	if(it != topics_.end())
	{
		return it->second.get();
	}

	return nullptr;
}

void KafkaConsumer::close()
{
	for (auto it = this->topics_.begin(); it != this->topics_.end(); ++it)
	{
		RdKafka::Topic *tpk = it->second->topic_.get();
		consumer_->stop(tpk, 0);
	}

	consumer_->poll(1000);
}

int KafkaConsumer::recv_batch(std::vector<std::vector<char>> &data, const int &size, const std::string &topic, 
	const int &flag, const int &partition, const int &timeout)
{
	return 0;
}

	
}
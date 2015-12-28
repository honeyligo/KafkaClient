#include <fstream>
#include <string.h>
#include <iostream>
#include <time.h>

#include "ustd/ustd_timer.h"
#include "ustd/ustd_system.h"
#include "MMRTBDataModel.pb.h"
#include "Consumer.h"

#define		DEFAULT_COUNT			1000
#define		DEFAULT_PORT			9092
#define		DEFAULT_INTERVAL		300
#define		OFFSET_LEN				32
#define		SIZE_LEN				4
//#define		PROTO_SIZE				2048*10
#define		BID_FORMAT				"bid.0.%Y-%m-%d-%H-%M"
#define		WN_FORMAT				"wn.0.%Y-%m-%d-%H-%M"
#define		TC_FORMAT				"tc.0.%Y-%m-%d-%H-%M"
#define		TM_FORMAT				"tm.0.%Y-%m-%d-%H-%M"
#define		DEFAULT_LOG_PATH		"/data/maxmob/logconsumer/logs"

namespace ustd
{
USTD_DEFINE_SINGLETON(Consumer)

bool Consumer::start()
{
	time(&last_time_);

	partition_ = 0;

	read_config();

	if(!consumer_.init(host_, port_))
	{
		std::cout << "init failed" << std::endl;
		return false;
	}

	for(auto it : topics_)
	{
		consumer_.create_topic(it.first);
		consumer_.start_topic(it.first, partition_, read_offset(it.first));

		threads_.push_back(std::thread(std::bind(&Consumer::worker, this, consumer_, it.first)));
	}

	
	return true;
}

void Consumer::wait()
{
	for (auto& it : threads_)
	{
		it.join();
	}
}

void Consumer::process_msg(const std::string& topic, const std::vector<char>& data)
{
	std::string file_name;
	//char proto[PROTO_SIZE] = {0};
	int size = data.size() - SIZE_LEN;

	if (data.size() > SIZE_LEN)
	{
		//memcpy(proto, (void*)&data[SIZE_LEN], data.size() - SIZE_LEN);

		if (topic == "bid")
		{
			maxmob::rtb::RTBDetail_t bid;
			bid.ParseFromArray((void*)&data[SIZE_LEN], size);
			file_name = ustd::get_local_time(BID_FORMAT, bid.time());
		}
		else if(topic == "wn")
		{
			maxmob::rtb::Wn_t wn;
			wn.ParseFromArray((void*)&data[SIZE_LEN], size);
			file_name = ustd::get_local_time(WN_FORMAT, wn.time());
		}
		else if(topic == "tc")
		{
			maxmob::rtb::Clk_t tc;
			tc.ParseFromArray((void*)&data[SIZE_LEN], size);
			file_name = ustd::get_local_time(TC_FORMAT, tc.time());
		}
		else if(topic == "tm")
		{
			maxmob::rtb::Pv_t tm;
			tm.ParseFromArray((void*)&data[SIZE_LEN], size);
			file_name = ustd::get_local_time(TM_FORMAT, tm.time());
		}
		else
		{
			return;
		}

		std::fstream out(get_full_name(log_path_, file_name), std::fstream::out | std::fstream::binary | std::fstream::app);
		out.write(&data[0], data.size());

		out.close();
	}
}

void Consumer::worker(ustd::KafkaConsumer c, const std::string& topic)
{
	std::vector<char> data;

	while(true)
	{
		if (c.recv(data, topic, 0, 0, 1000) > 0)
		{
			process_msg(topic, data);
			auto it = topics_.find(topic);
			if(it != topics_.end())
			{
				it->second++;

				save(false);
			}
			else
			{
				topics_.insert(make_pair(topic, 0));
			}

			data.clear();
		}
	}
}

void Consumer::read_config(const char* path /*= "/etc/maxmob/logconsumer/consumer.conf"*/)
{
	FILE *fp;
	char buf[512] = {0};
	int line = 0;

	if (!(fp = fopen(path, "r"))) {
		std::cout<<"fail to open confsumer.conf"<<std::endl;
	}

	while (fgets(buf, sizeof(buf)-1, fp)) {
		char *t;
		char *b = buf;
		char *key, *val;

		line++;
		if ((t = strchr(b, '\n')))
			*t = '\0';

		if (*b == '#' || !*b)
			continue;

		if (!(t = strchr(b, '=')))
		{
			std::cout<<path<<":"<<line<<":"<<"expected name=value format"<<std::endl;
		}

		key = b;
		*t = '\0';
		val = t+1;

		if (!strcmp(key, "log.path"))
		{
			log_path_.assign(val);
		}
		else if(!strcmp(key, "log.type.list"))
		{
			std::string list(val);

			std::vector<std::string> v;
			ustd::string::split(list, ",", v);

			for(auto it : v)
			{
				topics_.insert(make_pair(it, 0));
			}
		}
		else if(!strcmp(key, "host"))
		{
			host_.assign(val);
		}
		else if(!strcmp(key, "log.offset.path"))
		{
			offset_path_.assign(val);
		}
		else if(!strcmp(key, "port"))
		{
			port_ = strlen(val) == 0 ? DEFAULT_PORT : std::stoi(val, NULL, 10);
		}
		else if(!strcmp(key, "log.interval"))
		{
			interval_ = strlen(val) == 0 ? DEFAULT_INTERVAL : std::stoi(val, NULL, 10);
		}
		else if(!strcmp(key, "log.count"))
		{
			count_ = strlen(val) == 0 ? DEFAULT_COUNT : std::stoi(val, NULL, 10);
		}
		
	}

	fclose(fp);
}

void Consumer::write_offset(const std::string& topic)
{
	int64_t offset;
	if(consumer_.get_last_offset(topic, offset))
	{
		std::fstream out(get_full_name(offset_path_, topic + ".offset"), std::fstream::out | std::fstream::binary | std::fstream::trunc);
		std::string str(std::to_string((long long)offset));
		out.write(str.c_str(), str.size());
		out.close();
	}
}

int64_t Consumer::read_offset(const std::string& topic)
{
	char buf[OFFSET_LEN] = {0};
	int64_t offset = RdKafka::Topic::OFFSET_BEGINNING;

	std::fstream in(get_full_name(offset_path_, topic + ".offset"), std::fstream::in | std::fstream::binary);
	in.read(buf, OFFSET_LEN);
	if(strlen(buf) > 0)
	{
		offset = std::stoll(buf, NULL, 10);
	}
	in.close();

	//std::cout<<"offset:"<<offset<<std::endl;
	return offset;
}

void Consumer::save(bool immediate)
{
	time_t t(NULL);
	for (auto it : topics_)
	{
		int count = it.second;
		if((immediate || 
			(((count > 0) && count % count_ == 0)
			|| t - last_time_ >= interval_))
			)
		{
			write_offset(it.first);
			last_time_ = t;
		}
		
	}
}

std::string Consumer::get_full_name(const std::string& path, const std::string& file_name)
{
	return (path.rfind('/') == path.length()) ? 
		path + file_name : path + "/" + file_name;
}

}

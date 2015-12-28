#include <string>
#include <string.h>
#include <functional>
#include <thread>
#include <time.h>
#include <iostream>
#include <fstream>
//#include <sstream>

#include "KafkaProducer.h"


#define		SIZE_LEN		4
#define		PROTO_SIZE		2048*10

void func(ustd::KafkaProducer p, const std::string& topic, const std::string& file)
{
	int partition = 0;
	int timeout = 0;
	
	std::fstream input(file, std::fstream::in | std::fstream::binary);
	while (!input.eof())
	{
		int size = 0;
		char proto[PROTO_SIZE] = {0};
		char buf[PROTO_SIZE] = {0};

		input.read((char*)&size, SIZE_LEN);
		if (size == 0)
		{
			break;
		}
		input.read(proto, size);

		memcpy(buf, (char*)&size, 4);
		memcpy(buf + 4, proto, size);

		p.send(buf, size + 4, topic, partition, timeout);
	}

	input.close();
}

int main(int argc, char* args[])
{
	std::string msg1("message1");
	//std::string msg2("message2");
	std::string name1("q");
	//std::string name2("h");
	std::string host("srv-nm-test20");
	int port = 9092;
	std::string file;//("./tc.28127.2015-12-16-12-00");

	if(argc > 1)
	{
		name1 = args[1];
		file = args[2];

		ustd::KafkaProducer p;
		p.init(host, port);
		p.create_topic(name1);
		//p.create_topic(name2);

		std::thread t1(func, p, name1, file);
		//std::thread t2(func, p, name2, msg2);

		t1.join();
		//t2.join();

		p.close();
	}

	return 0;
}

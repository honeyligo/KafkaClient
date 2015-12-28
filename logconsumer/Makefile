PROJECT ?= logconsumer

all: $(PROJECT)

include ../librdkafka-0.8/mklove/Makefile.base

CFLAGS += -I../librdkafka-0.8/src
CXXFLAGS += -I../librdkafka-0.8/src-cpp -std=c++11

# librdkafka must be compiled with -gstrict-dwarf, but rdkafka_example must not,
# due to some clang bug on OSX 10.9
CPPFLAGS := $(subst strict-dwarf,,$(CPPFLAGS))

logconsumer: ../librdkafka-0.8/src-cpp/librdkafka++.a ../librdkafka-0.8/src/librdkafka.a KafkaConsumer.cpp main.cpp MMRTBDataModel.pb.cc Consumer.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) KafkaConsumer.cpp main.cpp MMRTBDataModel.pb.cc Consumer.cpp -o $@ $(LDFLAGS) \
		../librdkafka-0.8/src-cpp/librdkafka++.a ../librdkafka-0.8/src/librdkafka.a $(LIBS) -lstdc++ -lprotobuf

clean:
	rm -f $(PROJECT)


EXAMPLES ?= KafkaProducer

all: $(EXAMPLES)

include ../librdkafka-0.8/mklove/Makefile.base

CFLAGS += -I../librdkafka-0.8/src
CXXFLAGS += -I../librdkafka-0.8/src-cpp -std=c++11 -g

# librdkafka must be compiled with -gstrict-dwarf, but rdkafka_example must not,
# due to some clang bug on OSX 10.9
CPPFLAGS := $(subst strict-dwarf,,$(CPPFLAGS))

KafkaProducer: ../librdkafka-0.8/src-cpp/librdkafka++.a ../librdkafka-0.8/src/librdkafka.a KafkaProducer.cpp main.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) KafkaProducer.cpp main.cpp -o $@ $(LDFLAGS) \
		../librdkafka-0.8/src-cpp/librdkafka++.a ../librdkafka-0.8/src/librdkafka.a $(LIBS) -lstdc++

clean:
	rm -f $(EXAMPLES)


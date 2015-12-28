#ifndef _TIMER_TASK_H_
#define _TIMER_TASK_H_

#include <pthread.h>
#include <string>

namespace ustd
{

static std::string get_local_time(const char *format = "%Y-%m-%d %H:%M:%S", const time_t &gtime = time(NULL))
{
	tm *tm_now = localtime(&gtime);

	char time_now[128] = {0x00};
	strftime(time_now, 128, format, tm_now);

	return (std::string(time_now));
}

static std::string get_system_time(const char *format = "%a, %m %b %Y %H:%M:%S GMT", const time_t &gtime = time(NULL))
{
	//Mon, 10 Aug 2015 07:12:41 GMT
	tm *tm_now = gmtime(&gtime);

	char time_now[128] = {0x00};
	strftime(time_now, 128, format, tm_now);

	return (std::string(time_now));
}

static time_t str2time(const std::string &date_time, const char *format = "%Y-%m-%d %H:%M:%S")
{
	tm tm_time;
	strptime(date_time.c_str(), format, &tm_time);
	return mktime(&tm_time);
}

class timer
{
public:
	explicit timer(const int &millisecond = 0)
	{
		pthread_cond_init(&cond_t_, NULL);
		pthread_mutex_init(&mutex_t_, NULL);
		pthread_mutex_lock(&mutex_t_);

		this->reset(millisecond);
	}	

	virtual ~timer()
	{
		pthread_mutex_unlock(&mutex_t_);
		pthread_cond_destroy(&cond_t_);
		pthread_mutex_destroy(&mutex_t_);
	}

public:
	void wait()
	{
		if (this->timer_tick_ > 0)
		{
			clock_gettime(CLOCK_REALTIME_COARSE, &tm_);
			
			long int total_nsec = tm_.tv_nsec + this->timer_nsec_;
			tm_.tv_sec += this->timer_sec_ + total_nsec / 1000000000;
			tm_.tv_nsec = total_nsec % 1000000000;
			
			pthread_cond_timedwait(&cond_t_, &mutex_t_, &tm_);
		}
		else
		{
			pthread_cond_wait(&cond_t_, &mutex_t_);
		}
	}
	
	void reset(const int &millisecond = 0)
	{
		this->timer_tick_ = millisecond;
		if (this->timer_tick_ > 0)
		{
			this->timer_sec_ = this->timer_tick_ / 1000;
			this->timer_nsec_ = (this->timer_tick_ % 1000) * 1000000;
		}
	}
	
private:
	timespec tm_;
	int timer_tick_;
	time_t timer_sec_;
	long int timer_nsec_;
	pthread_cond_t cond_t_;
	pthread_mutex_t mutex_t_;
};

}

#endif


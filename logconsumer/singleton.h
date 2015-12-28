#ifndef _SINGLETON_
#define _SINGLETON_


// the macro used at class declaration.
#define USTD_DECLARE_SINGLETON(class_)						\
	public:													\
		static class_* getInstance ();						\
	private:												\
		class_(){};											\
		class_(const class_ &other);						\
		class_ &operator =(const class_ &other);			\
	private:												\
		static class_* instance_;

// the macro used at class definition.
#define USTD_DEFINE_SINGLETON(class_)							\
	class_* class_::instance_ = 0;							\
	class_* class_::getInstance ()							\
	{														\
		if (class_::instance_ == 0) {						\
			static class_ instance;							\
			class_::instance_ = &instance;					\
		}													\
		return class_::instance_;							\
	}

#endif // _SINGLETON_

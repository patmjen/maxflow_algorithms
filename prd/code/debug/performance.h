#ifndef performance_h
#define performance_h

namespace debug{
	class PerformanceCounter{
		private:
		long long t1;
		long long Dt;
		bool counting;
		public:
		PerformanceCounter();
		void start();
		long long tickcount()const;
		double time()const;
		void pause();
		void resume();
		void stop();
	};
};

#endif

/**
 * Test timers using different time services (aka virtual clocks)
 *
 * This is NOT an automated test. You have to visually parse the output
 * to determine if everything is ok.
 *
 * Debug output from this program has "now" first, as time since epoch (1970)
 *
 * The snprintf()'s help reduce the chance of log messages interacting with
 * each other, though they can still get mixed up a little.
 */
#include <stdio.h>
#include <iostream>
#include "os/main.h"
#include "Timer.hpp"

using namespace RTT;

// max acceptable error in desired vs actual wait time
static const double	EPSILON_SEC = 0.1;

static bool debug = true;

struct TestTimer : public Timer
{
    TestTimer(TimeService* clock = TimeService::Instance())
			:Timer(clock, 32, ORO_SCHED_RT, OS::HighestPriority)
    {
    }

    void timeout(Timer::TimerId id)
    {
        TimeService::nsecs now = 
			Seconds_to_nsecs(TimeService::Instance()->secondsSince( 0 ));
		if (debug) 
		{
			char str[100];
			snprintf(str, sizeof(str), "%20lld Timed out on %d", now, id);
			log(Debug) << str << endlog();
		}
    }
};

/// Provide a virtual clock whose rate can be scaled (e.g. 1x, 4x, 0.25x)
class ScaledTimeService : public RTT::TimeService
{
public:
    static ScaledTimeService* Instance()
	{
		if ( _instance == 0 )
		{
			_instance = new ScaledTimeService();
		}
		return _instance;
	}

    static bool Release()
	{
		if ( _instance != 0 )
		{
			delete _instance;
			_instance = 0;
			return true;
		}
		return false;
	}


    virtual ~ScaledTimeService()
	{}

    virtual RTT::TimeService::ticks getTicks() const
	{
		return -1; //use_clock ? rtos_get_time_ticks() + offset : 0 + offset;
	}

    virtual RTT::TimeService::nsecs getNSecs() const
	{
		TimeService::nsecs wallNow = TimeService::Instance()->getNSecs();
		return virtualLast + 
			(TimeService::nsecs)(scalar * (double)(wallNow - wallLast));
	}

    void setScalar(const double s)
	{
		assert((0.01 <= s) && (s <= 100));	// sanity checking
		TimeService::nsecs wallNow = TimeService::Instance()->getNSecs();
		virtualLast	+=
			(TimeService::nsecs)(scalar * (double)(wallNow - wallLast));
		wallLast	= wallNow;
		scalar		= s;
	}

    double getScalar() const 
	{ 
		return scalar; 
	}

    RTT::TimeService::nsecs toWallClock(const RTT::TimeService::nsecs virtualTime) const
	{
		// log(Error) << "toWallClock vTime=" << virtualTime << ", vLast="
		//            << virtualLast << ", wLast=" << wallLast << endlog();

		if (0 >= virtualTime) return 0;
		if (virtualTime < virtualLast) return 0;

		/// \todo deal better with virtualTime < virtualLast (at least up until
		/// virtualStart

		return wallLast + 
			(TimeService::nsecs)((double)(virtualTime - virtualLast) / scalar);
	}

protected:
    ScaledTimeService() :
        RTT::TimeService(),
        scalar(1.0),
        wallLast(TimeService::Instance()->getNSecs()),
        virtualLast(0)
	{}
	
private:
    static ScaledTimeService* _instance;

    double scalar;
    /// Wall clock time when last scalar was set
    RTT::TimeService::nsecs  wallLast;
    /// Virtual clock time when last scalar was set
    RTT::TimeService::nsecs  virtualLast;
};
ScaledTimeService* ScaledTimeService::_instance = 0;


void doArm(Timer& t, Timer::TimerId id, double wait_time, double scalar)
{
	TimeService::nsecs 	start;
	TimeService::nsecs 	end;
	TimeService::nsecs 	delta;		// wall
	TimeService::nsecs 	delta_s;	// scaled
	bool				ok;

	start = Seconds_to_nsecs(TimeService::Instance()->secondsSince(0));
	if (debug) { 
		char str[100];
		snprintf(str, sizeof(str), 
				 "%20lld doArm %d for %f secs", start, id, wait_time); 
		log(Debug) << str << endlog();
	}

	t.arm(id, wait_time);
	while (t.isArmed(id))
	{
		// do nothing
	}

	end		= Seconds_to_nsecs(TimeService::Instance()->secondsSince(0));
	delta	= (end - start);	// wall clock
	delta_s	= delta * scalar;	// scaled clock
	ok 		= fabs(nsecs_to_Seconds(delta_s) - wait_time) < EPSILON_SEC;
	char	str[100];
	snprintf(str, sizeof(str), 
			 "%20lld done in %lld wall %lld scaled, nsecs %s", 
			 end, delta, delta_s, ok?"OK":"** BAD **"); 
	log(Debug) << str << endlog();
}

void testTimer(TestTimer& timer, double scalar=1)
{
    std::cout << "." << std::flush;
	log(Debug) << "#### testTimer with scalar=" << scalar << endlog();

	doArm(timer, 0, 0.5, scalar);
	doArm(timer, 0, 0.6, scalar);
	doArm(timer, 0, 0.3, scalar);

	doArm(timer, 1, 0.3, scalar);

	doArm(timer, 2, 0.00001, scalar);
}

int main(int argc, char* argv[])
{
	if (0 == __os_init(argc, argv))
	{
        std::cout << "Running, see orocos.log for details " << std::flush;

		TestTimer	timer;
		log(Info) << "Test RTT TimeService" << endlog();
		testTimer(timer);

		TestTimer	timer2(ScaledTimeService::Instance());
		double scalar = 1.0;

		ScaledTimeService::Instance()->setScalar(scalar);
		log(Info) << "Test SCALED CLOCK: " << scalar << "x" << endlog();
		testTimer(timer2, scalar);

		scalar = 2.5;
		ScaledTimeService::Instance()->setScalar(scalar);
		log(Info) << "Test SCALED CLOCK: " << scalar << "x" << endlog();
		testTimer(timer2, scalar);

		scalar = 0.3;
		ScaledTimeService::Instance()->setScalar(scalar);
		log(Info) << "Test SCALED CLOCK: " << scalar << "x" << endlog();
		testTimer(timer2, scalar);
	}
	sleep(1);	// allow logging to complete
	__os_exit();

    std::cout << std::endl;
	return 0;
}

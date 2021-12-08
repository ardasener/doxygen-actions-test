#ifndef INCLUDE_TIMESLOT_CPP_
#define INCLUDE_TIMESLOT_CPP_


//! Represents a 1 hour time slot that a course can be scheduled in.
class TimeSlot
{

	public:

		//! An integer unique to this timeslot (assigned by load.cpp)
		int m_id;

		/**
		 * An integer denoting the time of the day.
		 * 0 -> 8.40,
		 * 1 -> 9.40,
		 * 2 -> 10.40,
		 * ...
		 */
		int m_hour;

		/** An integer denoting the day of the week.
		 *	0 -> Monday,
		 *	1 -> Tuesday,
		 *	...
		 */
		int m_day;

		//! Integer for the last day of the week (4=Friday)
		static const int MAX_DAY = 4;

		//! Integer for the last hour of the day
		static const int MAX_HOUR = 12;

		TimeSlot(int t_id,int t_hour, int t_day){
			m_id = t_id;
			m_hour = t_hour;
			m_day = t_day;
		}


		/** Compares given two timeslots by their ids.
		 * Due to the way the ids were assigned, this is equivalent to 
		 * sorting them by time. (Monday 0, Monday 1, ... , Friday 11, Friday 12)
		 */
		static bool compare(const TimeSlot & t1, const TimeSlot & t2) {
			return t1.m_id < t2.m_id;
		}
};



#endif

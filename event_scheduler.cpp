// #include <stdio.h>
// #include <stdlib.h>
#include <iostream>

#include <list>

using namespace std;

class EventScheduler
{
  /*
   * QUEUE that registers events 
   */
	public:
		EventScheduler();
		~EventScheduler();

		Event* get_front_event();		//pop from head of queue
		Event* read_ES();
		void register_event(Event*);
		void clear_ES();
		void purge_TO_event();
		void print_ES();

		bool is_ES_empty();
	private:
		list<Event*> _queue;
		Event* next_event;

		// bool is_to_event(Event *);

};

EventScheduler::EventScheduler()
{

}

EventScheduler::~EventScheduler()
{

}

Event* EventScheduler::read_ES()
{
	Event *e = NULL;
	if(!_queue.empty())
		e = _queue.front();

	return e;
}

Event* EventScheduler::get_front_event()
{
	Event *e = NULL;
	if(!_queue.empty())
	{
		e = _queue.front();
		_queue.pop_front();
	}

	return e;
}

bool sortEventByTime(Event *lhs, Event *rhs)
{
	return lhs->get_time_stamp() < rhs->get_time_stamp();
}

void EventScheduler::register_event(Event *e1)
{
	// Event e;
	//enqueue event in ES queue

	if(!_queue.empty())
	{
		//sort based on time stamp
		// cout << "here" << endl;
		_queue.push_back(e1);
		_queue.sort(sortEventByTime);
	}
	else
	{
		_queue.push_front(e1);	
	}
}

bool is_to_event(Event *e)
{
	return e->get_event_type() == 0;	//timeout
}

void EventScheduler::purge_TO_event()
{
	int itr = 0;
	if(!_queue.empty())
	{
		// cout << "Purging TIMEOUT" << endl;
		_queue.erase(
				std::remove_if(_queue.begin(), _queue.end(),
					is_to_event),
				_queue.end()
			);
	}
}

void EventScheduler::clear_ES()
{
	_queue.clear();
}

bool EventScheduler::is_ES_empty()
{
	return _queue.empty();
}

void EventScheduler::print_ES()
{
	cout << endl;
	cout << "**Printing ES**" << endl;
	if(!_queue.empty())
	{	
		for(std::list<Event*>::iterator it = _queue.begin(); it != _queue.end(); ++it)
		{
			Event *e = *it;
			if(e->get_event_type() == 0)
				cout << "event: TIMEOUT Event" << endl;
			else
			{
				if(e->get_error_flag() == 0)
					cout << "event: ACK Event" << endl;
				else
					cout << "event: NAK Event" << endl;
			}
				
			cout << "rn: " << e->get_sn() << endl;
			cout << "timestamp: " << e->get_time_stamp() << endl;

		}
	}
	else
	{
		cout << "ES is empty" << endl;
	}

	cout << endl;
}


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
		void register_event(Event*);
		void clear_ES();
	private:
		list<Event*> _queue;

		Event* next_event;

};

EventScheduler::EventScheduler()
{

}

EventScheduler::~EventScheduler()
{

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

void EventScheduler::register_event(Event *e)
{
	// Event e;
	//enqueue event in ES queue
	if(!_queue.empty())
	{
		//sort based on time stamp
		if(_queue.front()->get_time_stamp() > e->get_time_stamp())
			_queue.push_front(e);
		else
			_queue.push_back(e);		
	}
	else
	{
		_queue.push_front(e);	
	}
	
}

void EventScheduler::clear_ES()
{
	_queue.clear();
}

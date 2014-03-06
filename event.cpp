using namespace std;

class Event
{
	public:
		Event();
		Event(int, double, int, int);
		~Event();

		int get_event_type();
		double get_time_stamp();
		int get_sn();
		int get_error_flag();

	private:
		// enum Type { TIME_OUT, ACK};
		int event_type;	//TIME_OUT = 0, ACK = 1
	   	double total_time;
	   	int seq_num;	//sn for timeout, rn for ack
	   	int error_flag;
};

Event::Event()
{

}

Event::Event(int type, double curr_time, int sn, int error)
: event_type(type)
, total_time(curr_time)
, seq_num(sn)
, error_flag(error)
{

	// this->Type = type == 0 ? TIME_OUT : ACK;
	// this->total_time = curr_time;
	// this->seq_num = sn;
	// this->error_flag = error;
}

Event::~Event()
{

}

int Event::get_event_type()
{
	return event_type;
}

double Event::get_time_stamp()
{
	return total_time;
}

int Event::get_sn()
{
	return seq_num;
}

int Event::get_error_flag()
{
	return error_flag;
}
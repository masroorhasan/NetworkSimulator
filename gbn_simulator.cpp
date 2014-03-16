#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <list>
#include <vector>
// #include <iterator>

using namespace std;

class Frame
{
	public:
		Frame(int);
		Frame(int,int,int);
		Frame(int,int,int,double);
		~Frame();

		int get_frame_type() 			{ return type; }
		int get_frame_size() 			{ return length; }
		int get_frame_sn() 				{ return sn; }
		double get_frame_timestamp() 	{ return timestamp; }

		void set_timestamp(double);
		void set_sn(int);
	private:
		int type;		//0: pckt, 1: ack
		int length;
		int sn;
		double timestamp;
};

Frame::Frame(int type)
: type(type)
{

}

Frame::Frame(int type, int sn, int length)
: type(type)
, sn(sn)
, length(length)
{
	//
}

Frame::Frame(int type, int sn, int length, double timestamp)
: type(type)
, sn(sn)
, length(length)
, timestamp(timestamp)

{
	// length = type == 0 ? pckt_header + pckt_length : pckt_header;
}

Frame::~Frame()
{
	//
}

void Frame::set_timestamp(double new_time)
{
	this->timestamp = new_time;
}

void Frame::set_sn(int new_sn)
{
	this->sn = new_sn;
}

class GBN_Simulator
{
	public:
		GBN_Simulator(int, int, double, double, double, int);
		~GBN_Simulator();

		double channel(int);
		void receiver(int);
		Event* send(int);		

		int get_s1n();
		int get_rn();
		double get_Ttc(int);
		double get_tc();

		int get_seq_num(int);
		int shift_size();

		void fill_buffer();

		void update_tc(double);
		void update_pckt_T(int,double);
		void update_nea(int);
		int update_window(int,int);
		bool check_expected_acks(int);

		void print_buffer();
		void print_T();
	private:
		//sender
		int sequence_numer;
		// int next_expected_ack;	//need to use this, see flow chart
		double current_time;
		double delta; 			//delta: input to the simulator


		//channel
		double prop_delay;
		int frame_error;		//0: prob with BER, 1: prob with 1-BER
		int ack_error;			//0: prob with BER, 1: prob with 1-BER
		bool pckt_lost; 		//NIL if pckt lost
		bool ack_lost;
		int channel_capacity;	//c
		double ber; 			//bit error rate


		//receiver
		int next_expected_frame;
		int receive_num;

		//buffer and window
		// list<frame*> buffer; 		//M: to hold a single pckt
		vector<Frame*> buffer;
		vector<double> pckt_T;			//T: pckt transmission time
		vector<int> next_expected_ack;	//set of expected rn's
		int window_size;

		//packet
		int pckt_length; 	//l
		int pckt_header;	//H

		// frame *data_frame;
		// frame *ack_frame;
		// int ctr; 
		int shifting_size;
};

GBN_Simulator::GBN_Simulator(int header, int length, double c, double tao, double ber, int window)
: 	pckt_header(header)
,	pckt_length(length)
,	channel_capacity(c)
,	prop_delay(tao)
,	ber(ber)
,	window_size(window)
{
	current_time = 0.0;

	//receiver
	next_expected_frame = 1;
	receive_num = 0;


	double time_sent = 0.0;
	//populate buffer
	for(int i = 0; i < window_size; i++)
	{
		// time_sent += ((double)((pckt_header + pckt_length)*8.0) / (double)channel_capacity);
		Frame *d_frame = new Frame(0, i%(window_size+1), (pckt_header+pckt_length));
		buffer.push_back(d_frame);
	}

	// cout << "buffer size: " << buffer.size() << endl;

	int shifting_size = 0;

	srand(time(NULL));
}

GBN_Simulator::~GBN_Simulator()
{

}

double GBN_Simulator::get_tc()
{
	return current_time;
}

double GBN_Simulator::get_Ttc(int index)
{
	// cout << "index: " << index << endl;
	// print_buffer();
	// cout << "at T[" << index << "] = " << buffer.at(index)->get_frame_timestamp() << endl;
	return buffer.at(index)->get_frame_timestamp();
}

int GBN_Simulator::get_seq_num(int index)
{
	return buffer.at(index)->get_frame_sn();
}

int GBN_Simulator::get_s1n()
{
	return buffer.front()->get_frame_sn();
}

bool GBN_Simulator::check_expected_acks(int rn)
{
	// std::vector<int>::iterator it;
	// it = std::find(next_expected_ack.begin(), next_expected_ack.end(), rn);

	// if(it != next_expected_ack.end())
	// 	return true;
	
	// return false;

	int sn_acked = (rn+window_size)%(window_size+1);
	for(int i =0; i < window_size; i++)
	{
		if(buffer.at(i)->get_frame_sn() == sn_acked)
			return true;
	}

	cout << "mismatch_rn" << endl;

	return false;
}

void GBN_Simulator::fill_buffer()
{
	for(int i = buffer.size(); i < window_size; i++)
	{
		int sn = (buffer.at(i-1)->get_frame_sn()+1)%(window_size+1);
		Frame *d_frame = new Frame(0, sn, (pckt_header+pckt_length));
		d_frame->set_timestamp(current_time+((double)((pckt_header + pckt_length)*8.0) / (double)channel_capacity));
		buffer.push_back(d_frame);
	}
}

/*
//works for ber 1e-4
int GBN_Simulator::update_window(int ctr, int rn)
{
	// cout << "rn: " << rn << endl;
	// cout << "Before SHIFT" << endl;
	// cout << "SN[0] = " << buffer.at(0)->get_frame_sn() << endl;
	print_buffer();
	

	vector<Frame*> tmp_buffer;
	
	int sn_acked = (rn+window_size)%(window_size+1);
	// cout << "sn acked = " << sn_acked << endl;

	int itr = -1;
	for(int i = 0; i < buffer.size(); i++)
	{
		if(buffer.at(i)->get_frame_sn() == sn_acked)
		{
			itr = i;
			break;
		}
	}

	itr++;

	itr = itr >= window_size ? itr%(window_size) : itr;

	shifting_size += itr;
	cout << "shift_size: " << itr << endl;

	int new_index = 0;
	for(int i = itr; i < buffer.size(); i++)
	{
		tmp_buffer.insert(tmp_buffer.begin()+new_index, buffer.at(i));
		new_index++;
	}

	buffer.clear();
	buffer = tmp_buffer;

	for(int i = buffer.size(); i < window_size; i++)
	{
		int sn = (buffer.at(i-1)->get_frame_sn()+1)%(window_size+1);
		Frame *d_frame = new Frame(0, sn, (pckt_header+pckt_length));
		//d_frame->set_timestamp(current_time+((double)((pckt_header + pckt_length)*8.0) / (double)channel_capacity));
		buffer.push_back(d_frame);
	}
	
	ctr++;
	ctr = ctr >= window_size ? ctr%window_size : ctr;
	ctr = ctr-(itr) < 0 ? -1*(ctr-(itr)) : ctr-(itr);


	cout << "updated SN index: " << ctr << endl;

	// cout << "new ctr: " << ctr << endl;
	
	// cout << "After SHIFT" << endl;
	// cout << "SN[0] = " << buffer.at(0)->get_frame_sn() << endl;
	// // cout << "T[0] = " << pckt_T.at(0) << endl;

	cout << "num successful pckts: " << shifting_size << endl;
	print_buffer();
	return ctr;
}
*/

int GBN_Simulator::update_window(int ctr, int rn)
{
	cout << "rn: " << rn << endl;
	cout << "Before SHIFT" << endl;
	cout << "SN[0] = " << buffer.at(0)->get_frame_sn() << endl;
	print_buffer();
	

	vector<Frame*> tmp_buffer;
	
	int sn_acked = (rn + window_size)%(window_size+1);
	cout << "sn acked = " << sn_acked << endl;

	int itr = -1;
	for(int i = 0; i < buffer.size(); i++)
	{
		if(buffer.at(i)->get_frame_sn() == sn_acked)
		{
			itr = i;
			break;
		}
	}

	itr++;

	itr = itr >= window_size ? itr%(window_size) : itr;

	shifting_size += itr;
	cout << "successful pckts: " << itr << endl;

	//what if itr = 4
	// if(itr >= window_size)
	// 	itr = 0;

	int new_index = 0;
	for(int i = itr; i < buffer.size(); i++)
	{
		tmp_buffer.insert(tmp_buffer.begin()+new_index, buffer.at(i));
		new_index++;
	}

	buffer.clear();
	buffer = tmp_buffer;

	for(int i = buffer.size(); i < window_size; i++)
	{
		int sn = (buffer.at(i-1)->get_frame_sn()+1)%(window_size+1);
		Frame *d_frame = new Frame(0, sn, (pckt_header+pckt_length));
		//d_frame->set_timestamp(current_time+((double)((pckt_header + pckt_length)*8.0) / (double)channel_capacity));
		buffer.push_back(d_frame);
	}
	
	ctr++;
	ctr = ctr-(itr);
	ctr = ctr < 0 ? -1*ctr : ctr;
	ctr = ctr >= window_size ? ctr%window_size : ctr;
	

	// ctr++;
	// ctr = ctr >= window_size ? ctr%window_size : ctr;
	// ctr = ctr-(itr) < 0 ? -1*(ctr-(itr)) : ctr-(itr);

	cout << "updated SN index: " << ctr << endl;

	// cout << "new ctr: " << ctr << endl;
	
	// cout << "After SHIFT" << endl;
	// cout << "SN[0] = " << buffer.at(0)->get_frame_sn() << endl;
	// // cout << "T[0] = " << pckt_T.at(0) << endl;

	cout << "num successful pckts                   : " << shifting_size << endl;
	print_buffer();
	return ctr;
}


int GBN_Simulator::shift_size()
{
	
	return shifting_size;
}

void GBN_Simulator::update_tc(double new_current_time)
{
	// if(new_current_time >= current_time)
	current_time = new_current_time;
}

void GBN_Simulator::update_pckt_T(int ctr, double new_time)
{
	//check if indexing has to be mod N+1
	// double time_to_update = current_time;
	// time_to_update += new_time;

	// cout << "new transfer time: " << current_time+new_time << endl;
	buffer.at(ctr)->set_timestamp(current_time+new_time);

	// update_tc(time_to_update);
	
	// cout << "UPDATING pckt_T: T[" << ctr << "] = " << buffer.at(ctr)->get_frame_timestamp() << endl;
}

void GBN_Simulator::update_nea(int ctr)
{
	// cout << "SN[" << ctr << "] = " << buffer.at(ctr)->sn << endl;

	int expected_ack = (buffer.at(ctr)->get_frame_sn()+1)%(window_size+1);
	// next_expected_ack.insert(next_expected_ack.begin()+ctr, expected_ack);
	next_expected_ack.push_back(expected_ack);
	// cout << "UPDATING nea: next_expected_ack[" << ctr << "] = " << next_expected_ack.at(ctr) << endl;

}

double GBN_Simulator::channel(int length)
{
	//inputs: c, tao, ber
	//output: return time (+ prop delay) or Nil (data or ack frame was lost)
	int bit_error_num = 0;	
	double channel_time = current_time;
	frame_error = 0;
	ack_error = 0;
	pckt_lost = false;
	// ack_lost = false;

	int total_num_bits = (length) * 8;
	for(int bit = 0; bit < total_num_bits; bit++)
	{
		//generate random prob
		double uniform_random_prob = 0.0;
		uniform_random_prob = (((double) rand() ) / RAND_MAX);	//rv between [0,1]
		
		if(uniform_random_prob <= (1.0-ber))
			continue;
		else
			bit_error_num++;	//bit in error	
	}

	// cout << bit_error_num << endl;

	if(bit_error_num >= 5)
	{
		//lost pckt or ack
		// cout << "pckt_lost = true" << endl;
		pckt_lost = true;
	}
	else if(bit_error_num > 0 && bit_error_num < 5)
	{
		//frame or ack error
		if(length < pckt_header + pckt_length)
			ack_error = 1;
		else
			frame_error = 1;
	}	
	else
	{
		//no error
		// cout << "no error" << endl;
	}
		
	
	channel_time += (double)prop_delay;
	// cout << "after fc - current time: " << channel_time << endl;
	return channel_time;
}

void GBN_Simulator::receiver(int pckt_ctr)
{
	//generate ack frame
	Frame *ack_frame = new Frame(1);
	
	// cout << "next_expected_frame (rn): " << next_expected_frame << endl;

	if(frame_error == 1)
	{		
		// cout << "frame_error = true, data frame error, nef stays the same" << endl;
		cout << "frame_error = true" << endl;
		receive_num = next_expected_frame;
	} 
	else
	{
		//check frame sn == next_expected_frame
		if(buffer.at(pckt_ctr)->get_frame_sn() == next_expected_frame)
		{
			// cout << "no error, incrementing rn.." << endl;
			// cout << "frame_error = false, no data frame error, incrementing nef" << endl;
			next_expected_frame = (next_expected_frame+1)%(window_size+1);
			// ack_frame->set_sn(next_expected_frame);	//rn
			receive_num = next_expected_frame;
		}
		else
		{
			cout << "shouldnt be here" << endl;
		}
	}
	
	// current_time += ((double)pckt_header / (double)channel_capacity);
}

Event* GBN_Simulator::send(int pckt_ctr)
{
	//call forward channel 
	double fc_time = channel(pckt_header + pckt_length);
	
	//receiver logic
	receiver(pckt_ctr);

	//call reverse channel
	double rc_time = channel(pckt_header);
	
	if(pckt_lost)
		return NULL;
	
	//update current time to transmission time + prop delay
	// current_time += ((double)((pckt_header + pckt_length)*8.0) / (double)channel_capacity);
	// current_time += ((double)(pckt_header) * 8.0 / (double)channel_capacity);
	// current_time += (double)(prop_delay) * 2.0;

	double event_time = current_time;
	event_time += ((double)(pckt_header) * 8.0 / (double)channel_capacity);
	event_time += (double)(prop_delay) * 2.0;
	
	// cout << "ack time: " << event_time << endl;
	// cout << "requesting rn: " << receive_num << endl;
	

	//return ack event with timestamp, rn and flag
	if(ack_error == 0)
	{
		Event *ack = new Event(1, event_time, receive_num, ack_error);
		// receive_num	+= 1;
		// receive_num %= (window_size+1);
		return ack;
	}
	else
	{
		cout << "ack_error = true" << endl;
		Event *ack = new Event(1, event_time, receive_num, ack_error);
		return ack;
	}
}

void GBN_Simulator::print_buffer()
{
	cout << "PRINTING SN BUFFER" << endl;
	if(!buffer.empty())
	{	
		int i = 0;
		for(std::vector<Frame*>::iterator it = buffer.begin(); it != buffer.end(); ++it)
		{
			Frame *f = *it;
			cout << "SN[" << i << "] = " << f->get_frame_sn() << endl;
			i++;
		}
	}
	else
	{
		cout << "buffer is empty" << endl;
	}
}

void GBN_Simulator::print_T()
{
	cout << "PRINTING T BUFFER" << endl;
	if(!pckt_T.empty())
	{	
		int i = 0;
		for(std::vector<double>::iterator it = pckt_T.begin(); it != pckt_T.end(); ++it)
		{
			// frame *f = *it;
			double timee = *it;
			cout << "T[" << i << "] = " << timee << endl;
			i++;
		}
	}
	else
	{
		cout << "T is empty" << endl;
	}
}


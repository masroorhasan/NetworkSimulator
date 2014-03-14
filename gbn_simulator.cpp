#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <list>
#include <vector>

using namespace std;

struct frame
{
	int type;		//0: pckt, 1: ack
	int length;
	int sn;
};

class GBN_Simulator
{
	public:
		GBN_Simulator(int, int, double, double, double, int);
		~GBN_Simulator();

		double channel(int);
		void receiver();
		Event* send();		

		int get_s1n();
		int get_rn();
		double get_Ttc(int);
		double get_tc();


		void update_tc(double);
		void update_pckt_T(int,double);
		void update_nea(int);
		void update_window(int);
		void update_buffer(int);
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

		//buffer and window
		// list<frame*> buffer; 		//M: to hold a single pckt
		vector<frame*> buffer;
		vector<double> pckt_T;			//T: pckt transmission time
		vector<int> next_expected_ack;	//set of expected rn's
		int window_size;

		//packet
		int pckt_length; 	//l
		int pckt_header;	//H

		frame *data_frame;
		frame *ack_frame;
		// int ctr; 
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
	next_expected_frame = 0;

	// buffer.resize(window_size);
	// pckt_T.resize(window_size);
	// next_expected_ack.resize(window_size);
	//populate buffer
	// for(int i = 0; i < window_size; i++)
	// {
	// 	int sn = i % (window_size+1);
	// 	frame *d_frame = new frame();
	// 	d_frame->type = 0;
	// 	d_frame->length = pckt_header + pckt_length;
	// 	d_frame->sn = i % (window_size+1);
	// 	buffer.insert(buffer.begin()+i, d_frame);
	// 	// buffer.push_back(d_frame);
	// 	// next_expected_ack.insert(next_expected_ack.begin()+i, 0);
	// 	// next_expected_ack.push_back(0);
	// }

	frame *d_frame = new frame();
	d_frame->type = 0;
	d_frame->length = pckt_header + pckt_length;
	d_frame->sn = 0 % (window_size+1);
	buffer.insert(buffer.begin(), d_frame);

	cout << "buffer size: " << buffer.size() << endl;
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
	cout << "getting: T[" << index << "] = " << pckt_T.at(index) << endl;
	return pckt_T.at(index);
}

int GBN_Simulator::get_s1n()
{
	return buffer.front()->sn;
}

bool GBN_Simulator::check_expected_acks(int rn)
{
	// cout << "rn: " << rn << endl;
	for(int i = 0; i < window_size; i++)
	{

		if(rn == next_expected_ack.at(i))
			return true;
	}

	return false;
}

void GBN_Simulator::update_window(int rn)
{
	cout << "Before SHIFT" << endl;
	cout << "SN[0] = " << buffer.at(0)->sn << endl;
	cout << "T[0] = " << pckt_T.at(0) << endl;

	vector<frame*> tmp_buffer;

	int shift_size = (rn - buffer.front()->sn) % (window_size+1);
	shift_size = shift_size < 0 ? shift_size * -1 : shift_size;

	print_buffer();
	print_T();
	//rotate vector
	std::rotate(buffer.begin(), buffer.begin()+shift_size, buffer.end());
	std::rotate(pckt_T.begin(), pckt_T.begin()+shift_size, pckt_T.end());

	// buffer.erase(buffer.begin(), buffer.begin() + shift_size);
	// pckt_T.erase(pckt_T.begin(), pckt_T.begin() + shift_size);
	

	int itr = 0;
	while(itr < shift_size)
	{
		buffer.pop_back();
		pckt_T.pop_back();
		itr++;
	}

	cout << "SHIFTING SN and T by: " << shift_size << endl;
	cout << "SN[0] = " << buffer.at(0)->sn << endl;
	cout << "T[0] = " << pckt_T.at(0) << endl;

	print_buffer();
	print_T();
}

void GBN_Simulator::update_tc(double new_current_time)
{
	// if(new_current_time >= current_time)
	current_time = new_current_time;
}

void GBN_Simulator::update_pckt_T(int ctr, double new_time)
{
	//check if indexing has to be mod N+1
	// cout << "here" << endl;
	pckt_T.insert(pckt_T.begin()+ctr, new_time);
	cout << "UPDATING pckt_T: T[" << ctr << "] = " << pckt_T.at(ctr) << endl;
}

void GBN_Simulator::update_nea(int ctr)
{
	// cout << "SN[" << ctr << "] = " << buffer.at(ctr)->sn << endl;
	
	int expected_ack = (buffer.at(ctr)->sn+1)%(window_size+1);
	next_expected_ack.insert(next_expected_ack.begin()+ctr, expected_ack);
	// next_expected_ack.push_back(expected_ack);
	cout << "UPDATING nea: next_expected_ack[" << ctr << "] = " << next_expected_ack.at(ctr) << endl;

}

void GBN_Simulator::update_buffer(int index)
{
	// print_buffer();
	cout << "index: " << index << endl; 

	int frame_sn = (buffer.at(index-1)->sn+1)%(window_size+1);
	data_frame = new frame();
	data_frame->type = 0;
	data_frame->length = pckt_header + pckt_length;
	data_frame->sn = frame_sn;

	cout << "putting new pckt in buffer: SN[" << index << "] = " <<  frame_sn << endl;

	buffer.insert(buffer.begin()+index, data_frame);
	// buffer.push_back(data_frame);
	print_buffer();
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

	if(length < pckt_header + pckt_length)
		channel_time += ((double)pckt_header*8.0 / (double)channel_capacity);
	else
		channel_time += ((double)((pckt_header + pckt_length)*8.0) / (double)channel_capacity);


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

void GBN_Simulator::receiver()
{
	//generate ack frame
	ack_frame = new frame();
	ack_frame->type = 1; //ack
	ack_frame->length = pckt_header;

	//if error - send ack frame with current next_expected_frame
	// cout << "sn: " << this->get_s1n() << endl;

	if(frame_error == 1)
	{		
		ack_frame->sn = next_expected_frame;	//rn
	} 
	else
	{
		//check frame sn == next_expected_frame
		for(int i = 0; i < window_size; i++)
		{
			if(buffer.at(i)->sn == next_expected_frame)
			{
				cout << "nef: " << next_expected_frame << endl;
				next_expected_frame += 1;
				next_expected_frame %= window_size+1;
				ack_frame->sn = next_expected_frame;	//rn	
				cout << "updated nef (assigning rn): " << next_expected_frame << endl;
				break;
			}
		}
		
		// if(this->get_s1n() == next_expected_frame) 
		// {
		// 	cout << "incrementing nef" << endl;
		// 	next_expected_frame += 1;
		// 	next_expected_frame %= window_size+1;
		// 	ack_frame->sn = next_expected_frame;	//rn	
		// }
	}
	
	// current_time += ((double)pckt_header / (double)channel_capacity);
}

Event* GBN_Simulator::send()
{
	//call forward channel 
	double fc_time = channel(pckt_header + pckt_length);
	
	//receiver logic
	receiver();

	//call reverse channel
	double rc_time = channel(pckt_header);

	if(pckt_lost)
		return NULL;
	
	//update current time to transmission time + prop delay
	// current_time += ((double)((pckt_header + pckt_length)*8.0) / (double)channel_capacity);
	current_time += ((double)(pckt_header) * 8.0 / (double)channel_capacity);
	current_time += (double)(prop_delay) * 2.0;

	cout << "requesting rn: " << next_expected_frame << endl;
	
	//return ack event with timestamp, rn and flag
	return new Event(1, current_time, next_expected_frame, ack_error);
}

void GBN_Simulator::print_buffer()
{
	cout << "PRINTING SN BUFFER" << endl;
	if(!buffer.empty())
	{	
		int i = 0;
		for(std::vector<frame*>::iterator it = buffer.begin(); it != buffer.end(); ++it)
		{
			frame *f = *it;
			cout << "SN[" << i << "] = " << f->sn << endl;
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


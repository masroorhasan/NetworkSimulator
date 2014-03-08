#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <list>

using namespace std;

class ABP_SIMULATOR
{
	public:
		ABP_SIMULATOR(double, int, int, double, double, double);
		~ABP_SIMULATOR();

		void sender();
		double channel(int);
		void receiver();

		Event* send();		
		void reset_state();

		int get_sn();
		int get_rn();
		double get_tc();

		int update_state(int);
		void clear_state();

		void update_tc(double);
	private:
		//sender
		int sequence_numer;
		int next_expected_ack;
		list<int> buffer; 			//to hold a single pckt
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


		//packet
		int pckt_length; 	//l
		int pckt_header;	//H

		struct frame
		{
			int type;		//0: pckt, 1: ack
			int length;
			int sn;
		};

		frame *data_frame;
		frame *ack_frame;
		// int ctr; 

};

ABP_SIMULATOR::ABP_SIMULATOR(double delta, int header, int length, double c, double tao, double ber)
: delta(delta)
, pckt_header(header)
, pckt_length(length)
, channel_capacity(c)
, prop_delay(tao)
, ber(ber)	
{
	//sender
	sequence_numer = 0;
	next_expected_ack = (sequence_numer + 1) % 2;
	current_time = 0.0;
	buffer.resize(1);

	//receiver
	next_expected_frame = 0;

	// ctr = 0;

	srand(time(NULL));
}

ABP_SIMULATOR::~ABP_SIMULATOR()
{

}

int ABP_SIMULATOR::get_sn()
{
	return sequence_numer;
}

int ABP_SIMULATOR::get_rn()
{
	return next_expected_frame; 
}

double ABP_SIMULATOR::get_tc()
{
	return current_time;
}

void ABP_SIMULATOR::sender()
{
	// current_time = 0.0;
	// next_expected_ack = (sequence_numer + 1) % 2;

	//generate packet to put on frame
	data_frame = new frame();
	data_frame->type = 0;
	data_frame->length = pckt_header + pckt_length;
	data_frame->sn = sequence_numer;

	//store pct in buffer
	if(buffer.empty())
		buffer.push_front(sequence_numer);

	// current_time += ((double)((pckt_header + pckt_length)*8) / (double)channel_capacity);
}

int ABP_SIMULATOR::update_state(int rn)
{
	//no error and rn = next_expected_ack
	// cout << "rn (next expected frame): " << rn << endl;
	// cout << "rn (next expected frame): " << next_expected_ack << endl;
	if(/*ack_error == 0 && */rn == next_expected_ack)
	{
		// cout << "no error in ACK EVENT..." << endl;
		//increment sn
		//empty buffer
		sequence_numer += 1;
		sequence_numer %= 2;
		next_expected_ack = (sequence_numer + 1); 
		next_expected_ack %= 2;
		buffer.clear();

		return 1;
	} else
	{
		// cout << "HERE" << endl;
	}
	//ack_error == 1 || next_expected_frame != next_expected_ack
	// cout << "ack_error == 1 || "; 
	// cout << "frame_error == 1 || next_expected_frame != next_expected_ack" <<  endl;
	return 0;
}

void ABP_SIMULATOR::clear_state()
{

}

void ABP_SIMULATOR::update_tc(double new_current_time)
{
	// if(new_current_time >= current_time)
	current_time = new_current_time;
}

double ABP_SIMULATOR::channel(int length)
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

void ABP_SIMULATOR::receiver()
{
	//generate ack frame
	ack_frame = new frame();
	ack_frame->type = 1; //ack
	ack_frame->length = pckt_header;

	//if error - send ack frame with current next_expected_frame
	if(frame_error == 1)
	{		
		ack_frame->sn = next_expected_frame;	//rn
	} 
	else
	{
		//check frame sn == next_expected_frame
		if(data_frame->sn == next_expected_frame) 
		{
			next_expected_frame += 1;
			next_expected_frame %= 2;
			ack_frame->sn = next_expected_frame;	//rn	
		}
	}
	
	// current_time += ((double)pckt_header / (double)channel_capacity);
}

Event* ABP_SIMULATOR::send()
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
	current_time += ((double)((pckt_header + pckt_length)*8.0) / (double)channel_capacity);
	current_time += ((double)(pckt_header) * 8.0 / (double)channel_capacity);
	current_time += (double)(prop_delay) * 2.0;
	// cout << "rtt: " << current_time << endl;
	//return ack event
	return new Event(1, current_time, ack_frame->sn, ack_error);
}

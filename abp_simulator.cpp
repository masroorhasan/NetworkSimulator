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
		double get_rtt();
	private:
		//sender
		int sequence_numer;
		int next_expected_ack;
		list<int> buffer; 			//to hold a single pckt
		double current_time;
		double time_out; 			//delta: input to the simulator


		//channel
		int prop_delay;
		int frame_error;		//0: prob with BER, 1: prob with 1-BER
		int ack_error;			//0: prob with BER, 1: prob with 1-BER
		bool pckt_lost; 		//NIL if pckt lost
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

};

ABP_SIMULATOR::ABP_SIMULATOR(double delta, int header, int length, double c, double tao, double ber)
: time_out(delta)
, pckt_header(header)
, pckt_length(length)
, channel_capacity(c)
, prop_delay(tao)
, ber(ber)	
{
	//sender
	sequence_numer = 0;
	current_time = 0.0;
	buffer.resize(1);

	//receiver
	next_expected_frame = 0;

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
	return next_expected_frame; //??
}

double ABP_SIMULATOR::get_rtt()
{
	return current_time;
}

void ABP_SIMULATOR::sender()
{
	// current_time = 0.0;
	next_expected_ack = (sequence_numer + 1) % 2;

	//generate packet to put on frame
	data_frame = new frame();
	data_frame->type = 0;
	data_frame->length = pckt_header + pckt_length;
	data_frame->sn = sequence_numer;

	//store pct in buffer
	if(buffer.empty())
		buffer.push_front(sequence_numer);

	current_time += ((double)((pckt_header + pckt_length)*8) / (double)channel_capacity);
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
	//check length of packet in channel to see if data or ack frame

	int total_num_bits = (length) * 8;
	for(int bit = 0; bit < total_num_bits; bit++)
	{
		//generate random prob
		double uniform_random_prob = 0.0;
		uniform_random_prob = (((double) rand() ) / RAND_MAX);	//rv between [0,1]
		
		// cout << uniform_random_prob << endl;
		
		if(uniform_random_prob <= (1.0-ber))
		{
			
			// cout << "1";
			continue;
		}
		else
		{
			// cout << "0";
			bit_error_num++;	//bit in error	
		}

		// cout << endl;	
	}

	// cout << bit_error_num << endl;

	if(bit_error_num >= 5)
	{
		pckt_lost = true;
		// cout << "pckt lost" << endl;
		// return NULL;		//NOTE: verify this
	}
	else if(bit_error_num > 0 && bit_error_num < 5)
	{
		if(length < pckt_header + pckt_length)
			ack_error = 1;
		else
			frame_error = 1;

		// cout << "pckt in error" << endl;
	}	
	else
	{
		//no error
		// cout << "no error" << endl;
	}
		
	
	channel_time += (double)prop_delay;
	// cout << "after fc - current time: " << channel_time << endl;
	return channel_time;
	//set flag for frame/ack error
}

void ABP_SIMULATOR::receiver()
{
	//check flag
	//if pckt lost - do nothing
	if(pckt_lost)
		return;

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
		//if no err - increment next_expected_frame
		//set rn == next_expected_frame
		next_expected_frame += 1 % 2;
		ack_frame->sn = next_expected_frame;	//rn
	}
	
	current_time += ((double)pckt_header / (double)channel_capacity);
}

Event* ABP_SIMULATOR::send()
{
	//call forward channel 
	double fc_time = channel(pckt_header + pckt_length);
	
	//update current time
	current_time = fc_time;
	
	// if(current_time == NULL)
	// 	return NULL;

	//receiver logic
	receiver();

	//call reverse channel
	double rc_time = channel(pckt_header);

	//update current time
	current_time = rc_time;
	// cout << "after rc - current time: " << current_time << endl;
	
	Event * ack_event = new Event(1, current_time, next_expected_frame, ack_error);


	//move to another method that just updates vars/counters
	if(ack_error == 0 && next_expected_frame == next_expected_ack)
	{
		//increment sn
		//empty buffer
		sequence_numer += 1 % 2;
		buffer.clear();
	} 
	else
	{
		//ack_error == 1 || next_expected_frame != next_expected_ack
		//
	}
		


	return ack_event;
}


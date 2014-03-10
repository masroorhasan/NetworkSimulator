#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <list>

using namespace std;

class GBN_Simulator
{
	public:
		GBN_Simulator(double, int, int, double, double, double);
		~GBN_Simulator();

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
		list<int> buffer; 			//M: to hold a single pckt
		list<int> pckt_SN;			//ith sn
		list<int> pckt_L;			//ith L
		list<int> pckt_T;				//T: time ith pckt sent

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


#include <stdlib.h>
#include <stdio.h>

#include "event.cpp"
#include "abp_simulator.cpp"
#include "event_scheduler.cpp"

using namespace std;

int main()
{
	double tao = 0.01; 		//in ms, testing
	int successful_pckts_num = 100;

	//sender side input params
	int header_length = 5;				//bytes
	int pckt_length = 15;				//bytes
	double delta = 5.0 * tao;

	//channel params
	double transfer_rate = 5000000;		//C (bps)
	double prop_delay = 0.01;			//Tao
	double ber = 0.001; 					//BER


	double clk = 0.0;


	ABP_SIMULATOR * abp_sim = new ABP_SIMULATOR(delta, header_length, pckt_length,
									transfer_rate, prop_delay, ber);

	EventScheduler * ES = new EventScheduler();
	
	int succ_pckt_ctr = 0;
	while(succ_pckt_ctr < successful_pckts_num)
	{
		abp_sim->sender();
		//register timeout event
		ES->register_event(new Event(0, delta, abp_sim->get_sn(), 0));
		//fc, receiever, rc
		Event * ack_event = abp_sim->send();
		
		//insert based on time stamp
		ES->register_event(ack_event);


		//pop from queue
		//if toE then retransmit
		//if ackE then check error flag
			//if error, then do nothing
			//else increment succ_pckt_ctr


		if(ack_event->get_error_flag() == 0)
		{
			//update abp simulator instance vars/counters
			succ_pckt_ctr++;
		}
		else
		{
			//ack in error
			//retransmit pckt
		}
	}

	return 0;
}

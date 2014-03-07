#include <stdlib.h>
#include <stdio.h>

#include "event.cpp"
#include "abp_simulator.cpp"
#include "event_scheduler.cpp"

using namespace std;

int main()
{
	double tao = 0.005; 		//in ms, testing
	int successful_pckts_num = 10;

	//sender side input params
	int header_length = 54;				//bytes
	int pckt_length = 1500;				//bytes
	double delta = 2.5 * tao;

	//channel params
	double transfer_rate = 5000000;		//C (bps)
	double prop_delay = 0.05;			//Tao
	double ber = 0.00001; 					//BER


	double clk = 0.0;
	double timeout = 0.0;
	
	ABP_SIMULATOR * abp_sim = new ABP_SIMULATOR(delta, header_length, pckt_length,
												transfer_rate, prop_delay, ber);
	EventScheduler * ES = new EventScheduler();

	//initialize state and sender
	abp_sim->sender();
	cout << "initializing first sender, with sn " << abp_sim->get_sn() << endl;
	timeout += abp_sim->get_tc();
	timeout += (double)((header_length + pckt_length)*8.0) / (double)transfer_rate;
	timeout += delta;
	cout << "registering initial TIMEOUT EVENT" << endl;
	//register timeout event
	ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0));
	
	int succ_pckt_ctr = 0;
	int itr = 0;
	while(succ_pckt_ctr < successful_pckts_num)
	{
		//fc, receiever, rc
		Event * ack_event = abp_sim->send();
		if(ack_event != NULL)
		{
			cout << "registering ACK EVENT, with rn " << abp_sim->get_rn() << ", because..." << endl;
			ES->register_event(ack_event);
		}
		else
		{
			cout << "pckt/ack dropped...";
		}
		
		//pop from queue, also check if event != NULL
		Event * event = ES->get_front_event();
		//update tc and timeout
		// clk = abp_sim->get_tc();
		timeout += abp_sim->get_tc();
		timeout += (double)((header_length + pckt_length)*8.0) / (double)transfer_rate;
		timeout += delta;		

		// if(event == NULL)
		// 	continue;

		if(event->get_event_type() == 0) //TIMEOUT EVENT
		{
			//retransmit
			cout << "TIMED OUT" << endl;
			//register new timeout event
			ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0)); 
			cout << "resending pckt, with sn " << abp_sim->get_sn() << endl;
			itr++;
			continue;
		}
		else	//ACK EVENT
		{
			if(event->get_error_flag() == 0)
			{
				//update abp simulator instance vars/counters
				if(abp_sim->update_state(event->get_sn()) == 1)
				{
					succ_pckt_ctr++;
					cout << "SUCCESSFULL TRANSMISSION (no error)" << endl;
					itr++;
				} 
				else
				{
					//data frame error in forward channel
					cout << "frame error..." << endl;
					cout << "resending pckt, with sn " << abp_sim->get_sn() << endl;
					itr++;
					continue;
				}
	
				//clear ES
				ES->clear_ES();

				//start sending next pckt
				abp_sim->sender();
				cout << "initializing sender with sn " << abp_sim->get_sn() << endl;
				//note delta will change to current time + delta
				ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0)); 
			}
			else
			{
				cout << "ack error..." << endl;
				//pop ES again
				Event * next_event = ES->get_front_event();
				if(next_event->get_event_type() == 0)
				{
					cout << "timed out" << endl;
					//register new timeout event
					ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0)); 
					cout << "resending pckt, with sn " << abp_sim->get_sn() << endl;
					itr++;
				}
				
				continue;
			}
		}

		// itr++;
	}

	cout << endl;
	cout << "ber: " << ber << endl;
	cout << "ITERATION " << itr << endl;
	cout << "successful packet count: " << succ_pckt_ctr << endl;
	cout << "tc: " << abp_sim->get_tc() << endl;
	
	return 0;
}

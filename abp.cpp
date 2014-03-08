#include <stdlib.h>
#include <stdio.h>

#include "event.cpp"
#include "event_scheduler.cpp"
#include "abp_simulator.cpp"


using namespace std;

/*
	EVENT PROCESSOR
*/

int main()
{
	double tao = 0.01; 		//in ms, testing
	int successful_pckts_num = 10000;

	//sender side input params
	int header_length = 54;				//bytes
	int pckt_length = 1500;				//bytes
	double delta = 2.5 * tao;

	//channel params
	double transfer_rate = 5000000;		//C (bps)
	double prop_delay = tao;			//Tao
	double ber = 0.0001;				//BER


	double clk = 0.0;
	double timeout = 0.0;
	
	ABP_SIMULATOR * abp_sim = new ABP_SIMULATOR(delta, header_length, pckt_length,
												transfer_rate, prop_delay, ber);
	EventScheduler * ES = new EventScheduler();

	//initialize state and sender
	//initiate_send(abp_sim->get_rn())

	abp_sim->sender();
	// cout << "initializing first sender, with sn " << abp_sim->get_sn() << endl;

	timeout += abp_sim->get_tc();	//0
	timeout += (double)((header_length + pckt_length)*8.0) / (double)transfer_rate;
	timeout += delta;
	
	// cout << "registering initial timeout: " << timeout << endl;

	//register timeout event
	ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0));
	
	int succ_pckt_ctr = 0;
	int itr = 0;
	while(succ_pckt_ctr < successful_pckts_num)
	{
		// timeout += abp_sim->get_tc();
		// timeout += (double)((header_length + pckt_length)*8.0) / (double)transfer_rate;
		// timeout += delta;	

		// ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0));			

		//fc, receiever, rc
		Event * ack_event = abp_sim->send();
		if(ack_event != NULL)
		{
			// cout << "registering ACK EVENT, with rn " << abp_sim->get_rn() << ", because..." << endl;
			ES->register_event(ack_event);
		}
		
		//pop from queue, also check if event != NULL
		Event * event = ES->get_front_event();
		//update tc and timeout

		if(event->get_event_type() == 0) //TIMEOUT EVENT
		{
			//retransmit
			cout << "TIMED_OUT" << endl;
			ES->clear_ES();
			//register new timeout event
			// ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0)); 
			// cout << "resending pckt, with sn " << abp_sim->get_sn() << endl;

			//update current_time = timeout:: CHECK THIS
			abp_sim->update_tc(event->get_time_stamp());

			timeout = abp_sim->get_tc();
			timeout += (double)((header_length + pckt_length)*8.0) / (double)transfer_rate;
			timeout += delta;	

			ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0));			


			itr++;
			continue;
		}
		else	//ACK EVENT
		{
			int no_df_error = abp_sim->update_state(event->get_sn());
			if(event->get_error_flag() == 0 && no_df_error == 1)
			{	
				// cout << "ack event..." << endl;
				succ_pckt_ctr++;
				ES->clear_ES();
				// cout << "ack timestamp: " << event->get_time_stamp() << endl;
				//update states (incl. current time)	
				abp_sim->update_tc(event->get_time_stamp());
				// abp_sim->update_state(event->get_sn());
				// abp_sim->initiate_send();
				abp_sim->sender();

				timeout = abp_sim->get_tc();
				timeout += (double)((header_length + pckt_length)*8.0) / (double)transfer_rate;
				timeout += delta;	

				// cout << "registering timeout after ack: " << timeout << endl;

				ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0));			
				// cout << "initializing sender with sn " << abp_sim->get_sn() << endl;
				//note delta will change to current time + delta
				// ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0));
				itr++;
			}
			else
			{
				// cout << "nak event..." << endl;
				// abp_sim->update_tc(event->get_time_stamp());	
				// ES->clear_ES();
				
				// cout << "NAK timestamp: " << event->get_time_stamp() << endl;

				Event * next_event = ES->get_front_event();
				if(next_event->get_event_type() == 0)
				{
					//last timeout event
					// cout << "TIMEOUT" << endl;
					// cout << "timeout time: " << next_event->get_time_stamp() << endl;
					abp_sim->update_tc(next_event->get_time_stamp());

					abp_sim->sender();

					timeout = abp_sim->get_tc();
					timeout += (double)((header_length + pckt_length)*8.0) / (double)transfer_rate;
					timeout += delta;	

					// cout << "registering timeout after NAK: " << timeout << endl;

					ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0));			

					itr++;
				}
				
				// abp_sim->update_state(event->get_sn())
			}
		}

		// itr++;
	}

	cout << endl;
	cout << "ABP" << endl;
	cout << "ber: " << ber << endl;
	cout << "ITERATION " << itr << endl;
	cout << "successful packet count: " << succ_pckt_ctr << endl;
	cout << "tc: " << abp_sim->get_tc() << " s" << endl;
	cout << "throughput: " << (double)succ_pckt_ctr / abp_sim->get_tc() << " pckts/s" << endl;
	
	return 0;
}

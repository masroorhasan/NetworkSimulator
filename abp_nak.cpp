#include <stdlib.h>
#include <stdio.h>

#include "event.cpp"
#include "event_scheduler.cpp"
#include "abp_simulator.cpp"


using namespace std;

/*
	EVENT PROCESSOR
*/

	double tao = 0.25; 		//in ms, testing
	int successful_pckts_num = 10000/2;

	//sender side input params
	int header_length = 54;				//bytes
	int pckt_length = 1500;				//bytes
	// double delta = 2.5 * tao;
	double delta = 0.0;
	//channel params
	double transfer_rate = 5000000;		//C (bps)
	double prop_delay = tao;			//Tao
	double ber = 0.0001; 					//BER

	double clk = 0.0;
	double timeout = 0.0;

void run_sim()
{
	
	ABP_SIMULATOR * abp_sim = new ABP_SIMULATOR(delta, header_length, pckt_length,
												transfer_rate, prop_delay, ber);
	EventScheduler * ES = new EventScheduler();

	//initialize state and sender
	abp_sim->sender();

	timeout += abp_sim->get_tc();	//0.0
	timeout += (double)((header_length + pckt_length)*8.0) / (double)transfer_rate;
	timeout += delta;

	//register timeout event
	ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0));
	
	int succ_pckt_ctr = 0;
	int itr = 0;
	while(succ_pckt_ctr < successful_pckts_num)
	{
		// timeout = abp_sim->get_tc();
		// timeout += (double)((header_length + pckt_length)*8.0) / (double)transfer_rate;
		// timeout += delta;	

		// ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0));			

		//fc, receiever, rc
		Event * ack_event = abp_sim->send();
		if(ack_event != NULL)
		{
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

			//update current_time = timeout:: CHECK THIS
			abp_sim->update_tc(event->get_time_stamp());

			abp_sim->sender();

			timeout = abp_sim->get_tc();
			timeout += (double)((header_length + pckt_length)*8.0) / (double)transfer_rate;
			timeout += delta;	

			ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0));			

			itr++;
			continue;
		}
		else	//ACK EVENT
		{
			//no data or ack frame errors
			if(event->get_error_flag() == 0 && abp_sim->update_state(event->get_sn()) == 1)
			{
				// cout << "ack event..." << endl;
				succ_pckt_ctr++;
				ES->clear_ES();
				//update states (incl. current time)	
				abp_sim->update_tc(event->get_time_stamp());
				abp_sim->sender();

				timeout = abp_sim->get_tc();
				timeout += (double)((header_length + pckt_length)*8.0) / (double)transfer_rate;
				timeout += delta;	

				ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0));			

				itr++;
				continue;
			}
			else
			{
				// cout << "nak event..." << endl;
				//retransmit pckt right away
				ES->clear_ES();
				abp_sim->update_tc(event->get_time_stamp());
				abp_sim->sender();

				timeout = abp_sim->get_tc();
				timeout += (double)((header_length + pckt_length)*8.0) / (double)transfer_rate;
				timeout += delta;	

				ES->register_event(new Event(0, timeout, abp_sim->get_sn(), 0));			

				itr++;
				continue;
			}
		}

		// itr++;
	}

	cout << endl;
	cout << "ABP_NAK" << endl;
	cout << "2Tao: " << 2.0 * tao << " ms" << endl;
	cout << "ber: " << ber << endl;
	// cout << "ITERATION " << itr << endl;
	cout << "successful packet count: " << succ_pckt_ctr << endl;
	cout << "tc: " << abp_sim->get_tc() << " s" << endl;
	// cout << "throughput: " << (double)succ_pckt_ctr / abp_sim->get_tc() << " pckts/s" << endl;
	cout << "throughput: " << (double)(succ_pckt_ctr*pckt_length*8.0) / abp_sim->get_tc() << " bits/s" << endl;
	

}

int main()
{
	for(double i = 2.5; i <= 12.5; i += 2.5)
	{
		cout << endl;
		cout << "***EXPERIMENT***" << endl;
		cout << "itr: " << i << endl;
		delta = (double)(i * tao);
		run_sim();
	}

	return 0;
}

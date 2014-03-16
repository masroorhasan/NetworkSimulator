#include <stdlib.h>
#include <stdio.h>

#include "event.cpp"
#include "event_scheduler.cpp"
#include "gbn_simulator.cpp"

using namespace std;

	double tao = 0.005;
		// double tao = 0.25;
	int successful_pckts_num = 10000;

	//sender side input params
	int pckt_header = 54;				//bytes
	int pckt_length = 1500;				//bytes
	double delta = 12.5 * tao;
	// double delta = 0.0;

	//channel params
	double transfer_rate = 5000000;		//C (bps)
	double prop_delay = tao;			//Tao
	// double ber = 0.00000;				//BER = 0
	double ber = 0.00001;				//BER = 1e-5
	// double ber = 0.00010;				//BER = 1e-4

	int window_size = 4;				//N

void run_sim()
{
	GBN_Simulator *gbn_sim = new GBN_Simulator(pckt_header, pckt_length, transfer_rate,
												prop_delay, ber, window_size);
	EventScheduler *ES = new EventScheduler();


	double transfer_time = ((double)((pckt_header + pckt_length)*8.0) / (double)transfer_rate);
	double tc = 0.0;

	int ctr = 0;			//buffer location of frame to be transmitted
	int succ_pckt_ctr = 0;	//num of successful pckts transmitted
	
	while(ctr < window_size)
	{
		cout << "sending pckt: " << ctr << endl;
		//update transfer time for each pckt
		//update global clk
		gbn_sim->update_pckt_T(ctr, transfer_time);
		// gbn_sim->update_nea(ctr);

		if(ctr == 0)
		{
			ES->purge_TO_event();
			double timeout = gbn_sim->get_Ttc(0);
			timeout += delta;
			ES->register_event(new Event(0, timeout, -1, 0));
		}

		double new_tc = gbn_sim->get_tc();
		new_tc += transfer_time;
		gbn_sim->update_tc(new_tc);

		Event *ack = gbn_sim->send(ctr);

		if(ack != NULL)
			ES->register_event(ack);

		Event *event = ES->read_ES();

		// cout << "even timestamp: " << event->get_time_stamp() << endl;
		// cout << "global clk: " << gbn_sim->get_tc() << endl;

		ES->print_ES();
		
		if(ctr == window_size-1)
		{
			//while check nak and mismatched rn
			cout << "last pckt" << endl;
			gbn_sim->update_tc(event->get_time_stamp());

			while(event->get_event_type() == 1 && (event->get_error_flag() == 1
				|| gbn_sim->check_expected_acks(event->get_sn()) == false))
			{
				cout << "ignoring NAKs" << endl;
				//check for empty ES
				
				ES->get_front_event();
				event = ES->read_ES();	

				if(ES->is_ES_empty())
					break;
			}

			if(!event)
			{
				ctr = 0;
				continue;
			}

			if(event->get_event_type() == 0) 	//TIMEOUT
			{
				cout << "Processing TIMEOUT" << endl;
				ES->purge_TO_event();

				//update T[i]'s, timeout and register new TO event
				gbn_sim->update_tc(event->get_time_stamp());
				double timeout = gbn_sim->get_Ttc(0);
				timeout += gbn_sim->get_tc();			//tc + t[0]
				timeout += delta;
				ES->register_event(new Event(0, timeout, -1, 0));

				ctr = 0;
				ES->get_front_event();
			}
			else
			{
				if(event->get_error_flag() == 0 && gbn_sim->check_expected_acks(event->get_sn()) == true)
				{
					//ack
					cout << "Processing ACK of rn = " << event->get_sn() << endl;
					ctr = gbn_sim->update_window(ctr, event->get_sn());
					succ_pckt_ctr = gbn_sim->shift_size();
					//set new timeout
					
					ES->purge_TO_event();
					double timeout = gbn_sim->get_Ttc(0);
					timeout += delta;
					ES->register_event(new Event(0, timeout, -1, 0));
					// cout << "new timeout: " << timeout << endl;

					ES->get_front_event();		
				}
				else
				{
					// cout << "ign NAK" << endl;
					cout << "shouldnt be here" << endl;
					// ES->get_front_event();
				}
			}

			if(succ_pckt_ctr >= successful_pckts_num)
				break;

			continue;
		}

		if(event->get_time_stamp() <= gbn_sim->get_tc())
		{
			if(event->get_event_type() == 0) 	//TIMEOUT
			{
				cout << "Processing TIMEOUT" << endl;
				ES->purge_TO_event();

				//update T[i]'s
				gbn_sim->update_tc(event->get_time_stamp());
				double timeout = gbn_sim->get_Ttc(0);
				timeout += gbn_sim->get_tc();			//tc + t[0]
				timeout += delta;
				ES->register_event(new Event(0, timeout, -1, 0));

				ctr = 0;
				ES->get_front_event();

				continue;
			}
			else
			{
				if(event->get_error_flag() == 0 && gbn_sim->check_expected_acks(event->get_sn()) == true)
				{
					//ack
					cout << "Processing ACK of rn = " << event->get_sn() << endl;
					ctr = gbn_sim->update_window(ctr, event->get_sn());
					succ_pckt_ctr = gbn_sim->shift_size();
					//set new timeout
					ES->purge_TO_event();
					
					double timeout = gbn_sim->get_Ttc(0);
					timeout += delta;
					ES->register_event(new Event(0, timeout, -1, 0));

					ES->get_front_event();		

					continue;
				}
				else
				{
					cout << "Ignoring NAK" << endl;
					ES->get_front_event();
				}
			}

			if(succ_pckt_ctr >= successful_pckts_num)
				break;

			// continue;
		}

		ctr++;
	}
	
	cout << endl;
	cout << endl;
	cout << endl;
	cout << "GBN" << endl;
	cout << "2Tao: " << 2.0 * tao << " s" << endl;
	cout << "ber: " << ber << endl;
	// cout << "ITERATION " << itr << endl;
	cout << "successful packet count: " << succ_pckt_ctr << endl;
	cout << "tc: " << gbn_sim->get_tc() << " s" << endl;
	// cout << "throughput: " << (double)succ_pckt_ctr / abp_sim->get_tc() << " pckts/s" << endl;
	cout << "throughput: " << (double)(succ_pckt_ctr*pckt_length*8.0) / gbn_sim->get_tc() << " bits/s" << endl;
}

int main()
{
	// double timeout = 0.0;

	// for(double i = 2.5; i <= 12.5; i += 2.5)
	{
		cout << endl;
		cout << "***EXPERIMENT***" << endl;
		// cout << "itr: " << i << endl;
		run_sim();
		// cout << "itr: " << i << endl;
	}

    return 0;
}

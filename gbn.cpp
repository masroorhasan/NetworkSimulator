#include <stdlib.h>
#include <stdio.h>

#include "event.cpp"
#include "event_scheduler.cpp"
#include "gbn_simulator.cpp"

using namespace std;

int main()
{
	double tao = 0.005;
	// double tao = 0.25;
	int successful_pckts_num = 10;

	//sender side input params
	int pckt_header = 54;				//bytes
	int pckt_length = 1500;				//bytes
	double delta = 2.5 * tao;

	//channel params
	double transfer_rate = 5000000;		//C (bps)
	double prop_delay = tao;			//Tao
	double ber = 0.00000;				//BER

	int window_size = 4;				//N

	double timeout = 0.0;

	GBN_Simulator *gbn_sim = new GBN_Simulator(pckt_header, pckt_length, transfer_rate,
												prop_delay, ber, window_size);
	EventScheduler *ES = new EventScheduler();


	double transfer_time = ((double)((pckt_header + pckt_length)*8.0) / (double)transfer_rate);
	double tc = 0.0;

	int ctr = 0;			//buffer location of frame to be transmitted
	int succ_pckt_ctr = 0;	//num of successful pckts transmitted

	while(ctr < (window_size))
	{
		cout << "ctr: " << ctr << endl;
		tc += transfer_time;
		//insert tc at T
		gbn_sim->update_pckt_T(ctr,tc);
		//insert SN[counter-1]+1 to next_expected_ack
		gbn_sim->update_nea(ctr);
		//if ctr = 0, insert new timeout event
		timeout = gbn_sim->get_Ttc(0);
		timeout += delta;
		if(ctr == 0)
			ES->register_event(new Event(0, timeout, -1, 0));

		Event *ack_event = gbn_sim->send(ctr);
		
		if(ack_event != NULL)
			ES->register_event(ack_event);

		Event *event = ES->read_ES();

		ES->print_ES();

		// if(ctr == window_size-1)
		// {
		// 	cout << "popping off ES" << endl;
		// 	Event *event_popped = ES->get_front_event();
		// 	while(event_popped->get_event_type() == 1 &&
		//  				event_popped->get_error_flag() == 1)
		// 	{
		// 		event_popped = ES->get_front_event();
		// 		event = event_popped;
		// 	}
		// }

		tc = event->get_time_stamp();
		if(event->get_time_stamp() > gbn_sim->get_Ttc(ctr))
		{
			//event occured after transfer time
			cout << "do nothing" << endl;
			ctr++;
		}

		if(event->get_time_stamp() < gbn_sim->get_Ttc(ctr))
		{
			if(event->get_event_type() == 0) 	//TIMEOUT
			{
				cout << "PROCESSING TIMEOUT" << endl;
				ES->purge_TO_event();
				
				timeout = gbn_sim->get_Ttc(0);
				timeout += delta;	
				ES->register_event(new Event(0, timeout, -1, 0));	

				ctr = 0;
				continue;
			}
			else
			{
				if(event->get_error_flag() == 0 && gbn_sim->check_expected_acks(event->get_sn()) == true)
				{
					cout << "PROCESSING ACK EVENT" << endl;
					
					// ES->get_front_event();
					tc = event->get_time_stamp();
					ctr = gbn_sim->update_window(ctr, event->get_sn());

					ES->purge_TO_event();
					
					timeout = gbn_sim->get_Ttc(0);
					timeout += delta;
					ES->register_event(new Event(0, timeout, -1, 0));												
					
					// update ctr
					// ctr -= shift_size%(window_size+1);
					// ctr++;
					// gbn_sim->update_buffer(ctr, shift_size);
					ES->get_front_event();
					succ_pckt_ctr++;
				}
				else
				{
					//nak
					cout << "PROCESSING NAK EVENT" << endl;
				}
			}
		}

		// ctr++;
		// gbn_sim->update_buffer(ctr,0);

		if(succ_pckt_ctr == successful_pckts_num)
			break;			
	}

	cout << endl;
	cout << "GBN" << endl;
	cout << "2Tao: " << 2.0 * tao << " s" << endl;
	cout << "ber: " << ber << endl;
	// cout << "ITERATION " << itr << endl;
	cout << "successful packet count: " << succ_pckt_ctr << endl;
	cout << "tc: " << tc << " s" << endl;
	// cout << "throughput: " << (double)succ_pckt_ctr / abp_sim->get_tc() << " pckts/s" << endl;
	cout << "throughput: " << (double)(succ_pckt_ctr*pckt_length*8.0) / tc << " bits/s" << endl;

    return 0;
}

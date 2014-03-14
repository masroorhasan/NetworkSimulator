#include <stdlib.h>
#include <stdio.h>

#include "event.cpp"
#include "event_scheduler.cpp"
#include "gbn_simulator.cpp"

using namespace std;

int main()
{
	double tao = 0.005;
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


	//initialize sender
		//set tc = 0 and counter = 1
		//generate pckts and populate buffer
		//

	// gbn_sim->initiate_send();

	// timeout = gbn_sim->get_t1c();
	// timeout += delta;

	// ES->register_event(new Event(0, timeout, gbn_sim->get_s1n(), 0));
	double total_time = 0.0;
	double batch_tc = 0.0;
	// batch_tc += ((double)((pckt_header + pckt_length)*8.0) / (double)transfer_rate);
	// gbn_sim->update_pckt_T(0, batch_tc);
	// gbn_sim->update_nea(0);

	int ctr = 0;			//buffer location of frame to be transmitted
	int succ_pckt_ctr = 0;	//num of successful pckts transmitted
	while(succ_pckt_ctr < successful_pckts_num)
	{
		while(ctr < (window_size))
		{
			cout << endl;
			cout << "ctr: " << ctr << endl;
			batch_tc += ((double)((pckt_header + pckt_length)*8.0) / (double)transfer_rate);
			
			gbn_sim->update_pckt_T(ctr, batch_tc);
			gbn_sim->update_nea(ctr);

			timeout = gbn_sim->get_Ttc(ctr);
			timeout += delta;

			if(ctr == 0)
			{
				cout << "registering new TO event" << endl;
				ES->register_event(new Event(0, timeout, gbn_sim->get_s1n(), 0));
			}

			Event *ack_event = gbn_sim->send();
			if(ack_event != NULL)
				ES->register_event(ack_event);

			ES->print_ES();

			Event *event = ES->read_ES();
			// cout << endl;
			// cout << "T[" << ctr << "]: " << gbn_sim->get_Ttc(ctr) << endl;
			cout << "event time stamp: " << event->get_time_stamp() << endl;

			// update tc
			batch_tc += event->get_time_stamp();

			if(event->get_time_stamp() > gbn_sim->get_Ttc(ctr))
			{
				cout << "do nothing" << endl;
				// ctr++;
				// continue;
			}
			// else
			if(event->get_time_stamp() < gbn_sim->get_Ttc(ctr))
			{
				if(event->get_event_type() == 0) 	//timeout event
				{
					cout << "Processing TIMEOUT event" << endl;
					// ES->get_front_event(); 	//pop
					//purge old timeout
					//set counter = 1
					//restart transmission
					ES->purge_TO_event();
					ctr = 1;

					//updating current time
					batch_tc += event->get_time_stamp();


					cout << "POPPING OFF ES" << endl;
					ES->get_front_event();
					ES->print_ES();
					cout << endl;
					continue;
				}
				else
				{
					
					if(event->get_error_flag() == 0 && gbn_sim->check_expected_acks(event->get_sn()) == true)
					{
						cout << "Processing ACK event" << endl;
						
						gbn_sim->update_window(event->get_sn());	//shifting
						gbn_sim->update_buffer(ctr);

						gbn_sim->update_pckt_T(ctr,((double)((pckt_header + pckt_length)*8.0) / (double)transfer_rate));
						ES->purge_TO_event();

						cout << "PURGING TIMEOUT EVENT" << endl;

						timeout = gbn_sim->get_Ttc(0);
						timeout += delta;

						cout << "registering new TO event" << endl;
						ES->register_event(new Event(0, timeout, gbn_sim->get_s1n(), 0));
						int shift_size = (event->get_sn() - gbn_sim->get_s1n()) % (window_size+1);
						shift_size = shift_size < 0 ? shift_size * -1 : shift_size;
						ctr -= shift_size;
						ctr++;

						cout << "updating ctr to value: " << ctr << endl;
						

						//updating current time
						batch_tc += event->get_time_stamp();
						cout << "updating current time to: " << batch_tc << endl;
						// gbn_sim->update_pckt_T(ctr, batch_tc);

						cout << endl;
						cout << endl;

						succ_pckt_ctr++;


						cout << "POPPING OFF ES" << endl;
						ES->get_front_event();
						ES->print_ES();
						cout << endl;
						continue;
					} 
					else
					{
						//nak
						// ES->get_front_event(); 	//pop
						batch_tc += event->get_time_stamp();

						cout << "POPPING OFF ES" << endl;
						ES->get_front_event();
						ES->print_ES();
						cout << endl;
						// continue;
					}
				}
			}

			ctr++;
			gbn_sim->update_buffer(ctr);	//increment sn
		}


		// ES->print_ES();
		// //start dequeing from ES
		// Event * ev = ES->get_front_event();	//pop event

		// cout << "DEQUEING off ES " << endl;

		// if(ev->get_event_type() == 0)
		// {
		// 	cout << "TIMEOUT" << endl;
		// 	ES->purge_TO_event();
		// 	cout << "PURGING TIMEOUT EVENT" << endl;
		// 	ctr = 0;
		// 	continue;	
		// }
 	// 	else
 	// 	{
 	// 		if(ev->get_error_flag() == 0 && gbn_sim->check_expected_acks(ev->get_sn()) == true)
 	// 		{
 	// 			cout << "ACK EVENT" << endl;
 	// 			gbn_sim->update_window(ev->get_sn());	//shifting
		// 		ES->purge_TO_event();
		// 		cout << "PURGING TIMEOUT EVENT" << endl;
		// 		int shift_size = (ev->get_sn() - gbn_sim->get_s1n()) % (window_size+1);
		// 		shift_size = shift_size < 0 ? shift_size * -1 : shift_size;
		// 		ctr -= shift_size;

		// 		gbn_sim->update_buffer(ctr);

		// 		succ_pckt_ctr++;

		// 		continue;
 	// 		}
 	// 		else
 	// 		{
 	// 			//nak
 	// 			//do nothing
 	// 			cout << "NAK EVENT" << endl;
 	// 		}
 	// 	}
	}

	cout << endl;
	cout << "GBN" << endl;
	cout << "2Tao: " << 2.0 * tao << " s" << endl;
	cout << "ber: " << ber << endl;
	// cout << "ITERATION " << itr << endl;
	cout << "successful packet count: " << succ_pckt_ctr << endl;
	cout << "tc: " << total_time << " s" << endl;
	// cout << "throughput: " << (double)succ_pckt_ctr / abp_sim->get_tc() << " pckts/s" << endl;
	cout << "throughput: " << (double)(succ_pckt_ctr*pckt_length*8.0) / total_time << " bits/s" << endl;

    return 0;
}

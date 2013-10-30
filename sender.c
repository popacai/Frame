#include "sender.h"
#include "chksum.h"

void print_frame(Frame* frame)
{
    fprintf(stderr, "\nframe:\n");
//    fprintf(stderr, "frame->src=%d\n", frame->src);
//    fprintf(stderr, "frame->dst=%d\n", frame->dst);
//    fprintf(stderr, "frame->checksum=%d\n", frame->checksum);
    fprintf(stderr, "frame->seq=%d\n", frame->seq);
    fprintf(stderr, "frame->ack=%d\n", frame->ack);
    fprintf(stderr, "frame->flag=%d\n", frame->flag);
    fprintf(stderr, "frame->size=%d\n", frame->size);
    fprintf(stderr, "frame->data=%s\n", frame->data);
    fprintf(stderr, "#frame--------\n");
}

void print_sender(Sender* sender)
{
    fprintf(stderr, "sender:\n");
    fprintf(stderr, "sender->LAR=%d\n", sender->LAR);
    fprintf(stderr, "sender->LFS=%d\n", sender->LFS);
    fprintf(stderr, "sender->SWS=%d\n", sender->SWS);
    fprintf(stderr, "sender->full=%d\n", sender->send_full);
    fprintf(stderr, "sender->fin=%d\n", sender->fin);
}
void init_sender(Sender * sender, int id)
{
    //TODO: You should fill in this function as necessary
    sender->send_id = id;
    sender->recv_id = -1;
    sender->input_cmdlist_head = NULL;
    sender->input_framelist_head = NULL;
    
    sender->pending_head = NULL;


    sender->LFS = 0;
    sender->LAR = 0;
    sender->SWS = 8;
    sender->fin = 1;

    sender->buffer = (struct Frame**) malloc(8 * sizeof(Frame*));
    sender->timestamp = malloc(8 * sizeof(struct timeval));

    int i;
    struct timeval init_time;
    init_time.tv_sec = 0;

    for (i = 0; i < 8; i++)
    {
	sender->buffer[i] = malloc(1);
	sender->timestamp[i] = init_time;
    }

}

struct timeval * sender_get_next_expiring_timeval(Sender * sender)
{
    //TODO: You should fill in this function so that it returns the next timeout that should occur
    return NULL;
}


int recv_ack(Sender* sender, Frame* frame)
{
    if ((frame->ack > sender->LAR )
    	&& (frame->ack <= sender->LFS))
    {
	sender->LAR = frame->ack;
	fprintf(stderr,"sender_ack:%d\n",sender->LAR);
    }
    return 0;
}
void handle_incoming_acks(Sender * sender,
                          LLnode ** outgoing_frames_head_ptr)
{
    int incoming_msgs_length = ll_get_length(sender->input_framelist_head);
    while (incoming_msgs_length > 0)
    {
        LLnode * ll_inmsg_node = ll_pop_node(&sender->input_framelist_head);
        incoming_msgs_length = ll_get_length(sender->input_framelist_head);
        char * raw_char_buf = (char *) ll_inmsg_node->value;
        Frame * inframe = convert_char_to_frame(raw_char_buf);
	short checksum;

	checksum = chksum((unsigned short*) raw_char_buf, 
			    MAX_FRAME_SIZE / 2);
	if (checksum)
	{
	    fprintf(stderr, "send checksum error\n");
	    free(raw_char_buf);
	    continue;
	}

	if (sender->send_id == inframe->dst)
	{
	    if (inframe->flag == ACK)
	    {
		recv_ack(sender, inframe);
	    }
	}

	free(inframe);
	free(raw_char_buf);
    }

}

Frame* build_frame(Sender* sender, char* message)
{

    Frame* frame;
    frame = (Frame*) malloc(sizeof(Frame));

    frame->dst = sender->recv_id;
    frame->src = sender->send_id;

    frame->seq = ++(sender->LFS);
    frame->ack = sender->LAR;
    frame->flag = SEND;
    frame->size = strlen(message);
    strcpy(frame->data, message);

    return frame;

}
void handle_pending(Sender * sender,
                       LLnode ** outgoing_frames_head_ptr)
{

    while ((sender->LFS - sender->LAR) < sender->SWS)
    {
	int pending_length = ll_get_length(sender->pending_head);
	//fprintf(stderr, "pending_length=%d\n", pending_length);
	if (!pending_length)
	{
	    sender->fin = 1;
	    break;
	}
	sender->fin = 0;
	LLnode * ll_message = ll_pop_node(&(sender->pending_head));
        char * message = (char *) ll_message->value;

	Frame* outframe;
	outframe = build_frame(sender, message);
	print_frame(outframe);

	//fprintf(stderr, "message=%s\n",message);
	
	//copy to buffer
	unsigned char pos;
	pos = outframe->seq % sender->SWS;
	free(sender->buffer[pos]);
	*(sender->buffer + pos) = (struct Frame*)outframe;
	
	//set time
	struct timeval now;
	gettimeofday(&now, NULL);
	sender->timestamp[pos] = now;

	//output
	char* buf;
	buf = add_chksum(outframe);
	
	ll_append_node(outgoing_frames_head_ptr, buf);
	free(message);
    }
}

void handle_input_cmds(Sender * sender,
                       LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling input cmd
    //    1) Dequeue the Cmd from sender->input_cmdlist_head
    //    2) Convert to Frame
    //    3) Set up the frame according to the sliding window protocol
    //    4) Compute CRC and add CRC to Frame

    int input_cmd_length = ll_get_length(sender->input_cmdlist_head);
    
        
    //Recheck the command queue length to see if stdin_thread dumped a command on us
    input_cmd_length = ll_get_length(sender->input_cmdlist_head);
    while (input_cmd_length > 0)
    {
        //Pop a node off and update the input_cmd_length
        LLnode * ll_input_cmd_node = ll_pop_node(&sender->input_cmdlist_head);
        input_cmd_length = ll_get_length(sender->input_cmdlist_head);

        //Cast to Cmd type and free up the memory for the node
        Cmd * outgoing_cmd = (Cmd *) ll_input_cmd_node->value;
        free(ll_input_cmd_node);
            

        //DUMMY CODE: Add the raw char buf to the outgoing_frames list
        //NOTE: You should not blindly send this message out!
        //      Ask yourself: Is this message actually going to the right receiver (recall that default behavior of send is to broadcast to all receivers)?
        //                    Does the receiver have enough space in in it's input queue to handle this message?
        //                    Were the previous messages sent to this receiver ACTUALLY delivered to the receiver?
	sender->recv_id = outgoing_cmd->dst_id;
        int msg_length = strlen(outgoing_cmd->message);
        if (msg_length)
        {
            //At this point, we don't need the outgoing_cmd
            free(outgoing_cmd);
	    ll_append_node(&(sender->pending_head), outgoing_cmd->message);
        }
    }   
}


void handle_timedout_frames(Sender * sender,
                            LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling timed out datagrams
    //    1) Iterate through the sliding window protocol information you maintain for each receiver
    //    2) Locate frames that are timed out and add them to the outgoing frames
    //    3) Update the next timeout field on the outgoing frames
    struct timeval now;
    struct timeval tmp;
    long long interval;
    //find the timeout and send out
    int pos;
    int seq;
    gettimeofday(&now, NULL);
    for (seq = (sender->LAR + 1); seq <= sender->LFS; seq++)
    {
	//seq = sender->LAR + 1;
	pos = seq % sender->SWS;

	tmp = sender->timestamp[pos];
	
	if (tmp.tv_sec == 0)
	{
	    continue;
	}
	interval = (now.tv_sec - tmp.tv_sec) * 1000000
		   + (now.tv_usec - tmp.tv_usec);

	//if (interval < 100000)
	if (interval < 10000)
	    return;
	fprintf(stderr, "sender:timeout!seq=%d\n",seq);
	fprintf(stderr, "sender,now=%ld:%ld\n", now.tv_sec, now.tv_usec);
	fprintf(stderr, "sender,tmp=%ld:%ld\n", tmp.tv_sec, tmp.tv_usec);
	
	Frame* outgoing_frame;
	outgoing_frame = (Frame*)sender->buffer[pos];
	sender->timestamp[pos] = now;

	//char * buf = convert_frame_to_char(outgoing_frame);

	//outgoing_frame->checksum = chksum((unsigned short*) buf, 
	//			    MAX_FRAME_SIZE / 2);
	//buf = convert_frame_to_char(outgoing_frame);
	char* buf = add_chksum(outgoing_frame);
	char* outgoing_charbuf = buf;
	    
	print_frame(outgoing_frame);
	ll_append_node(outgoing_frames_head_ptr,
		       outgoing_charbuf);
    }

}


void * run_sender(void * input_sender)
{    
    struct timespec   time_spec;
    struct timeval    curr_timeval;
    const int WAIT_SEC_TIME = 0;
    const long WAIT_USEC_TIME = 100000;
    Sender * sender = (Sender *) input_sender;    
    LLnode * outgoing_frames_head;
    struct timeval * expiring_timeval;
    long sleep_usec_time, sleep_sec_time;
    
    //This incomplete sender thread, at a high level, loops as follows:
    //1. Determine the next time the thread should wake up
    //2. Grab the mutex protecting the input_cmd/inframe queues
    //3. Dequeues messages from the input queue and adds them to the outgoing_frames list
    //4. Releases the lock
    //5. Sends out the messages

    pthread_cond_init(&sender->buffer_cv, NULL);
    pthread_mutex_init(&sender->buffer_mutex, NULL);

    while(1)
    {    
        outgoing_frames_head = NULL;

        //Get the current time
        gettimeofday(&curr_timeval, 
                     NULL);

        //time_spec is a data structure used to specify when the thread should wake up
        //The time is specified as an ABSOLUTE (meaning, conceptually, you specify 9/23/2010 @ 1pm, wakeup)
        time_spec.tv_sec  = curr_timeval.tv_sec;
        time_spec.tv_nsec = curr_timeval.tv_usec * 1000;

        //Check for the next event we should handle
        expiring_timeval = sender_get_next_expiring_timeval(sender);

        //Perform full on timeout
        if (expiring_timeval == NULL)
        {
            time_spec.tv_sec += WAIT_SEC_TIME;
            time_spec.tv_nsec += WAIT_USEC_TIME * 1000;
        }
        else
        {
            //Take the difference between the next event and the current time
            sleep_usec_time = timeval_usecdiff(&curr_timeval,
                                               expiring_timeval);

            //Sleep if the difference is positive
            if (sleep_usec_time > 0)
            {
                sleep_sec_time = sleep_usec_time/1000000;
                sleep_usec_time = sleep_usec_time % 1000000;   
                time_spec.tv_sec += sleep_sec_time;
                time_spec.tv_nsec += sleep_usec_time*1000;
            }   
        }

        //Check to make sure we didn't "overflow" the nanosecond field
        if (time_spec.tv_nsec >= 1000000000)
        {
            time_spec.tv_sec++;
            time_spec.tv_nsec -= 1000000000;
        }

        
        //*****************************************************************************************
        //NOTE: Anything that involves dequeing from the input frames or input commands should go 
        //      between the mutex lock and unlock, because other threads CAN/WILL access these structures
        //*****************************************************************************************
        pthread_mutex_lock(&sender->buffer_mutex);

        //Check whether anything has arrived
        int input_cmd_length = ll_get_length(sender->input_cmdlist_head);
        int inframe_queue_length = ll_get_length(sender->input_framelist_head);
        
        //Nothing (cmd nor incoming frame) has arrived, so do a timed wait on the sender's condition variable (releases lock)
        //A signal on the condition variable will wakeup the thread and reaquire the lock
        if (input_cmd_length == 0 &&
            inframe_queue_length == 0)
        {
            
            pthread_cond_timedwait(&sender->buffer_cv, 
                                   &sender->buffer_mutex,
                                   &time_spec);
        }
        //Implement this
        handle_incoming_acks(sender,
                             &outgoing_frames_head);

        //Implement this
        handle_input_cmds(sender,
                          &outgoing_frames_head);
	handle_pending(sender,
                          &outgoing_frames_head);

        pthread_mutex_unlock(&sender->buffer_mutex);


        //Implement this
	//Cancel temportoly
        handle_timedout_frames(sender,
                               &outgoing_frames_head);


        //CHANGE THIS AT YOUR OWN RISK!
        //Send out all the frames
        int ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        
        while(ll_outgoing_frame_length > 0)
        {
            LLnode * ll_outframe_node = ll_pop_node(&outgoing_frames_head);
            char * char_buf = (char *)  ll_outframe_node->value;

            //Don't worry about freeing the char_buf, the following function does that
            send_msg_to_receivers(char_buf);

            //Free up the ll_outframe_node
            free(ll_outframe_node);

            ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        }
    }
    pthread_exit(NULL);
    return 0;
}

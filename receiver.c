#include "receiver.h"
#include "chksum.h"

void print_f(Frame* frame)
{
    fprintf(stderr, "\nframe:\n");
//    fprintf(stderr, "frame->src=%d\n", frame->src);
//    fprintf(stderr, "frame->dst=%d\n", frame->dst);
//    fprintf(stderr, "frame->checksum=%d\n", frame->checksum);
    fprintf(stderr, "frame->seq=%d\n", frame->seq);
    fprintf(stderr, "frame->ack=%d\n", frame->ack);
    fprintf(stderr, "frame->flag=%d\n", frame->flag);
//    fprintf(stderr, "frame->window_size=%d\n", frame->window_size);
    fprintf(stderr, "frame->data=%s\n", frame->data);
    fprintf(stderr, "#frame--------\n");
}

void print_receiver(Receiver * receiver)
{
    fprintf(stderr, "receiver:\n");
    fprintf(stderr, "receiver->LFR=%d\n", receiver->LFR);
    fprintf(stderr, "receiver->LAF=%d\n", receiver->LAF);
    fprintf(stderr, "receiver->RWS=%d\n", receiver->RWS);
    fprintf(stderr, "receiver->fin=%d\n", receiver->fin);
}
void init_receiver(Receiver * receiver,
                   int id)
{
    receiver->recv_id = id;
    receiver->input_framelist_head = NULL;

    receiver->RWS = 8;
    receiver->LFR = 0;
    receiver->LAF = receiver->LFR + receiver->RWS;
    receiver->fin = 0;

    receiver->buffer = malloc(8 * sizeof(Frame*));
    int i; 
    Frame* frame;
    for (i = 0; i < receiver->RWS; i++)
    {
	frame = (Frame*) malloc(sizeof(Frame));
	frame->seq = -1;
	frame->ack = -1;
	receiver->buffer[i] = (struct Frame*)frame;
    }
}

Frame* build_ack(Receiver * receiver,
      			  Frame* inframe)
{
    Frame* outframe;
    outframe = (Frame*) malloc(sizeof(Frame));
    //dst
    outframe->src = inframe->dst;
    outframe->dst = inframe->src;
    outframe->flag = ACK;
    outframe->seq = inframe->seq;
    //outframe->ack = inframe->seq;
    outframe->ack = receiver->LFR;

    if (!(inframe->seq > (receiver->LFR % MAX_SEQ)
	&& inframe->seq <= (receiver->LAF % MAX_SEQ)))
    {
	fprintf(stderr, "error windows\n");
	print_f(inframe);
	print_receiver(receiver);
	return outframe;
    }

    int pos;
    pos = inframe->seq % receiver->RWS;

    free(receiver->buffer[pos]); // Free the pervious one
    receiver->buffer[pos] = (struct Frame*)inframe;

    unsigned char iseq; // temp seq;  [LAR .. tseq]
    unsigned char ipos;
    int all_recv = 1;
    Frame* tmp;
    //for (iseq = inframe->seq; (iseq > receiver->LFR) && iseq >= 0; iseq--)
    for (iseq = (receiver->LFR + 1); iseq != (inframe->seq + 1); iseq++)
    {
	ipos = iseq % receiver->RWS;
	tmp = (Frame*)receiver->buffer[ipos];
	if (iseq == tmp->seq)
	{
	    continue;
	}
	else
	{
	    all_recv = 0;
	    break;
	}
    }
    //update the LFR&LAR if all_recv
    if (all_recv)
    {
	//copy_buffer(receiver, inframe->seq);
	for (iseq = (receiver->LFR + 1); iseq != (inframe->seq + 1); iseq++)
	{
	    ipos = iseq % receiver->RWS;
	    tmp = (Frame*)receiver->buffer[ipos];
	    printf("<RECV_%d>:[%s]\n", receiver->recv_id, tmp->data);
	}
	receiver->LFR = inframe->seq;
	receiver->LAF = receiver->LFR + receiver->RWS;
    }

    outframe->ack = receiver->LFR;

    return outframe;

}

void handle_incoming_msgs(Receiver * receiver,
                          LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling incoming frames
    //    1) Dequeue the Frame from the sender->input_framelist_head
    //    2) Convert the char * buffer to a Frame data type
    //    3) Check whether the frame is corrupted
    //    4) Check whether the frame is for this receiver
    //    5) Do sliding window protocol for sender/receiver pair

    int incoming_msgs_length = ll_get_length(receiver->input_framelist_head);
    while (incoming_msgs_length > 0)
    {
        //Pop a node off the front of the link list and update the count
        LLnode * ll_inmsg_node = ll_pop_node(&receiver->input_framelist_head);
        incoming_msgs_length = ll_get_length(receiver->input_framelist_head);

        //DUMMY CODE: Print the raw_char_buf
        //NOTE: You should not blindly print messages!
        //      Ask yourself: Is this message really for me?
        //                    Is this message corrupted?
        //                    Is this an old, retransmitted message?           
        char * raw_char_buf = (char *) ll_inmsg_node->value;
        
	if (chksum_all(raw_char_buf))
	{
	    fprintf(stderr, "chksum error\n");
	    continue;
	}
        //Free raw_char_buf
        Frame * inframe = convert_char_to_frame(raw_char_buf);
	if (inframe->dst == receiver->recv_id)
	{
	    Frame * outframe;
	    char* buf;

	    outframe = build_ack(receiver, inframe);

	    //buf = convert_frame_to_char(outframe);
	    buf = add_chksum(outframe);
	    ll_append_node(outgoing_frames_head_ptr, buf);
	    //printf("<RECV_%d>:[%s]\n", receiver->recv_id, inframe->data);
	}


	//free(inframe);
	free(raw_char_buf);
        free(ll_inmsg_node);
    }
}

void * run_receiver(void * input_receiver)
{    
    struct timespec   time_spec;
    struct timeval    curr_timeval;
    const int WAIT_SEC_TIME = 0;
    const long WAIT_USEC_TIME = 100000;
    Receiver * receiver = (Receiver *) input_receiver;
    LLnode * outgoing_frames_head;


    //This incomplete receiver thread, at a high level, loops as follows:
    //1. Determine the next time the thread should wake up if there is nothing in the incoming queue(s)
    //2. Grab the mutex protecting the input_msg queue
    //3. Dequeues messages from the input_msg queue and prints them
    //4. Releases the lock
    //5. Sends out any outgoing messages

    pthread_cond_init(&receiver->buffer_cv, NULL);
    pthread_mutex_init(&receiver->buffer_mutex, NULL);

    while(1)
    {    
        //NOTE: Add outgoing messages to the outgoing_frames_head pointer
        outgoing_frames_head = NULL;
        gettimeofday(&curr_timeval, 
                     NULL);

        //Either timeout or get woken up because you've received a datagram
        //NOTE: You don't really need to do anything here, but it might be useful for debugging purposes to have the receivers periodically wakeup and print info
        time_spec.tv_sec  = curr_timeval.tv_sec;
        time_spec.tv_nsec = curr_timeval.tv_usec * 1000;
        time_spec.tv_sec += WAIT_SEC_TIME;
        time_spec.tv_nsec += WAIT_USEC_TIME * 1000;
        if (time_spec.tv_nsec >= 1000000000)
        {
            time_spec.tv_sec++;
            time_spec.tv_nsec -= 1000000000;
        }

        //*****************************************************************************************
        //NOTE: Anything that involves dequeing from the input frames should go 
        //      between the mutex lock and unlock, because other threads CAN/WILL access these structures
        //*****************************************************************************************
        pthread_mutex_lock(&receiver->buffer_mutex);

        //Check whether anything arrived
        int incoming_msgs_length = ll_get_length(receiver->input_framelist_head);
        if (incoming_msgs_length == 0)
        {
            //Nothing has arrived, do a timed wait on the condition variable (which releases the mutex). Again, you don't really need to do the timed wait.
            //A signal on the condition variable will wake up the thread and reacquire the lock
            pthread_cond_timedwait(&receiver->buffer_cv, 
                                   &receiver->buffer_mutex,
                                   &time_spec);
        }

        handle_incoming_msgs(receiver,
                             &outgoing_frames_head);

        pthread_mutex_unlock(&receiver->buffer_mutex);
        
        //CHANGE THIS AT YOUR OWN RISK!
        //Send out all the frames user has appended to the outgoing_frames list
        int ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        while(ll_outgoing_frame_length > 0)
        {
            LLnode * ll_outframe_node = ll_pop_node(&outgoing_frames_head);
            char * char_buf = (char *) ll_outframe_node->value;
            
            //The following function frees the memory for the char_buf object
            send_msg_to_senders(char_buf);

            //Free up the ll_outframe_node
            free(ll_outframe_node);

            ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        }
    }
    pthread_exit(NULL);

}

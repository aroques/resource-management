#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#define MSGSZ 50

struct msgbuf {
    long mtype;
    char mtext[MSGSZ];
};

int get_message_queue();
void remove_message_queue(int msgqid);
void receive_msg(int msgqid, struct msgbuf* mbuf, int mtype);
void receive_msg_no_wait(int msgqid, struct msgbuf* mbuf, int mtype);
void send_msg(int msgqid, struct msgbuf* mbuf, int mtype);

#endif

#ifndef __OSCAR_BUDDYLIST_H__
#define __OSCAR_BUDDYLIST_H__

#define AIM_CB_FAM_BUD 0x0003

/*
 * SNAC Family: Buddy List Management Services.
 */ 
#define AIM_CB_BUD_ERROR 0x0001
#define AIM_CB_BUD_REQRIGHTS 0x0002
#define AIM_CB_BUD_RIGHTSINFO 0x0003
#define AIM_CB_BUD_ADDBUDDY 0x0004
#define AIM_CB_BUD_REMBUDDY 0x0005
#define AIM_CB_BUD_REJECT 0x000a
#define AIM_CB_BUD_ONCOMING 0x000b
#define AIM_CB_BUD_OFFGOING 0x000c
#define AIM_CB_BUD_DEFAULT 0xffff

/* aim_buddylist.c */
int aim_add_buddy(aim_session_t *, aim_conn_t *, const char *);
int aim_remove_buddy(aim_session_t *, aim_conn_t *, const char *);

#endif /* __OSCAR_BUDDYLIST_H__ */

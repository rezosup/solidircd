/************************************************************************
 *   IRC - Internet Relay Chat, include/hooks.h
 *   Copyright (C) 2003 Lucas Madar
 *
 */

enum c_hooktype {
   CHOOK_10SEC,       /* Called every 10 seconds or so -- 
                       * not guaranteed to be 10 seconds *
                       * Params: None
                       * Returns void 
                       */
   CHOOK_PREACCESS,   /* Called before any access checks (dns, ident) 
                       * are done, acptr->ip is valid, 
                       * acptr->hostip is not "*"
                       * Params: 1: (aClient *) 
                       * Returns int
                       */
   CHOOK_POSTACCESS,  /* called after access checks are done 
                       * (right before client is put on network)
                       * Params: 1: (aClient *) 
                       * Returns int 
                       */
   CHOOK_POSTMOTD,    /* called after MOTD is shown to the client 
                       * Params: 1: (aClient *)
                       * Returns int 
                       */

   CHOOK_MSG,         /* called for every privmsg or notice
                       * Params: 3: (aClient *, int isnotice, char *msgtext), 
                       * Returns int 
                       */
   CHOOK_CHANMSG,     /* called for every privmsg or notice to a channel
                       * Params: 4: (aClient *source, aChannel *destination, 
\                      *             int isnotice, char *msgtxt)
                       * Returns int
                       */
   CHOOK_USERMSG,     /* called for every privmsg or notice to a user
                       * Params: 4: (aClient *source, aClient *destination,
                       *             int isnotice, char *msgtxt)
                       * Returns int
                       */
   CHOOK_MYMSG,       /* called for every privmsg or notice to 'me.name' 
                       * Params: 3: (aClient *, int isnotice, char *msgtext)
                       * Returns int 
                       */
   CHOOK_SIGNOFF,     /* called on client exit (exit_client)
                       * Params: 1: (aClient *)
                       * Returns void */
   MHOOK_LOAD,        /* Called for modules loading and unloading */
   MHOOK_UNLOAD       /* Params: 2: (char *modulename, void *moduleopaque) */
};

extern int call_hooks(enum c_hooktype hooktype, ...);
extern int init_modules();

#define MODULE_INTERFACE_VERSION 1006 /* the interface version (hooks, modules.c commands, etc) */

#ifdef BIRCMODULE
extern void *bircmodule_add_hook(enum c_hooktype, void *, void *);
extern void bircmodule_del_hook();
extern int bircmodule_malloc(int);
extern int bircmodule_free(void *);
#endif

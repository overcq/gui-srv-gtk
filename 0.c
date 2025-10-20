/*******************************************************************************
* ©overcq                on ‟Gentoo Linux 23.0” “x86_64”              2025‒9‒8 T
*******************************************************************************/
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
//==============================================================================
GtkApplication *Z_gtk_Q_app;
GtkWidget *Z_gtk_Q_main_window;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pid_t process_id;
static int shm_id;
//==============================================================================
static
void
Z_signal_V_process_call_req( int uid
, siginfo_t *siginfo
, void *data
){  if( siginfo->si_code != SI_QUEUE )
        return;
    void *shm = shmat( siginfo->si_value.sival_int, 0, 0 );
    if( !~( unsigned long )shm )
        return;
    char *p = shm;
    uint64_t command = *( uint64_t * )p;
    while(command)
    {   p += sizeof( uint64_t );
        bool invalid = false;
        switch(command)
        { case 1:
                gtk_window_close( GTK_WINDOW( Z_gtk_Q_main_window ));
                break;
          case 2:
            {   GtkBuilder *builder = gtk_builder_new_from_string( p, ~0 );
                p += strlen(p) + 1;
                GtkWidget *window = 0;
                GSList *objects = gtk_builder_get_objects(builder);
                GSList *next = objects;
                do
                {   if( GTK_IS_WINDOW( next->data ))
                    {   window = GTK_WIDGET( next->data );
                        break;
                    }
                    next = next->next;
                }while(next);
                gtk_window_set_application( GTK_WINDOW(window), Z_gtk_Q_app );
                gtk_window_close( GTK_WINDOW( Z_gtk_Q_main_window ));
                gtk_window_present( GTK_WINDOW(window) );
                next = next->next;
                while(next)
                {   if( GTK_IS_WINDOW( next->data ))
                    {   window = GTK_WIDGET( next->data );
                        gtk_window_present( GTK_WINDOW(window) );
                    }
                    next = next->next;
                }
                g_slist_free(objects);
                g_object_unref(builder);
                break;
            }
          default:
                invalid = true;
                break;
        }
        if(invalid)
            break;
        command = *( uint64_t * )p;
    }
    shmdt(shm);
    union sigval sv;
    sigqueue( siginfo->si_pid, SIGUSR2, sv );
}
static
void
Z_signal_V_process_call_reply( int uid
, siginfo_t *siginfo
, void *data
){  shmctl( shm_id, IPC_RMID, 0 );
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static
gboolean
Q_application_X_activate_I_1( void *data
){  size_t l = 0x1000 - 1;
    l = ( 2 * sizeof( uint64_t ) + l ) & ~l;
    shm_id = shmget( IPC_PRIVATE, l, 0600 | IPC_CREAT | IPC_EXCL );
    uint64_t *n = shmat( shm_id, 0, 0 );
    n[0] = 1;
    n[1] = 0;
    shmdt(n);
    union sigval sv;
    sv.sival_int = shm_id;
    sigqueue( process_id, SIGUSR1, sv );
    return G_SOURCE_REMOVE;
}
static
void
Q_application_X_activate( GtkApplication *Z_gtk_Q_app
, void *data
){  Z_gtk_Q_main_window = gtk_application_window_new( Z_gtk_Q_app );
    g_idle_add( &Q_application_X_activate_I_1, 0 );
}
//==============================================================================
int
main(
  int argc
, char *argv[]
){  struct sigaction sa;
    sigfillset( &sa.sa_mask );
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = Z_signal_V_process_call_req;
    sigaction( SIGUSR1, &sa, 0 );
    sa.sa_sigaction = Z_signal_V_process_call_reply;
    sigaction( SIGUSR2, &sa, 0 );
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;
    sigaction( SIGVTALRM, &sa, 0 );
    process_id = getppid();
    Z_gtk_Q_app = gtk_application_new( "org.gtk.gui-srv", G_APPLICATION_DEFAULT_FLAGS );
    g_signal_connect( Z_gtk_Q_app, "activate", G_CALLBACK( Q_application_X_activate ), 0 );
    int status = g_application_run( G_APPLICATION( Z_gtk_Q_app ), argc, argv );
    g_object_unref( Z_gtk_Q_app );
    return status;
}
/******************************************************************************/

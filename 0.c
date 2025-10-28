/*******************************************************************************
* ©overcq                on ‟Gentoo Linux 23.0” “x86_64”              2025‒9‒8 T
*******************************************************************************/
#include <sys/shm.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
//==============================================================================
GtkApplication *Z_gtk_Q_app;
GtkWidget *Z_gtk_Q_main_window;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pid_t process_id;
static volatile int shm_id = ~0;
static char *next_commands;
static size_t next_commands_l;
static unsigned Z_signal_I_timeout_S;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static unsigned Z_entry_X_changed_I_timeout_S;
static GHashTable *Z_widget_S_by_id;
//==============================================================================
static
gboolean
Z_signal_I_timeout( void *data
){  if( ~shm_id )
        return G_SOURCE_CONTINUE;
    shm_id = shmget( IPC_PRIVATE, next_commands_l, 0600 | IPC_CREAT | IPC_EXCL );
    if( !~shm_id )
        exit( EXIT_FAILURE );
    char *p = shmat( shm_id, 0, 0 );
    memcpy( p, next_commands, next_commands_l );
    shmdt(p);
    free( next_commands );
    next_commands = 0;
    union sigval sv;
    sv.sival_int = shm_id;
    sigqueue( process_id, SIGUSR1, sv );
    Z_signal_I_timeout_S = 0;
    return G_SOURCE_REMOVE;
}
static
void
Z_signal_I_process_call_req_Z_void(
  char *id
){  size_t l_1 = strlen(id) + 1;
    size_t l = 0x1000 - 1;
    l = ( sizeof( uint64_t ) + l_1 + sizeof( uint64_t ) + l ) & ~l;
    char *p = realloc( next_commands, next_commands_l + l );
    if( !p )
        exit( EXIT_FAILURE );
    next_commands = p;
    p += next_commands_l;
    next_commands_l += l;
    *( uint64_t * )p = 2;
    strcpy( p + sizeof( uint64_t ), id );
    *( uint64_t * )( p + sizeof( uint64_t ) + l_1 ) = 0;
    if( !Z_signal_I_timeout_S )
        g_idle_add( Z_signal_I_timeout, 0 );
}
static
void
Z_signal_I_process_call_req_Z_unsigned(
  char *id
, unsigned v
){  size_t l_1 = strlen(id) + 1;
    size_t l = 0x1000 - 1;
    l = ( sizeof( uint64_t ) + l_1 + sizeof(unsigned) + sizeof( uint64_t ) + l ) & ~l;
    char *p = realloc( next_commands, next_commands_l + l );
    if( !p )
        exit( EXIT_FAILURE );
    next_commands = p;
    p += next_commands_l;
    next_commands_l += l;
    *( uint64_t * )p = 2;
    strcpy( p + sizeof( uint64_t ), id );
    *( unsigned * )( p + sizeof( uint64_t ) + l_1 ) = v;
    *( uint64_t * )( p + sizeof( uint64_t ) + l_1 + sizeof(unsigned) ) = 0;
    if( !Z_signal_I_timeout_S )
        g_idle_add( Z_signal_I_timeout, 0 );
}
static
void
Z_signal_I_process_call_req_Z_string(
  char *id
, char *s
){  size_t l_1 = strlen(id) + 1;
    size_t l_2 = strlen(s) + 1;
    size_t l = 0x1000 - 1;
    l = ( sizeof( uint64_t ) + l_1 + l_2 + sizeof( uint64_t ) + l ) & ~l;
    char *p = realloc( next_commands, next_commands_l + l );
    if( !p )
        exit( EXIT_FAILURE );
    next_commands = p;
    p += next_commands_l;
    next_commands_l += l;
    *( uint64_t * )p = 2;
    strcpy( p + sizeof( uint64_t ), id );
    strcpy( p + sizeof( uint64_t ) + l_1, s );
    *( uint64_t * )( p + sizeof( uint64_t ) + l_1 + l_2 ) = 0;
    if( !Z_signal_I_timeout_S )
        g_idle_add( Z_signal_I_timeout, 0 );
}
static
void
Z_action_X_activate( GSimpleAction *action
, GVariant *parameter
, void *data
){  Z_signal_I_process_call_req_Z_void( gtk_buildable_get_buildable_id(( void *)action ));
}
static
void
Z_button_X_clicked( GtkButton *button
, void *data
){  Z_signal_I_process_call_req_Z_void( gtk_buildable_get_buildable_id(( void *)button ));
}
static
void
Z_checkbutton_X_toggled( GtkCheckButton *checkbutton
, void *data
){  Z_signal_I_process_call_req_Z_void( gtk_buildable_get_buildable_id(( void *)checkbutton ));
}
static
void
Z_dropdown_X_selected( GtkDropDown *dropdown
, GParamSpec *parameter
, void *data
){  Z_signal_I_process_call_req_Z_unsigned( gtk_buildable_get_buildable_id(( void *)dropdown ), gtk_drop_down_get_selected(dropdown) );
}
static
gboolean
Z_entry_X_changed_I_timeout( void *data
){  GtkEntry *entry = data;
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(entry);
    Z_signal_I_process_call_req_Z_string( gtk_buildable_get_buildable_id(data), gtk_entry_buffer_get_text(buffer) );
    Z_entry_X_changed_I_timeout_S = 0;
    return G_SOURCE_REMOVE;
}
static
void
Z_entry_X_changed( GtkEntry *entry
, void *data
){  if( Z_entry_X_changed_I_timeout_S )
        g_source_remove( Z_entry_X_changed_I_timeout_S );
    Z_entry_X_changed_I_timeout_S = g_timeout_add( 183, Z_entry_X_changed_I_timeout, entry );
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
            {   GList *windows_ = gtk_application_get_windows( Z_gtk_Q_app );
                GSList *windows = 0;
                while( windows_ )
                {   windows = g_slist_append( windows, windows_->data );
                    windows_ = windows_->next;
                }
                GSList *next = windows;
                do
                {   gtk_window_close( GTK_WINDOW( next->data ));
                    next = next->next;
                }while(next);
                g_slist_free(windows);
                break;
            }
          case 2:
            {   if( Z_widget_S_by_id )
                {   invalid = true;
                    break;
                }
                GtkBuilder *builder = gtk_builder_new_from_string( p, ~0 );
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
                        gtk_window_set_application( GTK_WINDOW(window), Z_gtk_Q_app );
                        gtk_window_present( GTK_WINDOW(window) );
                    }
                    next = next->next;
                }
                Z_widget_S_by_id = g_hash_table_new( g_str_hash, g_str_equal );
                next = objects;
                do
                {   if( GTK_IS_ENTRY( next->data )
                    || GTK_IS_PROGRESS_BAR( next->data )
                    || GTK_IS_SPINNER( next->data )
                    )
                        g_hash_table_insert( Z_widget_S_by_id, gtk_buildable_get_buildable_id( next->data ), next->data );
                    if( GTK_IS_BUTTON( next->data ))
                        g_signal_connect( next->data, "clicked", G_CALLBACK( Z_button_X_clicked ), 0 );
                    else if( GTK_IS_CHECK_BUTTON( next->data ))
                        g_signal_connect( next->data, "toggled", G_CALLBACK( Z_checkbutton_X_toggled ), 0 );
                    else if( GTK_IS_DROP_DOWN( next->data ))
                        g_signal_connect( next->data, "notify::selected", G_CALLBACK( Z_dropdown_X_selected ), 0 );
                    else if( GTK_IS_ENTRY( next->data ))
                        g_signal_connect( next->data, "changed", G_CALLBACK( Z_entry_X_changed ), 0 );
                    else if( GTK_IS_MENU_BUTTON( next->data ))
                    {   GMenuModel *menu = gtk_menu_button_get_menu_model( GTK_MENU_BUTTON( next->data ));
                        for( int i = 0; i != g_menu_model_get_n_items(menu); i++ )
                        {   char *s;
                            g_menu_model_get_item_attribute( menu, i, "action", "s", &s );
                            GSimpleAction *action = g_simple_action_new( s + 4, 0 );
                            g_free(s);
                            g_signal_connect( action, "activate", G_CALLBACK( Z_action_X_activate ), 0 );
                            g_action_map_add_action( G_ACTION_MAP( Z_gtk_Q_app ), G_ACTION(action) );
                        }
                    }
                    next = next->next;
                }while(next);
                g_slist_free(objects);
                g_object_unref(builder);
                break;
            }
          case 3:
            {   char *key = p;
                p += strlen(p) + 1;
                GtkWidget *widget = g_hash_table_lookup( Z_widget_S_by_id, key );
                if( GTK_IS_ENTRY(widget) )
                {   GtkEntry *entry = GTK_ENTRY(widget);
                    GtkEntryBuffer *buffer = gtk_entry_get_buffer(entry);
                    gtk_entry_buffer_set_text( buffer, p, -1 );
                    p += strlen(p) + 1;
                }else if( GTK_IS_PROGRESS_BAR(widget) )
                {   GtkProgressBar *progress_bar = GTK_PROGRESS_BAR(widget);
                    gtk_progress_bar_set_fraction( progress_bar, *( double * )p );
                    p += sizeof(double);
                }else if( GTK_IS_SPINNER(widget) )
                {   GtkSpinner *spinner = GTK_SPINNER(widget);
                    gtk_spinner_set_spinning( spinner, !gtk_spinner_get_spinning(spinner) );
                    p += sizeof(double);
                }
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
    shm_id = ~0;
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
Q_application_X_activate( GtkApplication *app
, void *data
){  Z_gtk_Q_main_window = gtk_application_window_new(app);
    g_idle_add( &Q_application_X_activate_I_1, 0 );
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

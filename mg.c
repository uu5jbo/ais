#include <mongoose.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <json.h>

static bool interrupted = false;
static const char* msg = "{ \"APIKey\": \"\", \
                          \"BoundingBoxes\": [ [[59.011, -1.7 ],[27.7, 35.3]] ]}";//,\
						//  \"FilterMessageTypes\": [\"PositionReport\"]}";
static const char* url = "wss://stream.aisstream.io/v0/stream";

char vessl[ 80 ];

                 
static void callback(struct mg_connection* c, int ev, void* ev_data, void* fn_data)
{
    SHIP_T ship_t; //declare the structure 'ship_t' of SHIP_T type
    SHIP_T *pship = &ship_t; // create pointer 'pship' to 'ship_t' structure

    if (ev == MG_EV_OPEN)
    {
        c->is_hexdumping = 0; // Hides log messages, 1 == enable :)
    }
    else if (ev == MG_EV_CONNECT)
    {
        struct mg_tls_opts opts;
        memset(&opts, 0, sizeof(struct mg_tls_opts));
        mg_tls_init(c, &opts);
    }
    else if (ev == MG_EV_ERROR)
    {
       	MG_ERROR(("%p %s", c->fd, (char*)ev_data));
        interrupted = true;
    }
    else if (ev == MG_EV_WS_OPEN)
    {
        mg_ws_send(c, msg, strlen(msg), WEBSOCKET_OP_TEXT);
    }
    else if (ev == MG_EV_WS_MSG)
    {
        // When we get response, print it
        struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;
      
        printf("RX RAW JSON: [%.*s]\n", (int)wm->data.len, wm->data.buf);
        printf( "**************** END OF RAW JSON MESSAGE *******************\n\n" );

       	static int flag; // for swithching cursor view
        //printf( "Searching Vessel: %s %s \n" , vessl, ( !flag ) ? "\\" : "/" );
		flag = 1 ^ flag;
        
        //parser( wm->data.buf, pship ); // test of parser -> fills the ship_t
                                        // structure

        //parser1( wm->data.buf, &ship );  // testing new parser( JUST TEST! )
                                          
        if( findessentials( wm->data.buf, vessl, &ship_t ) )
        {
            puts( "******* Found!!! ********" );
            //printf("RX: [%.*s]\n", (int)wm->data.len, wm->data.buf);
            parser( wm->data.buf, pship );
           // parser1( wm->data.buf, pship );  // testing new parser( JUST TEST! )
            display_all( pship );
            putchar( '\n' );
            //sleep( 2 );
            memset ( pship, 0, sizeof( struct vessel_info ) );
            interrupted = true;
            exit( EXIT_SUCCESS ); 
        }
        static long count = 0; //couter of ais messages
        printf( "Searching Vessel: %s, count: %lu, %s \n" ,
                vessl, count++, ( !flag ) ? "\\" : "/" );

    }

    else if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE)
    {
        interrupted = true;
    }
    else
    {
    }
}

static void sigint_handler(int sig)
{
	interrupted = true;
}

void usage( char prog[] )
{
    fprintf( stderr, "AIS Data Search Program\n"
            "Usage: <%s> MMSI, Name or Callsign of a vessel\n", prog );
    exit( -1 );
    //signal( SIGKILL, sigint_handler );
}

int main( int argc, char** argv )
{
    signal(SIGINT, sigint_handler);

    struct mg_mgr mgr;        // Event manager
    struct mg_connection* c;  // Client connection
    
    if( argc < 2 ) usage( argv[0] );
    if( argc >= 2 ) 
    {
        strcpy( vessl, argv[1] ); 
        toupperstr( vessl );
        puts( vessl );
    }

    mg_mgr_init(&mgr);        // Initialise event manager
    mg_log_set(MG_LL_INFO);   // Set log level
    //mg_log_set(MG_LL_NONE);   // Set log level
    c = mg_ws_connect(&mgr, url, callback, NULL, NULL); // Create client
    while (c && interrupted == false ) mg_mgr_poll(&mgr, 100); // Wait for echo
    mg_mgr_free(&mgr);        // Deallocate resources
    return 0;
}

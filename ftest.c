#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "filemid.h"

char *fname = "MID.csv";

void usage( char *progname )
{
    fprintf( stderr, "MMSI MID coutry resolution program v 0.1 July 19 2024 before breakfast:)\n"
            "Usage <%s> MID number within 201-775\n", progname );
    exit( -1 );
}

int main( int argc, char *argv[] )
{
    FILE *fd;
    int i = 0;
    char ch;
    long fcount;
    char *ptmp;

    if( argc < 2 ) usage( argv[0] );

    fd = fopen( fname, "r" );
    if( !fd )
    {
        fprintf( stderr, "Can't open file <%s>\n", fname );
        exit(EXIT_FAILURE);
    }
   // printf( "File %s opened\n", fname );
    fseek( fd, 0L, SEEK_END );
    fcount = ftell( fd ); //get the file size
    rewind( fd );

    ptmp = (char*)calloc( 1, (int)fcount * sizeof( char ) );
    if( !ptmp)
    {
        printf( "Can't allocate memory\n" );
        exit( -1 );
    }
    //puts( "Memory allocated OK." );
    //printf( "The file size is: %ld bytes\n", fcount );
    
    while( ( ch = getc(fd) ) != EOF ) 
    {
        ptmp[i] = ch;
       // putchar( ch );
        i++;
    }
    //puts( "File contents copied into memory" );
    fclose( fd ); //close file since it's in heap
    
    //char input[10];
    int mid;
    char *temp;

   /* printf( "%s", "Enter MID: " );
    while( scanf( "%d", &mid ) && mid != 0 )
    {*/
        mid = atoi( argv[1] );
        if( mid < 201 || mid > 775 )
        {
            printf( "Unknown MID. It must be within 201..775\n" );
            free( ptmp );
            return 0;
            //continue;
        }
        //sprintf( input, "%d", mid ); //transform it to text form
        //temp = memcpy( input, argv[1], strlen( argv[1] ) );
       // temp = input;
        temp = argv[1];
        
        //if( strlen( temp ) >= 3 )
       // {   
            if( ( temp = strstr( ptmp, temp )  ) )
            {
                temp += 4; // skipping digits and comma
                printf( "%d - ", mid );
                //while( *temp != '\n' ) // until we meet '\n' symbol
                while( *temp != '\n' ) // until we meet '\n' symbol
                {
                    putchar( *temp ); // print symbols
                    temp++;
                }
                //*temp 
            } 
            else 
            {
                printf( "%d - Unknown MId. \n", mid );
            } 
        //}
       // else
           // printf( "%s unknown MID..\n", input );
        putchar( '\n' );
        //printf( "Enter next MID: " );
    //}

   // puts("Program finished!" );

    free( ptmp );
    

    return 0;
}
    

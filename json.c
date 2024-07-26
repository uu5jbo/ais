/*------------------ json.c file ------------*/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <json.h>

// const search patterns
    const char *mmsi_str = "\"MMSI\":";
    const char *lat = "\"latitude\":";
    const char *lon = "\"longitude\":";
    const char *name = "\"ShipName\":";
    const char *sog = "\"Sog\":";
    const char *cog = "\"Cog\":";
    const char *pstrue = "\"PositionAccuracy\":";
    const char *CallSign = "\"CallSign\":";
    const char *timeutc = "\"time_utc\":";


//transforms string to uppercase 
char* toupperstr( char *str )
{
    char *ptmp = str;
    while( *ptmp )
    {
        *ptmp = toupper( *ptmp );
        ptmp++;
    }
    return str;
}

//modifies string inserting ',' in the end of string
void modify_str( char* str )
{
    char* ptmp;

    if( ( ptmp = strstr(str, "UTC\"" ) ) ) {
        ptmp += 4;
        *ptmp = ',';
    }
}
 
int parse_comma_delimited_str( char *str, char **fields, int max_fields )
{
    int i = 0;
    char *ptmp;
   
    fields[ i++ ] = str;
    modify_str( str );    
    /*if( ( ptmp = strstr(str, "UTC\"" ) ) ) {
        ptmp += 4;
        *ptmp = ',';
    } */

    while( ( i <= max_fields ) && ( str = strchr( str, ',' ) ) ) 
    {
        *str = '\0';
        fields[ i++ ] = ++str;
    }
    return --i;
}

void parser1( char *rawdata, SHIP_T *ship )
{
    char *fields[ 50 ];
    int count_fields = parse_comma_delimited_str( rawdata, fields, 50 );

    printf( "There are %d messages comma separated\n", count_fields );
    for( int i = 0; i < count_fields; i++ )
        printf( "%d. %s\n", i, fields[ i ] );

    //exit( -1 );
}

static void country( SHIP_T *ship )
{
    FILE *fd;
    int i = 0;
    char ch;
    long fcount;
    char *pdata;
    
    ship->country = NULL;

    fd = fopen( fname, "r" );
    if( !fd )
    {
        fprintf( stderr, "Can't open file <%s>\n", fname );
        ship->country = NULL;
        return;
        exit(EXIT_FAILURE);
    }
    
    //get the file size
    fseek( fd, 0L, SEEK_END );
    fcount = ftell( fd ); //get the file size
    rewind( fd );
    
    //allocate memory in heap
    pdata = (char*)calloc( 1, (int)fcount * sizeof( char ) );
    if( !pdata)
    {
        printf( "Can't allocate memory\n" );
        return;
        exit( -1 );
    }

    //copy file contents to heap
    while( ( ch = getc(fd) ) != EOF ) 
    {
        pdata[i++] = ch;
       // i++;
    }
    fclose( fd ); //close the file since it's already in heap
    
    int mid;
    char *ptemp;
    char atemp[ 4 ]; 
    //char auxarr[580]; 
    
    //copy 3 digits to array from ship->mmsi to 'mid' variable
    mid = atoi( strncpy( atemp, ship->mmsi, 3  ) );
    if( mid < 201 || mid > 775 )
    {
        printf( "Unknown MID. It must be within 201..775\n" );
        free( pdata );
        return;
    }

    //copy address of array with 3 MID digits to pointer
    //for further process
    ptemp = &atemp[0];

    //if string with all MIDs contains 3 digits from array
    //and copy address of string found to pointer
    if( ( ptemp = strstr( pdata, atemp )  ) ) 
    {
        ptemp += 4; // skipping digits and comma
        ship->country = ptemp;
        while( *ptemp != '\n' ) // until we meet '\n' symbol in string
        {   
            ptemp++;
        }
        *ptemp = '\0'; //string terminator
    } 
    free( pdata );
}

static void convertcoords( double lat, double lon, char *slat, char *slon, size_t len )
{
    double fractpart;
    double intarr[1];
    char letter[2];

    //lat and lon already passed as arguments
    enum{ latitude, longitude };
    
    //assign neccesary letter
    if( lat < 0 )
        letter[latitude] = 'S';
    else
        letter[latitude] = 'N';
    if( lon < 0 )
        letter[longitude] = 'W';
    else letter[longitude] = 'E';
    
    //assign absolute values
    lat = fabs( lat );
    lon = fabs( lon );

    fractpart = modf( lat, intarr ) * 60.0;
    printf( "%f\n", fractpart );
    snprintf( slat, len, "%02.f %06.3f\'%c", *intarr, fractpart, letter[latitude] );

    fractpart = modf( lon, intarr ) * 60.0;
    snprintf( slon, len, "%03.f %06.3f\'%c", *intarr, fractpart, letter[longitude] );

}

static char* isolatestr( const char* rawstr, const char* pattern, 
                        char* outstr, const char delim )
{
    char* paux = NULL;
    
    if( ( paux = strstr( rawstr, pattern ) ) )
    {
       // printf( "Outstring with pattern <%s> has %ld bytes\n", pattern, strlen(outstr) );
        int i = 0;
        paux += strlen( pattern ); //raw string pointer now points to the end of the
                                   //pattern string
        //printf( "paux += strlen(pattern) passed\n" );
        while( *paux != delim && *paux )
        {
           // printf("Forming outstring with %d bytes\n", i+1 );
            outstr[i] = *paux;
            i++;
            paux++;
        }
        outstr[i] = '\0';
        //printf( "Outstring <%s> now filled with %d bytes\n", outstr, i );
        
        return outstr;
    }
    else 
        return NULL;
}

void parser( char *str, SHIP_T *ship )
{
    char *ptmp = str;
    char *paux;
    int flag = 0;
    static long int counter = 0;

    //search for COG
    if( ( paux = strstr( ptmp, cog ) ) )
    {
        ship->cog = atof( paux += strlen(cog) );
    }
    else ship->cog = 0.0f;

    //search for SOG
    if( ( paux = strstr( ptmp, sog ) ) )
    {
        ship->sog = atof( paux += strlen(sog) );
    }
    else ship->sog = 0.0f;


    //search callsign
    printf( "Searching Callsign..." );
    if( !isolatestr( str, CallSign, ship->callsign, ',' ) )
    {
        puts( " Not found call." );
        strcpy( ship->callsign, "---" );
    }
    else puts( " OK!" );

    //search time
    printf( "Searching time UTC..." );
    if( !isolatestr( str, timeutc, ship->time, '}' ) )
    {
        puts( "Not found time" );
        strcpy( ship->time, "---" );
    } 
    else puts( " OK!" );

    //search and isolate PositionAccuracy string
    printf( "Searching Pos Accuracy..." );
    if( !isolatestr( str, pstrue, ship->truepos, ',' ) )
    {
        puts( "Not found PosAccur str" );
        strcpy( ship->truepos, "???" );
    }
    else puts( " OK!" );

    /*
    //search mmsi
    if( ( paux = strstr( ptmp, mmsi_str ) ) )
    {
        paux += strlen( mmsi_str );
        ship->mmsi = atoi( paux ) ;
    } */

    //mmsi in string
    printf("Searching MMSI...");
    if( !isolatestr( str, mmsi_str, ship->mmsi, ',' ) )
    {
       puts( "Not found MMSI" );
       strcpy( ship->mmsi, "000000000" );
    }
    else
    {
       puts( " OK!" );

    }

    //search ship's name
    printf("Searching Vessel's Name...");
    if( !isolatestr( ptmp, name, ship->name, ',' ) )
    {
        printf( "Not found Name\n" );
        strcpy( ship->name, "---" );
    }
    else puts( " OK!" );

      //search latitude
    if( ( paux = strstr( ptmp, lat ) ) )
    {
        ship->lat = atof( paux += strlen(lat) );
    }
    
    //search longitude
    if( ( paux = strstr( ptmp, lon ) ) )
    {
        ship->lon = atof( paux += strlen(lon ) );
    }
   // display_all( ship );
   // printf( "Vessel count: %ld\n", counter++ );
   //memset( str, 0, strlen( str ));
}


void display_all( SHIP_T* ship )
{
    country( ship );
    convertcoords( ship->lat, ship->lon, ship->slat, ship->slon, sizeof( ship->slat ) );
    puts( "*********************************************************" );
    printf( "Ship Name: %s\nMMSI: %s\nCountry: %s\nCall: %s\n"
            "Lat: %f - %s\nLon: %f - %s\nSOG: %.1f kn\nCOG: %.1f Deg\n"
            "Pos accuracy: %s\nUTC: %s\n",
            ship->name, ship->mmsi, ship->country,ship->callsign,
            ship->lat, ship->slat, ship->lon, ship->slon, ship->sog, 
            ship->cog, ship->truepos, ship->time );
        puts( "*********************************************************" );
}

//parses raw string and writes to struct 3
//basic data fields: mmsi, name and callsign
//if found at least one of them func. returns 1, otherwise 0
int findessentials( char *rawstr, char *needle, struct vessel_info *ship )
{
    char *ptmp = NULL;
    //search name
    //extract a string with pattern from a buffer 
    ptmp = isolatestr( rawstr, name, ship->name, ',' );
    if ( strstr( ship->name, needle ) ) //compare ship's name in struct with
                                        //needle
        return 1;

   //ssearch mmsi
    ptmp = isolatestr( rawstr, mmsi_str, ship->mmsi, ',' );
    if( strstr( ship->mmsi, needle ) )
       return 1;
   
   //search callsign
    ptmp = isolatestr( rawstr, CallSign, ship->callsign, ',' );
    if( strstr( ship->callsign, needle ) )
        return 1;

   return 0;
}


   

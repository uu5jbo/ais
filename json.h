/*------------------- json.h file --------------*/
#ifndef JSON_H
#define JSON_H

#define fname "MID.csv"

//#pragma pack(push,1) // to disable padding
typedef struct vessel_info
{
    float lat;
    float lon;
    float cog;
    float sog;
    char  slat[15];
    char  slon[15];
    char name[80];
    char truepos[10];
    char callsign[20];
    char time[80];
    char mmsi[12];
    char *country;
} SHIP_T;
//#pragma pack(pop)



char* toupperstr( char *str );
int parse_comma_delimited_str( char*, char**, int );
void parser1( char*, SHIP_T* );
void display_all( SHIP_T* );
void parser( char *str, SHIP_T* );
int findessentials( char *rawstr, char *needle, SHIP_T* );
#endif /* JSON_H */

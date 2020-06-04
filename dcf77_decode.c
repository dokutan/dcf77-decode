/*
 * dcf77_decode.c
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// The first two digits of the year are not transmitted
#define CENTURY "20"

int parity_check( int* data, int size, int parity )
{
	int sum = 0;
	for(int i = 0; i < size; i++){
		sum += data[i];
	}
	
	return ((sum % 2) == !parity );
}

int bcd2dec( int* data, int size )
{
	int bcd_lookup[8] = {1, 2, 4, 8, 10, 20, 40, 80};
	int sum = 0;
	
	for(int i = 0; i < size; i++){
		
		if( data[i] == 1 )
			sum += bcd_lookup[i];
		
	}
	
	return sum;
}

int main( int argc, char** argv )
{
	
	// check arguments, print usage
	if( argc == 1 ){
		
		printf("Usage: %s datafile\n\n", argv[0]);
		printf("This program decodes the data coming from the DCF77 time signal station.\n");
		printf("Due to the differences in receiving hardware, the data is read from a file.\n");
		printf("If \"-\" is specified as the datafile, input is read from stdin.\n\n");
		printf("The datafile is expected to be in the following format:\n");
		printf("Empty line (\"\\n\") = Missing pulse at the start of a minute\n");
		printf("Line only with a 1 (\"1\\n\") = Bit 1\n");
		printf("Line only with a 0 (\"0\\n\") = Bit 0\n\n");
		printf("Licensed under the GNU GPL v3 or later.\n");
		
		return 0;
	}
	
	// open file
	FILE* input;
	if( !strcmp(argv[1], "-") )
		input = stdin;
	else
		input = fopen(argv[1], "r");
	
	if( input == NULL )
	{
		perror("Could not open file");
		return 1;
	}
	
	
	
	// individual line from input file
	char line[8];
	
	// wait for empty line = beginning of a minute
	printf("Waiting for the beginning of the next minute...\n");
	while( fgets(line, sizeof(line), input) != NULL )
	{
		if( line[0] == '\n' )
			break;
		
	}
	
	// decode received data
	int counter = 0;
	
	int tz1 = 0, tz2 = 0;
	int minutes[7];
	int hours[6];
	int day[6];
	int weekday[3];
	int month[5];
	int year[8];
	
	// read input file line by line
	while( 1 )
	{
		
		// read line from file
		char* l = fgets(line, sizeof(line), input);
		
		// EOF ?
		if( l == NULL ){
			break;
		}
		
		// beginning of a minute ? → print time, reset counter
		if( line[0] == '\n' ){
			
			// print date and time
			int YY = bcd2dec(year, 8);
			int MM = bcd2dec(month, 5);
			int DD = bcd2dec(day, 6);
			int hh = bcd2dec(hours, 6);
			int mm = bcd2dec(minutes, 7);
			int tz = 0;
			if( tz1 == 1 && tz2 == 0 )
				tz = 2;
			else if( tz1 == 0 && tz2 == 1 )
				tz = 1;
			else
				printf("Warning: Timezone data is invalid.\n");
			
			printf("%s%02d-%02d-%02dT%02d:%02d+%02d:00\n", CENTURY, YY, MM, DD, hh, mm, tz );
			
			counter = 0;
			continue;
		}
		
		
		// bit 0 → always 0
		if( counter == 0 ){
			
			printf("\n");
			
			if( line[0] != '0' )
				printf("Warning: First bit is not 0, this should never happen.\n");
		
		// weather broadcast (encrypted)
		} else if( counter >= 1 && counter <=14 ){
			
			if( counter == 1 )
				printf("Weather broadcast data: ");
			
			if( line[0] == '0' )
				printf("0");
			else if( line[0] == '1' )
				printf("1");
			else
				printf("?");
			
			if( counter == 14 )
				printf("\n");
			
		// call bit
		} else if( counter == 15 ){
			
			printf("Call bit: ");
			
			if( line[0] == '0' )
				printf("0\n");
			else if( line[0] == '1' )
				printf("1\n");
			else
				printf("?\n");
		
		// summer time announcement
		} else if( counter == 16 ){
			
			if( line[0] == '1' )
				printf("Summer time announcement.\n");
				
			
		// Timezone: CET or CEST ? part 1/2
		} else if( counter == 17 ){
			
			if( line[0] == '0' )
				tz1 = 0;
			else if( line[0] == '1' )
				tz1 = 1;
			else
				tz1 = 2;
			
		// Timezone: CET or CEST ? part 2/2
		} else if( counter == 18 ){
			
			if( line[0] == '0' )
				tz2 = 0;
			else if( line[0] == '1' )
				tz2 = 1;
			else
				tz2 = 2;
			
		// leap second announcement
		} else if( counter == 19 ){
			
			if( line[0] == '1' )
				printf("Leap second announcement.\n");
			
		// start of time information → always 1
		} else if( counter == 20 ){
			
			if( line[0] != '1' )
				printf("Warning: Bit 20 is not 1, this should never happen.\n");
			
		// minute information
		} else if( counter >= 21 && counter <= 27 ){
			
			if( line[0] == '0' )
				minutes[counter-21] = 0;
			else if( line[0] == '1' )
				minutes[counter-21] = 1;
			else
				minutes[counter-21] = 2;
			
		// minute bits parity
		} else if( counter == 28 ){
			
			int p = 2;
			if( line[0] == '0' )
				p = 0;
			else if( line[0] == '1' )
				p = 1;
			
			if( parity_check(minutes, 7, p) )
				printf("Warning: Parity check for the minutes failed.\n");
			
		// hour information	
		} else if( counter >= 29 && counter <= 34 ){
			
			if( line[0] == '0' )
				hours[counter-29] = 0;
			else if( line[0] == '1' )
				hours[counter-29] = 1;
			else
				hours[counter-29] = 2;
			
		// hour bits parity
		} else if( counter == 35 ){
			
			int p = 2;
			if( line[0] == '0' )
				p = 0;
			else if( line[0] == '1' )
				p = 1;
			
			if( parity_check(hours, 6, p) )
				printf("Warning: Parity check for the hours failed.\n");
			
		// day of the month
		} else if( counter >= 36 && counter <= 41 ){
			
			if( line[0] == '0' )
				day[counter-36] = 0;
			else if( line[0] == '1' )
				day[counter-36] = 1;
			else
				day[counter-36] = 2;
			
		// day of the week
		} else if( counter >= 42 && counter <= 44 ){
			
			if( line[0] == '0' )
				weekday[counter-42] = 0;
			else if( line[0] == '1' )
				weekday[counter-42] = 1;
			else
				weekday[counter-42] = 2;
			
			if( counter == 44 )
			{
				int wd = bcd2dec(weekday, 3);
				printf("Weekday (1 = Monday): %d\n", wd);
			}
			
		// month
		} else if( counter >= 45 && counter <= 49 ){
			
			if( line[0] == '0' )
				month[counter-45] = 0;
			else if( line[0] == '1' )
				month[counter-45] = 1;
			else
				month[counter-45] = 2;
			
		// year
		} else if( counter >= 50 && counter <= 57 ){
			
			if( line[0] == '0' )
				year[counter-50] = 0;
			else if( line[0] == '1' )
				year[counter-50] = 1;
			else
				year[counter-50] = 2;
			
		// date parity
		} else if( counter == 58 ){
			
			int p = 2;
			if( line[0] == '0' )
				p = 0;
			else if( line[0] == '1' )
				p = 1;
			
			// concatenate all date fields
			int date[22];
			int d = 0;
			for( int i = 0; i < 6; i++ ){
				date[d] = day[i];
				d++;
			}
			for( int i = 0; i < 3; i++ ){
				date[d] = weekday[i];
				d++;
			}
			for( int i = 0; i < 5; i++ ){
				date[d] = month[i];
				d++;
			}
			for( int i = 0; i < 8; i++ ){
				date[d] = year[i];
				d++;
			}
			
			if( parity_check(date, 22, p) )
				printf("Warning: Parity check for the date failed.\n");
		
		// leap second
		} else if( counter == 59 ){
			
			printf("Leap second now.\n");
			
		}
		
		// increment bit counter
		counter++;
		
	}
	
	// close input file
	fclose(input);
	
	return 0;
}

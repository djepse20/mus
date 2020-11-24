#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "csv_parser\csv.h"

enum Name{NAME,SHIFTSTART};
enum Date{YEAR,MONTH,DAY,HOUR,MINUTE};
enum Time{START_TIME,END_TIME};

typedef struct csvValues{ /* en sturct der indeholder et af csv values samt antal employees og vagter*/
    int numberOfemployees;
    int numberOfShifts;
    int timeBetweenShifts;
    char** value; 
}csvValues; 
typedef struct date{
    int year[2];
    int month[2];
    int day[2];
    int hour[2];
    int minute[2];
}date;
typedef struct shifts{
    int hoursWorked; /*timer der er blevet arbejder ved en given vagt*/
    int minutesWorked; /*minutter arbejder ved en given vagt*/
    char startTimeOfShift[30]; /*start tidspunkt for vagten talt i minutter, ud fra unix tid*/
    char endTimeOfShift[30]; /*slut tidspunkt for vagten talt i minutter, ud fra unix tid*/
    int TotalNumberOfShift; /* gør det nemt at finde det totale antal vagter en person har arbejdet*/
    date date;
}shifts;


typedef struct betweenShifts{ /* struct til tid mellem vagter */
    int timeBetweenShifts; /* antal timer mellem to shifts*/
    char startDay[30]; /*start datoen mellem de to vagter*/
    char endDay[30]; /* slut datoen mellem de to vagter*/
}betweenShifts;

typedef struct employees{
    int totalHoursWorked; /* antal timer medarbejderen har arbejdet*/
    int totalMinutesWorked;/* antal minutter medarbejderen har arbejdet*/
    char employeesName[854];  /*medarbejderens navn* antager at */
    betweenShifts* TimeBetween; /* et array af struct der indeholder tid mellem vagter */
    shifts* shift;
}employees;



void StartProgram(); /*måske smart*/
void PrintAssert(int,char*); /* bruges til at stoppe programmet i at kører samt at printe en error message ud på skærmen*/
FILE* OpenCsvFile(); /*åbner csv filen og returner pointere til FILE structen*/ 
csvValues* CsvValueFromFile(FILE*); /* funktion til at modtage csv string */
int CountNumberOfCsvLines(FILE*);
int* CountNumberOfCarachtersPerCsvLine(FILE*,int); /*kalkuler antallet af carachters */
void remove_all_chars(char*, char);  /*bruges til at fjerne ""  før medarbjeder navn ligges over i deres repspektive structs*/
int CountTotalNumberOfemployeess(csvValues*); /*tæl det totale antal medarbejdere*/ 
int CountTotaltShifts(char**); /*tæl det totale antal vagter */
employees* AllocateStructs(csvValues*); /*allokere plads for structs */
employees* LoadIntoStruct(csvValues*,employees*); /*putter de rette felter ind de rette steder */
void PutNameInStruct(csvValues*,employees*); /*lig navnet over i shift og employees structen*/
void PutTimeWorkedInStruct(csvValues*,employees*);
int CalculateTimeWorked(int,int,int,int,int); /*udregn antal timer og minuter en given vagt varer samt ligge det ind i shift samt employees */
void CalculateTimeBetweenShifts(employees*); /*udregner tiden mellem to vagter*/


int main()
{
    
    StartProgram();

    return 0;
}


void StartProgram()
{
    FILE* csvFile;
    csvValues* csvValue;
    employees* employee;
    csvFile = OpenCsvFile();
    csvValue  = CsvValueFromFile(csvFile);

    employee = AllocateStructs(csvValue);
    employee = LoadIntoStruct(csvValue,employee);
    printf("%d",employee[0].shift[0].date.hour[1]);
}

FILE* OpenCsvFile()
{
    FILE* csvFile = fopen("eksempel.csv","r"); /*r står for read*/
    PrintAssert(csvFile != NULL,"File cannot be read");
    return csvFile;    
}




 csvValues* CsvValueFromFile(FILE* csvFile)
{
    int done=0,err=0,numberOfLines,i;
    int* numberOfChars;
    char **csvLines;
    numberOfLines = CountNumberOfCsvLines(csvFile);
    numberOfChars = CountNumberOfCarachtersPerCsvLine(csvFile,numberOfLines); /*allokere et array af integer der svarer til hvor mange carachters hver linje har */  

    csvLines = calloc(numberOfLines+1,sizeof(char*)); /* her allokeres der plads for et array af char arrays som hver indeholder en csv linje*/

    csvValues *csvValue = calloc(numberOfLines+1,sizeof(csvValues)); /* her allokeres et array af struct som indeholde et array af char arrays(strings)*/

    csvValue[0].numberOfemployees = numberOfLines; /*da at hver linje repræsentere en medarbeder svarer numberOfLines her til antal medarbejderre  */
    for(i=0;i<numberOfLines;i++)
    {   
        
        
        csvLines[i] = fread_csv_line(csvFile,numberOfChars[i],&done,&err); /*læse den første csv linje her */
        csvValue[i].value =parse_csv(csvLines[i]); 

    }
 
    return csvValue;
    
}


employees* AllocateStructs(csvValues* csvValue)
{
    int i,numberofShitfs,timeBetweenShifts;

    employees* employee = malloc(sizeof(employees)*csvValue[0].numberOfemployees);
    for(i=0;i<csvValue[0].numberOfemployees;i++)
    {
        
        numberofShitfs = CountTotaltShifts(csvValue[i].value);
        timeBetweenShifts = numberofShitfs-1; /* der er en mindre tid mellem vagter end der er vagter */
        employee[i].shift = calloc(numberofShitfs+1,sizeof(shifts)); /* addere med en, da char arrays skal være null terminerede.*/
        employee[i].TimeBetween = calloc(timeBetweenShifts+1,sizeof(shifts)); /* samme årsag her */
        csvValue[i].numberOfShifts = numberofShitfs;
        csvValue[i].timeBetweenShifts= numberofShitfs-1;        

    }

    return employee;
}



int CountTotaltShifts(char** CsvString)
{
    int n =0;

    while(*CsvString)
    {
        if(!strchr(*CsvString,'"'))
            n++;

        CsvString++;

    }
    
    return n;
}


int* CountNumberOfCarachtersPerCsvLine(FILE* csvFile,int numberOfLines)
{
    int n =0,i = 0;
    int* numberOfChars = malloc(sizeof(int)*numberOfLines);
    char ch;

    for(i=0;i<numberOfLines;i++)
    {
    n =0;
    
    while( (ch = fgetc(csvFile)) != EOF)
    {
        n++;
        if(ch == '\n')
            break;
       

    } 
   
    numberOfChars[i] = n;

    }

    rewind(csvFile);
    return numberOfChars;
}

void remove_all_chars(char* str, char c) {
    char*pr = str;
    while (*pr) {
        *str = *pr++; /*putter char i pr over i str og inkrementere derefter pr */ 
        if(*str != c) /* check om char ved str ikke er ligmed den char du vil fjerne*/
            str++; /*hvis de ikke er lig hinaden inkrementeres str også */
    }
    *str = '\0';
    
}

 

int CountNumberOfCsvLines(FILE* csvFile)
{
    
    int numberOfCsvLines = 0;
    char ch;
    while((ch = fgetc(csvFile))!=EOF)
    {
        if(ch == '\n')
            numberOfCsvLines++; 
    }
    rewind(csvFile);
    return numberOfCsvLines+1;
}

employees* LoadIntoStruct(csvValues* csvValue,employees* employee)
{
    PutNameInStruct(csvValue,employee);
    PutTimeWorkedInStruct(csvValue,employee);
    return employee;
}
void PutTimeWorkedInStruct(csvValues* csvValue,employees* employee) /* BETA version skal lige have set det her smart op */
{
    int i,j,k,n = 0;

    

    for(i=0;i<csvValue[0].numberOfemployees;i++)
    {
        for(j=0;j<csvValue[0].numberOfShifts;j++) 
        {
            remove_all_chars(csvValue[i].value[j+SHIFTSTART],'-');

            for(k=START_TIME;k<=END_TIME;k++)
            {

                sscanf(csvValue[i].value[j+SHIFTSTART]+n,"%d/%d/%d %d.%d%n",&employee[i].shift[j].date.day[k],
                                                                    &employee[i].shift[j].date.month[k],
                                                                    &employee[i].shift[j].date.year[k],
                                                                    &employee[i].shift[j].date.hour[k],
                                                                    &employee[i].shift[j].date.minute[k],
                                                                    &n
                );
                
            }
    
            
        }
    }

}

void PutNameInStruct(csvValues* csvValue,employees* employee)
{
   int i;
   for (i=0;i<csvValue[0].numberOfemployees;i++)
   {
       
        strcpy(employee[i].employeesName,csvValue[i].value[NAME]);
        remove_all_chars(employee[i].employeesName,'"');
        printf("%s\n",employee[i].employeesName);
   }

}


void PrintAssert(int expression,char* str) 
{
    if(!expression)
    {
        printf("%s",str);
        exit(EXIT_FAILURE);
    }
    
}  
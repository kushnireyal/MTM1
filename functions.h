#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdbool.h>
#include "map.h"
#include "eurovision.h"
#include "state.h"
#include "judge.h"
#include "list.h"

#define SPACECHAR ' '
#define SPACE " "
#define COMMA ","
#define DASH "-"
#define EXTRA_CHARACTERS_IN_STATE_PAIR 4
#define ONE_HUNDREND_PRECENT 1

typedef enum mapType_t {
    STATES_MAP,
    JUDGES_MAP
} MapType;

typedef enum {
    TENTH_PLACE = 1,
    NINTH_PLACE,
    EIGHT_PLACE,
    SEVENTH_PLACE,
    SIXTH_PLACE,
    FIFTH_PLACE,
    FOURTH_PLACE,
    THIRD_PLACE,
    SECOND_PLACE = 10,
    FIRST_PLACE = 12
} Ranking;

const Ranking ranking[NUMBER_OF_STATES_TO_RANK] = {FIRST_PLACE, SECOND_PLACE, THIRD_PLACE, FOURTH_PLACE, FIFTH_PLACE, SIXTH_PLACE, SEVENTH_PLACE, EIGHT_PLACE, NINTH_PLACE, TENTH_PLACE};

typedef struct countData_t {
    int id;
    unsigned int count;
} *CountData;


EurovisionResult isIDValid(Map map, MapType type, int id);
bool isLowerCase(char c);
bool checkValidName(const char* name);
bool resultsContain (Map states, Map judges, int judge_id, int state_id);

int* copyInteger(int num);
int compareIntegers(int a, int b);
List audiencePoints(Map states, int audiencePrecent);

/********************** COUNT LIST FUNCTIONS ***********************/

ListElement copyCountData(ListElement elem);
void freeCountData(ListElement elem);
int compareCountData(ListElement data1, ListElement data2);

List countListCreate(Map map);

// Assumption: Given map has keys of type int*
List countListCreate(Map map);
List convertVotesToList(Map votes);
// Converts final results to list of state names
List convertToStringlist(List finalResults, Map states);
ListElement copyString(ListElement str);
void  freeString(ListElement str);

#endif //FUNCTIONS_H

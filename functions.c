#include <string.h>
#include <stdlib.h>
#include "functions.h"

EurovisionResult isIDValid(Map map, int id) {
    //check ID >= 0
    //check with mapContain if Id already exist
    if (id < 0) {
        return EUROVISION_INVALID_ID;
    } else if (mapContains(map, &id)) {
        return EUROVISION_STATE_ALREADY_EXIST;
    }
    // else state not exist
    return EUROVISION_STATE_NOT_EXIST;
}

bool isLowerCase(char c) {
    return ('a' <= c && c <= 'z');
}

bool checkValidName(char* name) {
    //check the given string only contains small letters and spaces
    for (int i = 0; i < strlen(name); i++) {
        if (!isLowerCase(name[i]) && name[i] != SPACE) return false;
    }
    return true;
}

int* copyInteger(int num) {
    //allocation for int
    int* copy = malloc(sizeof(*copy));
    //initialize the int
    *copy = num;
    //return address
    return copy;
}

int compareIntegers(int a, int b) {
    //compare the ints
    //return 0 if equal
    //return bigger than 0 if first is bigger
    //else the small than 0
    return a - b;
}

bool resultsContain (Eurovision eurovision, int judge_id, int state_id) {
    assert(eurovision != NULL && isIDValid(eurovision->States, state_id) == EUROVISION_STATE_ALREADY_EXIST);
    JudgeDataElement tmp_judge = mapGet(eurovision->Judges, &judge_id);
    assert(tmp_judge != NULL);
    for (int i=0; i < NUMBER_OF_STATES_TO_RANK; i++) {
        if (tmp_judge->states[i] == state_id) {
            return true;
        }
    }
    // else
    return false;
}

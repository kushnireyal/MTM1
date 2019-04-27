#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "functions.h"

/*
 * These are included in functions.h:
 * #include "eurovision.h"
 * #include "map.h"
 * #include "list.h"
 * #include "judge.h"
 * #include "state.h"
*/

struct eurovision_t {
    Map States; // key = State ID, data = State's name, song name and votes it gives
    Map Judges; // key = Judge ID, data = Judge's name and results
};

Eurovision eurovisionCreate() {
    Eurovision eurovision = malloc(sizeof(*eurovision));    // allocate memory for the struct
    if (!eurovision) return NULL;       // allocation failed

    // create the States map using the appropriate functions (from state.h)
    eurovision->States = mapCreate(copyStateDataElement,
                                   copyStateKeyElement,
                                   freeStateDataElement,
                                   freeStateKeyElement,
                                   compareStateKeyElements);
    if (!eurovision->States) {
        free(eurovision);
        return NULL;                    // allocation failed
    }

    // create the Judges map using the appropriate functions (from judge.h)
    eurovision->Judges = mapCreate(copyJudgeDataElement,
                                   copyJudgeKeyElement,
                                   freeJudgeDataElement,
                                   freeJudgeKeyElement,
                                   compareJudgeKeyElements);

    //check mapCreate return value
    if (!eurovision->Judges) {
        mapDestroy(eurovision->States);
        free(eurovision);
        return NULL;                    // allocation failed
    }

    return eurovision;
}

void eurovisionDestroy(Eurovision eurovision) {
    if (eurovision) {
        // destroy the States and Judges maps:
        mapDestroy(eurovision->States);     // votes maps are destroyed in the freeStateDataElement function
        mapDestroy(eurovision->Judges);

        free(eurovision);                   // free the eurovision struct
    }
    // if NULL pointer was received, nothing is done
}

EurovisionResult eurovisionAddState(Eurovision eurovision, int stateId,
                                    const char *stateName,
                                    const char *songName) {
    if (!eurovision || !stateName || !songName) return EUROVISION_NULL_ARGUMENT;    // NULL pointer received

    EurovisionResult result = isIDValid(eurovision->States, STATES_MAP, stateId);
    if (result != EUROVISION_STATE_NOT_EXIST) return result;    // ID not valid or state is already in Eurovision

    if (!isValidName(stateName) || !isValidName(songName)) {
        return EUROVISION_INVALID_NAME;                         // state name or song name not valid
    }

    // temporarily allocate memory for the state's data
    StateData state_data = createStateData(stateName, songName);
    if (!state_data) return EUROVISION_OUT_OF_MEMORY;   // state's data allocation failed

    // add the state to Eurovision's States
    MapResult put_result = mapPut(eurovision->States, &stateId, state_data);

    // deallocate the temporary state data
    freeStateDataElement(state_data);

    if (put_result == MAP_OUT_OF_MEMORY) return EUROVISION_OUT_OF_MEMORY;           // copy in mapPut failed

    return EUROVISION_SUCCESS;
}

EurovisionResult eurovisionRemoveState(Eurovision eurovision, int stateId) {
    if (!eurovision) return EUROVISION_NULL_ARGUMENT;       // NULL pointer received

    EurovisionResult result = isIDValid(eurovision->States, STATES_MAP, stateId);
    if (result != EUROVISION_STATE_ALREADY_EXIST) return result;    // ID not valid or state is not in Eurovision

    // for each state in Eurovision, remove the votes that state has for given stateId
    MAP_FOREACH(int *, id, eurovision->States) {
        StateData state_data = mapGet(eurovision->States, id);
        assert(state_data->votes != NULL);
        mapRemove(state_data->votes, &stateId);
    }

    // for each judge in Eurovision, remove it if it voted for the given stateId
    MAP_FOREACH(int *, judge_id, eurovision->Judges) {
        JudgeData judgeData = mapGet(eurovision->Judges, judge_id);
        if (judgeResultsContain(judgeData, stateId)) {
            eurovisionRemoveJudge(eurovision, *judge_id);
        }
    }

    // Remove the state from Eurovision's States
    mapRemove(eurovision->States, &stateId);

    return EUROVISION_SUCCESS;
}

EurovisionResult eurovisionAddJudge(Eurovision eurovision, int judgeId,
                                    const char *judgeName,
                                    int *judgeResults) {
    if (!eurovision || !judgeName || !judgeResults) return EUROVISION_NULL_ARGUMENT;    // NULL pointer received

    EurovisionResult result = isIDValid(eurovision->Judges, JUDGES_MAP, judgeId);
    if (result != EUROVISION_JUDGE_NOT_EXIST) return result;        // ID not valid or Judge already exists in Eurovision

    if (!isValidName(judgeName)) return EUROVISION_INVALID_NAME;    // judge name not valid

    // check valid judge results
    for (int i=0; i < NUMBER_OF_RANKINGS; i++) {
        result = isIDValid(eurovision->States, STATES_MAP, judgeResults[i]);
        if (result != EUROVISION_STATE_ALREADY_EXIST) return result;        // invalid state ID in judge's results
    }

    // temporarily allocate memory for the judge's data
    JudgeData judge_data = createJudgeData(judgeName, judgeResults);
    if (!judge_data) return EUROVISION_OUT_OF_MEMORY;   // judge's data allocation failed

    // add the judge to Eurovision's Judges
    MapResult put_result = mapPut(eurovision->Judges, &judgeId, judge_data);

    // deallocate the temporary judge data
    freeJudgeDataElement(judge_data);

    if (put_result == MAP_OUT_OF_MEMORY) return EUROVISION_OUT_OF_MEMORY;   // copy in mapPut failed

    return EUROVISION_SUCCESS;
}

EurovisionResult eurovisionRemoveJudge(Eurovision eurovision, int judgeId) {
    if (!eurovision) return EUROVISION_NULL_ARGUMENT;       // NULL pointer received

    EurovisionResult result = isIDValid(eurovision->Judges,  JUDGES_MAP, judgeId);
   if (result != EUROVISION_JUDGE_ALREADY_EXIST) return result;    // ID not valid or judge is not in Eurovision

    // Remove the judge from Eurovision's Judges
    mapRemove(eurovision->Judges, &judgeId);

    return EUROVISION_SUCCESS;
}

EurovisionResult eurovisionAddVote(Eurovision eurovision, int stateGiver,
                                   int stateTaker) {
    // add one vote to stateTaker in the stateGiver's votes map
    return eurovisionChangeVote(eurovision->States, stateGiver, stateTaker, 1);
}


EurovisionResult eurovisionRemoveVote(Eurovision eurovision, int stateGiver,
                                      int stateTaker) {
    // remove one vote from stateTaker in the stateGiver's votes map
    return eurovisionChangeVote(eurovision->States, stateGiver, stateTaker, -1);
}

List eurovisionRunContest(Eurovision eurovision, int audiencePercent) {
    if (!eurovision || audiencePercent > 100 || audiencePercent < 0) return NULL;   // invalid parameter received

    // if state map is empty return empty string List
    if (mapGetFirst(eurovision->States) == NULL) return listCreate(copyString, freeString);

    // get the points each state got from the audience
    List points_list = getAudiencePoints(eurovision->States);
    if (!points_list) return NULL;

    // get the list of points each state got from the judges
    List judge_points = getJudgesPoints(eurovision->Judges, eurovision->States);
    if (!judge_points) {
        listDestroy(points_list);
        return NULL;
    }

    int judge_percent = 100 - audiencePercent;
    int num_of_states = mapGetSize(eurovision->States);
    int num_of_judges = mapGetSize(eurovision->Judges);

    // Add judge points to audience points
    StatePoints judge_points_ptr = listGetFirst(judge_points);
    LIST_FOREACH(StatePoints, point_data, points_list) {
        assert(judge_points_ptr != NULL);

        // Divide each state's audience points by the number of states
        point_data->points /= num_of_states;
        // Multiply each state's audience points by audience percentage
        point_data->points *= audiencePercent;
        // Divide each state's judge points by the number of judges
        judge_points_ptr->points /= num_of_judges;
        // Multiply each state's judge points by audience percentage
        judge_points_ptr->points *= judge_percent;

        // Add judge points to audience points
        point_data->points += judge_points_ptr->points;

        // increment judge_points pointer
        judge_points_ptr = listGetNext(judge_points);
    }

    listDestroy(judge_points);      // deallocate the judge points list

    // sort the final points list
    if (listSort(points_list, compareStatePoints) != LIST_SUCCESS) {
        listDestroy(points_list);
        return NULL;                // sort failed
    }

    // convert sorted points list to string list of states' names
    List final_results = convertToStringList(points_list, eurovision->States);

    listDestroy(points_list);       // deallocate the points list

    return final_results;
}

List eurovisionRunAudienceFavorite(Eurovision eurovision) {
    if (!eurovision) return NULL;   // NULL pointer received

    // get the points each state got from the audience
    List audience_points = getAudiencePoints(eurovision->States);
    if (!audience_points) return NULL;  // error in getAudiencePoints function

    // sort the list
    if (listSort(audience_points, compareStatePoints) != LIST_SUCCESS) {
        listDestroy(audience_points);
        return NULL;                    // list sort failed
    }

    // convert sorted points list to sorted string list of states' names
    List final_results = convertToStringList(audience_points, eurovision->States);

    listDestroy(audience_points);       // deallocate the audience points list

    return final_results;
}

List eurovisionRunGetFriendlyStates(Eurovision eurovision) {
    // if state map is empty return empty List
    if (mapGetSize(eurovision->States) == 0) return listCreate(copyString, freeString);

    List friendly_states = getFriendlyStates(eurovision->States);
    if (!friendly_states) return NULL;      // error in getFriendlyStates function

    // sort the strings array lexicographically
    if (listSort(friendly_states, stringCompare) != LIST_SUCCESS) {
        listDestroy(friendly_states);
        return NULL;                        // list sort failed
    }

    return friendly_states;
}


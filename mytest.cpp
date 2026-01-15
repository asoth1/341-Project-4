// CMSC 341 - Fall 2025 - Project 4
// mytest.cpp - Test file for Cache class
#include "cache.h"
#include <math.h>
#include <algorithm>
#include <random>
#include <vector>
using namespace std;

const int MINSEARCH = 0;
const int MAXSEARCH = 7;
// the following array defines sample search strings for testing
string searchStr[MAXSEARCH+1]={"c++","python","java","scheme","prolog","c#","c","js"};

enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE};
class Random {
public:
    Random(){}
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            m_generator = mt19937(m_device());
            m_normdist = normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            m_generator = mt19937(10);
            m_unidist = uniform_int_distribution<>(min,max);
        }
        else if (type == UNIFORMREAL) {
            m_generator = mt19937(10);
            m_uniReal = uniform_real_distribution<double>((double)min,(double)max);
        }
        else {
            m_generator = mt19937(m_device());
        }
    }
    void setSeed(int seedNum){
        m_generator = mt19937(seedNum);
    }
    void init(int min, int max){
        m_min = min;
        m_max = max;
        m_type = UNIFORMINT;
        m_generator = mt19937(10);
        m_unidist = uniform_int_distribution<>(min,max);
    }
    void getShuffle(vector<int> & array){
        for (int i = m_min; i<=m_max; i++){
            array.push_back(i);
        }
        shuffle(array.begin(),array.end(),m_generator);
    }

    void getShuffle(int array[]){
        vector<int> temp;
        for (int i = m_min; i<=m_max; i++){
            temp.push_back(i);
        }
        shuffle(temp.begin(), temp.end(), m_generator);
        vector<int>::iterator it;
        int i = 0;
        for (it=temp.begin(); it != temp.end(); it++){
            array[i] = *it;
            i++;
        }
    }

    int getRandNum(){
        int result = 0;
        if(m_type == NORMAL){
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum(){
        double result = m_uniReal(m_generator);
        result = floor(result*100.0)/100.0;
        return result;
    }

    string getRandString(int size){
        string output = "";
        for (int i=0;i<size;i++){
            output = output + (char)getRandNum();
        }
        return output;
    }

    int getMin(){return m_min;}
    int getMax(){return m_max;}
private:
    int m_min;
    int m_max;
    RANDOM m_type;
    random_device m_device;
    mt19937 m_generator;
    normal_distribution<> m_normdist;
    uniform_int_distribution<> m_unidist;
    uniform_real_distribution<double> m_uniReal;
};

// Hash function
unsigned int hashCode(const string str) {
    unsigned int val = 0;
    const unsigned int thirtyThree = 33;
    for (int i = 0; i < (int)(str.length()); i++)
        val = val * thirtyThree + str[i];
    return val;
}

class Tester {
public:
    // Test insertion with non-colliding keys
    bool testInsertNonColliding();
    // Test insertion with colliding keys
    bool testInsertColliding();
    // Test getPerson for error case (not found)
    bool testGetPersonNotFound();
    // Test getPerson with non-colliding keys
    bool testGetPersonNonColliding();
    // Test getPerson with colliding keys
    bool testGetPersonColliding();
    // Test remove with non-colliding keys
    bool testRemoveNonColliding();
    // Test remove with colliding keys
    bool testRemoveColliding();
    // Test rehash triggered by insertion (load factor)
    bool testRehashTriggeredByInsertion();
    // Test rehash completion after load factor trigger
    bool testRehashCompletionAfterInsertion();
    // Test rehash triggered by removal (delete ratio)
    bool testRehashTriggeredByRemoval();
    // Test rehash completion after delete ratio trigger
    bool testRehashCompletionAfterRemoval();

    // Additional tests (12-21)
    // Test updateID normal case
    bool testUpdateIDNormal();
    // Test updateID error case (invalid ID)
    bool testUpdateIDError();
    // Test insert with invalid ID (below MINID)
    bool testInsertInvalidIDBelowMin();
    // Test insert with invalid ID (above MAXID)
    bool testInsertInvalidIDAboveMax();
    // Test remove non-existent person
    bool testRemoveNonExistent();
    // Test duplicate insertion prevention
    bool testDuplicatePrevention();
    // Test probing policy change
    bool testProbingPolicyChange();
    // Test linear probing
    bool testLinearProbing();
    // Test quadratic probing
    bool testQuadraticProbing();
    // Test lambda and deletedRatio calculations
    bool testLambdaAndDeletedRatio();

private:
    // Helper function to generate unique keys for non-colliding tests
    string generateUniqueKey(int index);
};

// Generate unique keys that won't collide
string Tester::generateUniqueKey(int index) {
    string base = "key";
    string result = base;
    for (int i = 0; i < index; i++) {
        result = result + "x";
    }
    return result;
}

// Test 1: Test insertion with non-colliding keys (50 nodes)
// Tests that 50 persons with unique keys can be inserted and retrieved correctly
bool Tester::testInsertNonColliding() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    vector<Person> dataList;
    bool result = true;

    // Insert 50 non-colliding data points
    for (int i = 0; i < 50; i++) {
        string key = generateUniqueKey(i);
        int id = RndID.getRandNum();
        Person person(key, id, true);
        dataList.push_back(person);

        if (!cache.insert(person)) {
            result = false;
        }
    }

    // Verify all data was inserted correctly by finding each person
    for (int i = 0; i < 50; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == dataList[i])) {
            result = false;
        }
    }

    return result;
}

// Test 2: Test insertion with colliding keys (50 nodes, half colliding)
// Tests that 50 persons with only 4 unique keys (causing collisions) can be inserted and retrieved
bool Tester::testInsertColliding() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    vector<Person> dataList;
    bool result = true;

    // Insert 50 data points, using only 4 different keys to force collisions
    for (int i = 0; i < 50; i++) {
        // Use only searchStr[0] through searchStr[3] to create collisions
        string key = searchStr[i % 4];
        int id = MINID + i; // Unique IDs to differentiate
        Person person(key, id, true);
        dataList.push_back(person);

        if (!cache.insert(person)) {
            result = false;
        }
    }

    // Verify all data was inserted correctly
    for (int i = 0; i < 50; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == dataList[i])) {
            result = false;
        }
    }

    return result;
}

// Test 3: Test getPerson for error case (Person not in database)
// Tests that getPerson returns an empty Person when searching for a non-existent person
bool Tester::testGetPersonNotFound() {
    Random RndID(MINID, MAXID);
    Random RndStr(MINSEARCH, MAXSEARCH);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);

    // Insert some data
    for (int i = 0; i < 50; i++) {
        Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum(), true);
        cache.insert(person);
    }

    // Try to find a person that doesn't exist
    Person notFound = cache.getPerson("nonexistentkey", 99999);

    // Should return an empty Person
    Person emptyPerson;
    return (notFound == emptyPerson);
}

// Test 4: Test getPerson with non-colliding keys
// Tests that getPerson correctly finds all 50 persons inserted with unique keys
bool Tester::testGetPersonNonColliding() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    vector<Person> dataList;
    bool result = true;

    // Insert 50 non-colliding data points
    for (int i = 0; i < 50; i++) {
        string key = generateUniqueKey(i);
        int id = RndID.getRandNum();
        Person person(key, id, true);
        dataList.push_back(person);
        cache.insert(person);
    }

    // Find each person and verify
    for (int i = 0; i < 50; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == dataList[i])) {
            result = false;
        }
    }

    return result;
}

// Test 5: Test getPerson with colliding keys (without triggering rehash)
// Tests that getPerson correctly finds persons when collisions occur (using only 4 keys)
bool Tester::testGetPersonColliding() {
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    vector<Person> dataList;
    bool result = true;

    // Insert fewer items to avoid rehash, using colliding keys
    // Keep under 50% load factor
    int numItems = 20;
    for (int i = 0; i < numItems; i++) {
        string key = searchStr[i % 4]; // Only 4 unique keys = collisions
        int id = MINID + i;
        Person person(key, id, true);
        dataList.push_back(person);
        cache.insert(person);
    }

    // Find each person with colliding keys
    for (int i = 0; i < numItems; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == dataList[i])) {
            result = false;
        }
    }

    return result;
}

// Test 6: Test remove with non-colliding keys
// Tests that remove works correctly with unique keys and remaining items are still accessible
bool Tester::testRemoveNonColliding() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    vector<Person> dataList;
    bool result = true;

    // Insert 50 non-colliding data points
    for (int i = 0; i < 50; i++) {
        string key = generateUniqueKey(i);
        int id = RndID.getRandNum();
        Person person(key, id, true);
        dataList.push_back(person);
        cache.insert(person);
    }

    // Remove first 10 items
    for (int i = 0; i < 10; i++) {
        if (!cache.remove(dataList[i])) {
            result = false;
        }
    }

    // Verify removed items are not found
    Person emptyPerson;
    for (int i = 0; i < 10; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == emptyPerson)) {
            result = false;
        }
    }

    // Verify remaining items still exist
    for (int i = 10; i < 50; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == dataList[i])) {
            result = false;
        }
    }

    return result;
}

// Test 7: Test remove with colliding keys (without triggering rehash)
// Tests that remove works correctly when collisions exist and remaining items are still accessible
bool Tester::testRemoveColliding() {
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    vector<Person> dataList;
    bool result = true;

    // Insert items with colliding keys
    int numItems = 20;
    for (int i = 0; i < numItems; i++) {
        string key = searchStr[i % 4];
        int id = MINID + i;
        Person person(key, id, true);
        dataList.push_back(person);
        cache.insert(person);
    }

    // Remove 5 items
    for (int i = 0; i < 5; i++) {
        if (!cache.remove(dataList[i])) {
            result = false;
        }
    }

    // Verify removed items are not found
    Person emptyPerson;
    for (int i = 0; i < 5; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == emptyPerson)) {
            result = false;
        }
    }

    // Verify remaining items still exist
    for (int i = 5; i < numItems; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == dataList[i])) {
            result = false;
        }
    }

    return result;
}

// Test 8: Test rehash is triggered after insertion (load factor > 0.5)
// Tests that inserting more than 50% capacity triggers a rehash (table grows, lambda decreases)
bool Tester::testRehashTriggeredByInsertion() {
    Random RndID(MINID, MAXID);
    Random RndStr(MINSEARCH, MAXSEARCH);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);

    // MINPRIME is 101, so inserting more than 50 items should trigger rehash
    // Insert enough items to exceed 50% load factor
    for (int i = 0; i < 55; i++) {
        Person person(searchStr[RndStr.getRandNum()], RndID.getRandNum(), true);
        cache.insert(person);
    }

    // After inserting 55 items into table of size 101, load factor > 0.5
    // Rehash should have been triggered
    // Check that lambda is now below 0.5 (rehash happened and table grew)
    float loadFactor = cache.lambda();

    // After rehash, load factor should be lower because table size increased
    return (loadFactor < 0.5);
}

// Test 9: Test rehash completion after load factor trigger
// Tests that all data is preserved and accessible after rehash completes
bool Tester::testRehashCompletionAfterInsertion() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    vector<Person> dataList;
    bool result = true;

    // Insert enough items to trigger rehash and complete it
    for (int i = 0; i < 60; i++) {
        string key = generateUniqueKey(i);
        int id = RndID.getRandNum();
        Person person(key, id, true);
        dataList.push_back(person);
        cache.insert(person);
    }

    // Continue inserting to ensure rehash completes (transfers all data)
    for (int i = 60; i < 80; i++) {
        string key = generateUniqueKey(i);
        int id = RndID.getRandNum();
        Person person(key, id, true);
        dataList.push_back(person);
        cache.insert(person);
    }

    // Verify all data can still be found after rehash completion
    for (int i = 0; i < 80; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == dataList[i])) {
            result = false;
        }
    }

    return result;
}

// Test 10: Test rehash is triggered by removal (delete ratio > 0.8)
// Tests that removing more than 80% of items triggers a rehash (deleted ratio decreases)
bool Tester::testRehashTriggeredByRemoval() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    vector<Person> dataList;

    // Insert 50 items
    for (int i = 0; i < 50; i++) {
        string key = generateUniqueKey(i);
        int id = RndID.getRandNum();
        Person person(key, id, true);
        dataList.push_back(person);
        cache.insert(person);
    }

    // Remove more than 80% of items to trigger rehash due to delete ratio
    for (int i = 0; i < 41; i++) {
        cache.remove(dataList[i]);
    }

    // After removing 40 out of 50 items, delete ratio should trigger rehash
    // Check that deleted ratio is now low (rehash cleaned up deleted entries)
    float delRatio = cache.deletedRatio();

    // After rehash, deleted ratio should be 0 or very low
    return (delRatio < 0.8);
}

// Test 11: Test rehash completion after delete ratio trigger
// Tests that remaining and new data is accessible after rehash triggered by deletions
bool Tester::testRehashCompletionAfterRemoval() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    vector<Person> dataList;
    bool result = true;

    // Insert 50 items
    for (int i = 0; i < 50; i++) {
        string key = generateUniqueKey(i);
        int id = RndID.getRandNum();
        Person person(key, id, true);
        dataList.push_back(person);
        cache.insert(person);
    }

    // Remove many items to trigger rehash
    for (int i = 0; i < 40; i++) {
        cache.remove(dataList[i]);
    }

    // Perform more operations to complete rehash transfer
    for (int i = 50; i < 60; i++) {
        string key = generateUniqueKey(i);
        int id = RndID.getRandNum();
        Person person(key, id, true);
        dataList.push_back(person);
        cache.insert(person);
    }

    // Verify remaining original data still exists
    for (int i = 40; i < 50; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == dataList[i])) {
            result = false;
        }
    }

    // Verify new data exists
    for (int i = 50; i < 60; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == dataList[i])) {
            result = false;
        }
    }

    // Verify removed data does not exist
    Person emptyPerson;
    for (int i = 0; i < 40; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == emptyPerson)) {
            result = false;
        }
    }

    return result;
}

// Test 12: Test updateID normal case
// Tests that updateID successfully changes a person's ID and the person
// can be found with the new ID but not the old ID
bool Tester::testUpdateIDNormal() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    bool result = true;

    // Insert a person
    string key = "testkey";
    int oldID = RndID.getRandNum();
    Person person(key, oldID, true);
    cache.insert(person);

    // Update the ID
    int newID = RndID.getRandNum();
    if (!cache.updateID(person, newID)) {
        result = false;
    }

    // Verify person can be found with new ID
    Person found = cache.getPerson(key, newID);
    if (found.getKey() != key || found.getID() != newID) {
        result = false;
    }

    // Verify person cannot be found with old ID
    Person notFound = cache.getPerson(key, oldID);
    Person emptyPerson;
    if (!(notFound == emptyPerson)) {
        result = false;
    }

    return result;
}

// Test 13: Test updateID error case (invalid ID)
// Tests that updateID returns false for invalid IDs (out of range) and non-existent persons
bool Tester::testUpdateIDError() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    bool result = true;

    // Insert a person
    Person person("testkey", RndID.getRandNum(), true);
    cache.insert(person);

    // Try to update with ID below MINID - should fail
    if (cache.updateID(person, MINID - 1)) {
        result = false;
    }

    // Try to update with ID above MAXID - should fail
    if (cache.updateID(person, MAXID + 1)) {
        result = false;
    }

    // Try to update non-existent person - should fail
    Person fakePerson("nonexistent", 999999, true);
    if (cache.updateID(fakePerson, RndID.getRandNum())) {
        result = false;
    }

    return result;
}

// Test 14: Test insert with invalid ID (below MINID)
// Tests that insert returns false when given an ID below the minimum allowed
bool Tester::testInsertInvalidIDBelowMin() {
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);

    // Try to insert with ID below MINID - should fail
    Person person("testkey", MINID - 1, true);
    return !cache.insert(person);
}

// Test 15: Test insert with invalid ID (above MAXID)
// Tests that insert returns false when given an ID above the maximum allowed
bool Tester::testInsertInvalidIDAboveMax() {
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);

    // Try to insert with ID above MAXID - should fail
    Person person("testkey", MAXID + 1, true);
    return !cache.insert(person);
}

// Test 16: Test remove non-existent person
// Tests that remove returns false when trying to remove a person not in the cache
bool Tester::testRemoveNonExistent() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    bool result = true;

    // Insert some data
    for (int i = 0; i < 50; i++) {
        Person person(searchStr[i % 8], RndID.getRandNum(), true);
        cache.insert(person);
    }

    // Try to remove a person that doesn't exist - should fail
    Person fakePerson("nonexistent", 999999, true);
    if (cache.remove(fakePerson)) {
        result = false;
    }

    return result;
}

// Test 17: Test duplicate insertion prevention
// Tests that inserting the same person twice is rejected (second insert returns false)
bool Tester::testDuplicatePrevention() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    bool result = true;

    // Insert a person
    int id = RndID.getRandNum();
    Person person("testkey", id, true);
    if (!cache.insert(person)) {
        result = false;
    }

    // Try to insert the same person again - should fail
    if (cache.insert(person)) {
        result = false;
    }

    return result;
}

// Test 18: Test probing policy change
// Tests that changing the probing policy during operation preserves all data
bool Tester::testProbingPolicyChange() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, QUADRATIC);
    vector<Person> dataList;
    bool result = true;

    // Insert enough data to trigger rehash
    for (int i = 0; i < 60; i++) {
        string key = generateUniqueKey(i);
        int id = RndID.getRandNum();
        Person person(key, id, true);
        dataList.push_back(person);
        cache.insert(person);
    }

    // Change probing policy
    cache.changeProbPolicy(LINEAR);

    // Insert more to complete rehash with new policy
    for (int i = 60; i < 80; i++) {
        string key = generateUniqueKey(i);
        int id = RndID.getRandNum();
        Person person(key, id, true);
        dataList.push_back(person);
        cache.insert(person);
    }

    // Verify all data is still accessible
    for (int i = 0; i < 80; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == dataList[i])) {
            result = false;
        }
    }

    return result;
}

// Test 19: Test linear probing
// Tests that linear probing correctly handles collisions and all data is retrievable
bool Tester::testLinearProbing() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, LINEAR);
    vector<Person> dataList;
    bool result = true;

    // Insert 50 items with colliding keys to test linear probing
    for (int i = 0; i < 50; i++) {
        string key = searchStr[i % 4];  // Use only 4 keys to force collisions
        int id = MINID + i;
        Person person(key, id, true);
        dataList.push_back(person);
        if (!cache.insert(person)) {
            result = false;
        }
    }

    // Verify all items can be found
    for (int i = 0; i < 50; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == dataList[i])) {
            result = false;
        }
    }

    return result;
}

// Test 20: Test quadratic probing
// Tests that quadratic probing correctly handles collisions and all data is retrievable
bool Tester::testQuadraticProbing() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, QUADRATIC);
    vector<Person> dataList;
    bool result = true;

    // Insert 50 items with colliding keys to test quadratic probing
    for (int i = 0; i < 50; i++) {
        string key = searchStr[i % 4];  // Use only 4 keys to force collisions
        int id = MINID + i;
        Person person(key, id, true);
        dataList.push_back(person);
        if (!cache.insert(person)) {
            result = false;
        }
    }

    // Verify all items can be found
    for (int i = 0; i < 50; i++) {
        Person found = cache.getPerson(dataList[i].getKey(), dataList[i].getID());
        if (!(found == dataList[i])) {
            result = false;
        }
    }

    return result;
}

// Test 21: Test lambda and deletedRatio calculations
// Tests that lambda (load factor) and deletedRatio are calculated correctly
bool Tester::testLambdaAndDeletedRatio() {
    Random RndID(MINID, MAXID);
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    vector<Person> dataList;
    bool result = true;

    // Initially, lambda and deletedRatio should be 0
    if (cache.lambda() != 0.0f) {
        result = false;
    }
    if (cache.deletedRatio() != 0.0f) {
        result = false;
    }

    // Insert 10 items
    for (int i = 0; i < 10; i++) {
        string key = generateUniqueKey(i);
        int id = RndID.getRandNum();
        Person person(key, id, true);
        dataList.push_back(person);
        cache.insert(person);
    }

    // Check lambda is approximately 10/101
    float expectedLambda = 10.0f / MINPRIME;
    if (cache.lambda() < expectedLambda - 0.01f || cache.lambda() > expectedLambda + 0.01f) {
        result = false;
    }

    // Delete 5 items
    for (int i = 0; i < 5; i++) {
        cache.remove(dataList[i]);
    }

    // Check deletedRatio is approximately 5/10 = 0.5
    float expectedRatio = 0.5f;
    if (cache.deletedRatio() < expectedRatio - 0.01f || cache.deletedRatio() > expectedRatio + 0.01f) {
        result = false;
    }

    return result;
}

int main() {
    Tester tester;

    cout << "Testing Cache Class" << endl;
    cout << "===================" << endl << endl;

    // Test 1: Insert non-colliding keys
    cout << "Test 1: Insert with non-colliding keys (50 nodes): ";
    if (tester.testInsertNonColliding()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 2: Insert colliding keys
    cout << "Test 2: Insert with colliding keys (50 nodes): ";
    if (tester.testInsertColliding()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 3: getPerson error case
    cout << "Test 3: getPerson for non-existent person: ";
    if (tester.testGetPersonNotFound()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 4: getPerson with non-colliding keys
    cout << "Test 4: getPerson with non-colliding keys: ";
    if (tester.testGetPersonNonColliding()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 5: getPerson with colliding keys
    cout << "Test 5: getPerson with colliding keys (no rehash): ";
    if (tester.testGetPersonColliding()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 6: Remove non-colliding keys
    cout << "Test 6: Remove with non-colliding keys: ";
    if (tester.testRemoveNonColliding()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 7: Remove colliding keys
    cout << "Test 7: Remove with colliding keys (no rehash): ";
    if (tester.testRemoveColliding()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 8: Rehash triggered by insertion
    cout << "Test 8: Rehash triggered by insertion (load factor): ";
    if (tester.testRehashTriggeredByInsertion()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 9: Rehash completion after insertion
    cout << "Test 9: Rehash completion after load factor trigger: ";
    if (tester.testRehashCompletionAfterInsertion()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 10: Rehash triggered by removal
    cout << "Test 10: Rehash triggered by removal (delete ratio): ";
    if (tester.testRehashTriggeredByRemoval()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 11: Rehash completion after removal
    cout << "Test 11: Rehash completion after delete ratio trigger: ";
    if (tester.testRehashCompletionAfterRemoval()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 12: updateID normal case
    cout << "Test 12: updateID normal case: ";
    if (tester.testUpdateIDNormal()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 13: updateID error case
    cout << "Test 13: updateID error cases (invalid ID): ";
    if (tester.testUpdateIDError()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 14: Insert with invalid ID below MINID
    cout << "Test 14: Insert with invalid ID (below MINID): ";
    if (tester.testInsertInvalidIDBelowMin()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 15: Insert with invalid ID above MAXID
    cout << "Test 15: Insert with invalid ID (above MAXID): ";
    if (tester.testInsertInvalidIDAboveMax()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 16: Remove non-existent person
    cout << "Test 16: Remove non-existent person: ";
    if (tester.testRemoveNonExistent()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 17: Duplicate insertion prevention
    cout << "Test 17: Duplicate insertion prevention: ";
    if (tester.testDuplicatePrevention()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 18: Probing policy change
    cout << "Test 18: Probing policy change: ";
    if (tester.testProbingPolicyChange()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 19: Linear probing
    cout << "Test 19: Linear probing with collisions: ";
    if (tester.testLinearProbing()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 20: Quadratic probing
    cout << "Test 20: Quadratic probing with collisions: ";
    if (tester.testQuadraticProbing()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    // Test 21: Lambda and deletedRatio calculations
    cout << "Test 21: Lambda and deletedRatio calculations: ";
    if (tester.testLambdaAndDeletedRatio()) {
        cout << "PASSED" << endl;
    } else {
        cout << "FAILED" << endl;
    }

    cout << endl << "All tests completed." << endl;

    return 0;
}

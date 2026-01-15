// CMSC 341 - Fall 25 - Project 4
#ifndef CACHE_H
#define CACHE_H
#include <iostream>
#include <string>
#include "math.h"
using namespace std;
class Grader;   // forward declaration, will be used for grdaing
class Tester;   // forward declaration, will be used for testing
class Person;   // forward declaration
class Cache;    // forward declaration
const int MINPRIME = 101;   // Min size for hash table
const int MAXPRIME = 99991; // Max size for hash table
const int MINID = 100000;
const int MAXID = 999999;
typedef unsigned int (*hash_fn)(string); // declaration of hash function
enum prob_t {QUADRATIC, DOUBLEHASH, LINEAR}; // types of collision handling policy
#define DEFPOLCY QUADRATIC

class Person{
    public:
    friend class Grader;
    friend class Tester;
    friend class Cache;
    Person(string key="", int id=0, bool used=false){
        m_key = key; m_id = id; m_used=used;
    }
    string getKey() const {return m_key;}
    int getID() const {return m_id;}
    void setKey(string key){m_key=key;}
    void setID(int id){m_id=id;}
    bool getUsed() const {return m_used;}
    void setUsed(bool used) {m_used=used;}
    // the following function is a friend function
    friend ostream& operator<<(ostream& sout, const Person *person ){
        if ((person != nullptr) && !(person->getKey().empty()))
            sout << person->getKey() << " (" << person->getID() << ", "<< person->getUsed() <<  ")";
        else
            sout << "";
        return sout;
    }
    // the following function is a friend function
    friend bool operator==(const Person& lhs, const Person& rhs){
        // since the uniqueness of an object is defined by sequence and ID
        // the equality operator considers only those two criteria
        return ((lhs.getKey() == rhs.getKey()) && (lhs.getID() == rhs.getID()));
    }
    // the following function is a class function
    bool operator==(const Person* & rhs){
        // since the uniqueness of an object is defined by sequence and ID
        // the equality operator considers only those two criteria
        return ((getKey() == rhs->getKey()) && (getID() == rhs->getID()));
    }
    const Person& operator=(const Person& rhs){
        if (this != &rhs){
            m_key = rhs.m_key;
            m_id = rhs.m_id;
            m_used = rhs.m_used;
        }
        return *this;
    }
    private:
    string m_key;   // the search string used as key in the hash table
    int m_id;       // a unique ID number identifying the object
    // the following variable is used for lazy delete scheme in hash table
    // if it is set to false, it means the bucket in the hash table is free for insert
    // if it is set to true, it means the bucket contains live data, and we cannot overwrite it
    bool m_used;
};
class Cache{
    public:
    friend class Grader;
    friend class Tester;
    Cache(int size, hash_fn hash, prob_t probing);
    ~Cache();
    // Returns Load factor of the new table
    float lambda() const;
    // Returns the ratio of deleted slots in the new table
    float deletedRatio() const;
    // insert only happens in the new table
    bool insert(Person person);
    // remove can happen from either table
    bool remove(Person person);
    // find can happen in either table
    const Person getPerson(string key, int id) const;
    // update the information
    bool updateID(Person person, int ID);
    void changeProbPolicy(prob_t policy);
    void dump() const;
    private:
    hash_fn    m_hash;          // hash function
    prob_t     m_newPolicy;     // stores the change of policy request

    Person**   m_currentTable;  // hash table
    int        m_currentCap;    // hash table size (capacity)
    int        m_currentSize;   // current number of entries
                                // m_currentSize includes deleted entries 
    int        m_currNumDeleted;// number of deleted entries
    prob_t     m_currProbing;   // collision handling policy

    Person**   m_oldTable;      // hash table
    int        m_oldCap;        // hash table size (capacity)
    int        m_oldSize;       // current number of entries
                                // m_oldSize includes deleted entries
    int        m_oldNumDeleted; // number of deleted entries
    prob_t     m_oldProbing;    // collision handling policy

    int        m_transferIndex; // this can be used as a temporary place holder
                                // during incremental transfer to scanning the table

    //private helper functions
    bool isPrime(int number);
    int findNextPrime(int current);

    /******************************************
    * Private function declarations go here! *
    ******************************************/
    void incrementalTransfer();
    int probeIndex(int baseIndex, int i, prob_t policy, int cap, hash_fn hash, const string& key) const;
    void startRehash();
    
};
#endif
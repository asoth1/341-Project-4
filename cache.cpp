// CMSC 341 - Fall 25 - Project 4
// student: Andrew Soth
// professor: Kartchner

#include "cache.h"
// parameterized constructor - takes input and assigns the parameter to the right data members
Cache::Cache(int size, hash_fn hash, prob_t probing = DEFPOLCY){
    // stores hash function and probing policy
    m_hash = hash;
    m_currProbing = probing;
    m_newPolicy = probing;  // same as current at the start

    // find a prime >= requested size
    // Handle edge cases for size bounds
    if (size < MINPRIME) {
        m_currentCap = MINPRIME;
    } else if (size > MAXPRIME) {
        m_currentCap = MAXPRIME;
    } else if (isPrime(size)) {
        m_currentCap = size;
    } else {
        m_currentCap = findNextPrime(size);
    }

    // allocate the current table
    m_currentTable = new Person*[m_currentCap];
    for (int i = 0; i < m_currentCap; i++) {
        m_currentTable[i] = nullptr;
    }

    // initialize the counters
    m_currentSize = 0;
    m_currNumDeleted = 0;

    // old table starts empty
    m_oldTable = nullptr;
    m_oldCap = 0;
    m_oldSize = 0;
    m_oldNumDeleted = 0;
    m_oldProbing = probing;

    // transfer index for incremental rehashing
    m_transferIndex = 0;
}

// destructor - deletes the Person objects in the array and the deallocate memory for the table
Cache::~Cache(){
    // clean up the current table
    if(m_currentTable != nullptr) {
        for (int i = 0; i < m_currentCap; i++) {    // traverses through the whole current table
            if (m_currentTable[i] != nullptr) {     // checks if not null, then the following steps:
                delete m_currentTable[i];           // delete Person object
                m_currentTable[i] = nullptr;        // sets the index in the table to null
            }
        }
        delete[] m_currentTable;                    // delete array of Person* pointers
        m_currentTable = nullptr;                   // sets the table to null
    }

    // clean up the old table (if rehashing was in progress)
    if (m_oldTable != nullptr) {
        for (int i = 0; i < m_oldCap; i++) {        // traverses through the whole old table
            if (m_oldTable[i] != nullptr) {         // checks if not null, then the following steps:
                delete m_oldTable[i];               // delete Person object
                m_oldTable[i] = nullptr;            // sets the index in the table to null
            }
        }
        delete[] m_oldTable;                        // delete array of Person* objects
        m_oldTable = nullptr;                       // sets the table to null
    }   
}

// sets the parameter value to the data member value m_newPolicy
void Cache::changeProbPolicy(prob_t policy){
    // store the new policy request
    m_newPolicy = policy;
}

// inserts a Person object into the hash table
// returns true if insertion succeeds, false otherwise
bool Cache::insert(Person person){
    // validate ID
    // checks if the Person object id is within the allowed range
    if (person.getID() < MINID || person.getID() > MAXID) {
        return false;
    }

    // check duplicates
    if (getPerson(person.getKey(), person.getID()).getUsed()) {
        return false;
    }

    // compute the initial index
    // hash the key and map it into the current table
    int index = m_hash(person.getKey()) % m_currentCap;
    int i = 0;
    int newIndex = index;

    // collision resolution
    while (true) {
        // if slot is empty or marked unused, insert
        if (m_currentTable[newIndex] == nullptr || !m_currentTable[newIndex]->getUsed()) {
            // allocate a new Person object if slot is empty
            if (m_currentTable[newIndex] == nullptr) {
                m_currentTable[newIndex] = new Person();
            }
            // copy the Person data into the slot
            *m_currentTable[newIndex] = person;
            m_currentTable[newIndex]->setUsed(true);
            m_currentSize++;
            break;
        }

        // probe the next index
        i++;
        newIndex = probeIndex(index, i, m_currProbing, m_currentCap, m_hash, person.getKey());

        // checks if the entire hash table has been probed
        if (i >= m_currentCap) {
            return false;   // table is full
        }
    }

    // check the rehash criteria
    // checks if the rehash is already in progress
    // if loads exceed 0.5, start rehashing into a larger table
    if (m_oldTable == nullptr) {
        float load = lambda();
        if (load > 0.5f) {
            startRehash();
        }
    }
    
    // moves portions of the elements from the old table into a new one
    incrementalTransfer();

    return true;    // successfully inserted to the table
}

// searches through the hash table to find the Person object in question to remove if it exist
// returns true if successful, false otherwise
bool Cache::remove(Person person){
    // validate input
    // checks if the Person object id is within the allowed range
    if (person.getID() < MINID || person.getID() > MAXID) {
        return false;
    }

    // search in the current table
    if (m_currentTable != nullptr) {
        int index = m_hash(person.getKey()) % m_currentCap;
        int i = 0;
        int newIndex = index;

        // probe through the table until either parameter Person is found or reached the end
        while (i < m_currentCap) {
            if (m_currentTable[newIndex] == nullptr) {
                break; // not found
            }
            if (m_currentTable[newIndex]->getUsed() && 
                m_currentTable[newIndex]->getKey() == person.getKey() &&
                m_currentTable[newIndex]->getID() == person.getID()) {
                
                // lazy delete
                // mark the slot as unused instead of freeing memory immediately
                m_currentTable[newIndex]->setUsed(false);
                m_currNumDeleted++;

                // check thresholds for rehash
                // checks if the rehash is already in progress
                // if too many slots are lazily deleted, do rehash
                if (m_oldTable == nullptr) {
                    float delRatio = deletedRatio();
                    if (delRatio > 0.8f) {
                        startRehash();
                    }
                }
                
                incrementalTransfer();

                return true;    // successfully removed
            }

            // collision probing
            i++;
            newIndex = probeIndex(index, i, m_currProbing, m_currentCap, m_hash, person.getKey());
        }
    }

    // search in old table if rehashing
    if (m_oldTable != nullptr) {
        int index = m_hash(person.getKey()) % m_oldCap;
        int i = 0;
        int newIndex = index;

        while (i < m_oldCap) {
            if (m_oldTable[newIndex] == nullptr) {
                break; // not found
            }
            if (m_oldTable[newIndex]->getUsed() &&
                m_oldTable[newIndex]->getKey() == person.getKey() &&
                m_oldTable[newIndex]->getID() == person.getID()) {
                
                // lazy delete
                m_oldTable[newIndex]->setUsed(false);
                m_oldNumDeleted++;

                incrementalTransfer();

                return true;    // successfully removed
            }

            // collision probing in the old table
            i++;
            newIndex = probeIndex(index, i, m_oldProbing, m_oldCap, m_hash,person.getKey());
        }
    }
    
    return false; // not found in either table
}

// searches for the Person object with the key and the ID in the hash table
const Person Cache::getPerson(string key, int ID) const{
    // validate input
    // checks if the Person object id is within the allowed range
    if (ID < MINID || ID > MAXID) {
        return Person();
    }

    // search the current table
    if (m_currentTable != nullptr) {
        int index = m_hash(key) % m_currentCap;
        int i = 0;
        int newIndex = index;

        // probe through the table until either parameter Person is found or reached the end
        while (i < m_currentCap) {
            if (m_currentTable[newIndex] == nullptr) {
                break;  // not found
            }
            if (m_currentTable[newIndex]->getUsed() && 
                m_currentTable[newIndex]->getKey() == key &&
                m_currentTable[newIndex]->getID() == ID) {
                return *m_currentTable[newIndex];   // found
            }

            // collision probing
            i++;
            newIndex = probeIndex(index, i, m_currProbing, m_currentCap, m_hash, key);
        }
    }

    // search old table if rehashing
    if (m_oldTable != nullptr) {
        int index = m_hash(key) % m_oldCap;
        int i = 0;
        int newIndex = index;

        while (i < m_oldCap) {
            if (m_oldTable[newIndex] == nullptr) {
                break; // not found
            }
            if (m_oldTable[newIndex]->getUsed() &&
                m_oldTable[newIndex]->getKey() == key &&
                m_oldTable[newIndex]->getID() == ID) {
                return *m_oldTable[newIndex];   // found
            }

            // collision probing
            i++;
            newIndex = probeIndex(index, i, m_oldProbing, m_oldCap, m_hash, key);

        }
    }

    // Not found
    return Person();    // empty object
}

// searches for the Person object in the hash table and updates the ID if found
bool Cache::updateID(Person person, int ID){
    // validate the new ID
    if (ID < MINID || ID > MAXID) {
        return false;
    }

    // search current table
    if (m_currentTable != nullptr) {
        int index = m_hash(person.getKey()) % m_currentCap;
        int i = 0;
        int newIndex = index;

        while (i < m_currentCap) {
            if (m_currentTable[newIndex] == nullptr) {
                break;  // not found in this chain
            }
            if (m_currentTable[newIndex]->getUsed() &&
                m_currentTable[newIndex]->getKey() == person.getKey() &&
                m_currentTable[newIndex]->getID() == person.getID()) {
                // found and update ID
                m_currentTable[newIndex]->setID(ID);
                return true;
            }
            // collision probing
            i++;
            newIndex = probeIndex(index, i, m_currProbing, m_currentCap, m_hash, person.getKey());

        }
    }

    // search old table if rehashing
    if (m_oldTable != nullptr) {
        int index = m_hash(person.getKey()) % m_oldCap;
        int i = 0;
        int newIndex = index;

        while (i < m_oldCap) {
            if (m_oldTable[newIndex] == nullptr) {
                break;  // not found
            }
            if (m_oldTable[newIndex]->getUsed() &&
                m_oldTable[newIndex]->getKey() == person.getKey() &&
                m_oldTable[newIndex]->getID() == person.getID()) {
                // found and update ID
                m_oldTable[newIndex]->setID(ID);
                return true;
            }

            // collision probing
            i++;
            newIndex = probeIndex(index, i, m_oldProbing, m_oldCap, m_hash, person.getKey());

        } 
    }

    // not found
    return false;
}

// returns load factor of the current hash table
// ratio of occupied buckets to the table capacity
float Cache::lambda() const {
    if (m_currentCap == 0) {
        return 0.0f;    //avoid division by zero
    }
    return static_cast<float>(m_currentSize) / static_cast<float>(m_currentCap);
}

// returns the ratio of deleted buckets to the total number of occupied buckets
float Cache::deletedRatio() const {
    if (m_currentSize == 0) {
        return 0.0f;    // avoid division by zero
    }
    return static_cast<float>(m_currNumDeleted) / static_cast<float>(m_currentSize);
}

// dumps the contents of the current and old hash table
// used for debugging
void Cache::dump() const {
    cout << "Dump for the current table: " << endl;
    if (m_currentTable != nullptr)
        for (int i = 0; i < m_currentCap; i++) {
            cout << "[" << i << "] : " << m_currentTable[i] << endl;
        }
    cout << "Dump for the old table: " << endl;
    if (m_oldTable != nullptr)
        for (int i = 0; i < m_oldCap; i++) {
            cout << "[" << i << "] : " << m_oldTable[i] << endl;
        }
}

// returns true if the parameter variable is a prime number
bool Cache::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

// returns the smallest prime number greater than the parameter variable
int Cache::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}


/*************************************
********** Private Functions**********
*************************************/

// incrementally transfer elements from the old has table to the current hash table
// designed to spread out the cost of rehashing
void Cache::incrementalTransfer() {
    // there is no old hash table to transfer ove
    if (m_oldTable == nullptr) {
        return;
    }

    // calculate the chunk size to transfer over in 1/4 portion of the old table
    int chunk = m_oldCap / 4;   // floor division
    if (chunk == 0) {
        chunk = 1;
    }

    // the range of the transfer
    int start = m_transferIndex;
    int end;

    // if the 4th transfer, move all the remaining elements
    int transfersDone = (chunk > 0) ? (m_transferIndex / chunk) : 0;
    if (transfersDone >= 3) {
        end = m_oldCap;
    } else {
        end = start + chunk;
        if (end > m_oldCap) {
            end = m_oldCap;
        }
    }

    // transfer elements from the old table from the transfer range
    for (int j = start; j < end; j++) {
        if (m_oldTable[j] != nullptr && m_oldTable[j]->getUsed()) {
            // copy the person object from the old table slot
            Person person = *m_oldTable[j];

            // compute the hash index in the new table
            int index = m_hash(person.getKey()) % m_currentCap;
            int i = 0;
            int newIndex = index;

            // probe until an empty or ununsed slot is found
            while (true) {
                if (m_currentTable[newIndex] == nullptr || !m_currentTable[newIndex]->getUsed()) {
                    // allocate a new Person object if the slot is empty
                    if (m_currentTable[newIndex] == nullptr) {
                        m_currentTable[newIndex] = new Person();
                    }
                    // copy data into the slot and mark as used
                    *m_currentTable[newIndex] = person;
                    m_currentTable[newIndex]->setUsed(true);
                    m_currentSize++;
                    break;
                }

                // collision probing
                i++;
                newIndex = probeIndex(index, i, m_currProbing, m_currentCap, m_hash, person.getKey());

                // checks if the entire table has been probed
                if (i >= m_currentCap) {
                    break;
                }
            }
            
            // deallocate the old table and clear the slot
            delete m_oldTable[j];
            m_oldTable[j] = nullptr;
        }
    }

    // update the transfer progress
    m_transferIndex = end;

    // if transfer is complete, clean up the old table
        if (m_transferIndex >= m_oldCap) {
        for (int i = 0; i < m_oldCap; i++) {
            if (m_oldTable[i] != nullptr) {
                delete m_oldTable[i];
                m_oldTable[i] = nullptr;
            }
        }
        delete[] m_oldTable;
        m_oldTable = nullptr;
        m_oldCap = 0;
        m_oldSize = 0;
        m_oldNumDeleted = 0;
        m_transferIndex = 0;
    }
}

// computes the next index to probe in the hash table based in parameter probe policy
// returns the next index to check in the table
int Cache::probeIndex(int baseIndex, int i, prob_t policy, int cap, hash_fn hash, const string& key) const {
    if (policy == LINEAR) {
        // each step moves forward by 1
        return (baseIndex + i) % cap;
    } else if (policy == QUADRATIC) {
        // each steps move quadratically
        return (baseIndex + i * i) % cap;
    } else if (policy == DOUBLEHASH) {
        // index = ((Hash(key) % TableSize) + i x (11-Hash(key) % 11))) % TableSize
        int step = 11 - (hash(key) % 11);
        return (baseIndex + i * step) % cap;
    }

    // default return the base index - should not happen
    return baseIndex;
}

// moves the current table into the old table and allocates a new larger table
// done incrementally to spread out cost
void Cache::startRehash() {
    // saves the current table element to the old table element
    m_oldTable = m_currentTable;
    m_oldCap = m_currentCap;
    m_oldSize = m_currentSize;
    m_oldNumDeleted = m_currNumDeleted;
    m_oldProbing = m_currProbing;

    //new capacity = new prime >= 4 x liveCount
    int liveCount = m_currentSize - m_currNumDeleted;
    int newSize = liveCount * 4;
    if (newSize < MINPRIME) {
        m_currentCap = MINPRIME;
    } else if (newSize > MAXPRIME) {
        m_currentCap = MAXPRIME;
    } else if (isPrime(newSize)) {
        m_currentCap = newSize;
    } else {
        m_currentCap = findNextPrime(newSize);
    }

    // allocate new table
    m_currentTable = new Person*[m_currentCap];
    for (int j = 0; j < m_currentCap; j++) {
        m_currentTable[j] = nullptr;
    }

    // resets the counter for the new table
    m_currentSize = 0;              // no elements yet
    m_currNumDeleted = 0;           // no deletions yet
    m_transferIndex = 0;            // starts at 0
    m_currProbing = m_newPolicy;    // sets the new probing policy
}

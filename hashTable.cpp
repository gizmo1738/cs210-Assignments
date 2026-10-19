// CS 210 - Assignment 6: Passwords and Hash Tables
// Build: clang++ -std=c++17 -Wall -Wextra assignment6_complete.cpp -o assignment6
// Run:   ./assignment6
//
// EDUCATIONAL ONLY: demoHash is NOT a secure password-hashing scheme.
// Restriction respected: unordered_map is NOT used for the user table.
//
// Four collision strategies are implemented behind one interface (HashTable):
//   1. Separate chaining      (the strategy chosen for the login demo, my fav)
//   2. Linear probing
//   3. Quadratic probing
//   4. Double hashing
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ---------------------------------------------------------------------------
// Given by the assignment.
// ---------------------------------------------------------------------------
uint64_t demoHash(const string& s) {
    uint64_t h = 14695981039346656037ULL;
    for (unsigned char c : s) {
        h = (h ^ c) * 1099511628211ULL;
    }
    return h;
}

// ---------------------------------------------------------------------------
// What is stored for each user. The plaintext password is never stored.
struct UserRecord {
    string salt;            // random per-user value, stored in the clear
    uint64_t passwordHash;  // demoHash(salt + password + pepper)
};

// One secret shared by ALL users and kept OUTSIDE the table / records.
static const string PEPPER = "x9#Kp!demo-pepper";


uint64_t hashPassword(const string& salt, const string& password) {
    return demoHash(salt + password + PEPPER);
}

// ---------------------------------------------------------------------------
// Common interface so all four strategies can be tested the same way.
// ---------------------------------------------------------------------------
class HashTable {
public:
    virtual ~HashTable() {}

    virtual bool insert(const string& username, const UserRecord& record) = 0;
    virtual UserRecord* find(const string& username) = 0;
    virtual bool remove(const string& username) = 0;
    virtual size_t size() const = 0;
    virtual size_t bucketCount() const = 0;
    virtual string name() const = 0;
    virtual void dump(ostream& out) const = 0;

    // Time: O(1)
    double loadFactor() const {
        return static_cast<double>(size()) / static_cast<double>(bucketCount());
    }

    long long probes = 0;   // total slots/nodes examined so far
    int rehashCount = 0;    // how many times the table has grown
    bool verbose = false;   // print a message when a rehash happens

protected:
    void announceRehash(size_t oldCount, size_t newCount) {
        ++rehashCount;
        if (verbose) {
            cout << "  >> REHASH: " << oldCount << " -> " << newCount
                 << " buckets (size=" << size() << ", load factor was "
                 << fixed << setprecision(2) << loadFactor() << ")\n";
        }
    }
};

// ===========================================================================
// STRATEGY 1: SEPARATE CHAINING
// Each bucket holds a list (a "chain"). Colliding keys simply share a chain.
// ===========================================================================
class SeparateChainingTable : public HashTable {
public:
    explicit SeparateChainingTable(size_t initialBuckets = 4, double maxLoad = 0.75)
        : buckets_(initialBuckets), size_(0), maxLoad_(maxLoad) {}

    // Time: average O(1), worst case O(n) (everything in one chain).
    //       A rehash costs O(n) but happens rarely
    bool insert(const string& username, const UserRecord& record) override {
        if (find(username) != nullptr) return false;  // usernames must be unique

        // Grow BEFORE adding if the new element would push us past the limit.
        if (static_cast<double>(size_ + 1) / buckets_.size() > maxLoad_) {
            rehash(buckets_.size() * 2);
        }
        buckets_[indexFor(username, buckets_.size())].push_back(Entry{username, record});
        ++size_;
        return true;
    }

    // Time: average O(1 + alpha) = O(1) 
    UserRecord* find(const string& username) override {
        list<Entry>& chain = buckets_[indexFor(username, buckets_.size())];
        long long steps = 0;
        for (Entry& e : chain) {
            ++steps;
            if (e.key == username) {
                probes += steps;
                return &e.value;
            }
        }
        probes += max(1LL, steps);  // looking at an empty bucket still counts as 1
        return nullptr;
    }

    // Time: average O(1), worst case O(n).
    bool remove(const string& username) override {
        list<Entry>& chain = buckets_[indexFor(username, buckets_.size())];
        long long steps = 0;
        for (auto it = chain.begin(); it != chain.end(); ++it) {
            ++steps;
            if (it->key == username) {
                probes += steps;
                chain.erase(it);  // O(1) for a list once we have the iterator
                --size_;
                return true;
            }
        }
        probes += max(1LL, steps);
        return false;
    }

    size_t size() const override { return size_; }
    size_t bucketCount() const override { return buckets_.size(); }
    string name() const override { return "Separate chaining"; }

    // Which bucket would this key land in right now? (used by the demo)
    size_t bucketOf(const string& username) const {
        return indexFor(username, buckets_.size());
    }

    void dump(ostream& out) const override {
        for (size_t i = 0; i < buckets_.size(); ++i) {
            out << "  [" << setw(2) << i << "] ";
            if (buckets_[i].empty()) {
                out << "(empty)";
            } else {
                bool first = true;
                for (const Entry& e : buckets_[i]) {
                    out << (first ? "" : " -> ") << e.key;
                    first = false;
                }
                if (buckets_[i].size() > 1) out << "   <-- collision: chain of " << buckets_[i].size();
            }
            out << "\n";
        }
    }

private:
    struct Entry {
        string key;
        UserRecord value;
    };

    static size_t indexFor(const string& key, size_t bucketCount) {
        return static_cast<size_t>(demoHash(key) % bucketCount);
    }

    // Time: O(n + m)
    void rehash(size_t newBucketCount) {
        announceRehash(buckets_.size(), newBucketCount);
        vector<list<Entry>> bigger(newBucketCount);
        for (list<Entry>& chain : buckets_) {
            for (Entry& e : chain) {
                size_t idx = indexFor(e.key, newBucketCount);
                ++probes;  // count the move so the comparison with open addressing is fair
                bigger[idx].push_back(std::move(e));
            }
        }
        buckets_ = std::move(bigger);
    }

    vector<list<Entry>> buckets_;
    size_t size_;
    double maxLoad_;
};

// ===========================================================================
// STRATEGIES 2-4: OPEN ADDRESSING
// One flat array. On a collision we "probe" for another slot in the SAME array.
// ===========================================================================
class OpenAddressingTable : public HashTable {
public:
    OpenAddressingTable(size_t initialSlots, double maxLoad)
        : slots_(initialSlots), size_(0), used_(0), maxLoad_(maxLoad) {}

    // Time: expected O(1/(1-alpha)) probes, worst case O(n).
    // Open addressing degrades sharply as alpha -> 1, which is why we
    // rehash at a lower load factor (0.5) than chaining (0.75).
    bool insert(const string& username, const UserRecord& record) override {
        if (find(username) != nullptr) return false;
        // used_ counts Occupied + Deleted slots: tombstones still lengthen probes.
        if (static_cast<double>(used_ + 1) / slots_.size() > maxLoad_) {
            rehash(slots_.size() * 2);
        }
        placeNew(username, record);
        return true;
    }

    // Time: expected O(1/(1-alpha)) for a miss, O(1/alpha * ln(1/(1-alpha)))
    // for a hit (linear probing is slightly worse due to clustering).
    UserRecord* find(const string& username) override {
        const size_t m = slots_.size();
        const size_t start = static_cast<size_t>(demoHash(username) % m);
        const size_t step = stepFor(username, m);
        for (size_t i = 0; i < m; ++i) {
            Slot& s = slots_[slotAt(start, step, i, m)];
            ++probes;
            if (s.state == Empty) return nullptr;  // an Empty slot ends the probe path
            if (s.state == Occupied && s.key == username) return &s.value;
            // Deleted or someone else's key keep probing
        }
        return nullptr;
    }

    // Time: same as find().
    bool remove(const string& username) override {
        const size_t m = slots_.size();
        const size_t start = static_cast<size_t>(demoHash(username) % m);
        const size_t step = stepFor(username, m);
        for (size_t i = 0; i < m; ++i) {
            Slot& s = slots_[slotAt(start, step, i, m)];
            ++probes;
            if (s.state == Empty) return false;
            if (s.state == Occupied && s.key == username) {
                s.state = Deleted;  // tombstone, NOT Empty
                --size_;
                return true;
            }
        }
        return false;
    }

    size_t size() const override { return size_; }
    size_t bucketCount() const override { return slots_.size(); }

    void dump(ostream& out) const override {
        for (size_t i = 0; i < slots_.size(); ++i) {
            out << "  [" << setw(2) << i << "] ";
            if (slots_[i].state == Empty) out << "(empty)";
            else if (slots_[i].state == Deleted) out << "(deleted - tombstone)";
            else out << slots_[i].key;
            out << "\n";
        }
    }

protected:
   
    virtual size_t offset(size_t i, size_t step) const = 0;
    virtual size_t stepFor(const string& /*key*/, size_t /*m*/) const { return 1; }

private:
    enum State { Empty, Occupied, Deleted };
    struct Slot {
        State state = Empty;
        string key;
        UserRecord value;
    };

    size_t slotAt(size_t start, size_t step, size_t i, size_t m) const {
        return (start + offset(i, step) % m) % m;
    }


    void placeNew(const string& username, const UserRecord& record) {
        const size_t m = slots_.size();
        const size_t start = static_cast<size_t>(demoHash(username) % m);
        const size_t step = stepFor(username, m);
        for (size_t i = 0; i < m; ++i) {
            Slot& s = slots_[slotAt(start, step, i, m)];
            ++probes;
            if (s.state != Occupied) {
                if (s.state == Empty) ++used_;  // reusing a tombstone adds nothing new
                s.state = Occupied;
                s.key = username;
                s.value = record;
                ++size_;
                return;
            }
        }
    }

    // Time: O(n + m). Also clears all tombstones.
    void rehash(size_t newSlotCount) {
        announceRehash(slots_.size(), newSlotCount);
        vector<Slot> old = std::move(slots_);
        slots_.assign(newSlotCount, Slot());
        size_ = 0;
        used_ = 0;
        for (Slot& s : old) {
            if (s.state == Occupied) placeNew(s.key, s.value);
        }
    }

    vector<Slot> slots_;
    size_t size_;      // live elements
    size_t used_;      // live elements + tombstones
    double maxLoad_;
};

// Linear probing: home, home+1, home+2, ...

class LinearProbingTable : public OpenAddressingTable {
public:
    explicit LinearProbingTable(size_t slots = 8, double maxLoad = 0.5)
        : OpenAddressingTable(slots, maxLoad) {}
    string name() const override { return "Linear probing"; }
protected:
    size_t offset(size_t i, size_t) const override { return i; }
};

// Quadratic probing: home + (i + i*i)/2  ->  +0, +1, +3, +6, +10, ...
// Jumps get bigger, which breaks up primary clustering.
class QuadraticProbingTable : public OpenAddressingTable {
public:
    explicit QuadraticProbingTable(size_t slots = 8, double maxLoad = 0.5)
        : OpenAddressingTable(slots, maxLoad) {}
    string name() const override { return "Quadratic probing"; }
protected:
    size_t offset(size_t i, size_t) const override { return (i + i * i) / 2; }
};

// Double hashing: home + i * step, where step comes from a SECOND hash of the key.
// Two keys with the same home slot usually get different steps, so they
// separate right away. The step is forced odd so it is coprime with the
// power-of-two table size and therefore can reach every slot.
class DoubleHashingTable : public OpenAddressingTable {
public:
    explicit DoubleHashingTable(size_t slots = 8, double maxLoad = 0.5)
        : OpenAddressingTable(slots, maxLoad) {}
    string name() const override { return "Double hashing"; }
protected:
    size_t offset(size_t i, size_t step) const override { return i * step; }
    size_t stepFor(const string& key, size_t m) const override {
        return static_cast<size_t>((demoHash(key + "#2") | 1ULL) % m);
    }
};

// ---------------------------------------------------------------------------
// Authentication layer built on top of ANY HashTable.
// ---------------------------------------------------------------------------
enum class LoginResult { Success, WrongPassword, UnknownUser };

class AuthSystem {
public:
    explicit AuthSystem(HashTable& table) : table_(table), rng_(210) {}

    // Time: O(L) to hash + the table's insert cost (avg O(1)).
    bool registerUser(const string& username, const string& password) {
        string salt = makeSalt();
        UserRecord rec{salt, hashPassword(salt, password)};
        return table_.insert(username, rec);
    }

    bool removeUser(const string& username) { return table_.remove(username); }

    // Time: average O(L) = one table lookup + one hash computation.
    // 1) look up the username  2) recompute the hash with THAT user's salt
    // and the pepper  3) compare with the stored hash.
    LoginResult login(const string& username, const string& password) {
        UserRecord* rec = table_.find(username);
        if (rec == nullptr) return LoginResult::UnknownUser;
        if (hashPassword(rec->salt, password) == rec->passwordHash) return LoginResult::Success;
        return LoginResult::WrongPassword;
    }

private:
    // 8 random bytes written as 16 hex characters. The fixed seed makes the
    // demo output repeatable; real systems use a cryptographically secure RNG.
    string makeSalt() {
        ostringstream os;
        os << hex << setfill('0') << setw(16) << rng_();
        return os.str();
    }

    HashTable& table_;
    mt19937_64 rng_;
};

static const char* describe(LoginResult r) {
    switch (r) {
        case LoginResult::Success:       return "SUCCESS";
        case LoginResult::WrongPassword: return "FAILED (wrong password)";
        case LoginResult::UnknownUser:   return "FAILED (unknown user)";
    }
    return "?";
}

static void tryLogin(AuthSystem& auth, const string& user, const string& pw) {
    cout << "  login(\"" << user << "\", \"" << pw << "\") -> "
         << describe(auth.login(user, pw)) << "\n";
}

// ---------------------------------------------------------------------------
// Compare the four strategies on identical data.
// ---------------------------------------------------------------------------
static vector<unique_ptr<HashTable>> makeAllStrategies() {
    vector<unique_ptr<HashTable>> v;
    v.push_back(make_unique<SeparateChainingTable>(8, 0.75));
    v.push_back(make_unique<LinearProbingTable>(8, 0.5));
    v.push_back(make_unique<QuadraticProbingTable>(8, 0.5));
    v.push_back(make_unique<DoubleHashingTable>(8, 0.5));
    return v;
}

// Insert N users, remove half, and verify every lookup is still correct.
// This exercises insert / find / remove / rehash / tombstones on all four.
static bool selfCheck(HashTable& t, int n) {
    UserRecord dummy{"salt", 0};
    for (int i = 0; i < n; ++i) {
        if (!t.insert("user" + to_string(i), dummy)) return false;
    }
    if (t.insert("user0", dummy)) return false;  // duplicates must be rejected
    for (int i = 0; i < n; i += 2) {
        if (!t.remove("user" + to_string(i))) return false;
    }
    if (t.remove("user0")) return false;         // already removed
    for (int i = 0; i < n; ++i) {
        bool found = t.find("user" + to_string(i)) != nullptr;
        if (found != (i % 2 == 1)) return false;
    }
    // Tombstoned slots must be reusable.
    for (int i = 0; i < n; i += 2) {
        if (!t.insert("user" + to_string(i), dummy)) return false;
    }
    return t.size() == static_cast<size_t>(n);
}

static void compareStrategies(int n) {
    cout << "  Same " << n << " usernames inserted into each table, then " << n
         << " lookups that succeed and " << n << " that fail.\n\n";
    cout << "  " << left << setw(20) << "Strategy" << right
         << setw(9) << "slots" << setw(10) << "rehashes" << setw(8) << "load"
         << setw(14) << "insert probes" << setw(11) << "hit avg" << setw(11) << "miss avg" << "\n";
    cout << "  " << string(83, '-') << "\n";

    UserRecord dummy{"salt", 0};
    for (auto& t : makeAllStrategies()) {
        for (int i = 0; i < n; ++i) t->insert("user" + to_string(i), dummy);
        long long buildProbes = t->probes;

        t->probes = 0;
        for (int i = 0; i < n; ++i) t->find("user" + to_string(i));
        double hitAvg = static_cast<double>(t->probes) / n;

        t->probes = 0;
        for (int i = 0; i < n; ++i) t->find("ghost" + to_string(i));
        double missAvg = static_cast<double>(t->probes) / n;

        cout << "  " << left << setw(20) << t->name() << right
             << setw(9) << t->bucketCount() << setw(10) << t->rehashCount
             << setw(8) << fixed << setprecision(2) << t->loadFactor()
             << setw(14) << buildProbes
             << setw(11) << hitAvg << setw(11) << missAvg << "\n";
    }
    cout << "\n  (probes = slots or chain nodes examined; lower is better)\n";
}

int main() {
    // =======================================================================
    // PART 1: Login demo on the CHOSEN strategy: separate chaining.
    // Starts with only 4 buckets so the rehash events are easy to see.
    // =======================================================================
    SeparateChainingTable table(4, 0.75);
    table.verbose = true;
    AuthSystem auth(table);

    cout << "=== 1. Registering 7 users (table starts with 4 buckets) ===\n";
    struct Seed { const char* user; const char* pw; };
    const Seed seeds[] = {
        {"alice", "wonderland1"}, {"bob", "builder#42"},   {"carol", "c4rol!pass"},
        {"dave", "d4v3_secret"},  {"erin", "tr0ub4dor&3"}, {"frank", "correct-horse"},
        {"grace", "hopper1906"},
    };
    for (const Seed& s : seeds) {
        cout << "  register " << s.user << "\n";
        auth.registerUser(s.user, s.pw);  // a rehash message prints from inside insert()
    }
    cout << "  size=" << table.size() << "  buckets=" << table.bucketCount()
         << "  loadFactor=" << fixed << setprecision(3) << table.loadFactor()
         << "  rehashes=" << table.rehashCount << "\n";

    cout << "\n=== 2. Table layout (chains show collisions) ===\n";
    table.dump(cout);

    cout << "\n=== 2b. Forcing a visible collision: 5 users in a tiny 4-bucket table ===\n";
    {
        SeparateChainingTable tiny(4, 10.0);  // huge max load => never rehashes
        UserRecord dummy{"salt", 0};
        for (const char* u : {"alice", "bob", "carol", "dave", "erin"}) tiny.insert(u, dummy);
        tiny.dump(cout);
        cout << "  load factor = " << fixed << setprecision(2) << tiny.loadFactor()
             << " (5 users / 4 buckets), so at least one chain MUST hold 2+ users\n";
    }

    cout << "\n=== 3. What is stored for a user (no plaintext password!) ===\n";
    UserRecord* a = table.find("alice");
    cout << "  alice -> bucket " << table.bucketOf("alice")
         << ", salt=" << a->salt << ", hash=" << a->passwordHash << "\n";

    cout << "\n=== 4. Login demonstrations ===\n";
    tryLogin(auth, "alice", "wonderland1");  // successful login
    tryLogin(auth, "alice", "wrongpass");    // failed password
    tryLogin(auth, "mallory", "anything");   // unknown user

    cout << "\n=== 5. Removal ===\n";
    cout << "  remove(\"bob\")        -> " << (auth.removeUser("bob") ? "removed" : "not found") << "\n";
    cout << "  remove(\"bob\") again  -> " << (auth.removeUser("bob") ? "removed" : "not found") << "\n";
    tryLogin(auth, "bob", "builder#42");     // bob is gone now
    cout << "  size=" << table.size() << "  loadFactor=" << table.loadFactor() << "\n";

    cout << "\n=== 6. Same password, different salts => different hashes ===\n";
    auth.registerUser("heidi", "samepassword");
    auth.registerUser("ivan", "samepassword");
    cout << "  heidi hash=" << table.find("heidi")->passwordHash << "\n";
    cout << "  ivan  hash=" << table.find("ivan")->passwordHash << "\n";

    cout << "\n=== 7. Another rehash event, then login still works ===\n";
    const char* more[] = {"judy", "karl", "liam", "mona", "nate", "olga"};
    for (const char* u : more) auth.registerUser(u, "pw-" + string(u));
    cout << "  buckets now: " << table.bucketCount() << ", rehashes so far: "
         << table.rehashCount << "\n";
    tryLogin(auth, "alice", "wonderland1");

    // =======================================================================
    // PART 2: Compare all four collision strategies.
    // =======================================================================
    cout << "\n=== 8. Correctness check of all four strategies ===\n";
    for (auto& t : makeAllStrategies()) {
        cout << "  " << left << setw(20) << t->name() << right
             << (selfCheck(*t, 300) ? "PASS" : "FAIL") << "\n";
    }

    cout << "\n=== 9. Collision strategy comparison ===\n";
    compareStrategies(500);

    cout << "\n=== 10. Choice ===\n";
    cout << "  Separate chaining is the strategy used for the user table above:\n"
         << "  it tolerates a higher load factor, never needs tombstones, and\n"
         << "  removal is a plain list erase. See the explanation at the bottom\n"
         << "  of this file for the full reasoning.\n";
    return 0;
}
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

enum class QueryType {
    NewBus,
    BusesForStop,
    StopsForBus,
    AllBuses,
};

struct Query {
    QueryType type;
    string bus;
    string stop;
    vector<string> stops;
};

template <typename T>
ostream& operator<<(ostream& os, const vector<T>& data) {
    bool is_first = true;
    for (const auto& item : data) {
        if (is_first) {
            is_first = false;
        } else {
            os << " "s;
        }
        os << item;
    }
    return os;
}

istream& operator>>(istream& is, Query& q) {
    string command;

    is >> command;

    q.stops = {};
    q.stop = "";
    q.bus = "";

    if (command == "NEW_BUS") {
        int n;
        is >> q.bus;
        is >> n;
        for (int i = 0; i < n; i++) {
            string bus_stop;
            is >> bus_stop;
            q.stops.push_back(bus_stop);
            q.type = QueryType::NewBus;
        }
    } else if (command == "BUSES_FOR_STOP") {
        q.type = QueryType::BusesForStop;
        is >> q.stop;
    } else if (command == "STOPS_FOR_BUS") {
        q.type = QueryType::StopsForBus;
        is >> q.bus;
    } else if (command == "ALL_BUSES") {
        q.type = QueryType::AllBuses;
    }

    return is;
}

struct BusesForStopResponse {
    vector<string> busses;
};

ostream& operator<<(ostream& os, const BusesForStopResponse& r) {
    if (r.busses.empty()) {
        os << "No stop"s;
    } else {
        os << r.busses;
    }
    return os;
}

struct StopsForBusResponse {
    map<string, vector<string>> stops;
};

ostream& operator<<(ostream& os, const StopsForBusResponse& r) {
    if (r.stops.empty()) {
        os << "No bus"s;
    } else {
        bool is_first = true;
        for (const auto& [stop, busses] : r.stops) {
            if (is_first) {
                is_first = false;
            } else {
                os << endl;
            }
            os << "Stop "s << stop << ": "s;
            if (busses.empty()) {
                os << "no interchange"s;
            } else {
                os << busses;
            }
        }
    }
    return os;
}

struct AllBusesResponse {
    map<string, vector<string>> buses_to_stops;
};

ostream& operator<<(ostream& os, const AllBusesResponse& r) {
    if (r.buses_to_stops.empty()) {
        os << "No buses"s;
    } else {
        bool is_first = true;
        for (const auto& bus_item : r.buses_to_stops) {
            if (is_first) {
                is_first = false;
            } else {
                os << endl;
            }
            os << "Bus "s << bus_item.first << ": "s << bus_item.second;
        }
    }
    return os;
}

class BusManager {
   public:
    void AddBus(const string& bus, const vector<string>& stops) {
        buses_to_stops_[bus] = stops;
        for (const string& stop : stops) {
            stops_to_buses_[stop].push_back(bus);
        }
    }

    BusesForStopResponse GetBusesForStop(const string& stop) const {
        if (stops_to_buses_.count(stop) == 0) {
            return {{}};
        }
        return {stops_to_buses_.at(stop)};
    }

    StopsForBusResponse GetStopsForBus(const string& bus) const {
        if (buses_to_stops_.count(bus) == 0) {
            return {{}};
        }

        map<string, vector<string>> stops;
        for (const string& stop : buses_to_stops_.at(bus)) {
            if (stops_to_buses_.at(stop).size() == 1) {
                stops[stop] = {};
            } else {
                for (const string& other_bus : stops_to_buses_.at(stop)) {
                    if (bus != other_bus) {
                        stops[stop].push_back(other_bus);
                    }
                }
            }
        }
        return {stops};
    }

    AllBusesResponse GetAllBuses() const {
        return {buses_to_stops_};
    }

   private:
    map<string, vector<string>> buses_to_stops_, stops_to_buses_;
};

// Реализуйте функции и классы, объявленные выше, чтобы эта функция main
// решала задачу "Автобусные остановки"

int main() {
    int query_count;
    Query q;

    cin >> query_count;

    BusManager bm;
    for (int i = 0; i < query_count; ++i) {
        cin >> q;
        switch (q.type) {
            case QueryType::NewBus:
                bm.AddBus(q.bus, q.stops);
                break;
            case QueryType::BusesForStop:
                cout << bm.GetBusesForStop(q.stop) << endl;
                break;
            case QueryType::StopsForBus:
                cout << bm.GetStopsForBus(q.bus) << endl;
                break;
            case QueryType::AllBuses:
                cout << bm.GetAllBuses() << endl;
                break;
        }
    }
}
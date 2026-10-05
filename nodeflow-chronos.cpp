#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <random>

// Include Windows header for fixing console output encoding
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// 1. Packet Definition
struct Packet {
    int node_id;
    int seq_num;
    long long timestamp;
    float temperature;

    // Operator for Min-Heap ordering (sorts by timestamp smallest first)
    bool operator>(const Packet& other) const {
        return timestamp > other.timestamp;
    }
};

// 2. Random Sensor Data Generator using a dynamic vector
vector<Packet> generate_sensor_data(int stream_size) {
    vector<Packet> stream(stream_size);

    mt19937 rng(1337); // Seeded for consistent results
    uniform_int_distribution<int> node_dist(1, 3);        // Nodes 1 to 3
    uniform_int_distribution<int> seq_dist(1, 4);         // Sequence numbers 1 to 4
    uniform_int_distribution<long long> time_dist(100, 115); // Timestamps (out-of-order chaos)
    uniform_real_distribution<float> temp_dist(20.0f, 35.0f); // Temperature readings

    for (int i = 0; i < stream_size; ++i) {
        stream[i].node_id = node_dist(rng);
        stream[i].seq_num = seq_dist(rng);
        stream[i].timestamp = time_dist(rng);
        stream[i].temperature = temp_dist(rng);

        // Simulate duplicates: explicitly overwrite an index with a copy of the previous packet
        if (i > 0 && i % 4 == 0) {
            stream[i] = stream[i - 1];
        }
    }

    return stream;
}

int main() {
    // --- Windows console fix to display the degree symbol correctly ---
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    int STREAM_SIZE;
    cout << "Enter Stream size: ";
    cin >> STREAM_SIZE;
    
    // Handle invalid inputs
    if (STREAM_SIZE <= 0) {
        cout << "Stream size must be greater than 0.\n";
        return 1;
    }

    // Generate raw stream using a dynamic vector
    vector<Packet> raw_stream = generate_sensor_data(STREAM_SIZE);

    cout << "\n=== 1. RAW INCOMING STREAM ===\n";
    for (const auto& p : raw_stream) {
        cout << "Node " << p.node_id << " | Seq " << p.seq_num
             << " | Time: " << p.timestamp << " | Temp: " << p.temperature << "\u00B0C\n";
    }

    // --- Data Structure 1: Hash Set for Deduplication ---
    unordered_set<string> seen_packets;

    // --- Data Structure 2: Priority Queue (Min-Heap) for Time Re-ordering ---
    priority_queue<Packet, vector<Packet>, greater<Packet>> reorder_heap;

    cout << "\n=== 2. FILTERING & RE-ORDERING STAGE ===\n";
    for (const auto& p : raw_stream) {
        // Create a unique key string (e.g., "1:3")
        string packet_key = to_string(p.node_id) + ":" + to_string(p.seq_num);

        // Check Hash Set: If already seen, drop it instantly
        if (seen_packets.count(packet_key)) {
            cout << "[DROPPED DUPLICATE] Node " << p.node_id << " Seq " << p.seq_num << "\n";
            continue;
        }

        // Mark as seen
        seen_packets.insert(packet_key);

        // Push unique packet into the Priority Queue (Min-Heap)
        reorder_heap.push(p);
    }

    // --- Data Structure 3: Hash Map for State Aggregation ---
    unordered_map<int, float> status_board;

    cout << "\n=== 3. CHRONOLOGICAL PROCESSING STAGE ===\n";
    while (!reorder_heap.empty()) {
        // Pop the oldest timestamp from the Min-Heap
        Packet p = reorder_heap.top();
        reorder_heap.pop();

        // Update the Hash Map state board with the latest value for this node
        status_board[p.node_id] = p.temperature;

        cout << "[PROCESSED IN-ORDER] Time: " << p.timestamp
             << " | Node " << p.node_id
             << " | Temp: " << p.temperature << "\u00B0C\n";
    }

    // --- Final Result ---
    cout << "\n=== 4. FINAL STATUS BOARD (Hash Map) ===\n";
    for (const auto& [node_id, temp] : status_board) {
        cout << "Sensor " << node_id << " Current Temperature: " << temp << "\u00B0C\n";
    }

    return 0;
}

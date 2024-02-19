#pragma once
#include <bits/stdc++.h>
#include "lcgrand.h" // Header file for random-number generator
using namespace std;

#define Q_LIMIT 100 /* Limit on queue length. */
#define MIN_TIME_TO_NEXT_EVENT 1.0e+29
#define INF  1.0e+30

enum ServerStatus {
  IDLE,  // 0 -> Mnemonic for server's being idle
  BUSY   // 1 -> Mnemonic for server's being busy
};

enum EventType {
  NONE,     // 0 -> Mnemonic for no event
  ARRIVAL,  // 1 -> Mnemonic for arrival event
  DEPARTURE // 2 -> Mnemonic for departure event
};



class MM1QSim {
  EventType next_event_type;
  int num_custs_delayed;
  int num_delays_required;
  int num_events; // Number of events for the timing function to consider
  int num_in_q;
  ServerStatus server_status;

  int event_count; // Number of events that have occurred so far
  int arrival_count;
  int departure_count;

  double area_num_in_q;
  double area_server_status;
  double mean_interarrival;
  double mean_service;
  double sim_time;
  queue<double> time_arrival;
  double time_last_event;
  double time_next_event[3]; // Next 3 events to consider. 
  //0th index is not used, 1st index is for arrival, 2nd index is for departure

  double total_of_delays;

  fstream event_orders;
  fstream results;

public:
  MM1QSim(double mean_interarrival, double mean_service, int num_delays_required);
  void initialize();
  void timing();
  void arrive();
  void depart();
  void report();
  void update_time_avg_stats();
  double expon(double mean);
  void log_next_event_type(EventType next_event_type);
  void log_customer_delayed();
  void run();
};


MM1QSim::MM1QSim(double mean_interarrival, double mean_service, int num_delays_required) {
  event_orders.open("event_orders.txt", ios::out);
  results.open("results.txt", ios::out);

  this->mean_interarrival = mean_interarrival;
  this->mean_service = mean_service;
  this->num_delays_required = num_delays_required;

  // log  
  results << "----Single-Server Queueing System----"<<endl<<endl;

  results << fixed << setprecision(6) << "Mean inter-arrival time: " << mean_interarrival << " minutes\n";
  results << fixed << setprecision(6) << "Mean service time: " << mean_service << " minutes\n";
  results << "Number of customers: " << num_delays_required<<endl;
}


// initialize the simulation
void MM1QSim::initialize() {
  // Initialize the simulation clock. 
  sim_time = 0.0;

  // Initialize the state variables. 
  server_status = ServerStatus::IDLE;
  num_in_q = 0;
  time_last_event = 0.0;

  // Initialize the statistical counters.
  num_custs_delayed = 0;
  total_of_delays = 0.0;
  area_num_in_q = 0.0;
  area_server_status = 0.0;

  /* Initialize event list. Since no customers are present, the departure
  (service completion) event is eliminated from consideration. */
  time_next_event[ARRIVAL] = sim_time + expon(mean_interarrival);
  time_next_event[DEPARTURE] = INF;

  // Intialize count for log
  event_count = 0;
  arrival_count = 0;
  departure_count = 0;
}

// Timing function
void MM1QSim::timing() {
  int i;
  double min_time_next_event = MIN_TIME_TO_NEXT_EVENT;

  next_event_type = EventType::NONE;

  // Determine the event type of the next event to occur.
  for (i = 1; i <= num_events; ++i) {
    if (time_next_event[i] < min_time_next_event) {
      min_time_next_event = time_next_event[i];
      next_event_type = (EventType)i;
    }
  }

  // Check to see whether the event list is empty. 
  if (next_event_type == EventType::NONE) {
    // The event list is empty, so stop the simulation.
    results << "\nEvent list empty at time " << sim_time;
    exit(1);
  }

  // The event list is not empty, so advance the simulation clock.
  sim_time = min_time_next_event;
}

// Arrival event function
void MM1QSim::arrive() {
  arrival_count++;
  log_next_event_type(ARRIVAL);

  double delay;

  /* Schedule next arrival. */
  time_next_event[1] = sim_time + expon(mean_interarrival);

  /* Check to see whether server is busy. */
  if (server_status == BUSY) {
    /* Server is busy, so increment number of customers in queue. */
    ++num_in_q;

    /* Check to see whether an overflow condition exists. */
    if (num_in_q > Q_LIMIT) {
      /* The queue has overflowed, so stop the simulation. */
      results << "\nOverflow of the array time_arrival at";
      results << " time " << sim_time;
      exit(2);
    }

    /* There is still room in the queue, so store the time of arrival of the
    arriving customer at the (new) end of time_arrival. */
    time_arrival.push(sim_time);
  } else {
    /* Server is idle, so arriving customer has a delay of zero. (The
    following two statements are for program clarity and do not affect
    the results of the simulation.) */
    delay = 0.0;
    total_of_delays += delay;

    // Increment the number of customers delayed, and make server busy.
    ++num_custs_delayed;
    this->log_customer_delayed();

    server_status = BUSY;

    // Schedule a departure (service completion). 
    time_next_event[DEPARTURE] = sim_time + expon(mean_service);
  }
}

// Departure event function
void MM1QSim::depart() {
  departure_count++;
  log_next_event_type(DEPARTURE);

  /* Check to see whether the queue is empty. */
  if (num_in_q == 0) {
    /* The queue is empty so make the server idle and eliminate the
    departure (service completion) event from consideration. */
    server_status = IDLE;
    time_next_event[DEPARTURE] = INF;
  } else {
    /* The queue is nonempty, so decrement the number of customers in
    queue. */
    --num_in_q;

    /* Compute the delay of the customer who is beginning service and update
    the total delay accumulator. */
    double delay = sim_time - time_arrival.front();
    total_of_delays += delay;

    // Increment the number of customers delayed, and schedule departure.
    ++num_custs_delayed;
    this->log_customer_delayed();

    time_next_event[DEPARTURE] = sim_time + expon(mean_service);

    // Move each customer in queue (if any) up one place.
    time_arrival.pop();
  }
}

// Report generator function
// Compute and write estimates of desired measures of performance. 
void MM1QSim::report() {
  results << fixed << setprecision(6) << "\nAvg delay in queue: "
    << total_of_delays / num_custs_delayed << " minutes\n";
  results << fixed << setprecision(6) << "Avg number in queue: "
    << area_num_in_q / sim_time << "\n";
  results << fixed << setprecision(6) << "Server utilization: "
    << area_server_status / sim_time;
  results << fixed << setprecision(6) << "\nTime simulation ended: " << sim_time << " minutes";
}


// Update area accumulators for time-average statistics.
void MM1QSim::update_time_avg_stats() {
  double time_since_last_event;

  // Compute time since last event, and update last-event-time marker.
  time_since_last_event = sim_time - time_last_event;
  time_last_event = sim_time;

  // Update area under number-in-queue function.
  area_num_in_q += num_in_q * time_since_last_event;

  // Update area under server-busy indicator function.
  area_server_status += server_status * time_since_last_event;
}

// Exponential variate generation function.
// Return an exponential random variate with mean "mean".
double MM1QSim::expon(double mean) {
  return -mean * log(lcgrand(1));
}

void MM1QSim::log_next_event_type(EventType next_event_type) {
  switch (next_event_type) {
    case EventType::ARRIVAL:
      event_orders << event_count << ". Next event: Customer "<< arrival_count <<" Arrival"<<endl;
      break;
    case EventType::DEPARTURE:
      event_orders << event_count << ". Next event: Customer "<< departure_count <<" Departure"<<endl;
      break;
  }
}

void MM1QSim::log_customer_delayed() {
  event_orders << "\n---------No. of customers delayed: " << num_custs_delayed << "--------";
  if (num_custs_delayed < num_delays_required) {
    event_orders << "\n\n";
  }
}

// Run the simulation
void MM1QSim::run() {
  // Specify the number of events for the timing function.
  num_events = 2;

  // Initialize the simulation.
  this->initialize();

  // Run the simulation while more delays are still needed. 
  while (num_custs_delayed < num_delays_required) {
    ++event_count;

    // Determine the next event.
    this->timing();

    // Update time-average statistical accumulators. 
    this->update_time_avg_stats();

    // Invoke the appropriate event function.
    switch (next_event_type) {
      case ARRIVAL:
        this->arrive();
        break;
      case DEPARTURE:
        this->depart();
        break;
    }
  }

  // Invoke the report generator and end the simulation.
  this->report();
}
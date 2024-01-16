// author: Hasan Masuk(1805052)
#pragma once

#include <bits/stdc++.h>
#include "lcgrand.h"
using namespace std;

enum EventType
{
  NONE,                 // No event
  ORDER_ARRIVAL,        // Arrival of an order to the company from the supplier
  EPRESS_ORDER_ARRIVAL, // Arrival of an express order to the company from the supplier
  DEMAND,               // Demand for the product from a customer
  END,                  // End of the simulation after n months
  EVALUATE              // Inventory evaluation (and possible ordering) at the beginning of a month
};

double expon(double mean_interdemand)
{
  return -mean_interdemand * log(lcgrand(1));
}

double uniform(double a, double b)
{
  return a + (b - a) * lcgrand(1);
}

class Inventory
{
  // inputs ----------------------------------------------
  int n_trial;           // number of trial
  int initial_inventory; // initial inventory level
  int n_months;          // total number of months
  int n_polices;         // total number of polices

  int n_demands;           // total number of demands
  double mean_interdemand; // mean interdemand in months

  double setup_cost;       // setup cost
  double incremental_cost; // incremental cost
  double holding_cost;     // holding cost
  double shortage_cost;    // shortage cost

  // epress order
  double express_setup_cost;
  double express_incremental_cost;

  //
  double avg_before_express;
  double avg_after_express;

  double total_avg_before_express;
  double total_avg_after_express;

  double min_lag;
  double max_lag;

  // express_order lag;
  double epress_min_lag;
  double epress_max_lag;

  // cumulative probability of the sequential demand size
  // i.e for demand size of 1, 2, ... , n_demands
  vector<double> cpd_demands;

  // sim variables ----------------------------------------
  ifstream in;
  ofstream out;
  double sim_time;                // simulation time
  EventType next_event_type;      // type of next event
  double num_events;              // number of events for the timing function to consider
  vector<double> time_next_event; // next 4 events to consider

  // state variables
  int inventory_level; // inventory level at time , I(t)
  double time_last_event;

  // stats variables
  double total_ordering_cost;
  double area_holding_cost;  // I(t)+
  double area_shortage_cost; // I(t)-
  // epress order cost
  double total_express_ordering_cost;
  int express_cnt;

  void initialize();
  void timing();                // determine the next event
  void update_time_avg_stats(); // update area_holding_cost, area_shortage_cost

  int randn()
  {
    float u = lcgrand(1);
    for (int i = 0; i < cpd_demands.size(); i++)
      if (u < cpd_demands[i])
        return i + 1;
  }

  void report(int policy_s, int policy_S);

public:
  Inventory();
  Inventory(string in_filename, string out_filename);

  double run_before_express(int policy_s, int policy_S);
  void run();
};

Inventory::Inventory(string filname, string out_filename)
{
  in.open(filname);
  if (!in)
  {
    cout << "Error opening file" << endl;
    exit(1);
  }

  out.open(out_filename);
  if (!out)
  {
    cout << "Error opening file" << out_filename << endl;
    exit(1);
  }

  in >> n_trial;

  in >> initial_inventory >> n_months >> n_polices;

  in >> n_demands >> mean_interdemand;

  in >> setup_cost >> incremental_cost >> holding_cost >> shortage_cost;

  in >> express_setup_cost >> express_incremental_cost;

  in >> min_lag >> max_lag;

  in >> epress_min_lag >> epress_max_lag;

  cpd_demands.resize(n_demands);

  for (int i = 0; i < n_demands; i++)
  {
    in >> cpd_demands[i];
  }
}

void Inventory::initialize()
{
  // init sim time
  sim_time = 0.0;

  // init state variables
  inventory_level = initial_inventory;
  time_last_event = 0.0;

  // init stats variables
  total_ordering_cost = 0.0;
  area_holding_cost = 0.0;
  area_shortage_cost = 0.0;
  // express
  total_express_ordering_cost = 0.0;
  express_cnt = 0;

  // init time of next event
  num_events = 4;
  time_next_event.resize(num_events);
  // scehdule first order
  time_next_event[ORDER_ARRIVAL] = 1.0e+30; // infinity
  // schedule first demand
  time_next_event[DEMAND] = sim_time + expon(mean_interdemand);
  // schedule end of simulation
  time_next_event[END] = n_months;
  // schedule first evaluation
  time_next_event[EVALUATE] = 0.0;
}

void Inventory::timing()
{
  double min_time_next_event = 1.0e+29;
  next_event_type = EventType::NONE;

  // find the next event
  for (int i = 1; i <= num_events; i++)
  {
    if (time_next_event[i] < min_time_next_event)
    {
      min_time_next_event = time_next_event[i];
      next_event_type = (EventType)i;
    }
  }

  // check if next event is valid
  if (next_event_type == EventType::NONE)
  {
    cout << "Event list empty at time " << sim_time << endl;
    exit(1);
  }

  // advance the sim time
  sim_time = min_time_next_event;
}

void Inventory::update_time_avg_stats()
{
  double time_since_last_event = sim_time - time_last_event;
  time_last_event = sim_time;

  area_holding_cost += max(0, inventory_level) * time_since_last_event;
  area_shortage_cost += -min(0, inventory_level) * time_since_last_event;
}

double Inventory::run_before_express(int policy_s, int policy_S)
{
  int ordered_amount = 0;

  // run n_trial times
  total_avg_before_express = 0;
  total_avg_after_express = 0;
  double total_expected_express_count = 0;

  for (int i = 0; i < n_trial; i++)
  {
    // init the sim
    this->initialize();

    int n_events = 0;
    // run the sim
    do
    {
      // determine the next event
      this->timing();

      // update time-average statistical accumulators
      this->update_time_avg_stats();

      // out<<"time: "<<sim_time<<", next_event_type: "<<next_event_type<<endl;

      // invoke the appropriate event function
      switch (next_event_type)
      {
      case EventType::ORDER_ARRIVAL: // on order arrival
        // add the ordered amount to the inventory
        inventory_level += ordered_amount;
        // schedule next order
        time_next_event[ORDER_ARRIVAL] = 1.0e+30; // infinity
        break;

      case EventType::DEMAND:
        // reduce the inventory level by the demand size
        inventory_level -= randn();
        // schedule next demand
        time_next_event[DEMAND] = sim_time + expon(mean_interdemand);
        break;

      case EventType::EVALUATE:
        // evaluate the inventory level
        // epress oder if inventory level is less than zero
        if (inventory_level < policy_s)
        {
          // place an order
          ordered_amount = policy_S - inventory_level;
          // update ordering cost
          total_ordering_cost += setup_cost + incremental_cost * ordered_amount;
          // schedule next order
          time_next_event[ORDER_ARRIVAL] = sim_time + uniform(min_lag, max_lag);
        }
        // schedule next evaluation
        time_next_event[EVALUATE] = sim_time + 1.0; // 1 month later
        break;

      case EventType::END:
        // end of simulation
        this->report(policy_s, policy_S);
        total_avg_before_express += avg_before_express;
        break;
      }
      n_events++;
    } while (next_event_type != EventType::END);
    // cout<<n_events<<endl;
  }

  return total_avg_before_express / n_trial;
}

void Inventory::run()
{
  cout << "Running the simulation..." << endl;

  out << "------Single-Product Inventory System------" << endl
      << endl;
  out << "Initial inventory level: " << initial_inventory << " items" << endl
      << endl;
  out << "Number of demand sizes: " << n_demands << endl
      << endl;
  out << "Distribution function of demand sizes: ";
  for (double d : cpd_demands)
    out << setprecision(2) << fixed << d << " ";
  out << endl
      << endl;
  out << "Mean inter-demand time: " << mean_interdemand << " months" << endl
      << endl;
  out << "Delivery lag range: " << min_lag << " to " << max_lag << " months" << endl
      << endl;
  out << "Length of simulation: " << n_months << " months" << endl
      << endl;

  out << "Costs:" << endl;
  out << "K = " << setup_cost << endl;
  out << "i = " << incremental_cost << endl;
  out << "h = " << holding_cost << endl;
  out << "pi = " << shortage_cost << endl;
  out << "E_K = " << express_setup_cost << endl;
  out << "E_i = " << express_incremental_cost << endl
      << endl;

  out << "Number of policies: " << n_polices << endl
      << endl;

  out << "Policies:\n";
  out << "--------------------------------------------------------------------------------------------------\n";
  out << " Policy        Avg_before_express     Avg_after_express      Expected_express_count ";
  out << "--------------------------------------------------------------------------------------------------";

  // run the sim by looping over number of polices
  for (int i = 0; i < n_polices; i++)
  {
    // read the policy from in file
    int policy_s, policy_S;
    in >> policy_s >> policy_S;

    double temp = run_before_express(policy_s, policy_S);

    int ordered_amount = 0;

    // run n_trial times
    total_avg_before_express = 0;
    total_avg_after_express = 0;
    double total_expected_express_count = 0;

    for (int i = 0; i < n_trial; i++)
    {
      // init the sim
      this->initialize();

      int n_events = 0;
      // run the sim
      do
      {
        // determine the next event
        this->timing();

        // update time-average statistical accumulators
        this->update_time_avg_stats();

        // out<<"time: "<<sim_time<<", next_event_type: "<<next_event_type<<endl;

        // invoke the appropriate event function
        switch (next_event_type)
        {
          
        case EventType::ORDER_ARRIVAL: // on order arrival
          // add the ordered amount to the inventory
          inventory_level += ordered_amount;
          // schedule next order
          time_next_event[ORDER_ARRIVAL] = 1.0e+30; // infinity
          break;

        case EventType::DEMAND:
          // reduce the inventory level by the demand size
          inventory_level -= randn();
          // schedule next demand
          time_next_event[DEMAND] = sim_time + expon(mean_interdemand);
          break;

        case EventType::EVALUATE:
          // evaluate the inventory level
          // epress oder if inventory level is less than zero
          if (inventory_level < 0)
          {
            // place an express order
            ordered_amount = -inventory_level;
            // update ordering cost
            total_express_ordering_cost += express_setup_cost + express_incremental_cost * ordered_amount;
            // schedule next order
            time_next_event[ORDER_ARRIVAL] = sim_time + uniform(epress_min_lag, epress_max_lag);
            // update express count
            express_cnt++;
          }
          else if (inventory_level < policy_s)
          {
            // place an order
            ordered_amount = policy_S - inventory_level;
            // update ordering cost
            total_ordering_cost += setup_cost + incremental_cost * ordered_amount;
            // schedule next order
            time_next_event[ORDER_ARRIVAL] = sim_time + uniform(min_lag, max_lag);
          }
          // schedule next evaluation
          time_next_event[EVALUATE] = sim_time + 1.0; // 1 month later
          break;

        case EventType::END:
          // end of simulation
          this->report(policy_s, policy_S);
          total_avg_before_express += avg_before_express;
          total_avg_after_express += avg_after_express;
          // exprected express count
          double expected_express_count = express_cnt * 1.0 / n_months;
          total_expected_express_count += expected_express_count;
          break;
        }
        n_events++;
      } while (next_event_type != EventType::END);
      // cout<<n_events<<endl;
    }

    double final_before = total_avg_before_express / n_trial;
    double final_after = total_avg_after_express / n_trial;
    double final_expected_express_count = total_expected_express_count / n_trial;

    int w = 20;
    out << "\n\n"
        << "(" << policy_s << "," << setw(3) << policy_S << ")"
        << setw(w) << fixed << setprecision(2) << temp
        << setw(w) << fixed << setprecision(2) << final_after
        << setw(w) << fixed << setprecision(2) << final_expected_express_count;
  }
  out << "\n\n--------------------------------------------------------------------------------------------------";
  cout << "Simulation completed" << endl;
}

void Inventory::report(int policy_s, int policy_S)
{
  double avg_ordering_cost = total_ordering_cost / n_months;
  double avg_holding_cost = holding_cost * area_holding_cost / n_months;
  double avg_shortage_cost = shortage_cost * area_shortage_cost / n_months;
  avg_before_express = avg_ordering_cost + avg_holding_cost + avg_shortage_cost;

  double avg_express_ordering_cost = total_express_ordering_cost / n_months;
  avg_after_express = avg_express_ordering_cost + avg_before_express;
}
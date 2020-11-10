#include <scorep_plugin_mpi.h>
#include <iostream>
#include <sstream>
#include <utils.h>
#include <stdlib.h>

scorep_plugin_mpi::scorep_plugin_mpi()
{
    std::cout << "Loading Metric Plugin: MPI Sampling\n";

    m_mpi_t_initialized = 0;
}

scorep_plugin_mpi::~scorep_plugin_mpi()
{
}


std::vector<MetricProperty>
scorep_plugin_mpi::get_metric_properties(const std::string& metric_name)
{
    const mpi_t_counters *pvars;
    unsigned long long int hex_dummy;
    int assigned_event = 0;
    std::vector<MetricProperty> metric_properties;

    std::cout << "scorep_plugin_mpi::get_metric_properties() called with: " <<
            metric_name << '\n';

    auto [event, dummy] = parse_metric(metric_name, &hex_dummy);

    std::cout << "Event = " << event << " dummy = " << dummy <<
            " hex_dummy = " << hex_dummy << std::endl;

    /* MPI_T? */
    if (event == "MPI_T") {
        /* Enumerate PVARs */
#if 0 /* Cannot enumerate since MPI_Init() has not executed yet */
        //mpi_t_sampling_object.MPI_T_pvars_enumerate();
#else
        /* Get static initialization of counters list */
        mpi_t_sampling_object.pvars_enumeration_get(&pvars, &m_num_pvars);
#endif
        std::cout << "MPI_T m_num_pvars=" << m_num_pvars << std::endl;
        for (int i = 0; i < m_num_pvars; i++) {
            metric_properties.push_back(
                MetricProperty(metric_name + "_" + pvars[i].counter_name, "", "").absolute_point().value_uint().decimal());
        }
        /*
            Example of adding a variable:
            metric_properties.push_back(MetricProperty(metric_name, "", "MPI_T_counter0").absolute_point().value_uint());
        */
    }

    /* Debug print */
    if (assigned_event) {
        std::cout << event << " = " << (uintptr_t)hex_dummy << std::endl;
    }

    return metric_properties;
}


int32_t
scorep_plugin_mpi::add_metric(const std::string& metric)
{
    const mpi_t_counters *pvars;
    size_t n_pvars;
    int32_t id = 0;
    unsigned long long int hex_dummy;

    auto [event, period] = parse_metric(metric, &hex_dummy);

    std::cout << "add_metric() called with: " << metric << '\n';

    /* MPI_T? */
    if (event == "MPI_T") {
        static int32_t allocated_index = 0;

        /* Enumerate PVARs */
#if 0 /* Cannot enumerate since MPI_Init() has not executed yet */
        //mpi_t_sampling_object.MPI_T_pvars_enumerate();
#else
        /* Get static initialization of counters list */
        mpi_t_sampling_object.pvars_enumeration_get(&pvars, &n_pvars);
#endif
        if (allocated_index < n_pvars) {
            id = pvars[allocated_index].counter_index;
            allocated_index++;
        }
    }

    return id;
}


void
scorep_plugin_mpi::start()
{
    std::cout << "scorep_plugin_mpi::start()\n";
}


void
scorep_plugin_mpi::stop()
{
    std::cout << "scorep_plugin_mpi::stop()";
}

SCOREP_METRIC_PLUGIN_CLASS(scorep_plugin_mpi, "scorep_plugin_mpi")

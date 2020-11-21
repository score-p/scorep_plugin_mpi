#include <scorep_plugin_mpi.h>
#include <iostream>
#include <sstream>
#include <utils.h>
#include <stdlib.h>
#include <stdio.h>

#define ENV_VARIABLE_SCOREP_PLUGIN_MPI_DECIMATION "SCOREP_PLUGIN_MPI_DECIMATION"
#define VARIABLE_SCOREP_PLUGIN_MPI_DECIMATION_DEFAULT 4

scorep_plugin_mpi::scorep_plugin_mpi()
{
    const char *decimation = getenv(ENV_VARIABLE_SCOREP_PLUGIN_MPI_DECIMATION);

    /* Get decimation set by user */
    if (decimation == NULL) {
        m_mpi_samples_decimation = VARIABLE_SCOREP_PLUGIN_MPI_DECIMATION_DEFAULT;
    }
    else {
        m_mpi_samples_decimation = atoi(decimation);
        if ((m_mpi_samples_decimation & (m_mpi_samples_decimation-1)) != 0) {
            printf("Warning: Samples decimation value must be a power of 2, "
                "setting to default\n");
        }
    }

    printf("Loading Metric Plugin: MPI Sampling (Samples decimation=%u)\n",
        m_mpi_samples_decimation);

    m_mpi_t_initialized = 0;
    m_mpi_decimation_counter = 0;
    m_metrics_last_read_values = NULL;
    m_decimation_duty_cycle = 0;

}

scorep_plugin_mpi::~scorep_plugin_mpi()
{
    free(m_metrics_last_read_values);
    m_metrics_last_read_values = NULL;
}


std::vector<MetricProperty>
scorep_plugin_mpi::get_metric_properties(const std::string& metric_name)
{
    const mpi_t_counters *pvars;
    unsigned long long int hex_dummy;
    int assigned_event = 0;
    std::vector<MetricProperty> metric_properties;
    uint32_t i;
    uint32_t pvars_max_index;

    DEBUG_PRINT("scorep_plugin_mpi::get_metric_properties() called with: %s\n", metric_name);

    auto [event, dummy] = parse_metric(metric_name, &hex_dummy);

    DEBUG_PRINT("Event=%s, dummy=%lu, hex_dummy=%lx\n", event, dummy, hex_dummy);

    /* MPI_T? */
    if (event == "MPI_T") {
        /* Enumerate PVARs */
#if 0 /* Cannot enumerate since MPI_Init() has not executed yet */
        //mpi_t_sampling_object.MPI_T_pvars_enumerate();
#else
        /* Get static initialization of counters list */
        mpi_t_sampling_object.pvars_enumeration_get(&pvars, &m_num_pvars);
#endif
        printf("MPI_T m_num_pvars = %zu\n", m_num_pvars);

        /* Allocate metrics last values */
        pvars_max_index = 0;
        for (i = 0; i < m_num_pvars; i++) {
            if (pvars[i].counter_index > pvars_max_index) {
                pvars_max_index = pvars[i].counter_index;
            }
        }
        /* Add 1 since the last index also requires an element */
        pvars_max_index = pvars_max_index + 1;

        /* Allocate memory */
        m_metrics_last_read_values = (uint64_t *)malloc( sizeof(uint64_t) * pvars_max_index );

        m_decimation_duty_cycle = 0;

        for (int i = 0; i < m_num_pvars; i++) {
            m_metrics_last_read_values[i] = 0;
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
        DEBUG_PRINT("%s = %lu\n", event, (uintptr_t)hex_dummy);
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

    DEBUG_PRINT("add_metric() called with: %s = %lu\n", event, (uintptr_t)hex_dummy);

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
    DEBUG_PRINT("scorep_plugin_mpi::start()\n");
}


void
scorep_plugin_mpi::stop()
{
    DEBUG_PRINT("scorep_plugin_mpi::stop()\n");
}

SCOREP_METRIC_PLUGIN_CLASS(scorep_plugin_mpi, "scorep_plugin_mpi")
